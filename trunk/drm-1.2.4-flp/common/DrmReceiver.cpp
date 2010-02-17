/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	DRM-receiver
 * The hand over of data is done via an intermediate-buffer. The calling
 * convention is always "input-buffer, output-buffer". Additional, the
 * DRM-parameters are fed to the function.
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "DrmReceiver.h"


/* Implementation *************************************************************/
void CDRMReceiver::Run()
{
	_BOOLEAN bEnoughData;

	/* Reset all parameters to start parameter settings */
	SetInStartMode();

	do
	{
		/* Check for parameter changes from GUI thread ---------------------- */
		/* The parameter changes are done through flags, the actual
		   initialization is done in this (the working) thread to avoid
		   problems with shared data */
		if (eNewReceiverMode != RM_NONE)
			InitReceiverMode();


		/* Receive data ----------------------------------------------------- */
		ReceiveData.ReadData(ReceiverParam, RecDataBuf);

		bEnoughData = TRUE;

		while (bEnoughData && ReceiverParam.bRunThread)
		{
			/* Init flag */
			bEnoughData = FALSE;

			if ((eReceiverMode == RM_AM) || bDoInitRun)
			{
				/* AM demodulation ------------------------------------------ */
				if (AMDemodulation.ProcessData(ReceiverParam, RecDataBuf,
					AudSoDecBuf))
				{
					bEnoughData = TRUE;
				}
			}

			if ((eReceiverMode == RM_DRM) || bDoInitRun)
			{
				/* Resample input DRM-stream -------------------------------- */
				if (InputResample.ProcessData(ReceiverParam, RecDataBuf,
					InpResBuf))
				{
					bEnoughData = TRUE;
				}

				/* Frequency synchronization acquisition -------------------- */
				if (FreqSyncAcq.ProcessData(ReceiverParam, InpResBuf,
					FreqSyncAcqBuf))
				{
					bEnoughData = TRUE;
				}

				/* Time synchronization ------------------------------------- */
				if (TimeSync.ProcessData(ReceiverParam, FreqSyncAcqBuf,
					TimeSyncBuf))
				{
					bEnoughData = TRUE;

					/* Use count of OFDM-symbols for detecting aquisition
					   state */
					DetectAcquiSymbol();
				}

				/* OFDM-demodulation ---------------------------------------- */
				if (OFDMDemodulation.ProcessData(ReceiverParam, TimeSyncBuf,
					OFDMDemodBuf))
				{
					bEnoughData = TRUE;
				}

				/* Synchronization in the frequency domain (using pilots) --- */
				if (SyncUsingPil.ProcessData(ReceiverParam, OFDMDemodBuf,
					SyncUsingPilBuf))
				{
					bEnoughData = TRUE;
				}

				/* Channel estimation and equalisation ---------------------- */
				if (ChannelEstimation.ProcessData(ReceiverParam,
					SyncUsingPilBuf, ChanEstBuf))
				{
					bEnoughData = TRUE;

					/* If this module has finished, all synchronization units
					   have also finished their OFDM symbol based estimates.
					   Update synchronization parameters histories */
					UpdateParamHistories();
				}

				/* Demapping of the MSC, FAC, SDC and pilots off the carriers */
				if (OFDMCellDemapping.ProcessData(ReceiverParam, ChanEstBuf,
					MSCCarDemapBuf, FACCarDemapBuf, SDCCarDemapBuf))
				{
					bEnoughData = TRUE;
				}

				/* FAC ------------------------------------------------------ */
				if (FACMLCDecoder.ProcessData(ReceiverParam, FACCarDemapBuf,
					FACDecBuf))
				{
					bEnoughData = TRUE;
				}
				if (UtilizeFACData.WriteData(ReceiverParam, FACDecBuf))
				{
					bEnoughData = TRUE;

					/* Use information of FAC CRC for detecting the acquisition
					   requirement */
					DetectAcquiFAC();
				}


				/* SDC ------------------------------------------------------ */
				if (SDCMLCDecoder.ProcessData(ReceiverParam, SDCCarDemapBuf,
					SDCDecBuf))
				{
					bEnoughData = TRUE;
				}
				if (UtilizeSDCData.WriteData(ReceiverParam, SDCDecBuf))
					bEnoughData = TRUE;


				/* MSC ------------------------------------------------------ */
				/* Symbol de-interleaver */
				if (SymbDeinterleaver.ProcessData(ReceiverParam, MSCCarDemapBuf,
					DeintlBuf))
				{
					bEnoughData = TRUE;
				}

				/* MLC decoder */
				if (MSCMLCDecoder.ProcessData(ReceiverParam, DeintlBuf,
					MSCMLCDecBuf))
				{
					bEnoughData = TRUE;
				}

				/* MSC data/audio demultiplexer */
				if (MSCDemultiplexer.ProcessData(ReceiverParam,
					MSCMLCDecBuf, MSCDeMUXBufAud, MSCDeMUXBufData))
				{
					bEnoughData = TRUE;
				}

				/* Data decoding */
				if (DataDecoder.WriteData(ReceiverParam, MSCDeMUXBufData))
					bEnoughData = TRUE;

				/* Source decoding (audio) */
				if (AudioSourceDecoder.ProcessData(ReceiverParam,
					MSCDeMUXBufAud, AudSoDecBuf))
				{
					bEnoughData = TRUE;

					/* Store the number of correctly decoded audio blocks for
					   the history */
					iCurrentCDAud = AudioSourceDecoder.GetNumCorDecAudio();
				}
			}

			/* Save or dump the data */
			if (WriteData.WriteData(ReceiverParam, AudSoDecBuf))
				bEnoughData = TRUE;
		}
	} while (ReceiverParam.bRunThread && (!bDoInitRun));
}

