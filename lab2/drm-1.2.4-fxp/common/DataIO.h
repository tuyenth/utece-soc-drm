/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See Data.cpp
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

#if !defined(DATA_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define DATA_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

#include "Parameter.h"
#include "util/Modul.h"
#include "FAC/FAC.h"
#include "SDC/SDC.h"
#include "TextMessage.h"
#include "util/AudioFile.h"
#include "util/Utilities.h"
#ifdef USE_QT_GUI
# include "MDI.h"
#endif
#ifdef _WIN32
# include "../../Windows/source/sound.h"
#else
# include "source/sound.h"
#endif
#ifdef HAVE_DFFTW_H
# include <dfftw.h>
#else
# include <fftw.h>
#endif
#include <time.h>


/* Definitions ****************************************************************/
/* In case of random-noise, define number of blocks */
#define DEFAULT_NUM_SIM_BLOCKS		50

/* Length of vector for audio spectrum. We use a power-of-two length to 
   make the FFT work more efficient */
#define NUM_SMPLS_4_AUDIO_SPECTRUM	256

/* Time span used for averaging the audio spectrum. Shall be higher than the
   400 ms DRM audio block */
#define TIME_AV_AUDIO_SPECT_MS		500 /* ms */		

/* Number of blocks for averaging the audio spectrum */
#define NUM_BLOCKS_AV_AUDIO_SPEC	Ceil(((_REAL) SOUNDCRD_SAMPLE_RATE * \
	TIME_AV_AUDIO_SPECT_MS / 1000 / NUM_SMPLS_4_AUDIO_SPECTRUM))

/* Normalization constant for two mixed signals. If this constant is 2, no
   overrun of the "short" variable can happen but signal has quite much lower
   power -> compromise */
#define MIX_OUT_CHAN_NORM_CONST		((_REAL) 1.0 / sqrt((_REAL) 2.0))


/* Classes ********************************************************************/
/* MSC ---------------------------------------------------------------------- */
class CReadData : public CTransmitterModul<_SAMPLE, _SAMPLE>
{
public:
	CReadData(CSound* pNS) : pSound(pNS) {}
	virtual ~CReadData() {}

	_REAL GetLevelMeter() {return SignalLevelMeter.Level();}

protected:
	CSound*				pSound;
	CVector<_SAMPLE>	vecsSoundBuffer;
	CSignalLevelMeter	SignalLevelMeter;

	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};

class CWriteData : public CReceiverModul<_SAMPLE, _SAMPLE>
{
public:
	enum EOutChanSel {CS_BOTH_BOTH, CS_LEFT_LEFT, CS_RIGHT_RIGHT,
		CS_LEFT_MIX, CS_RIGHT_MIX};

	CWriteData(CSound* pNS);
	virtual ~CWriteData() {}

	void StartWriteWaveFile(const string strFileName);
	_BOOLEAN GetIsWriteWaveFile() {return bDoWriteWaveFile;}
	void StopWriteWaveFile();

	void MuteAudio(_BOOLEAN bNewMA) {bMuteAudio = bNewMA;}
	_BOOLEAN GetMuteAudio() {return bMuteAudio;}

	void SetSoundBlocking(const _BOOLEAN bNewBl)
		{bNewSoundBlocking = bNewBl; SetInitFlag();}

	void GetAudioSpec(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale);

	void SetOutChanSel(const EOutChanSel eNS) {eOutChanSel = eNS;}
	EOutChanSel GetOutChanSel() {return eOutChanSel;}

protected:
	CSound*					pSound;
	_BOOLEAN				bMuteAudio;
	CWaveFile				WaveFileAudio;
	_BOOLEAN				bDoWriteWaveFile;
	_BOOLEAN				bSoundBlocking;
	_BOOLEAN				bNewSoundBlocking;
	CVector<_SAMPLE>		vecsTmpAudData;
	EOutChanSel				eOutChanSel;
	_REAL					rMixNormConst;
        FXP                                   fMixNormConst;

