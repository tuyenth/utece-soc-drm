/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Channel estimation and equalization
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

#include "ChannelEstimation.h"

#define FXP_SIGNAL_EST_SCALE 28


/* Implementation *************************************************************/
void CChannelEstimation::ProcessDataInternal(CParameter& ReceiverParam)
{
  
  /* cases: only calling FWIENER and SNR_FAC
     
  fxn returns void but is passed a CParameter&
  _REAL = rCurSNREst, rOffsPDSEst, rSNRAftTiInt, rSNREstimate, rSNRTotToPilCorrFact, rLenPDSEst, rSignalEst, rNoiseEst, rSNRFACSigCorrFact, rLamMSCSNREst, rCurErrPow,rLamSNREstFast
  CMatrix<_COMPLEX> = matcHistory[i][j], matcFiltFreq[i][j] both are called here only mathfiltfreq unchanged here
  CComplexVector = veccPilots[i], veccChanEst[i]
  CParameter& = ReceiverParam ?? how do i deal with this ??
  CRealVector = vecrSqMagChanEst[i] (DOESNT! affect wave)
  Cvector<REAL> = vecrSigEstMSC, vecrNoiseEstMSC (doesnt work but doesnt affect wave i guess)
  CShiftRegister<CReal> = vecrDelayHist
  CVectorEx<_COMPLEX>* = pvecInputData
  
  not done:  pvecInputData, ReceiverParam
  issues: vecrSigEstMSC, vecrNoiseEstMSC, vecrSqMagChanEst
  
  
  issues
  1. FXP_rSignalEst & FXP_rSignalEst are overflowing in some cases where numbers are huge, real casting works not sure about above two, no errors in above wave file
  2. vecrSigEstMSC, vecrNoiseEstMSC, vecrSqMagChanEst are initializing at some unknown place and when buffering it zeroes out BUT DOESNT! affect wave file! has
  ben left out
  */
  
  int		i, j, k;
  int		iModSymNum;
  
  /* Check if symbol ID index has changed by the synchronization unit. If it
     has changed, reinit this module */
  
  int w, e;
  FXP FXP_rOffsPDSEst;
  FXP FXP_rSNRAftTiInt;
    
  if ((*pvecInputData).GetExData().bSymbolIDHasChanged == TRUE) //no change here since the fxp version doesnt use this crazy inherited data type
    {
      // FIXME: we loose one OFDM symbol by this call -> slower DRM signal acquisition
      SetInitFlag();
      return;
    }
  
  /* Move data in history-buffer (from iLenHistBuff - 1 towards 0) */
  for (j = 0; j < iLenHistBuff - 1; j++)
    {
      for (i = 0; i < iNumCarrier; i++){
	FXP_matcHistory[j][i] = FXP_matcHistory[j + 1][i];
      }
      
    }
  
  /* Write new symbol in memory */
  for (i = 0; i < iNumCarrier; i++){
    FXP_matcHistory[iLenHistBuff - 1][i] = _FCOMPLEX((*pvecInputData)[i]); 
  }
  
  
  /* Time interpolation *****************************************************/
  /* Get symbol-counter for next symbol. Use the count from the frame 
     synchronization (in OFDM.cpp). Call estimation routine */
  
  FXP_rSNRAftTiInt = (FXP) pTimeInt->Estimate(pvecInputData, veccPilots, 
					   ReceiverParam.matiMapTab[(*pvecInputData).
								    GetExData().iSymbolID],
					   ReceiverParam.matcPilotCells[(*pvecInputData).
									GetExData().iSymbolID],
					   /* The channel estimation is based on the pilots so
					      it needs the SNR on the pilots. Do a correction */
					   FXP_rSNREstimate * FXP_rSNRTotToPilCorrFact);
  
  /* Debar initialization of channel estimation in time direction */
  if (iInitCnt > 0) //nothing needed here
    {
      iInitCnt--;
      
      /* Do not put out data in initialization phase */
      iOutputBlockSize = 0;
      
      /* Do not continue */
      return;
    }
  else
    iOutputBlockSize = iNumCarrier; 
  
  /* Define DC carrier for robustness mode D because there is no pilot */
  if (iDCPos != 0) {
    veccPilots[iDCPos] = (CReal) 0.0; //needs change    
  }
  
  
  /* -------------------------------------------------------------------------
     Use time-interpolated channel estimate for timing synchronization 
     tracking */
  
  TimeSyncTrack.Process(ReceiverParam, veccPilots, 
			(*pvecInputData).GetExData().iCurTimeCorr, FXP_rLenPDSEst /* out */,
			FXP_rOffsPDSEst /* out */);
  
  /* Store current delay in history */
  FXP_vecrDelayHist.AddEnd(FXP_rLenPDSEst);
  
  /* Frequency-interploation ************************************************/
  
  switch (TypeIntFreq)
    {
    case FLINEAR: /*this isn't called*/
      /********************************************************************** \
       * Linear interpolation												  
		\**********************************************************************/
      /* Set first pilot position */
      FXP_veccChanEst[0] = veccPilots[0];
      
      for (j = 0, k = 1; j < iNumCarrier - iScatPilFreqInt;
	   j += iScatPilFreqInt, k++)
	{
	  /* Set values at second time pilot position in cluster */
	  FXP_veccChanEst[j + iScatPilFreqInt] = veccPilots[k];
	  
	  /* Interpolation cluster */
	  for (i = 1; i < iScatPilFreqInt; i++)
	    {
	      /* E.g.: c(x) = (c_4 - c_0) / 4 * x + c_0 */
	      FXP_veccChanEst[j + i] =
		(FXP_veccChanEst[j + iScatPilFreqInt] - FXP_veccChanEst[j]) /
		(FXP) (iScatPilFreqInt) * (FXP) i + FXP_veccChanEst[j];
	    }
	}
      break;
      
    case FDFTFILTER:  /*this isn't called*/
      /********************************************************************** \
       * DFT based algorithm												  *
		\**********************************************************************/
      /* ---------------------------------------------------------------------
	 Put all pilots at the beginning of the vector. The "real" length of
	 the vector "pcFFTWInput" is longer than the No of pilots, but we 
	 calculate the FFT only over "iNumCarrier / iScatPilFreqInt + 1"
	 values (this is the number of pilot positions) */
      /* Weighting pilots with window */
      veccPilots *= vecrDFTWindow;
      
      /* Transform in time-domain */
      veccPilots = Ifft(veccPilots, FftPlanShort);
      
      /* Set values outside a defined bound to zero, zero padding (noise
	 filtering). Copy second half of spectrum at the end of the new vector
	 length and zero out samples between the two parts of the spectrum */
      veccIntPil.Merge(
		       /* First part of spectrum */
		       veccPilots(1, iStartZeroPadding), 
		       /* Zero padding in the middle, length: Total length minus length of
			  the two parts at the beginning and end */
		       CComplexVector(Zeros(iLongLenFreq - 2 * iStartZeroPadding), 
				      Zeros(iLongLenFreq - 2 * iStartZeroPadding)), 
		       /* Set the second part of the actual spectrum at the end of the new
			  vector */
		       veccPilots(iNumIntpFreqPil - iStartZeroPadding + 1, 
				  iNumIntpFreqPil));
      
      /* Transform back in frequency-domain */
      veccIntPil = Fft(veccIntPil, FftPlanLong);
      
      /* Remove weighting with DFT window by inverse multiplication */
      FXP_veccChanEst = (veccIntPil(1, iNumCarrier) * vecrDFTwindowInv);
      break;
      
    case FWIENER:
      /********************************************************************** \
       * Wiener filter													   *
		\**********************************************************************/
      /* Wiener filter update --------------------------------------------- */
      /* Do not update filter in case of simulation */
      if (ReceiverParam.eSimType == CParameter::ST_NONE)
	{
	  /* Update Wiener filter each OFDM symbol. Use current estimates */	  
	  UpdateWienerFiltCoef(FXP_rSNRAftTiInt, FXP_rLenPDSEst / iNumCarrier, FXP_rOffsPDSEst / iNumCarrier); //matcFiltFreq changes here!!!
	  
      
      /* Actual Wiener interpolation (FIR filtering) ---------------------- */
      /* FIR filter of the pilots with filter taps. We need to filter the
	 pilot positions as well to improve the SNR estimation (which 
	 follows this procedure) */
      for (j = 0; j < iNumCarrier; j++)
	{
	  // TODO: Do only calculate channel estimation for data cells, not for pilot
	  // cells (exeption: if we want to use SNR estimation based on pilots, we also
	  // need Wiener on these cells!)
	  /* Convolution */
	  FXP_veccChanEst[j] = _FCOMPLEX((FXP) 0.0, (FXP) 0.0); 
	  
	  for (i = 0; i < iLengthWiener; i++)
	    {
	      
	      FComplex veccPil = _FCOMPLEX(veccPilots[veciPilOffTab[j] + i].real(), veccPilots[veciPilOffTab[j] + i].imag());
	      FXP_veccChanEst[j] +=  FXP_matcFiltFreq[j][i] * veccPil; // needs change	      
	      
	    }
	}
      }
      break;
    }
  
  
  /* Equalize the output vector ------------------------------------------- */
  /* Calculate squared magnitude of channel estimation */
  
  FXP_vecrSqMagChanEst = SqMag(FXP_veccChanEst);	//needs change
  
  /* Write to output vector. Take oldest symbol of history for output. Also,
     ship the channel state at a certain cell */
  for (i = 0; i < iNumCarrier; i++)
    {
      (*pvecOutputData)[i].cSig = FXP_matcHistory[0][i] / FXP_veccChanEst[i];	
      
#ifdef USE_MAX_LOG_MAP
      /*this is not enabled*/
      /* In case of MAP we need the squared magnitude for the calculation of
	 the metric */
      (*pvecOutputData)[i].rChan = FXP_vecrSqMagChanEst[i];	
      
#else
      /* In case of hard-desicions, we need the magnitude of the channel for
	 the calculation of the metric */
      (*pvecOutputData)[i].rChan = sqrt(FXP_vecrSqMagChanEst[i]);     
#endif
    }
  
  
  /* -------------------------------------------------------------------------
     Calculate symbol ID of the current output block and set parameter */
  (*pvecOutputData).GetExData().iSymbolID = (*pvecInputData).GetExData().iSymbolID - iLenHistBuff + 1;	//no change needed right now due to crazy data	type
  
  
  /* SNR estimation ------------------------------------------------------- */
  /* Modified symbol ID, check range {0, ..., iNumSymPerFrame} */
  iModSymNum = (*pvecOutputData).GetExData().iSymbolID;		//needs change
  
  while (iModSymNum < 0)
    iModSymNum += iNumSymPerFrame;	//no change
  
  /* Two different types of SNR estimation are available */
  switch (TypeSNREst)
    {
    case SNR_PIL: /*this isn't called*/
      /* Use estimated channel and compare it to the received pilots. This
	 estimation works only if the channel estimation was successful */
      for (i = 0; i < iNumCarrier; i++)
	{
	  /* Identify pilot positions. Use MODIFIED "iSymbolID" (See lines
	     above) */
	  if (_IsScatPil(ReceiverParam.matiMapTab[iModSymNum][i]))
	    {
	      /* We assume that the channel estimation in "veccChanEst" is
		 noise free (e.g., the wiener interpolation does noise
		 reduction). Thus, we have an estimate of the received signal
		 power \hat{r} = s * \hat{h}_{wiener} */

	      FComplex param = _FCOMPLEX(ReceiverParam.matcPilotCells[iModSymNum][i].real(), 
                                         ReceiverParam.matcPilotCells[iModSymNum][i].imag());
	      const _FCOMPLEX cModChanEst = FXP_veccChanEst[i] * param;
	      
	      _REAL rSNREstimate = FXP_rSNREstimate;

	      _REAL rSignalEst = ((double)FXP_rSignalEst)/(1<<FXP_SIGNAL_EST_SCALE);
	      _REAL rNoiseEst = FXP_rNoiseEst;

	      /* Calculate and average noise and signal estimates --------- */
	      /* The noise estimation is difference between the noise reduced
		 signal and the noisy received signal
		 \tilde{n} = \hat{r} - r */
	      
	      IIR1(rNoiseEst, (_REAL)SqMag(FXP_matcHistory[0][i] - cModChanEst),
		   (_REAL)FXP_rLamSNREstFast);
	      
	      /* The received signal power estimation is just \hat{r} */
	      IIR1(rSignalEst, (_REAL)SqMag(cModChanEst), (_REAL)FXP_rLamSNREstFast);
	      
	      /* Calculate final result (signal to noise ratio) */
	      _REAL rCurSNREst = CalAndBoundSNR(rSignalEst, rNoiseEst);
	      
	      /* Average the SNR with a two sided recursion. Apply correction
		 factor, too */
	      IIR1TwoSided(rSNREstimate, rCurSNREst / (_REAL)FXP_rSNRTotToPilCorrFact,
			   (_REAL)FXP_rLamSNREstFast,	(_REAL)FXP_rLamSNREstSlow);

              FXP_rSNREstimate = rSNREstimate;
	    }
	}
      break;
      
    case SNR_FAC:
      /* SNR estimation based on FAC cells and hard decisions */
      /* SNR estimation with initialization */
      if (iSNREstInitCnt > 0)
	{
	  for (i = 0; i < iNumCarrier; i++)
	    {
	      /* Only use the last frame of the initialization phase for
		 initial SNR estimation to debar initialization phase of
		 synchronization and channel estimation units */
	      if (iSNREstInitCnt < iNumSymPerFrame * iNumCarrier)
		{
		  const int iCurCellFlag =
		    ReceiverParam.matiMapTab[iModSymNum][i];	//recieved param?
		  
		  /* Initial signal estimate. Use channel estimation from all
		     data and pilot cells. Apply averaging */
		  if ((_IsData(iCurCellFlag)) || (_IsPilot(iCurCellFlag)))
		    {
		      /* Signal estimation */
		      FXP_rSignalEst += (long long)((int)FXP_vecrSqMagChanEst[i])*(1<<FXP_SIGNAL_EST_SCALE);
		      iSNREstIniSigAvCnt++;
		    }
		  
		  /* Noise estimation from all data cells from tentative
		     decisions */
		  if (_IsFAC(iCurCellFlag)) /* FAC cell */
		    {
		      
		      FXP_rNoiseEst += FXP_vecrSqMagChanEst[i] * (FXP)SqMag(MinDist4QAM((*pvecOutputData)[i].cSig));
		      
		      iSNREstIniNoiseAvCnt++;
		    }
		}
	    }
	  
	  iSNREstInitCnt--;
	}
      else
	{
	  /* Only right after initialization phase apply initial SNR
	     value */
	  if (bSNRInitPhase == TRUE)
	    {
	      /* Normalize average */
	      FXP_rSignalEst /= (long long)(iSNREstIniSigAvCnt*(1<<FXP_SIGNAL_EST_SCALE));
	      FXP_rNoiseEst /= (FXP) iSNREstIniNoiseAvCnt;
	      
	      bSNRInitPhase = FALSE;
	      
	    }
	  
	  for (i = 0; i < iNumCarrier; i++)
	    {
	      /* Only use FAC cells for this SNR estimation method */
	      if (_IsFAC(ReceiverParam.matiMapTab[iModSymNum][i]))
		{
		  /* Get tentative decision for this FAC QAM symbol. FAC is
		     always 4-QAM. Calculate all distances to the four
		     possible constellation points of a 4-QAM and use the
		     squared result of the returned distance vector */
		  FXP FXP_rCurErrPow;
		  FXP_rCurErrPow =  (FXP) SqMag(MinDist4QAM((*pvecOutputData)[i].cSig));					
		  
		  /* Use decision together with channel estimate to get
		     estimates for signal and noise */
		  FXP rSignalEst(FXP_rSignalEst, FXP_SIGNAL_EST_SCALE);

		  IIR1(FXP_rNoiseEst, FXP_rCurErrPow * FXP_vecrSqMagChanEst[i],
		       FXP_rLamSNREstFast);
		  
		  IIR1(rSignalEst, FXP_vecrSqMagChanEst[i],
		       FXP_rLamSNREstFast);
		  
		  FXP_rSignalEst = (long long)rSignalEst.GetValue()<<(FXP_SIGNAL_EST_SCALE-FXP_PREC);
		}
	    }
	  
	  /* Calculate final result (signal to noise ratio) */
	  long long FXP_rCurSNREst = FXP_CalAndBoundSNR(FXP_rSignalEst, 
							((long long)(FXP_rNoiseEst.GetValue()))<<(FXP_SIGNAL_EST_SCALE-FXP_PREC));

	  /* Consider correction factor for average signal energy */
	  FXP_rSNREstimate = FXP(FXP_rCurSNREst * (long long)(FXP_rSNRFACSigCorrFact.GetValue()), FXP_SIGNAL_EST_SCALE+FXP_PREC);
	}
      break;
    }
  
  
  /* WMER on MSC cells estimation ----------------------------------------- */
  for (i = 0; i < iNumCarrier; i++)
    {
      /* Use MSC cells for this SNR estimation method */
      if (_IsMSC(ReceiverParam.matiMapTab[iModSymNum][i]))
	{
	  FXP FXP_rCurErrPow;	  
	  
	  /* Get tentative decision for this MSC QAM symbol and calculate
	     squared distance as a measure for the noise. MSC can be 16 or
	     64 QAM */
	  switch (ReceiverParam.eMSCCodingScheme)
	    {
	    case CParameter::CS_3_SM:
	    case CParameter::CS_3_HMSYM:
	    case CParameter::CS_3_HMMIX:
	      FXP_rCurErrPow = (FXP) SqMag(MinDist64QAM((*pvecOutputData)[i].cSig));
	      break;
	      
	    case CParameter::CS_2_SM:
	      FXP_rCurErrPow = SqMag(MinDist16QAM((*pvecOutputData)[i].cSig));
	      break;
	    }
	  
	  /* Use decision together with channel estimate to get
	     estimates for signal and noise (for each carrier) */
	  
	  IIR1(FXP_vecrNoiseEstMSC[i], FXP_rCurErrPow * FXP_vecrSqMagChanEst[i],
	       FXP_rLamMSCSNREst);
	  
	  IIR1(FXP_vecrSigEstMSC[i], (_REAL)FXP_vecrSqMagChanEst[i],
	       FXP_rLamMSCSNREst);

	  /* Calculate MER on MSC cells */
	  IIR1(FXP_rNoiseEstMSCMER, FXP_rCurErrPow, FXP_rLamSNREstFast);
	  
	}
    }
  
  
  /* Interferer consideration --------------------------------------------- */
  if (bInterfConsid == TRUE)
    {
      for (i = 0; i < iNumCarrier; i++)
	{
	  /* Weight the channel estimates with the SNR estimate of the current
	     carrier to consider the higher noise variance caused by
	     interferers */
	  
	  (*pvecOutputData)[i].rChan *= (_REAL)CalAndBoundSNR(FXP_vecrSigEstMSC[i], FXP_vecrNoiseEstMSC[i]); 
	}
    }
  
}

