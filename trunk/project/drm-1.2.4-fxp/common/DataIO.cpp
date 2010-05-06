/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	
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

#include "DataIO.h"


/* Implementation *************************************************************/
/******************************************************************************\
* MSC data																	   *
\******************************************************************************/
/* Transmitter -------------------------------------------------------------- */
void CReadData::ProcessDataInternal(CParameter& TransmParam)
{
	/* Get data from sound interface */
	pSound->Read(vecsSoundBuffer);

	/* Write data to output buffer */
	for (int i = 0; i < iOutputBlockSize; i++)
		(*pvecOutputData)[i] = vecsSoundBuffer[i];

	/* Update level meter */
	SignalLevelMeter.Update((*pvecOutputData));
}

void CReadData::InitInternal(CParameter& TransmParam)
{
	/* Define block-size for output, an audio frame always corresponds
	   to 400 ms. We use always stereo blocks */
	iOutputBlockSize = (int) (SOUNDCRD_SAMPLE_RATE *
		 4/10 /* 400 ms */ * 2 /* stereo */);

	/* Init sound interface and intermediate buffer */
	pSound->InitRecording(iOutputBlockSize, FALSE);
	vecsSoundBuffer.Init(iOutputBlockSize);

	/* Init level meter */
	SignalLevelMeter.Init(0);
}


/* Receiver ----------------------------------------------------------------- */
void CWriteData::ProcessDataInternal(CParameter& ReceiverParam)
{
	int i;

	/* Calculate size of each individual audio channel */
	const int iHalfBlSi = iInputBlockSize / 2;

	switch (eOutChanSel)
	{
	case CS_BOTH_BOTH:
		/* left -> left, right -> right (vector sizes might not be the
		   same -> use for-loop for copying) */
		for (i = 0; i < iInputBlockSize; i++)
			vecsTmpAudData[i] = (*pvecInputData)[i]; /* Just copy data */
		break;

	case CS_LEFT_LEFT:
		/* left -> left, right muted */
		for (i = 0; i < iHalfBlSi; i++)
		{
			vecsTmpAudData[2 * i] = (*pvecInputData)[2 * i];
			vecsTmpAudData[2 * i + 1] = 0; /* mute */
		}
		break;

	case CS_RIGHT_RIGHT:
		/* left muted, right -> right */
		for (i = 0; i < iHalfBlSi; i++)
		{
			vecsTmpAudData[2 * i] = 0; /* mute */
			vecsTmpAudData[2 * i + 1] = (*pvecInputData)[2 * i + 1];
		}
		break;

	case CS_LEFT_MIX:
		/* left -> mix, right muted */
		for (i = 0; i < iHalfBlSi; i++)
		{
			/* Mix left and right channel together. Prevent overflow! First,
			   copy recorded data from "short" in "_REAL" type variables */
			//const _REAL rLeftChan = (*pvecInputData)[2 * i];
			//const _REAL rRightChan = (*pvecInputData)[2 * i + 1];

			// short in FXP and save as a REAL
                        const int rLeftChan = (*pvecInputData)[2 * i];
			const int  rRightChan = (*pvecInputData)[2 * i + 1];
			fMixNormConst=rMixNormConst;
			FXP vecs = Real2Sample(rLeftChan+rRightChan);
			vecs =vecs* fMixNormConst;

			vecsTmpAudData[2 * i] = vecs.GetFValue();
			//	Real2Sample((rLeftChan + rRightChan) * rMixNormConst);

			vecsTmpAudData[2 * i + 1] = 0; /* mute */
		}
		break;

	case CS_RIGHT_MIX:
		/* left muted, right -> mix */
		for (i = 0; i < iHalfBlSi; i++)
		{
			/* Mix left and right channel together. Prevent overflow! First,
			   copy recorded data from "short" in "_REAL" type variables */
			//const _REAL rLeftChan = (*pvecInputData)[2 * i];
			//const _REAL rRightChan = (*pvecInputData)[2 * i + 1];
                        const int rLeftChan = (*pvecInputData)[2 * i];
                        const int  rRightChan = (*pvecInputData)[2 * i + 1];
	                fMixNormConst=rMixNormConst;
	                FXP vecs = Real2Sample(rLeftChan+rRightChan);
	                vecs =vecs* fMixNormConst;

			vecsTmpAudData[2 * i] = 0; /* mute */
			vecsTmpAudData[2 * i + 1] = vecs.GetFValue();
			//	Real2Sample((rLeftChan + rRightChan) * rMixNormConst);
		}
		break;
	}

	if (bMuteAudio == TRUE)
	{
		/* Clear both channels if muted */
		for (i = 0; i < iInputBlockSize; i++)
			vecsTmpAudData[i] = 0;
	}

	/* Put data to sound card interface. Show sound card state on GUI */
	if (pSound->Write(vecsTmpAudData) == FALSE)
		PostWinMessage(MS_IOINTERFACE, 0); /* green light */
	else
		PostWinMessage(MS_IOINTERFACE, 1); /* yellow light */

	/* Write data as wave in file */
	if (bDoWriteWaveFile == TRUE)
	{
		/* Write audio data to file only if it is not zero */
		_BOOLEAN bDoNotWrite = TRUE;
		for (i = 0; i < iInputBlockSize; i++)
		{
			if ((*pvecInputData)[i] != 0)
				bDoNotWrite = FALSE;
		}

		if (bDoNotWrite == FALSE)
		{
			for (i = 0; i < iInputBlockSize; i += 2)
			{
				WaveFileAudio.AddStereoSample((*pvecInputData)[i] /* left */,
					(*pvecInputData)[i + 1] /* right */);
			}
		}
	}

	/* Store data in buffer for spectrum calculation */
	vecsOutputData.AddEnd((*pvecInputData), iInputBlockSize);
}