void CDRMReceiver::DetectAcquiSymbol()
{
	/* Only for aquisition detection if no signal was decoded before */
	if (eAcquiState == AS_NO_SIGNAL)
	{
		/* Increment symbol counter and check if bound is reached */
		iAcquDetecCnt++;

		if (iAcquDetecCnt > NUM_OFDMSYM_U_ACQ_WITHOUT)
			SetInStartMode();			
	}
}

void CDRMReceiver::DetectAcquiFAC()
{
#ifdef USE_QT_GUI
	/* If MDI in is enabled, do not check for acquisition state because we want
	   to stay in tracking mode all the time */
	if (MDI.GetMDIInEnabled() == TRUE)
		return;
#endif

	/* Acquisition switch */
	if (!UtilizeFACData.GetCRCOk())
	{
		/* Reset "good signal" count */
		iGoodSignCnt = 0;

		iAcquRestartCnt++;

		/* Check situation when receiver must be set back in start mode */
		if ((eAcquiState == AS_WITH_SIGNAL) &&
			(iAcquRestartCnt > NUM_FAC_FRA_U_ACQ_WITH))
		{
			SetInStartMode();
		}
	}
	else
	{
		/* Set the receiver state to "with signal" not until two successive FAC
		   frames are "ok", because there is only a 8-bit CRC which is not good
		   for many bit errors. But it is very unlikely that we have two
		   successive FAC blocks "ok" if no good signal is received */
		if (iGoodSignCnt > 0)
		{
			eAcquiState = AS_WITH_SIGNAL;

			/* Take care of delayed tracking mode switch */
			if (iDelayedTrackModeCnt > 0)
				iDelayedTrackModeCnt--;
			else
				SetInTrackingModeDelayed();
		}
		else
		{
			/* If one CRC was correct, reset acquisition since
			   we assume, that this was a correct detected signal */
			iAcquRestartCnt = 0;
			iAcquDetecCnt = 0;

			/* Set in tracking mode */
			SetInTrackingMode();

			iGoodSignCnt++;
		}
	}
}

void CDRMReceiver::Init()
{
	/* Set flags so that we have only one loop in the Run() routine which is
	   enough for initializing all modues */
	bDoInitRun = TRUE;
	ReceiverParam.bRunThread = TRUE;

	/* Set init flags in all modules */
	InitsForAllModules();

	/* Now the actual initialization */
	Run();

	/* Reset flags */
	bDoInitRun = FALSE;
	ReceiverParam.bRunThread = FALSE;
}

void CDRMReceiver::InitReceiverMode()
{
	eReceiverMode = eNewReceiverMode;

	/* Init all modules */
	SetInStartMode();

	/* Reset new mode flag */
	eNewReceiverMode = RM_NONE;
}

void CDRMReceiver::Start()
{
	/* Set run flag so that the thread can work */
	ReceiverParam.bRunThread = TRUE;

	Run();
}

void CDRMReceiver::Stop()
{
	ReceiverParam.bRunThread = FALSE;

	SoundInterface.Close();
}

void CDRMReceiver::SetAMDemodAcq(_REAL rNewNorCen)
{
	/* Set the frequency where the AM demodulation should look for the
	   aquisition. Receiver must be in AM demodulation mode */
	if (eReceiverMode == RM_AM)
		AMDemodulation.SetAcqFreq(rNewNorCen);
}

