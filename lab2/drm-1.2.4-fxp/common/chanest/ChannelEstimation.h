/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See ChannelEstimation.cpp
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

#if !defined(CHANEST_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define CHANEST_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

#include "../Parameter.h"
#include "../util/Modul.h"
#include "../ofdmcellmapping/OFDMCellMapping.h"
#include "../ofdmcellmapping/CellMappingTable.h"
#include "../tables/TableQAMMapping.h"
#include "../matlib/Matlib.h"
#include "TimeLinear.h"
#include "TimeWiener.h"

#ifdef HAVE_DFFTW_H
# include <dfftw.h>
#else
# include <fftw.h>
#endif

#include "../sync/TimeSyncTrack.h"


/* Definitions ****************************************************************/
#define LEN_WIENER_FILT_FREQ_RMA		6
#define LEN_WIENER_FILT_FREQ_RMB		11
#define LEN_WIENER_FILT_FREQ_RMC		11
#define LEN_WIENER_FILT_FREQ_RMD		13

/* Time constant for IIR averaging of fast signal power estimation */
#define TICONST_SNREST_FAST				((CReal) 30.0) /* sec */

/* Time constant for IIR averaging of slow signal power estimation */
#define TICONST_SNREST_SLOW				((CReal) 100.0) /* sec */

/* Time constant for IIR averaging of MSC signal / noise power estimation */
#define TICONST_SNREST_MSC				((CReal) 1.0) /* sec */

/* Initial value for SNR */
#define INIT_VALUE_SNR_WIEN_FREQ_DB		((_REAL) 30.0) /* dB */

/* SNR estimation initial SNR value */
#define INIT_VALUE_SNR_ESTIM_DB			((_REAL) 20.0) /* dB */

/* Wrap around bound for calculation of group delay. It is wraped by the 2 pi
   periodicity of the angle() function */
#define WRAP_AROUND_BOUND_GRP_DLY		((_REAL) 4.0)

/* Set length of history for delay values for minimum search. Since the
   delay estimation is optimized for channel estimation performance and
   therefore the delay is usually estimated as too long, it is better for the
   log file to use the minimum value in a certain time period for a good
   estimate of the true delay */
#define LEN_HIST_DELAY_LOG_FILE_S		((CReal) 1.0) /* sec */
#define FXP_ONE_CE                                 ((FXP) (1<<16))

/* Classes ********************************************************************/
class CChannelEstimation : public CReceiverModul<_COMPLEX, CEquSig>
{
public:
	CChannelEstimation() : iLenHistBuff(0), TypeIntFreq(FWIENER), 
		TypeIntTime(TWIENER), eDFTWindowingMethod(DFT_WIN_HAMM),
		TypeSNREst(SNR_FAC), bInterfConsid(FALSE) {}
	virtual ~CChannelEstimation() {}

	enum ETypeIntFreq {FLINEAR, FDFTFILTER, FWIENER};
	enum ETypeIntTime {TLINEAR, TWIENER};
	enum ETypeSNREst {SNR_FAC, SNR_PIL};

	void GetTransferFunction(CVector<_REAL>& vecrData,
		CVector<_REAL>& vecrGrpDly,	CVector<_REAL>& vecrScale);
	void GetAvPoDeSp(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale, 
					 _REAL& rLowerBound, _REAL& rHigherBound,
					 _REAL& rStartGuard, _REAL& rEndGuard, _REAL& rPDSBegin,
					 _REAL& rPDSEnd);
	void GetSNRProfile(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale);

	CTimeLinear* GetTimeLinear() {return &TimeLinear;}
	CTimeWiener* GetTimeWiener() {return &TimeWiener;}
	CTimeSyncTrack* GetTimeSyncTrack() {return &TimeSyncTrack;}

	/* Set (get) frequency and time interpolation algorithm */
	void SetFreqInt(ETypeIntFreq eNewTy) {TypeIntFreq = eNewTy;}
	ETypeIntFreq GetFreqInt() {return TypeIntFreq;}
	void SetTimeInt(ETypeIntTime eNewTy) {TypeIntTime = eNewTy;
		SetInitFlag();}
	ETypeIntTime GetTimeInt() const {return TypeIntTime;}

	void SetIntCons(const _BOOLEAN bNewIntCons) {bInterfConsid = bNewIntCons;}
	_BOOLEAN GetIntCons() {return bInterfConsid;}


	/* Which SNR estimation algorithm */
	void SetSNREst(ETypeSNREst eNewTy) {TypeSNREst = eNewTy; SetInitFlag();}
	ETypeSNREst GetSNREst() {return TypeSNREst;}