void CWriteData::InitInternal(CParameter& ReceiverParam)
{
	/* An audio frame always corresponds to 400 ms.
	   We use always stereo blocks */
	const int iAudFrameSize = (int) (SOUNDCRD_SAMPLE_RATE *
		4/10 /* 400 ms */);

	/* Check if blocking behaviour of sound interface shall be changed */
	if (bNewSoundBlocking != bSoundBlocking)
		bSoundBlocking = bNewSoundBlocking;

	/* Init sound interface with blocking or non-blocking behaviour */
	pSound->InitPlayback(iAudFrameSize * 2 /* stereo */, bSoundBlocking);

	/* Init intermediate buffer needed for different channel selections */
	vecsTmpAudData.Init(iAudFrameSize * 2 /* stereo */);

	/* Inits for audio spectrum plot */
	vecrHammingWindow = Hamming(NUM_SMPLS_4_AUDIO_SPECTRUM);
	vecsOutputData.Reset(0); /* Reset audio data storage vector */

	/* Define block-size for input (stereo input) */
	iInputBlockSize = iAudFrameSize * 2 /* stereo */;
}

CWriteData::CWriteData(CSound* pNS) : pSound(pNS), /* Sound interface */
	bMuteAudio(FALSE), bDoWriteWaveFile(FALSE),
	bSoundBlocking(FALSE), bNewSoundBlocking(FALSE),
	/* Inits for audio spectrum plotting */
	vecsOutputData((int) NUM_BLOCKS_AV_AUDIO_SPEC * NUM_SMPLS_4_AUDIO_SPECTRUM *
	2 /* stereo */, 0), /* Init with zeros */
	FftPlan(NUM_SMPLS_4_AUDIO_SPECTRUM),
	veccFFTInput(NUM_SMPLS_4_AUDIO_SPECTRUM),
	veccFFTOutput(NUM_SMPLS_4_AUDIO_SPECTRUM),
	vecrHammingWindow(NUM_SMPLS_4_AUDIO_SPECTRUM),
	eOutChanSel(CS_BOTH_BOTH), rMixNormConst(MIX_OUT_CHAN_NORM_CONST)
{
	/* Constructor */
}

void CWriteData::StartWriteWaveFile(const string strFileName)
{ 
	/* No Lock(), Unlock() needed here */
	if (bDoWriteWaveFile == FALSE)
	{
		WaveFileAudio.Open(strFileName);
		bDoWriteWaveFile = TRUE;
	}
}

void CWriteData::StopWriteWaveFile()
{
	Lock();

	WaveFileAudio.Close();
	bDoWriteWaveFile = FALSE;

	Unlock();
}