void CDRMReceiver::SetInStartMode()
{
	/* Load start parameters for all modules */
	StartParameters(ReceiverParam);

	/* Activate acquisition */
	FreqSyncAcq.StartAcquisition();
	TimeSync.StartAcquisition();
	ChannelEstimation.GetTimeSyncTrack()->StopTracking();
	ChannelEstimation.StartSaRaOffAcq();
	ChannelEstimation.GetTimeWiener()->StopTracking();

	SyncUsingPil.StartAcquisition();
	SyncUsingPil.StopTrackPil();

	/* Set flag that no signal is currently received */
	eAcquiState = AS_NO_SIGNAL;

	/* Set flag for receiver state */
	eReceiverState = RS_ACQUISITION;

	/* Reset counters for acquisition decision, "good signal" and delayed
	   tracking mode counter */
	iAcquRestartCnt = 0;
	iAcquDetecCnt = 0;
	iGoodSignCnt = 0;
	iDelayedTrackModeCnt = NUM_FAC_DEL_TRACK_SWITCH;

	/* Reset GUI lights */
	PostWinMessage(MS_RESET_ALL);

#ifdef USE_QT_GUI
	/* In case MDI is enabled, go directly to tracking mode, do not activate the
	   synchronization units */
	if (MDI.GetMDIInEnabled() == TRUE)
	{
		/* We want to have as low CPU usage as possible, therefore set the
		   synchronization units in a state where they do only a minimum
		   work */
		FreqSyncAcq.StopAcquisition();
		TimeSync.StopTimingAcqu();
		InputResample.SetSyncInput(TRUE);
		SyncUsingPil.SetSyncInput(TRUE);

		/* This is important so that always the same amount of module input
		   data is queried, otherwise it could be that amount of input data is
		   set to zero and the receiver gets into an infinite loop */
		TimeSync.SetSyncInput(TRUE);

		/* Always tracking mode for MDI */
		eAcquiState = AS_WITH_SIGNAL;

		SetInTrackingMode();
	}
#endif
}

void CDRMReceiver::SetInTrackingMode()
{
	/* We do this with the flag "eReceiverState" to ensure that the following
	   routines are only called once when the tracking is actually started */
	if (eReceiverState == RS_ACQUISITION)
	{
		/* In case the acquisition estimation is still in progress, stop it now
		   to avoid a false estimation which could destroy synchronization */
		TimeSync.StopRMDetAcqu();

		/* Acquisition is done, deactivate it now and start tracking */
		ChannelEstimation.GetTimeWiener()->StartTracking();

		/* Reset acquisition for frame synchronization */
		SyncUsingPil.StopAcquisition();
		SyncUsingPil.StartTrackPil();

		/* Set receiver flag to tracking */
		eReceiverState = RS_TRACKING;
	}
}

void CDRMReceiver::SetInTrackingModeDelayed()
{
	/* The timing tracking must be enabled delayed because it must wait until
	   the channel estimation has initialized its estimation */
	TimeSync.StopTimingAcqu();
	ChannelEstimation.GetTimeSyncTrack()->StartTracking();
}

void CDRMReceiver::SetReadDRMFromFile(const string strNFN)
{
	/* If DRM data is read from file instead of using the sound card, the sound
	   output must be a blocking function otherwise we cannot achieve a
	   synchronized stream */
	ReceiveData.SetReadFromFile(strNFN);
	WriteData.SetSoundBlocking(TRUE);
}

