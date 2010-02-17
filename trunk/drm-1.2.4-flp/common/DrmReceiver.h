/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See DrmReceiver.cpp
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

#if !defined(DRMRECEIVER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define DRMRECEIVER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

#include <iostream>
#include "Parameter.h"
#include "util/Buffer.h"
#include "util/Utilities.h"
#include "DataIO.h"
#include "OFDM.h"
#include "DRMSignalIO.h"
#include "MSCMultiplexer.h"
#include "InputResample.h"
#include "datadecoding/DataDecoder.h"
#include "sourcedecoders/AudioSourceDecoder.h"
#include "mlc/MLC.h"
#include "interleaver/SymbolInterleaver.h"
#include "ofdmcellmapping/OFDMCellMapping.h"
#include "chanest/ChannelEstimation.h"
#include "sync/FreqSyncAcq.h"
#include "sync/TimeSync.h"
#include "sync/SyncUsingPil.h"
#include "AMDemodulation.h"
#ifdef USE_QT_GUI
# include "MDI.h"
#endif
#ifdef _WIN32
# include "../../Windows/source/sound.h"
#else
# include "source/sound.h"
#endif


/* Definitions ****************************************************************/
/* Number of FAC frames until the acquisition is activated in case a signal
   was successfully decoded */
#define	NUM_FAC_FRA_U_ACQ_WITH			10

/* Number of OFDM symbols until the acquisition is activated in case no signal
   could be decoded after previous acquisition try */
#define	NUM_OFDMSYM_U_ACQ_WITHOUT		150

/* Number of FAC blocks for delayed tracking mode switch (caused by time needed
   for initalizing the channel estimation */
#define NUM_FAC_DEL_TRACK_SWITCH		2

/* Length of the history for synchronization parameters (used for the plot) */
#define LEN_HIST_PLOT_SYNC_PARMS		2250


/* Classes ********************************************************************/
class CDRMReceiver
{
public:
	/* Acquisition state of receiver */
	enum EAcqStat {AS_NO_SIGNAL, AS_WITH_SIGNAL};

	/* Receiver state */
	enum ERecState {RS_TRACKING, RS_ACQUISITION};

	/* RM: Receiver mode (analog or digital demodulation) */
	enum ERecMode {RM_DRM, RM_AM, RM_NONE};


	CDRMReceiver() : eAcquiState(AS_NO_SIGNAL), iAcquRestartCnt(0),
		iGoodSignCnt(0), bDoInitRun(FALSE),
		eReceiverMode(RM_DRM), 	eNewReceiverMode(RM_NONE),
		ReceiveData(&SoundInterface), WriteData(&SoundInterface),
		rInitResampleOffset((_REAL) 0.0), iAcquDetecCnt(0),
		vecrFreqSyncValHist(LEN_HIST_PLOT_SYNC_PARMS),
		vecrSamOffsValHist(LEN_HIST_PLOT_SYNC_PARMS),
		vecrLenIRHist(LEN_HIST_PLOT_SYNC_PARMS),
		vecrDopplerHist(LEN_HIST_PLOT_SYNC_PARMS),
		vecrSNRHist(LEN_HIST_PLOT_SYNC_PARMS),
		veciCDAudHist(LEN_HIST_PLOT_SYNC_PARMS), iAvCntParamHist(0),
		rAvLenIRHist((_REAL) 0.0), rAvDopplerHist((_REAL) 0.0),
		rAvSNRHist((_REAL) 0.0), iCurrentCDAud(0)
#ifdef USE_QT_GUI
		, UtilizeFACData(&MDI), UtilizeSDCData(&MDI), MSCDemultiplexer(&MDI)
#endif
#if defined(USE_QT_GUI) || defined(_WIN32)
		, iMainPlotColorStyle(0), /* default color scheme: blue-white */
		iSecondsPreview(0), GeomChartWindows(0), bEnableSMeter(TRUE),
		iSysEvalDlgPlotType(0)
#endif
		{}
	virtual ~CDRMReceiver() {}

	/* For GUI */
	void					Init();
	void					Start();
	void					Stop();
	EAcqStat				GetReceiverState() {return eAcquiState;}
	ERecMode				GetReceiverMode() {return eReceiverMode;}
	void					SetReceiverMode(ERecMode eNewMode)
								{eNewReceiverMode = eNewMode;}