void CWriteData::GetAudioSpec(CVector<_REAL>& vecrData,
							  CVector<_REAL>& vecrScale)
{
	/* Real input signal -> symmetrical spectrum -> use only half of spectrum */
	const int iLenPowSpec = NUM_SMPLS_4_AUDIO_SPECTRUM / 2;

	/* Init output vectors */
	vecrData.Init(iLenPowSpec, (_REAL) 0.0);
	vecrScale.Init(iLenPowSpec, (_REAL) 0.0);

	/* Do copying of data only if vector is of non-zero length which means that
	   the module was already initialized */
	if (iLenPowSpec != 0)
	{
		int i, j;

		/* Lock resources */
		Lock();

		/* Init vector storing the average spectrum with zeros */
		CVector<_REAL> veccAvSpectrum(iLenPowSpec, (_REAL) 0.0);

		int iCurPosInStream = 0;
		for (j = 0; j < NUM_BLOCKS_AV_AUDIO_SPEC; j++)
		{
			for (i = 0; i < NUM_SMPLS_4_AUDIO_SPECTRUM; i++)
			{
				/* Mix both channels */
				veccFFTInput[i] =
					((_REAL) vecsOutputData[(i + iCurPosInStream) * 2] +
					vecsOutputData[(i + iCurPosInStream) * 2 + 1]) / 2;
			}

			/* Apply hamming window */
			veccFFTInput *= vecrHammingWindow;

			/* Calculate Fourier transformation to get the spectrum */
			veccFFTOutput = Fft(veccFFTInput, FftPlan);

			/* Average power (using power of this tap) */
			for (i = 0; i < iLenPowSpec; i++)
				veccAvSpectrum[i] += SqMag(veccFFTOutput[i]);

			iCurPosInStream += NUM_SMPLS_4_AUDIO_SPECTRUM;
		}

		/* Calculate norm constand and scale factor */
		const _REAL rNormData = (_REAL) NUM_SMPLS_4_AUDIO_SPECTRUM *
			NUM_SMPLS_4_AUDIO_SPECTRUM * _MAXSHORT * _MAXSHORT *
			NUM_BLOCKS_AV_AUDIO_SPEC;
		const _REAL rFactorScale =
			(_REAL) SOUNDCRD_SAMPLE_RATE / iLenPowSpec / 2000;

		/* Apply the normalization (due to the FFT) */
		for (i = 0; i < iLenPowSpec; i++)
		{
			const _REAL rNormPowSpec = veccAvSpectrum[i] / rNormData;

			if (rNormPowSpec > 0)
				vecrData[i] = (_REAL) 10.0 * log10(rNormPowSpec);
			else
				vecrData[i] = RET_VAL_LOG_0;

			vecrScale[i] = (_REAL) i * rFactorScale;
		}

		/* Release resources */
		Unlock();
	}
}