void CDRMReceiver::StartParameters(CParameter& Param)
{
/*
	Reset all parameters to starting values. This is done at the startup of the
	application and also when the S/N of the received signal is too low and
	no receiption is left -> Reset all parameters
*/

	/* Define with which parameters the receiver should try to decode the
	   signal. If we are correct with our assumptions, the receiver does not
	   need to reinitialize */
	Param.InitCellMapTable(RM_ROBUSTNESS_MODE_B, SO_3);

	/* Set initial MLC parameters */
	Param.SetInterleaverDepth(CParameter::SI_LONG);
	Param.SetMSCCodingScheme(CParameter::CS_3_SM);
	Param.SetSDCCodingScheme(CParameter::CS_2_SM);

	/* Select the service we want to decode. Always zero, because we do not
	   know how many services are transmitted in the signal we want to
	   decode */
	Param.ResetCurSelAudDatServ();

	/* Reset alternative frequencys */
	Param.AltFreqSign.Reset();


	/* Set the following parameters to zero states (initial states) --------- */
	Param.ResetServicesStreams();

	/* Protection levels */
	Param.MSCPrLe.iPartA = 0;
	Param.MSCPrLe.iPartB = 1;
	Param.MSCPrLe.iHierarch = 0;

	/* Number of audio and data services */
	Param.iNumAudioService = 0;
	Param.iNumDataService = 0;

	/* Date, time */
	Param.iDay = 0;
	Param.iMonth = 0;
	Param.iYear = 0;
	Param.iUTCHour = 0;
	Param.iUTCMin = 0;

	/* We start with FAC ID = 0 (arbitrary) */
	Param.iFrameIDReceiv = 0;

	/* Set synchronization parameters */
	Param.rResampleOffset = rInitResampleOffset; /* Initial resample offset */
	Param.rFreqOffsetAcqui = (_REAL) 0.0;
	Param.rFreqOffsetTrack = (_REAL) 0.0;
	Param.iTimingOffsTrack = 0;

	/* Init reception log (log long) transmission parameters. TODO: better solution */
	Param.ReceptLog.ResetTransParams();

	/* Initialization of the modules */
	InitsForAllModules();
}

void CDRMReceiver::InitsForAllModules()
{
	/* Set init flags */
	ReceiveData.SetInitFlag();
	InputResample.SetInitFlag();
	FreqSyncAcq.SetInitFlag();
	TimeSync.SetInitFlag();
	OFDMDemodulation.SetInitFlag();
	SyncUsingPil.SetInitFlag();
	ChannelEstimation.SetInitFlag();
	OFDMCellDemapping.SetInitFlag();
	FACMLCDecoder.SetInitFlag();
	UtilizeFACData.SetInitFlag();
	SDCMLCDecoder.SetInitFlag();
	UtilizeSDCData.SetInitFlag();
	SymbDeinterleaver.SetInitFlag();
	MSCMLCDecoder.SetInitFlag();
	MSCDemultiplexer.SetInitFlag();
	AudioSourceDecoder.SetInitFlag();
	DataDecoder.SetInitFlag();
	WriteData.SetInitFlag();
	AMDemodulation.SetInitFlag();

	/* Clear all buffers (this is especially important for the "AudSoDecBuf"
	   buffer since AM mode and DRM mode use the same buffer. When init is
	   called or modes are switched, the buffer could have some data left which
	   lead to an overrun) */
	RecDataBuf.Clear();
	InpResBuf.Clear();
	FreqSyncAcqBuf.Clear();
	TimeSyncBuf.Clear();
	OFDMDemodBuf.Clear();
	SyncUsingPilBuf.Clear();
	ChanEstBuf.Clear();
	MSCCarDemapBuf.Clear();
	FACCarDemapBuf.Clear();
	SDCCarDemapBuf.Clear();
	DeintlBuf.Clear();
	FACDecBuf.Clear();
	SDCDecBuf.Clear();
	MSCMLCDecBuf.Clear();
	MSCDeMUXBufAud.Clear();
	MSCDeMUXBufData.Clear();
	AudSoDecBuf.Clear();
}


/* -----------------------------------------------------------------------------
   Initialization routines for the modules. We have to look into the modules
   and decide on which parameters the modules depend on */
void CDRMReceiver::InitsForWaveMode()
{
	/* Reset averaging of the parameter histories (needed, e.g., because the
	   number of OFDM symbols per DRM frame might have changed) */
	iAvCntParamHist = 0;
	rAvLenIRHist = (_REAL) 0.0;
	rAvDopplerHist = (_REAL) 0.0;
	rAvSNRHist = (_REAL) 0.0;

	/* After a new robustness mode was detected, give the time synchronization
	   a bit more time for its job */
	iAcquDetecCnt = 0;

	/* Set init flags */
	ReceiveData.SetInitFlag();
	InputResample.SetInitFlag();
	FreqSyncAcq.SetInitFlag();
	AMDemodulation.SetInitFlag();
	TimeSync.SetInitFlag();
	OFDMDemodulation.SetInitFlag();
	SyncUsingPil.SetInitFlag();
	ChannelEstimation.SetInitFlag();
	OFDMCellDemapping.SetInitFlag();
	SymbDeinterleaver.SetInitFlag(); // Because of "iNumUsefMSCCellsPerFrame"
	MSCMLCDecoder.SetInitFlag(); // Because of "iNumUsefMSCCellsPerFrame"
	SDCMLCDecoder.SetInitFlag(); // Because of "iNumSDCCellsPerSFrame"
}

