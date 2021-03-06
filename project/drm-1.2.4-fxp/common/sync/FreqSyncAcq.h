/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See FreqSyncAcq.cpp
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

#if !defined(FREQSYNC_H__3B0BA660EDOINBEROUEBGF4344_BB2B_23E7912__INCLUDED_)
#define FREQSYNC_H__3B0BA660EDOINBEROUEBGF4344_BB2B_23E7912__INCLUDED_

#include "../Parameter.h"
#include "../util/Modul.h"
#include "../matlib/Matlib.h"
#include "../util/Utilities.h"

#ifdef HAVE_DRFFTW_H
# include <drfftw.h>
#else
# include "../rfftw.h"
#endif


/* Definitions ****************************************************************/
/* Bound for peak detection between filtered signal (in frequency direction)
   and the unfiltered signal. Define different bounds for different relative
   search window sizes */
#define PEAK_BOUND_FILT2SIGNAL_1_fxp		((CFReal) 9)
#define PEAK_BOUND_FILT2SIGNAL_0_042_fxp	((CFReal) 7)

/* This value MUST BE AT LEAST 2, because otherwise we would get an overrun
   when we try to add a complete symbol to the buffer! */
#ifdef USE_FRQOFFS_TRACK_GUARDCORR
# define NUM_BLOCKS_4_FREQ_ACQU			30 /* Accuracy must be higher */
#else
# define NUM_BLOCKS_4_FREQ_ACQU			6
#endif

/* Number of block used for averaging */
#define NUM_BLOCKS_USED_FOR_AV			3

/* Lambda for IIR filter for estimating noise floor in frequency domain */
#define LAMBDA_FREQ_IIR_FILT_fxp		((CFReal) 0.87)

/* Ratio between highest and second highest peak at the frequency pilot
   positions in the PSD estimation (after peak detection) */
#define MAX_RAT_PEAKS_AT_PIL_POS_HIGH_fxp	((FXP) 0.99)

/* Ratio between highest and lowest peak at frequency pilot positions (see
   above) */
#define MAX_RAT_PEAKS_AT_PIL_POS_LOW_fxp	((FXP) 0.8)

/* Number of blocks storing the squared magnitude of FFT used for
   averaging */
#define NUM_FFT_RES_AV_BLOCKS			(NUM_BLOCKS_4_FREQ_ACQU * (NUM_BLOCKS_USED_FOR_AV - 1) + 1)


/* Classes ********************************************************************/
class CFreqSyncAcq : public CReceiverModul<_REAL, _COMPLEX>
{
public:
	CFreqSyncAcq() : bSyncInput(FALSE), bAquisition(FALSE),
		rWinSize((_REAL) SOUNDCRD_SAMPLE_RATE / 2),
		veciTableFreqPilots(3), /* 3 freqency pilots */
		rCenterFreq((_REAL) SOUNDCRD_SAMPLE_RATE / 4), bUseRecFilter(FALSE) {}
	virtual ~CFreqSyncAcq() {}

	void SetSearchWindow(_REAL rNewCenterFreq, _REAL rNewWinSize);

	void StartAcquisition();
	void StopAcquisition() {bAquisition = FALSE;}
	_BOOLEAN GetAcquisition() {return bAquisition;}

	void SetRecFilter(const _BOOLEAN bNewF) {bUseRecFilter = bNewF;}
	_BOOLEAN GetRecFilter() {return bUseRecFilter;}

	/* To set the module up for synchronized DRM input data stream */
	void SetSyncInput(_BOOLEAN bNewS) {bSyncInput = bNewS;}

protected:
	CVector<FXP>			vecInputData_fxp;
	CVector<int>			veciTableFreqPilots;
	CShiftRegister<FXP>		vecrFFTHistory_fxp;

	CFftPlans			FftPlan;
	CFRealVector			vecrFFTInput_fxp;
	CFRealVector			vecrSqMagFFTOut_fxp;
	CRealVector			vecrHammingWin;
	CFRealVector			vecrHammingWin_fxp;
	CMovingAv<CFRealVector>		vvrPSDMovAv_fxp;

	int				iFrAcFFTSize;
	int				iHistBufSize;

	int				iFFTSize;

	_BOOLEAN			bAquisition;

	int				iAquisitionCounter;
	int				iAverageCounter;

	_BOOLEAN			bSyncInput;

	_REAL				rCenterFreq;
	_REAL				rWinSize;
	int				iStartDCSearch;
	int				iEndDCSearch;
	FXP				rPeakBoundFiltToSig_fxp;

	CFRealVector			vecrPSDPilCor_fxp;
	int				iHalfBuffer;
	int				iSearchWinSize;
	CFRealVector			vecrFiltResLR_fxp;
	CFRealVector			vecrFiltResRL_fxp;
	CFRealVector			vecrFiltRes_fxp;
	CVector<int>			veciPeakIndex_fxp;

	_COMPLEX			cCurExp;
	_REAL				rInternIFNorm;

	CDRMBandpassFilt		BPFilter;
	_BOOLEAN			bUseRecFilter;

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};


#endif // !defined(FREQSYNC_H__3B0BA660EDOINBEROUEBGF4344_BB2B_23E7912__INCLUDED_)