/* Simulation --------------------------------------------------------------- */
void CGenSimData::ProcessDataInternal(CParameter& TransmParam)
{
	int			i;
	uint32_t	iTempShiftRegister1;
	_BINARY		biPRBSbit;
	uint32_t	iShiftRegister;
	FILE*		pFileCurPos;
	time_t		tiElTi;
	long int	lReTi;

	/* Get elapsed time since this run was started (seconds) */
	tiElTi = time(NULL) - tiStartTime;

	/* Stop simulation if stop condition is true */
	iCounter++;
	switch (eCntType)
	{
	case CT_TIME:
		//try
		{
			/* Estimate remaining time */
			lReTi = (long int) (((_REAL) iNumSimBlocks - iCounter) /
				iCounter * tiElTi);

			/* Store current counter position in file */
			pFileCurPos = fopen(strFileName.c_str(), "w");
			if (pFileCurPos != NULL)
			{
				fprintf(pFileCurPos, "%d / %d (%d min elapsed, estimated "
					"time remaining: %d min)", iCounter, iNumSimBlocks,
					tiElTi / 60, lReTi / 60);

				if (TransmParam.eSimType == CParameter::ST_SYNC_PARAM)
				{
					/* Write sync paramter */
					fprintf(pFileCurPos, "\n%e %e",
						TransmParam.GetNominalSNRdB(),
						TransmParam.rSyncTestParam);
				}
				else
				{
					/* Add current value of BER */
					fprintf(pFileCurPos, "\n%e %e",
						TransmParam.GetNominalSNRdB(),
						TransmParam.rBitErrRate);
				}
				fclose(pFileCurPos);
			}
		}

		//catch (...)
		//{
			///* Catch all file errors to avoid stopping the simulation */
		//}

		if (iCounter == iNumSimBlocks)
		{
			TransmParam.bRunThread = FALSE;
			iCounter = 0;
		}
		break;

	case CT_ERRORS:
		//try
		{
			if (iCounter >= iMinNumBlocks)
			{
				/* Estimate remaining time */
				lReTi = (long int)
					(((_REAL) iNumErrors - TransmParam.iNumBitErrors) /
					TransmParam.iNumBitErrors * tiElTi);

				/* Store current counter position in file */
				pFileCurPos = fopen(strFileName.c_str(), "w");
				if (pFileCurPos != NULL)
				{
					fprintf(pFileCurPos, "%d / %d (%d min elapsed, estimated "
						"time remaining: %d min)", TransmParam.iNumBitErrors,
						iNumErrors,	tiElTi / 60, lReTi / 60);

					/* Add current value of BER */
					fprintf(pFileCurPos, "\n%e %e", TransmParam.
						GetNominalSNRdB(), TransmParam.rBitErrRate);
					fclose(pFileCurPos);
				}
			}
			else
			{
				/* Estimate remaining time */
				lReTi = (long int)
					(((_REAL) iMinNumBlocks - iCounter) / iCounter * tiElTi);

				/* Store current counter position in file */
				pFileCurPos = fopen(strFileName.c_str(), "w");
				if (pFileCurPos != NULL)
				{
					fprintf(pFileCurPos,
						"%d / %d (%d min elapsed, estimated minimum"
						" time remaining: %d min)\n",
						iCounter, iMinNumBlocks, tiElTi / 60, lReTi / 60);

					lReTi = (long int)
						(((_REAL) iNumErrors - TransmParam.iNumBitErrors) /
						TransmParam.iNumBitErrors * tiElTi);
					fprintf(pFileCurPos,
						"%d / %d (%d min elapsed, estimated"
						" time remaining: %d min)",
						TransmParam.iNumBitErrors, iNumErrors, tiElTi / 60,
						lReTi / 60);

					/* Add current value of BER */
					fprintf(pFileCurPos, "\n%e %e", TransmParam.
						GetNominalSNRdB(), TransmParam.rBitErrRate);
					fclose(pFileCurPos);
				}
			}
		}

		//catch (...)
		//{
			///* Catch all file errors to avoid stopping the simulation */
		//}

		if (TransmParam.iNumBitErrors >= iNumErrors)
		{
			/* A minimum simulation time must be elapsed */
			if (iCounter >= iMinNumBlocks)
			{
				TransmParam.bRunThread = FALSE;
				iCounter = 0;
			}
		}
		break;
	}

	/* Generate a pseudo-noise test-signal (PRBS) */
	/* Init shift register with an arbitrary number (Must be known at the
	   receiver AND transmitter!) */
	iShiftRegister = (uint32_t) (time(NULL) + rand());
	TransmParam.RawSimDa.Add(iShiftRegister);

	for (i = 0; i < iOutputBlockSize; i++)
	{
		/* Calculate new PRBS bit */
		iTempShiftRegister1 = iShiftRegister;

		/* P(X) = X^9 + X^5 + 1,
		   in this implementation we have to shift n-1! */
		biPRBSbit = ((iTempShiftRegister1 >> 4) & 1) ^
			((iTempShiftRegister1 >> 8) & 1);

		/* Shift bits in shift register and add new bit */
		iShiftRegister <<= 1;
		iShiftRegister |= (biPRBSbit & 1);

		/* Use PRBS output */
		(*pvecOutputData)[i] = biPRBSbit;
	}
}