void CDRMReceiver::InitsForSpectrumOccup()
{
	/* Set init flags */
	FreqSyncAcq.SetInitFlag(); // Because of bandpass filter
	OFDMDemodulation.SetInitFlag();
	SyncUsingPil.SetInitFlag();
	ChannelEstimation.SetInitFlag();
	OFDMCellDemapping.SetInitFlag();
	SymbDeinterleaver.SetInitFlag(); // Because of "iNumUsefMSCCellsPerFrame"
	MSCMLCDecoder.SetInitFlag(); // Because of "iNumUsefMSCCellsPerFrame"
	SDCMLCDecoder.SetInitFlag(); // Because of "iNumSDCCellsPerSFrame"
}


/* SDC ---------------------------------------------------------------------- */
void CDRMReceiver::InitsForSDCCodSche()
{
	/* Set init flags */
	SDCMLCDecoder.SetInitFlag();
}

void CDRMReceiver::InitsForNoDecBitsSDC()
{
	/* Set init flag */
	UtilizeSDCData.SetInitFlag();
}


/* MSC ---------------------------------------------------------------------- */
void CDRMReceiver::InitsForInterlDepth()
{
	/* Can be absolutely handled seperately */
	SymbDeinterleaver.SetInitFlag();
}

void CDRMReceiver::InitsForMSCCodSche()
{
	/* Set init flags */
	MSCMLCDecoder.SetInitFlag();
	MSCDemultiplexer.SetInitFlag(); // Not sure if really needed, look at code! TODO
}

void CDRMReceiver::InitsForMSC()
{
	/* Set init flags */
	MSCMLCDecoder.SetInitFlag();

	InitsForMSCDemux();
}

void CDRMReceiver::InitsForMSCDemux()
{
	/* Set init flags */
	MSCDemultiplexer.SetInitFlag();
	AudioSourceDecoder.SetInitFlag();
	DataDecoder.SetInitFlag();

	/* Reset value used for the history because if an audio service was selected
	   but then only a data service is selected, the value would remain with the
	   last state */
	iCurrentCDAud = 0;
}

void CDRMReceiver::InitsForAudParam()
{
	/* Set init flags */
	MSCDemultiplexer.SetInitFlag();
	AudioSourceDecoder.SetInitFlag();
}

void CDRMReceiver::InitsForDataParam()
{
	/* Set init flags */
	MSCDemultiplexer.SetInitFlag();
	DataDecoder.SetInitFlag();
}


/* Parameter histories for plot --------------------------------------------- */
void CDRMReceiver::UpdateParamHistories()
{
	/* TODO: do not use the shift register class, build a new
	   one which just incremets a pointer in a buffer and put
	   the new value at the position of the pointer instead of
	   moving the total data all the time -> special care has
	   to be taken when reading out the data */

	/* Only update histories if the receiver is in tracking mode */
	if (eReceiverState == RS_TRACKING)
	{
		MutexHist.Lock(); /* MUTEX vvvvvvvvvv */

		/* Frequency offset tracking values */
		vecrFreqSyncValHist.AddEnd(
			ReceiverParam.rFreqOffsetTrack *
			SOUNDCRD_SAMPLE_RATE);

		/* Sample rate offset estimation */
		vecrSamOffsValHist.AddEnd(ReceiverParam.
			GetSampFreqEst());

		/* Get estimated Doppler value and SNR */
		_REAL rDoppler, rSNREstimate;
		ChannelEstimation.GetSigma(rDoppler);
		ChannelEstimation.GetSNREstdB(rSNREstimate);

		/* Average Doppler and delay estimates */
		rAvLenIRHist += ChannelEstimation.GetDelay();
		rAvDopplerHist += rDoppler;
		rAvSNRHist += rSNREstimate;

		/* Only evaluate Doppler and delay once in one DRM frame */
		iAvCntParamHist++;
		if (iAvCntParamHist == ReceiverParam.iNumSymPerFrame)
		{
			/* Apply averaged values to the history vectors */
			vecrLenIRHist.AddEnd(
				rAvLenIRHist / ReceiverParam.iNumSymPerFrame);
			vecrDopplerHist.AddEnd(
				rAvDopplerHist / ReceiverParam.iNumSymPerFrame);
			vecrSNRHist.AddEnd(
				rAvSNRHist / ReceiverParam.iNumSymPerFrame);

			/* At the same time, add number of correctly decoded audio blocks.
			   This number is updated once a DRM frame. Since the other
			   parameters like SNR is also updated once a DRM frame, the two
			   values are synchronized by one DRM frame */
			veciCDAudHist.AddEnd(iCurrentCDAud);

			/* Reset parameters used for averaging */
			iAvCntParamHist = 0;
			rAvLenIRHist = (_REAL) 0.0;
			rAvDopplerHist = (_REAL) 0.0;
			rAvSNRHist = (_REAL) 0.0;
		}

		MutexHist.Unlock(); /* MUTEX ^^^^^^^^^^ */
	}
}