	_BOOLEAN GetSNREstdB(_REAL& rSNREstRes) const;
	
	
	_BOOLEAN FXP_GetSNREstdB(FXP& FXP_rSNREstRes) const; /*fxp version*/
	
	_REAL GetMSCMEREstdB(); /* MER on MSC cells */
	_REAL GetMSCWMEREstdB(); /* Weighted MER on MSC cells */
	_BOOLEAN GetSigma(_REAL& rSigma);
	_REAL GetDelay() const;
	_REAL GetMinDelay();

	void StartSaRaOffAcq() {TimeSyncTrack.StartSaRaOffAcq(); SetInitFlag();}

protected:
	enum EDFTWinType {DFT_WIN_RECT, DFT_WIN_HAMM};
	EDFTWinType			eDFTWindowingMethod;

	int				iNumSymPerFrame;

	CChanEstTime*			pTimeInt;

	CTimeLinear			TimeLinear;
	CTimeWiener			TimeWiener;

	CTimeSyncTrack			TimeSyncTrack;

	ETypeIntFreq			TypeIntFreq;
	ETypeIntTime			TypeIntTime;
	ETypeSNREst			TypeSNREst;

	int				iNumCarrier;

	CMatrix<_FCOMPLEX>		FXP_matcHistory;

	int				iLenHistBuff;

	int				iScatPilFreqInt; /* Frequency interpolation */
	int				iScatPilTimeInt; /* Time interpolation */

	CFComplexVector			FXP_veccChanEst;
	CFRealVector			FXP_vecrSqMagChanEst;

	int				iFFTSizeN;

	CReal				rGuardSizeFFT;

	CRealVector			vecrDFTWindow;
	CRealVector			vecrDFTwindowInv;

	int				iLongLenFreq;
	CComplexVector			veccPilots;
	CComplexVector			veccIntPil;
	CFftPlans			FftPlanShort;
	CFftPlans			FftPlanLong;

	int				iNumIntpFreqPil;

	FXP				FXP_rLamSNREstFast;
	FXP				FXP_rLamSNREstSlow;
	FXP				FXP_rLamMSCSNREst;

	FXP				FXP_rNoiseEst; /*FXP version*/
	FXP				FXP_rNoiseEstMSCMER;
	long long			FXP_rSignalEst; /*FXP version*/
	CVector<FXP>			FXP_vecrNoiseEstMSC;
	CVector<FXP>			FXP_vecrSigEstMSC;
	FXP				FXP_rSNREstimate; /*FXP version*/
	FXP				FXP_rSNRChanEstCorrFact;
	FXP				FXP_rSNRFACSigCorrFact;
	FXP				FXP_rSNRTotToPilCorrFact;
	FXP				FXP_rSNRSysToNomBWCorrFact;  /*FXP version*/

	_BOOLEAN			bInterfConsid;


	/* Needed for GetDelay() */
	FXP				FXP_rLenPDSEst;
	CShiftRegister<FXP>		FXP_vecrDelayHist;
	int				iLenDelayHist;

	int				iStartZeroPadding;

	int				iInitCnt;
	int				iSNREstIniSigAvCnt;
	int				iSNREstIniNoiseAvCnt;
	int				iSNREstInitCnt;
	_BOOLEAN			bSNRInitPhase;

	_REAL CalAndBoundSNR(const _REAL rSignalEst, const _REAL rNoiseEst);	
	FXP   CalAndBoundSNR(FXP rSignalEst, FXP rNoiseEst);	
	long long FXP_CalAndBoundSNR(const long long FXP_rSignalEst,const long long FXP_rNoiseEst); /*FXP version*/
	
	
	/* Wiener interpolation in frequency direction */
	void UpdateWienerFiltCoef(FXP FXP_rNewSNR, FXP FXP_rRatPDSLen, FXP FXP_rRatPDSOffs); /*FXP version*/

	CFComplexVector FreqOptimalFilter(int iFreqInt, int iDiff, FXP FXP_rSNR, 
					 FXP FXP_rRatPDSLen, FXP FXP_rRatPDSOffs,
					 int iLength);
	CMatrix<_FCOMPLEX>		FXP_matcFiltFreq;	/*FXP version*/
	int				iLengthWiener;
	CVector<int>			veciPilOffTab;

	int				iDCPos;
	int				iPilOffset;
	int				iNumWienerFilt;
	FComplexMatrix			FXP_matcWienerFilter;  /*FXP version*/ 

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};


#endif // !defined(CHANEST_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