void CGenSimData::InitInternal(CParameter& TransmParam)
{
	/* Define output block size */
	iOutputBlockSize = TransmParam.iNumDecodedBitsMSC;

	/* Minimum simulation time depends on the selected channel */
	switch (TransmParam.iDRMChannelNum)
	{
	case 1:
		/* AWGN: No fading */
		iMinNumBlocks = (int) ((_REAL) 2000.0 / (_REAL) 0.4);
		break;

	case 2:
		/* Rice with delay: 0.1 Hz */
		iMinNumBlocks = (int) ((_REAL) 5000.0 / (_REAL) 0.4);
		break;

	case 3:
		/* US Consortium: slowest 0.1 Hz */
		iMinNumBlocks = (int) ((_REAL) 15000.0 / (_REAL) 0.4);
		break;

	case 4:
		/* CCIR Poor: 1 Hz */
		iMinNumBlocks = (int) ((_REAL) 4000.0 / (_REAL) 0.4);
		break;

	case 5:
		/* Channel no 5: 2 Hz -> 30 sec */
		iMinNumBlocks = (int) ((_REAL) 3000.0 / (_REAL) 0.4);
		break;

	case 6:
		/* Channel no 6: same as case "2" */
		iMinNumBlocks = (int) ((_REAL) 2000.0 / (_REAL) 0.4);
		break;

	default:
		/* My own channels */
		iMinNumBlocks = (int) ((_REAL) 2000.0 / (_REAL) 0.4);
		break;
	}

	/* Prepare shift register used for storing the start values of the PRBS
	   shift register */
	TransmParam.RawSimDa.Reset();

	/* Init start time */
	tiStartTime = time(NULL);
}

void CGenSimData::SetSimTime(int iNewTi, string strNewFileName)
{
	/* One MSC frame is 400 ms long */
	iNumSimBlocks = (int) ((_REAL) iNewTi /* sec */ / (_REAL) 0.4);

	/* Set simulation count type */
	eCntType = CT_TIME;

	/* Reset counter */
	iCounter = 0;

	/* Set file name */
	strFileName = string(SIM_OUT_FILES_PATH) +
		strNewFileName + "__SIMTIME" + string(".dat");
}

void CGenSimData::SetNumErrors(int iNewNE, string strNewFileName)
{
	iNumErrors = iNewNE;

	/* Set simulation count type */
	eCntType = CT_ERRORS;

	/* Reset counter, because we also use it at the beginning of a run */
	iCounter = 0;

	/* Set file name */
	strFileName = string(SIM_OUT_FILES_PATH) +
		strNewFileName + "__SIMTIME" + string(".dat");
}

void CEvaSimData::ProcessDataInternal(CParameter& ReceiverParam)
{
	uint32_t		iTempShiftRegister1;
	_BINARY			biPRBSbit;
	uint32_t		iShiftRegister;
	int				iNumBitErrors;
	int				i;

	/* -------------------------------------------------------------------------
	   Generate a pseudo-noise test-signal (PRBS) for comparison with
	   received signal */
	/* Init shift register with an arbitrary number (Must be known at the
	   receiver AND transmitter!) */
	iShiftRegister = ReceiverParam.RawSimDa.Get();

	iNumBitErrors = 0;

	for (i = 0; i < iInputBlockSize; i++)
	{
		/* Calculate new PRBS bit */
		iTempShiftRegister1 = iShiftRegister;

		/* P(X) = X^9 + X^5 + 1,
		   in this implementation we have to shift n-1! */
		biPRBSbit = ((iTempShiftRegister1 >> 4) & 1) ^
			((iTempShiftRegister1 >> 8) & 1);

		/* Shift bits in shift register and add new bit */
		iShiftRegister <<= 1;
		iShiftRegister |= (biPRBSbit & 1);

		/* Count bit errors */
		if (biPRBSbit != (*pvecInputData)[i])
			iNumBitErrors++;
	}

	/* Save bit error rate, debar initialization blocks */
	if (iIniCnt > 0)
		iIniCnt--;
	else
	{
		rAccBitErrRate += (_REAL) iNumBitErrors / iInputBlockSize;
		iNumAccBitErrRate++;

		ReceiverParam.rBitErrRate = rAccBitErrRate / iNumAccBitErrRate;
		ReceiverParam.iNumBitErrors += iNumBitErrors;
	}
}