void CChannelEstimation::InitInternal(CParameter& ReceiverParam)
{
  /* Get parameters from global struct */
  iScatPilTimeInt = ReceiverParam.iScatPilTimeInt;
  iScatPilFreqInt = ReceiverParam.iScatPilFreqInt;
  iNumIntpFreqPil = ReceiverParam.iNumIntpFreqPil;
  iNumCarrier = ReceiverParam.iNumCarrier;
  iFFTSizeN = ReceiverParam.iFFTSizeN;
  iNumSymPerFrame = ReceiverParam.iNumSymPerFrame;
  
  /* Length of guard-interval with respect to FFT-size! */
  /* operation in FXP from integer */
  FXP fGuardSizeFFT =  iNumCarrier *
    ReceiverParam.RatioTgTu.iEnum;
  fGuardSizeFFT = fGuardSizeFFT / ReceiverParam.RatioTgTu.iDenom;
  rGuardSizeFFT = fGuardSizeFFT.GetFValue();
  
  //rGuardSizeFFT = (_REAL) iNumCarrier *
  //ReceiverParam.RatioTgTu.iEnum / ReceiverParam.RatioTgTu.iDenom;
  
  /* If robustness mode D is active, get DC position. This position cannot
     be "0" since in mode D no 5 kHz mode is defined (see DRM-standard). 
     Therefore we can also use this variable to get information whether
     mode D is active or not (by simply write: "if (iDCPos != 0)") */
  if (ReceiverParam.GetWaveMode() == RM_ROBUSTNESS_MODE_D)
    {
      /* Identify DC carrier position */
      for (int i = 0; i < iNumCarrier; i++)
	{
	  if (_IsDC(ReceiverParam.matiMapTab[0][i]))
	    iDCPos = i;
	}
    }
  else
    iDCPos = 0;
  
  /* FFT must be longer than "iNumCarrier" because of zero padding effect (
     not in robustness mode D! -> "iLongLenFreq = iNumCarrier") */
  iLongLenFreq = iNumCarrier + iScatPilFreqInt - 1;
  
  /* Init vector for received data at pilot positions */
  veccPilots.Init(iNumIntpFreqPil);
  
  /* Init vector for interpolated pilots */
  veccIntPil.Init(iLongLenFreq);
  
  /* Init plans for FFT (faster processing of Fft and Ifft commands) */
  FftPlanShort.Init(iNumIntpFreqPil);
  FftPlanLong.Init(iLongLenFreq);
  
  /* Choose time interpolation method and set pointer to correcponding 
     object */
  switch (TypeIntTime)
    {
    case TLINEAR:
      pTimeInt = &TimeLinear;
      break;
      
    case TWIENER:
      pTimeInt = &TimeWiener;
      break;
    }
  
  /* Init time interpolation interface and set delay for interpolation */
  iLenHistBuff = pTimeInt->Init(ReceiverParam);
  
  /* Init time synchronization tracking unit */
  TimeSyncTrack.Init(ReceiverParam, iLenHistBuff);
  
  /* Set channel estimation delay in global struct. This is needed for 
     simulation */
  ReceiverParam.iChanEstDelay = iLenHistBuff;
  
  
  /* Init window for DFT operation for frequency interpolation ------------ */
  /* Init memory */
  vecrDFTWindow.Init(iNumIntpFreqPil);
  vecrDFTwindowInv.Init(iNumCarrier);
  
  /* Set window coefficients */
  switch (eDFTWindowingMethod)
    {
    case DFT_WIN_RECT:
      vecrDFTWindow = Ones(iNumIntpFreqPil);
      vecrDFTwindowInv = Ones(iNumCarrier);
      break;
      
    case DFT_WIN_HAMM:
      vecrDFTWindow = Hamming(iNumIntpFreqPil);
      vecrDFTwindowInv = (CReal) 1.0 / Hamming(iNumCarrier);
      break;
    }
  
  
  /* Set start index for zero padding in time domain for DFT method */
  iStartZeroPadding = (int) rGuardSizeFFT;
  if (iStartZeroPadding > iNumIntpFreqPil)
    iStartZeroPadding = iNumIntpFreqPil;
  
  /* Allocate memory for channel estimation */
  FXP_veccChanEst.Init(iNumCarrier);
  FXP_vecrSqMagChanEst.Init(iNumCarrier);
  
  /* Allocate memory for history buffer (Matrix) and zero out */
  FXP_matcHistory.Init(iLenHistBuff, iNumCarrier,
		   _FCOMPLEX((_REAL) 0.0, (_REAL) 0.0));
  
  /* After an initialization we do not put out data before the number symbols
     of the channel estimation delay have been processed */
  iInitCnt = iLenHistBuff - 1;
  
  /* SNR correction factor for the different SNR estimation types. For the
     FAC method, the average signal power has to be considered. For the pilot
     based method, only the SNR on the pilots are evaluated. Therefore, to get
     the total SNR, a correction has to be applied */
  FXP_rSNRFACSigCorrFact = ReceiverParam.rAvPowPerSymbol / CReal(iNumCarrier);
  FXP_rSNRTotToPilCorrFact = ReceiverParam.rAvScatPilPow *
    (_REAL) iNumCarrier / ReceiverParam.rAvPowPerSymbol;
  
  /* Correction factor for transforming the estimated system SNR in the SNR
     where the noise bandwidth is according to the nominal DRM bandwidth */
  FXP_rSNRSysToNomBWCorrFact = ReceiverParam.GetSysToNomBWCorrFact();
  
  /* Inits for SNR estimation (noise and signal averages) */
  FXP_rSignalEst = 0;
  FXP_rNoiseEst = 0;
  FXP_rNoiseEstMSCMER = 0;
  FXP_rSNREstimate = (FXP) pow(10, INIT_VALUE_SNR_ESTIM_DB / 10);
  FXP_vecrNoiseEstMSC.Init(iNumCarrier, 0);
  FXP_vecrSigEstMSC.Init(iNumCarrier, 0);
  
  /* For SNR estimation initialization */
  iSNREstIniSigAvCnt = 0;
  iSNREstIniNoiseAvCnt = 0;
  
  /* We only have an initialization phase for SNR estimation method based on
     the tentative decisions of FAC cells */
  if (TypeSNREst == SNR_FAC)
    bSNRInitPhase = TRUE;
  else
    bSNRInitPhase = FALSE;
  
  /* 5 DRM frames to start initial SNR estimation after initialization phase
     of other units */
  iSNREstInitCnt = 5 * iNumSymPerFrame;
  
  /* Lambda for IIR filter */
  FXP_rLamSNREstFast = IIR1Lam(TICONST_SNREST_FAST, (CReal) SOUNDCRD_SAMPLE_RATE /
			   ReceiverParam.iSymbolBlockSize);
  FXP_rLamSNREstSlow = IIR1Lam(TICONST_SNREST_SLOW, (CReal) SOUNDCRD_SAMPLE_RATE /
			   ReceiverParam.iSymbolBlockSize);
  FXP_rLamMSCSNREst = IIR1Lam(TICONST_SNREST_MSC, (CReal) SOUNDCRD_SAMPLE_RATE /
			  ReceiverParam.iSymbolBlockSize);
  
  /* Init delay spread length estimation (index) */
  FXP_rLenPDSEst = (_REAL) 0.0;
  
  /* Init history for delay values */
  /* Duration of OFDM symbol */
  const _REAL rTs = (CReal) (ReceiverParam.iFFTSizeN +
			     ReceiverParam.iGuardSize) / SOUNDCRD_SAMPLE_RATE;
  
  iLenDelayHist = (int) (LEN_HIST_DELAY_LOG_FILE_S / rTs);
  FXP_vecrDelayHist.Init(iLenDelayHist, (CFReal) 0.0);
  
  
  /* Inits for Wiener interpolation in frequency direction ---------------- */
  /* Length of wiener filter */
  switch (ReceiverParam.GetWaveMode())
    {
    case RM_ROBUSTNESS_MODE_A:
      iLengthWiener = LEN_WIENER_FILT_FREQ_RMA;
      break;
      
    case RM_ROBUSTNESS_MODE_B:
      iLengthWiener = LEN_WIENER_FILT_FREQ_RMB;
      break;
      
    case RM_ROBUSTNESS_MODE_C:
      iLengthWiener = LEN_WIENER_FILT_FREQ_RMC;
      break;
      
    case RM_ROBUSTNESS_MODE_D:
      iLengthWiener = LEN_WIENER_FILT_FREQ_RMD;
      break;
    }
  
  
  /* Inits for wiener filter ---------------------------------------------- */
  /* In frequency direction we can use pilots from both sides for 
     interpolation */
  iPilOffset = iLengthWiener / 2;
  
  /* Allocate memory */
  FXP_matcFiltFreq.Init(iNumCarrier, iLengthWiener);
  
  
  /* Pilot offset table */
  veciPilOffTab.Init(iNumCarrier);
  
  /* Number of different wiener filters */
  iNumWienerFilt = (iLengthWiener - 1) * iScatPilFreqInt + 1;
  
  /* Allocate temporary matlib vector for filter coefficients */
  FXP_matcWienerFilter.Init(iNumWienerFilt, iLengthWiener);
  
  /* Distinguish between simulation and regular receiver. When we run a
     simulation, the parameters are taken from simulation init */
  if (ReceiverParam.eSimType == CParameter::ST_NONE)
    {
      /* Initial Wiener filter. Use initial SNR definition and assume that the
	 PDS ranges from the beginning of the guard-intervall to the end */
      UpdateWienerFiltCoef(pow(10, INIT_VALUE_SNR_WIEN_FREQ_DB / 10),
			   (_REAL) ReceiverParam.RatioTgTu.iEnum /
			   ReceiverParam.RatioTgTu.iDenom, (CReal) 0.0);
    }
  else
    {
      /* Get simulation SNR on the pilot positions and set PDS to entire
	 guard-interval length */
      UpdateWienerFiltCoef(pow(10, ReceiverParam.GetSysSNRdBPilPos() / 10),
			   (_REAL) ReceiverParam.RatioTgTu.iEnum /
			   ReceiverParam.RatioTgTu.iDenom, (CReal) 0.0);
    }
  
  
  /* Define block-sizes for input and output */
  iInputBlockSize = iNumCarrier;
  iMaxOutputBlockSize = iNumCarrier; 
}