	CShiftRegister<_SAMPLE>	vecsOutputData;
	CFftPlans				FftPlan;
	CComplexVector			veccFFTInput;
	CComplexVector			veccFFTOutput;
	CRealVector				vecrHammingWindow;

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};

class CGenSimData : public CTransmitterModul<_BINARY, _BINARY>
{
public:
	CGenSimData() : iNumSimBlocks(DEFAULT_NUM_SIM_BLOCKS), eCntType(CT_TIME),
		iCounter(0), iNumErrors(0), strFileName("SimTime.dat"), tiStartTime(0) {}
	virtual ~CGenSimData() {}

	void SetSimTime(int iNewTi, string strNewFileName);
	void SetNumErrors(int iNewNE, string strNewFileName);

protected:
	enum ECntType {CT_TIME, CT_ERRORS};
	ECntType	eCntType;
	int			iNumSimBlocks;
	int			iNumErrors;
	int			iCounter;
	int			iMinNumBlocks;
	string		strFileName;
	time_t		tiStartTime;

	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};

class CEvaSimData : public CReceiverModul<_BINARY, _BINARY>
{
public:
	CEvaSimData() {}
	virtual ~CEvaSimData() {}

protected:
	int		iIniCnt;
	int		iNumAccBitErrRate;
	_REAL	rAccBitErrRate;

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};


/* FAC ---------------------------------------------------------------------- */
class CGenerateFACData : public CTransmitterModul<_BINARY, _BINARY>
{
public:
	CGenerateFACData() {}
	virtual ~CGenerateFACData() {}

protected:
	CFACTransmit FACTransmit;

	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};

class CUtilizeFACData : public CReceiverModul<_BINARY, _BINARY>
{
public:
#ifdef USE_QT_GUI
	CUtilizeFACData(CMDI* pNM) : pMDI(pNM),
#else
	CUtilizeFACData() :
#endif
		bSyncInput(FALSE), bCRCOk(FALSE) {}
	virtual ~CUtilizeFACData() {}

	/* To set the module up for synchronized DRM input data stream */
	void SetSyncInput(_BOOLEAN bNewS) {bSyncInput = bNewS;}

	_BOOLEAN GetCRCOk() const {return bCRCOk;}

protected:
	CFACReceive FACReceive;

#ifdef USE_QT_GUI
	CMDI*		pMDI;
#endif

	_BOOLEAN	bCRCOk;
	_BOOLEAN	bSyncInput;

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};


/* SDC ---------------------------------------------------------------------- */
class CGenerateSDCData : public CTransmitterModul<_BINARY, _BINARY>
{
public:
	CGenerateSDCData() {}
	virtual ~CGenerateSDCData() {}

protected:
	CSDCTransmit SDCTransmit;
		
	virtual void InitInternal(CParameter& TransmParam);
	virtual void ProcessDataInternal(CParameter& TransmParam);
};

class CUtilizeSDCData : public CReceiverModul<_BINARY, _BINARY>
{
public:
#ifdef USE_QT_GUI
	CUtilizeSDCData(CMDI* pNM) : pMDI(pNM) {}
#else
	CUtilizeSDCData() {}
#endif
	virtual ~CUtilizeSDCData() {}

protected:
	CSDCReceive SDCReceive;

#ifdef USE_QT_GUI
	CMDI*		pMDI;
#endif

	_BOOLEAN	bFirstBlock;

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};


/******************************************************************************\
* Data type conversion classes needed for simulation						   *
\******************************************************************************/
/* Conversion from channel output to resample module input */
class CDataConvChanResam : public CReceiverModul<CChanSimDataMod, _REAL>
{
protected:
	virtual void InitInternal(CParameter& ReceiverParam)
	{
		iInputBlockSize = ReceiverParam.iSymbolBlockSize;
		iOutputBlockSize = ReceiverParam.iSymbolBlockSize;
	}
	virtual void ProcessDataInternal(CParameter& ReceiverParam)
	{
		for (int i = 0; i < iOutputBlockSize; i++)
			(*pvecOutputData)[i] = (*pvecInputData)[i].tOut;
	}
};


#endif // !defined(DATA_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