void CEvaSimData::InitInternal(CParameter& ReceiverParam)
{
	/* Reset bit error rate parameters */
	rAccBitErrRate = (_REAL) 0.0;
	iNumAccBitErrRate = 0;

	/* Number of blocks at the beginning we do not want to use */
	iIniCnt = 10;

	/* Init global parameters */
	ReceiverParam.rBitErrRate = (_REAL) 0.0;
	ReceiverParam.iNumBitErrors = 0;

	/* Define block-size for input */
	iInputBlockSize = ReceiverParam.iNumDecodedBitsMSC;
}


/******************************************************************************\
* FAC data																	   *
\******************************************************************************/
/* Transmitter */
void CGenerateFACData::ProcessDataInternal(CParameter& TransmParam)
{
	FACTransmit.FACParam(pvecOutputData, TransmParam);
}

void CGenerateFACData::InitInternal(CParameter& TransmParam)
{
	FACTransmit.Init(TransmParam);

	/* Define block-size for output */
	iOutputBlockSize = NUM_FAC_BITS_PER_BLOCK;
}

/* Receiver */
void CUtilizeFACData::ProcessDataInternal(CParameter& ReceiverParam)
{
#ifdef USE_QT_GUI
	/* MDI (check that the pointer to the MDI object is not NULL. It can be NULL
	   in case of simulation because in this case there is no MDI) */
	if (pMDI != NULL)
	{
		/* Only put data in MDI object if MDI is enabled */
		if (pMDI->GetMDIOutEnabled() == TRUE)
			pMDI->SetFACData(*pvecInputData, ReceiverParam);

		/* Check if MDI in is enabled and query data if enabled */
		if (pMDI->GetMDIInEnabled() == TRUE)
		{
			/* Data in "pvecInputData" will be overwritten! */
			ReceiverParam.SetWaveMode(pMDI->GetFACData(*pvecInputData));
		}
	}
#endif

	/* Do not use received FAC data in case of simulation */
	if (bSyncInput == FALSE)
	{
		bCRCOk = FACReceive.FACParam(pvecInputData, ReceiverParam);

		if (bCRCOk == TRUE)
		{
			PostWinMessage(MS_FAC_CRC, 0);

			/* Set FAC status in log file */
			ReceiverParam.ReceptLog.SetFAC(TRUE);
		}
		else
		{
			PostWinMessage(MS_FAC_CRC, 2);

			/* Set FAC status in log file */
			ReceiverParam.ReceptLog.SetFAC(FALSE);
		}
	}

	if ((bSyncInput == TRUE) || (bCRCOk == FALSE))
	{
		/* If FAC CRC check failed we should increase the frame-counter 
		   manually. If only FAC data was corrupted, the others can still
		   decode if they have the right frame number. In case of simulation
		   no FAC data is used, we have to increase the counter here */
		ReceiverParam.iFrameIDReceiv++;

		if (ReceiverParam.iFrameIDReceiv == NUM_FRAMES_IN_SUPERFRAME)
			ReceiverParam.iFrameIDReceiv = 0;
	}
}

void CUtilizeFACData::InitInternal(CParameter& ReceiverParam)
{

// This should be in FAC class in an Init() routine which has to be defined, this
// would be cleaner code! TODO
/* Init frame ID so that a "0" comes after increasing the init value once */
ReceiverParam.iFrameIDReceiv = NUM_FRAMES_IN_SUPERFRAME - 1;

	/* Reset flag */
	bCRCOk = FALSE;

	/* Define block-size for input */
	iInputBlockSize = NUM_FAC_BITS_PER_BLOCK;
}


/******************************************************************************\
* SDC data																	   *
\******************************************************************************/
/* Transmitter */
void CGenerateSDCData::ProcessDataInternal(CParameter& TransmParam)
{
	SDCTransmit.SDCParam(pvecOutputData, TransmParam);
}

void CGenerateSDCData::InitInternal(CParameter& TransmParam)
{
	/* Define block-size for output */
	iOutputBlockSize = TransmParam.iNumSDCBitsPerSFrame;
}