	void					SetInitResOff(_REAL rNRO)
								{rInitResampleOffset = rNRO;}

	void					SetAMDemodAcq(_REAL rNewNorCen);

	/* Get pointer to internal modules */
	CUtilizeFACData*		GetFAC() {return &UtilizeFACData;}
	CUtilizeSDCData*		GetSDC() {return &UtilizeSDCData;}
	CTimeSync*				GetTimeSync() {return &TimeSync;}
	CChannelEstimation*		GetChanEst() {return &ChannelEstimation;}
	CFACMLCDecoder*			GetFACMLC() {return &FACMLCDecoder;}
	CSDCMLCDecoder*			GetSDCMLC() {return &SDCMLCDecoder;}
	CMSCMLCDecoder*			GetMSCMLC() {return &MSCMLCDecoder;}
	CReceiveData*			GetReceiver() {return &ReceiveData;}
	COFDMDemodulation*		GetOFDMDemod() {return &OFDMDemodulation;}
	CSyncUsingPil*			GetSyncUsPil() {return &SyncUsingPil;}
	CWriteData*				GetWriteData() {return &WriteData;}
	CSound*					GetSoundInterface() {return &SoundInterface;}
	CDataDecoder*			GetDataDecoder() {return &DataDecoder;}
	CAMDemodulation*		GetAMDemod() {return &AMDemodulation;}
	CFreqSyncAcq*			GetFreqSyncAcq() {return &FreqSyncAcq;}
	CAudioSourceDecoder*	GetAudSorceDec() {return &AudioSourceDecoder;}
#ifdef USE_QT_GUI
	CMDI*					GetMDI() {return &MDI;}
#endif
#ifdef HAVE_LIBHAMLIB
	CHamlib*				GetHamlib() {return &Hamlib;}
#endif

	CParameter*				GetParameters() {return &ReceiverParam;}
	void					StartParameters(CParameter& Param);
	void					SetInStartMode();
	void					SetInTrackingMode();
	void					SetInTrackingModeDelayed();

	void					SetReadDRMFromFile(const string strNFN);

	void					InitsForAllModules();

	void					InitsForWaveMode();
	void					InitsForSpectrumOccup();
	void					InitsForNoDecBitsSDC();
	void					InitsForAudParam();
	void					InitsForDataParam();
	void					InitsForInterlDepth();
	void					InitsForMSCCodSche();
	void					InitsForSDCCodSche();
	void					InitsForMSC();
	void					InitsForMSCDemux();

/* _WIN32 check because in Visual c++ the GUI files are always compiled even
   if USE_QT_GUI is set or not */
#if defined(USE_QT_GUI) || defined(_WIN32)
	/* DRMReceiver object serves as a storage a "exchange platform" for the
	   window size and position parameters for init-file usage. This is not
	   nice but it works for now. TODO: better solution */
	class CWinGeom
	{
	public:
		CWinGeom() : iXPos(0), iYPos(0), iHSize(0), iWSize(0),
			bVisible(FALSE), iType(0) {}

		int iXPos, iYPos;
		int iHSize, iWSize;
		_BOOLEAN bVisible;
		int iType;
	};

	/* Main window */
	CWinGeom GeomFdrmdialog;

	/* System evaluation window */
	CWinGeom GeomSystemEvalDlg;

	/* Multimedia window */
	CWinGeom GeomMultimediaDlg;

	/* Stations dialog */
	CWinGeom GeomStationsDlg;

	/* Analog demodulation dialog */
	CWinGeom GeomAnalogDemDlg;

	/* Chart windows */
	CVector<CWinGeom> GeomChartWindows;

	int			iMainPlotColorStyle;
	int			iSecondsPreview;
	_BOOLEAN	bEnableSMeter;
	int			iSysEvalDlgPlotType;
#endif