CFComplexVector CChannelEstimation::FreqOptimalFilter(int iFreqInt, int iDiff, FXP FXP_rSNR, FXP FXP_rRatPDSLen, FXP FXP_rRatPDSOffs, int iLength)
{
  /* 
     We assume that the power delay spread is a rectangle function in the time
     domain (sinc-function in the frequency domain). Length and position of this
     window are adapted according to the current estimated PDS.
  */

	int				i;
	CFRealVector		FXP_vecrRpp(iLength);
	CFRealVector		FXP_vecrRhp(iLength);
	CFRealVector		FXP_vecrH(iLength);
	CFComplexVector		FXP_veccH(iLength);

	/* Calculation of R_hp, this is the SHIFTED correlation function */
	for (i = 0; i < iLength; i++)
	{
		const int iCurPos = i * iFreqInt - iDiff;

		FXP_vecrRhp[i] = (CFReal) Sinc((CReal) iCurPos * (CReal)FXP_rRatPDSLen);
		//FXP_vecrRhp[i] = FXP_Sinc((CReal) iCurPos * (CReal)FXP_rRatPDSLen); // loss in precision in Sinc
	}

	/* Calculation of R_pp */
	for (i = 0; i < iLength; i++)
	{
		const int iCurPos = i * iFreqInt;

		FXP_vecrRpp[i] = (CFReal) Sinc((CReal) iCurPos * (CReal)FXP_rRatPDSLen);
		//FXP_vecrRpp[i] = FXP_Sinc((CReal) iCurPos * (CReal)FXP_rRatPDSLen); // loss in precision in Sinc
	}

	/* Add SNR at first tap */
	FXP_vecrRpp[0] += (FXP) 1.0 / FXP_rSNR;

	/* Call levinson algorithm to solve matrix system for optimal solution */
	FXP_vecrH = Levinson(FXP_vecrRpp, FXP_vecrRhp);

	/* Correct the optimal filter coefficients. Shift the rectangular
	   function in the time domain to the correct position (determined by
	   the "rRatPDSOffs") by multiplying in the frequency domain
	   with exp(j w T) */
	for (i = 0; i < iLength; i++)
	{
		const int iCurPos = i * iFreqInt - iDiff;

		const CFReal rArgExp =
		  iCurPos * (FXP_rRatPDSLen + FXP_rRatPDSOffs * 2);

		FXP_veccH[i] = FXP_vecrH[i] * CFComplex(FXP_Cos(rArgExp), FXP_Sin(rArgExp));
	}

	return FXP_veccH;
}