/* Receiver */
void CUtilizeSDCData::ProcessDataInternal(CParameter& ReceiverParam)
{
#ifdef USE_QT_GUI
	/* MDI (check that the pointer to the MDI object is not NULL. It can be NULL
	   in case of simulation because in this case there is no MDI) */
	if (pMDI != NULL)
	{
		/* Only put data in MDI object if MDI is enabled */
		if (pMDI->GetMDIOutEnabled() == TRUE)
			pMDI->SetSDCData(*pvecInputData);

		/* Check if MDI in is enabled and query data if enabled */
		if (pMDI->GetMDIInEnabled() == TRUE)
		{
			/* Data in "pvecInputData" will be overwritten! */
			pMDI->GetSDCData(*pvecInputData);
		}
	}
#endif

	/* Decode SDC block and return CRC status */
	switch (SDCReceive.SDCParam(pvecInputData, ReceiverParam))
	{
	case CSDCReceive::SR_OK:
		PostWinMessage(MS_SDC_CRC, 0); /* Green light */
		break;

	case CSDCReceive::SR_BAD_CRC:
		/* SDC block depends on only a few parameters: robustness mode,
		   DRM bandwidth and coding scheme (can be 4 or 16 QAM). If we
		   initialize these parameters with resonable parameters it might
		   be possible that these are the correct parameters. Therefore
		   try to decode SDC even in case FAC wasn't decoded. That might
		   speed up the DRM signal acqisition. But quite often it is the
		   case that the parameters are not correct. In this case do not
		   show a red light if SDC CRC was not ok */
		if (bFirstBlock == FALSE)
			PostWinMessage(MS_SDC_CRC, 2); /* Red light */
		break;

	case CSDCReceive::SR_BAD_DATA:
		/* CRC was ok but data seems to be incorrect */
		PostWinMessage(MS_SDC_CRC, 1); /* Yellow light */
		break;
	}

	/* Reset "first block" flag */
	bFirstBlock = FALSE;
}

void CUtilizeSDCData::InitInternal(CParameter& ReceiverParam)
{
	/* Init "first block" flag */
	bFirstBlock = TRUE;

	/* Define block-size for input */
	iInputBlockSize = ReceiverParam.iNumSDCBitsPerSFrame;
}

/******************************************************************************\
* File I/O (util/AudioFile.h)						   *
\******************************************************************************/

	void CWaveFile::Open(const string strFileName)
	{
		if (pFile != NULL)
			Close();

		const CWaveHdr WaveHeader =
		{
			/* Use always stereo and PCM */
			{'R', 'I', 'F', 'F'}, 0, {'W', 'A', 'V', 'E'},
			{'f', 'm', 't', ' '}, 16, 1, 2, SOUNDCRD_SAMPLE_RATE,
			SOUNDCRD_SAMPLE_RATE * 4 /* same as block align */,
			4 /* block align */, 16,
			{'d', 'a', 't', 'a'}, 0
		};

		pFile = fopen(strFileName.c_str(), "wb");
		if (pFile != NULL)
		{
			iBytesWritten = sizeof(CWaveHdr);
			fwrite((const void*) &WaveHeader, size_t(sizeof(CWaveHdr)),
				size_t(1), pFile);
		}
		else
			printf("Error opening input file!\n");
	}

	void CWaveFile::AddStereoSample(const _SAMPLE sLeft, const _SAMPLE sRight)
	{
		if (pFile != NULL)
		{
			iBytesWritten += 2 * sizeof(_SAMPLE);
			fwrite((const void*) &sLeft, size_t(2), size_t(1), pFile);
			fwrite((const void*) &sRight, size_t(2), size_t(1), pFile);
		}
	}
	void CWaveFile::Close()
	{
		if (pFile != NULL)
		{
			const uint32_t iFileLength = iBytesWritten - 8;
			fseek(pFile, 4 // offset 
			, SEEK_SET // origin 
			);
			fwrite((const void*) &iFileLength, size_t(4), size_t(1), pFile);

			const uint32_t iDataLength = iBytesWritten - sizeof(CWaveHdr);
			fseek(pFile, 40 // offset 
			, SEEK_SET // origin 
			);
			fwrite((const void*) &iDataLength, size_t(4), size_t(1), pFile);

			fclose(pFile);
			pFile = NULL;
		}
	}