	/* Interfaces to internal parameters/vectors used for the plot */
	void GetFreqSamOffsHist(CVector<_REAL>& vecrFreqOffs,
		CVector<_REAL>& vecrSamOffs, CVector<_REAL>& vecrScale,
		_REAL& rFreqAquVal);
	void GetDopplerDelHist(CVector<_REAL>& vecrLenIR,
		CVector<_REAL>& vecrDoppler, CVector<_REAL>& vecrScale);
	void GetSNRHist(CVector<_REAL>& vecrSNR, CVector<_REAL>& vecrCDAud,
		CVector<_REAL>& vecrScale);

protected:
	void					Run();
	void					DetectAcquiFAC();
	void					DetectAcquiSymbol();
	void					InitReceiverMode();
	void					UpdateParamHistories();

	/* Modules */
	CReceiveData			ReceiveData;
	CInputResample			InputResample;
	CFreqSyncAcq			FreqSyncAcq;
	CTimeSync				TimeSync;
	COFDMDemodulation		OFDMDemodulation;
	CSyncUsingPil			SyncUsingPil;
	CChannelEstimation		ChannelEstimation;
	COFDMCellDemapping		OFDMCellDemapping;
	CFACMLCDecoder			FACMLCDecoder;
	CUtilizeFACData			UtilizeFACData;
	CSDCMLCDecoder			SDCMLCDecoder;
	CUtilizeSDCData			UtilizeSDCData;
	CSymbDeinterleaver		SymbDeinterleaver;
	CMSCMLCDecoder			MSCMLCDecoder;
	CMSCDemultiplexer		MSCDemultiplexer;
	CAudioSourceDecoder		AudioSourceDecoder;
	CDataDecoder			DataDecoder;
	CWriteData				WriteData;
	CAMDemodulation			AMDemodulation;

	/* Parameters */
	CParameter				ReceiverParam;

	/* Buffers */
	CSingleBuffer<_REAL>	RecDataBuf;
	CCyclicBuffer<_REAL>	InpResBuf;
	CCyclicBuffer<_COMPLEX>	FreqSyncAcqBuf;
	CSingleBuffer<_COMPLEX>	TimeSyncBuf;
	CSingleBuffer<_COMPLEX>	OFDMDemodBuf;
	CSingleBuffer<_COMPLEX>	SyncUsingPilBuf;
	CSingleBuffer<CEquSig>	ChanEstBuf;
	CCyclicBuffer<CEquSig>	MSCCarDemapBuf;
	CCyclicBuffer<CEquSig>	FACCarDemapBuf;
	CCyclicBuffer<CEquSig>	SDCCarDemapBuf;
	CSingleBuffer<CEquSig>	DeintlBuf;
	CSingleBuffer<_BINARY>	FACDecBuf;
	CSingleBuffer<_BINARY>	SDCDecBuf;
	CSingleBuffer<_BINARY>	MSCMLCDecBuf;
	CSingleBuffer<_BINARY>	MSCDeMUXBufAud;
	CSingleBuffer<_BINARY>	MSCDeMUXBufData;
	CCyclicBuffer<_SAMPLE>	AudSoDecBuf;

	EAcqStat				eAcquiState;
	int						iAcquRestartCnt;
	int						iAcquDetecCnt;
	int						iGoodSignCnt;
	int						iDelayedTrackModeCnt;
	ERecState				eReceiverState;
	ERecMode				eReceiverMode;
	ERecMode				eNewReceiverMode;

	CSound					SoundInterface;

#ifdef USE_QT_GUI
	CMDI					MDI;
#endif

	_BOOLEAN				bDoInitRun;

	_REAL					rInitResampleOffset;

#ifdef HAVE_LIBHAMLIB
	CHamlib					Hamlib;
#endif

	int						iSoundCrdDevIn;
	int						iSoundCrdDevOut;


	/* Storing parameters for plot */
	CShiftRegister<_REAL>	vecrFreqSyncValHist;
	CShiftRegister<_REAL>	vecrSamOffsValHist;
	CShiftRegister<_REAL>	vecrLenIRHist;
	CShiftRegister<_REAL>	vecrDopplerHist;
	CShiftRegister<_REAL>	vecrSNRHist;
	CShiftRegister<int>		veciCDAudHist;
	int						iAvCntParamHist;
	_REAL					rAvLenIRHist;
	_REAL					rAvDopplerHist;
	_REAL					rAvSNRHist;
	int						iCurrentCDAud;

	CMutex					MutexHist;
};


#endif // !defined(DRMRECEIVER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