void CChannelEstimation::UpdateWienerFiltCoef(FXP FXP_rNewSNR, FXP FXP_rRatPDSLen, FXP FXP_rRatPDSOffs)
{
  
  /*fxn returns void passes in const Creal
    CComplexMatrix = matcWienerFilter[i] (changed) but only used in this file. Each time fxn is called the values are created
    Creal = rNewSNR, rRatPDSLen, rRatPDSOffs (all 3 unchanged)
    CMatrix<_COMPLEX> = matcFiltFreq[i][j] (changed) but only used in this file. These values are only changed slightly
    
    int = iScatPilFreqInt
    CVector<int> = veciPilOffTab[i] (changed)
    
    to do:
    1. matcFiltFreq seems to screw up
    
  */
  
  int	j, i;
  int	iDiff;
  int	iCurPil;
  
  /* Calculate all possible wiener filters */
  for (j = 0; j < iNumWienerFilt; j++) {  // Calculate all possible wiener filters
    FXP_matcWienerFilter[j] = FreqOptimalFilter(iScatPilFreqInt, j, FXP_rNewSNR, FXP_rRatPDSLen, FXP_rRatPDSOffs, iLengthWiener);
  }
  
#if 0 /*not enabled*/
  //printf(" the #if 0 is enabled!!!\n");
  /* Save filter coefficients */
  static FILE* pFile = fopen("test/wienerfreq.dat", "w");
  for (j = 0; j < iNumWienerFilt; j++)
    {
      for (i = 0; i < iLengthWiener; i++)
	{
	  fprintf(pFile, "%e %e\n", Real(matcWienerFilter[j][i]),
		  Imag(matcWienerFilter[j][i]));
	}
    }
  
  
  fflush(pFile);
#endif
  
  
  /* Set matrix with filter taps, one filter for each carrier */
  for (j = 0; j < iNumCarrier; j++)
    {
      /* We define the current pilot position as the last pilot which the
	 index "j" has passed */
      iCurPil = (int) (j / iScatPilFreqInt); //no change
      
      /* Consider special cases at the edges of the DRM spectrum */
      if (iCurPil < iPilOffset)
	{
	  /* Special case: left edge */
	  veciPilOffTab[j] = 0;	//no change
	}
      else if (iCurPil - iPilOffset > iNumIntpFreqPil - iLengthWiener)
	{
	  /* Special case: right edge */
	  veciPilOffTab[j] = iNumIntpFreqPil - iLengthWiener; //no change
	}
      else
	{
	  /* In the middle */
	  veciPilOffTab[j] = iCurPil - iPilOffset;	//no change
	}
      
      /* Special case for robustness mode D, since the DC carrier is not used
	 as a pilot and therefore we use the same method for the edges of the
	 spectrum also in the middle of robustness mode D */
      if (iDCPos != 0)
	{
	  if ((iDCPos - iCurPil < iLengthWiener) && (iDCPos - iCurPil > 0))
	    {
	      /* Left side of DC carrier */
	      veciPilOffTab[j] = iDCPos - iLengthWiener;  //no change
	    }
	  
	  if ((iCurPil - iDCPos < iLengthWiener) && (iCurPil - iDCPos > 0))
	    {
	      /* Right side of DC carrier */
	      veciPilOffTab[j] = iDCPos + 1;	//no change
	    }
	}
      
      /* Difference between the position of the first pilot (for filtering)
	 and the position of the observed carrier */
      iDiff = j - veciPilOffTab[j] * iScatPilFreqInt; //no change
      
      /* Copy correct filter in matrix */
      for (i = 0; i < iLengthWiener; i++)
	{
          FXP_matcFiltFreq[j][i] = FXP_matcWienerFilter[iDiff][i];
	}		
      
    }
  
}