void CDRMReceiver::GetFreqSamOffsHist(CVector<_REAL>& vecrFreqOffs,
									  CVector<_REAL>& vecrSamOffs,
									  CVector<_REAL>& vecrScale,
									  _REAL& rFreqAquVal)
{
	/* Init output vectors */
	vecrFreqOffs.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
	vecrSamOffs.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
	vecrScale.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);

	/* Lock resources */
	MutexHist.Lock();

	/* Simply copy history buffers in output buffers */
	vecrFreqOffs = vecrFreqSyncValHist;
	vecrSamOffs = vecrSamOffsValHist;

	/* Duration of OFDM symbol */
	const _REAL rTs = (CReal) (ReceiverParam.iFFTSizeN +
		ReceiverParam.iGuardSize) / SOUNDCRD_SAMPLE_RATE;

	/* Calculate time scale */
	for (int i = 0; i < LEN_HIST_PLOT_SYNC_PARMS; i++)
		vecrScale[i] = (i - LEN_HIST_PLOT_SYNC_PARMS + 1) * rTs;

	/* Value from frequency acquisition */
	rFreqAquVal = ReceiverParam.rFreqOffsetAcqui * SOUNDCRD_SAMPLE_RATE;

	/* Release resources */
	MutexHist.Unlock();
}

void CDRMReceiver::GetDopplerDelHist(CVector<_REAL>& vecrLenIR,
									 CVector<_REAL>& vecrDoppler,
									 CVector<_REAL>& vecrScale)
{
	/* Init output vectors */
	vecrLenIR.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
	vecrDoppler.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
	vecrScale.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);

	/* Lock resources */
	MutexHist.Lock();

	/* Simply copy history buffers in output buffers */
	vecrLenIR = vecrLenIRHist;
	vecrDoppler = vecrDopplerHist;

	/* Duration of DRM frame */
	const _REAL rDRMFrameDur = (CReal) (ReceiverParam.iFFTSizeN +
		ReceiverParam.iGuardSize) / SOUNDCRD_SAMPLE_RATE *
		ReceiverParam.iNumSymPerFrame;

	/* Calculate time scale in minutes */
	for (int i = 0; i < LEN_HIST_PLOT_SYNC_PARMS; i++)
		vecrScale[i] = (i - LEN_HIST_PLOT_SYNC_PARMS + 1) * rDRMFrameDur / 60;

	/* Release resources */
	MutexHist.Unlock();
}

void CDRMReceiver::GetSNRHist(CVector<_REAL>& vecrSNR,
							  CVector<_REAL>& vecrCDAud,
							  CVector<_REAL>& vecrScale)
{
	/* Init output vectors */
	vecrSNR.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
	vecrCDAud.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);
	vecrScale.Init(LEN_HIST_PLOT_SYNC_PARMS, (_REAL) 0.0);

	/* Lock resources */
	MutexHist.Lock();

	/* Simply copy history buffer in output buffer */
	vecrSNR = vecrSNRHist;

	/* Duration of DRM frame */
	const _REAL rDRMFrameDur = (CReal) (ReceiverParam.iFFTSizeN +
		ReceiverParam.iGuardSize) / SOUNDCRD_SAMPLE_RATE *
		ReceiverParam.iNumSymPerFrame;

	/* Calculate time scale. Copy correctly decoded audio blocks history (must
	   be transformed from "int" to "real", therefore we need a for-loop */
	for (int i = 0; i < LEN_HIST_PLOT_SYNC_PARMS; i++)
	{
		/* Scale in minutes */
		vecrScale[i] = (i - LEN_HIST_PLOT_SYNC_PARMS + 1) * rDRMFrameDur / 60;

		/* Correctly decoded audio blocks */
		vecrCDAud[i] = (_REAL) veciCDAudHist[i];
	}

	/* Release resources */
	MutexHist.Unlock();
}