/*************** BEGIN FiXED version ***********************/


/*************** END FiXED version ***********************/



/********************************************************************************************/

long long CChannelEstimation::FXP_CalAndBoundSNR(const long long FXP_rSignalEst, const long long FXP_rNoiseEst)
{	
  /* fxn returns _REAL nand passes in const _REAL
     _REAL = rReturn(created), rSignalEst(unchanged), rNoiseEst(unchanged)
     1. check division cuz I had to change it in FXP.h
     ++++++ DONE AND PROVED!!! +++++++++
     bogus value for rReturn doesnt affect wave file
  */	 
  
  long long FXP_rReturn;
  
  /* "rNoiseEst" must not be zero */
  if ( (FXP_rNoiseEst == 0) || (FXP_rSignalEst < FXP_rNoiseEst) ) {
    FXP_rReturn  = (long long)( (1<<FXP_SIGNAL_EST_SCALE) );
  } else {
    // This was causing the numerator to exceed the range without the 2 bit position adjustment
    FXP_rReturn  = (long long)( (FXP_rSignalEst << (FXP_SIGNAL_EST_SCALE-2)) / (FXP_rNoiseEst>>2) );
  }

  /* Bound the SNR at 0 dB */
  if (FXP_rReturn < (long long)((1<<FXP_SIGNAL_EST_SCALE)))
    {FXP_rReturn = (long long)((1<<FXP_SIGNAL_EST_SCALE));}		
  
  return FXP_rReturn;
}

FXP CChannelEstimation::CalAndBoundSNR(FXP FXP_rSignalEst, FXP FXP_rNoiseEst)
{	
  long long scaled_FXP_rSignalEst=((long long)(FXP_rSignalEst.GetValue()))<<(FXP_SIGNAL_EST_SCALE-FXP_PREC);
  long long scaled_FXP_rNoiseEst=((long long)(FXP_rNoiseEst.GetValue()))<<(FXP_SIGNAL_EST_SCALE-FXP_PREC);
  return FXP(FXP_CalAndBoundSNR(scaled_FXP_rSignalEst, scaled_FXP_rNoiseEst), FXP_SIGNAL_EST_SCALE);
}

_REAL CChannelEstimation::CalAndBoundSNR(const _REAL rSignalEst, const _REAL rNoiseEst)
{
	_REAL rReturn;

	/* "rNoiseEst" must not be zero */
	if (rNoiseEst != (_REAL) 0.0)
		rReturn = rSignalEst / rNoiseEst;
	else
		rReturn = (_REAL) 1.0;

	/* Bound the SNR at 0 dB */
	if (rReturn < (_REAL) 1.0)
		rReturn = (_REAL) 1.0;

	return rReturn;
}


/********************************************************************************************/

_BOOLEAN CChannelEstimation::FXP_GetSNREstdB(FXP& FXP_rSNREstRes) const
{
  
  const FXP FXP_rNomBWSNR = FXP_rSNREstimate * FXP_rSNRSysToNomBWCorrFact;
  
  if ((FXP_rNomBWSNR >  1) && (bSNRInitPhase == FALSE))
    {
      FXP_rSNREstRes = (FXP) (10 * log10(FXP_rNomBWSNR));
      //FXP_rSNREstRes = (FXP) (10 * log10(rNomBWSNR)); //better estimate, but ultimatrely no difference on wave (need to see if i can cast to int)
    }
  else
    {
      FXP_rSNREstRes = 0;
    }	
  
  if (bSNRInitPhase == TRUE)
    return FALSE;
  else
    return TRUE;
}

_BOOLEAN CChannelEstimation::GetSNREstdB(_REAL& rSNREstRes) const
{
  
  /* fxn returns _BOOLEAN and passes in const _REAL
     _REAL = rNomBWSNR(created), rSNREstimate, rSNRSysToNomBWCorrFact, rSNREstRes(changed)
     _BOOLEAN = bSNRInitPhase
     
     1. rNomBWSNR doesnt work since the FXP muliplication is not resolved (maybe overflow?). HOwever the dummy
     wave is unaffected assuming mulitplication would work. However inspite of not being able to do FXP
     multiplication it seems like rNomBWSNR has no affect on the wave file.
     2. rSNREstRes comes up short, log10 is off by about 9 or 10. this doesnt! affect the dummy wave
     
  */ 
  
  FXP FXP_rNomBWSNR ;
  FXP FXP_rSNREstRes;

  FXP_rNomBWSNR = FXP_rSNREstimate * FXP_rSNRSysToNomBWCorrFact;
  
  /* Bound the SNR at 0 dB */
  if ((FXP_rNomBWSNR >  1) && (bSNRInitPhase == FALSE))
    {
      FXP_rSNREstRes = (FXP) (10 * log10(FXP_rNomBWSNR));
      //FXP_rSNREstRes = (FXP) (10 * log10(rNomBWSNR)); //better estimate, but ultimatrely no difference on wave (need to see if i can cast to int)
    }
  else
    {
      FXP_rSNREstRes = 0;
    }	
  
  rSNREstRes = FXP_rSNREstRes.GetFValue();
    
  if (bSNRInitPhase == TRUE)
    return FALSE;
  else
    return TRUE;
}

_REAL CChannelEstimation::GetMSCMEREstdB()
{
  /* Calculate final result (signal to noise ratio) and consider correction
     factor for average signal energy */
  const _REAL rCurMSCMEREst = (_REAL)FXP_rSNRSysToNomBWCorrFact *
    CalAndBoundSNR(AV_DATA_CELLS_POWER, (_REAL)FXP_rNoiseEstMSCMER);
  
  /* Bound the MCS MER at 0 dB */
  if (rCurMSCMEREst > (_REAL) 1.0)
    return (_REAL) 10.0 * log10(rCurMSCMEREst);
  else
    return (_REAL) 0.0;
}

_REAL CChannelEstimation::GetMSCWMEREstdB()
{
  FXP FXP_rAvNoiseEstMSC = 0;
  FXP FXP_rAvSigEstMSC = 0;
  
  /* Lock resources */
  Lock(); /* Lock start */
  {
    /* Average results from all carriers */
    for (int i = 0; i < iNumCarrier; i++)
      {
	FXP_rAvNoiseEstMSC += FXP_vecrNoiseEstMSC[i];
	FXP_rAvSigEstMSC += FXP_vecrSigEstMSC[i];
      }
  }
  Unlock(); /* Lock end */
  
  /* Calculate final result (signal to noise ratio) and consider correction
     factor for average signal energy */
  const FXP FXP_rCurMSCWMEREst = FXP_rSNRSysToNomBWCorrFact *
    CalAndBoundSNR(FXP_rAvSigEstMSC, FXP_rAvNoiseEstMSC);
  
  /* Bound the MCS MER at 0 dB */
  if (FXP_rCurMSCWMEREst > (FXP) 1)
    return (_REAL) 10.0 * log10((_REAL)FXP_rCurMSCWMEREst);
  else
    return (_REAL) 0.0;
}

_BOOLEAN CChannelEstimation::GetSigma(_REAL& rSigma)
{
  /* Doppler estimation is only implemented in the Wiener time interpolation
     module */
  if (TypeIntTime == TWIENER)
    {
      rSigma = TimeWiener.GetSigma();
      return TRUE;
    }
  else
    {
      rSigma = (_REAL) 0.0;
      return FALSE;
    }
}

_REAL CChannelEstimation::GetDelay() const
{
  /* Delay in ms */
  return (_REAL)(FXP_rLenPDSEst * (iFFTSizeN >> 2) / ((SOUNDCRD_SAMPLE_RATE/4000) * iNumIntpFreqPil * iScatPilFreqInt));
}

_REAL CChannelEstimation::GetMinDelay()
{
  /* Lock because of vector "vecrDelayHist" access */
  Lock();
  
  /* Return minimum delay in history */
  _FREAL FXP_rGetMinDelay_return;
  FXP_rGetMinDelay_return = FXP_vecrDelayHist[0];
  for (int i = 0; i < iLenDelayHist; i++) {
    if (FXP_rGetMinDelay_return > FXP_vecrDelayHist[i]) FXP_rGetMinDelay_return = FXP_vecrDelayHist[i];
  }

  Unlock();
  
  FXP_rGetMinDelay_return =  FXP_rGetMinDelay_return * (iFFTSizeN >> 2) /((SOUNDCRD_SAMPLE_RATE/4000) * iNumIntpFreqPil * iScatPilFreqInt);

  /* Return delay in ms */
  return (_REAL) FXP_rGetMinDelay_return;
}

void CChannelEstimation::GetTransferFunction(CVector<_REAL>& vecrData,
					     CVector<_REAL>& vecrGrpDly,
					     CVector<_REAL>& vecrScale)
{
  /* Init output vectors */
  vecrData.Init(iNumCarrier, (_REAL) 0.0);
  vecrGrpDly.Init(iNumCarrier, (_REAL) 0.0);
  vecrScale.Init(iNumCarrier, (_REAL) 0.0);
  /* Do copying of data only if vector is of non-zero length which means that
     the module was already initialized */
  if (iNumCarrier != 0)
    {
      _REAL rDiffPhase, rOldPhase;
      
      /* Lock resources */
      Lock();
      
      /* Init constants for normalization */
      const _REAL rTu = (CReal) iFFTSizeN / SOUNDCRD_SAMPLE_RATE;
      const _REAL rNormData = (_REAL) _MAXSHORT * _MAXSHORT;
      
      /* Copy data in output vector and set scale 
	 (carrier index as x-scale) */
      for (int i = 0; i < iNumCarrier; i++)
	{
	  /* Transfer function */
	  const _REAL rNormSqMagChanEst = (_REAL)SqMag(FXP_veccChanEst[i]) / rNormData;
	  
	  if (rNormSqMagChanEst > 0)
	    vecrData[i] = (_REAL) 10.0 * Log10(rNormSqMagChanEst);
	  else
	    vecrData[i] = RET_VAL_LOG_0;
	  
	  /* Group delay */
	  if (i == 0)
	    {
	      /* At position 0 we cannot calculate a derivation -> use
		 the same value as position 0 */
	      rDiffPhase = Angle(FXP_veccChanEst[1]) - Angle(FXP_veccChanEst[0]);
	    }
	  else
	    rDiffPhase = Angle(FXP_veccChanEst[i]) - rOldPhase;
	  
	  /* Take care of wrap around of angle() function */
	  if (rDiffPhase > WRAP_AROUND_BOUND_GRP_DLY)
	    rDiffPhase -= 2.0 * crPi;
	  if (rDiffPhase < -WRAP_AROUND_BOUND_GRP_DLY)
	    rDiffPhase += 2.0 * crPi;
	  
	  /* Apply normalization */
	  vecrGrpDly[i] = rDiffPhase * rTu * 1000.0 /* ms */;
	  
	  /* Store old phase */
	  rOldPhase = Angle(FXP_veccChanEst[i]);
	  
	  /* Scale (carrier index) */
	  vecrScale[i] = i;
	}
      
      /* Release resources */
      Unlock();
    }
}

void CChannelEstimation::GetSNRProfile(CVector<_REAL>& vecrData,
				       CVector<_REAL>& vecrScale)
{
  int i;
  /* Init output vectors */
  vecrData.Init(iNumCarrier, (_REAL) 0.0);
  vecrScale.Init(iNumCarrier, (_REAL) 0.0);
  
  /* Do copying of data only if vector is of non-zero length which means that
     the module was already initialized */
  if (iNumCarrier != 0)
    {
      /* Lock resources */
      Lock();
      
      /* We want to suppress the carriers on which no SNR measurement can be
	 performed (DC carrier, frequency pilots carriers) */
      int iNewNumCar = 0;
      for (i = 0; i < iNumCarrier; i++)
	{
	  if (FXP_vecrSigEstMSC[i] != (FXP) 0)
	    iNewNumCar++;
	}
      
      /* Init output vectors for new size */
      vecrData.Init(iNewNumCar, (_REAL) 0.0);
      vecrScale.Init(iNewNumCar, (_REAL) 0.0);
      
      /* Copy data in output vector and set scale 
	 (carrier index as x-scale) */
      int iCurOutIndx = 0;
      for (i = 0; i < iNumCarrier; i++)
	{
	  /* Suppress carriers where no SNR measurement is possible */
	  if (FXP_vecrSigEstMSC[i] != (FXP) 0)
	    {
	      /* Calculate final result (signal to noise ratio). Consider
		 correction factor for average signal energy */
	      const FXP FXP_rNomBWSNR =
		CalAndBoundSNR(FXP_vecrSigEstMSC[i], FXP_vecrNoiseEstMSC[i]) *
		FXP_rSNRFACSigCorrFact * FXP_rSNRSysToNomBWCorrFact;
	      
	      /* Bound the SNR at 0 dB */
	      if ((FXP_rNomBWSNR > (FXP) 1) && (bSNRInitPhase == FALSE))
		vecrData[iCurOutIndx] = (_REAL) 10.0 * Log10((_REAL)FXP_rNomBWSNR);
	      else
		vecrData[iCurOutIndx] = (_REAL) 0.0;
	      
	      /* Scale (carrier index) */
	      vecrScale[iCurOutIndx] = i;
	      
	      iCurOutIndx++;
	    }
	}
      
      /* Release resources */
      Unlock();
    }
}

void CChannelEstimation::GetAvPoDeSp(CVector<_REAL>& vecrData,
				     CVector<_REAL>& vecrScale,
				     _REAL& rLowerBound, _REAL& rHigherBound,
				     _REAL& rStartGuard, _REAL& rEndGuard,
				     _REAL& rPDSBegin, _REAL& rPDSEnd)
{
  /* Lock resources */
  Lock();
  TimeSyncTrack.GetAvPoDeSp(vecrData, vecrScale, rLowerBound,
			    rHigherBound, rStartGuard, rEndGuard, rPDSBegin, rPDSEnd);
  
  /* Release resources */
  Unlock();
}
