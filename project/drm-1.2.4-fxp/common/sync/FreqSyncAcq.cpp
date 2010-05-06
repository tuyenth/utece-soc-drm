/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Frequency synchronization acquisition (FFT-based)
 *
 * The input data is not modified by this module, it is just a measurement
 * of the frequency offset. The data is fed through this module.
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

//#ifndef HC_DEBUG
//#define HC_DEBUG
//#endif
#include "FreqSyncAcq.h"


/* Implementation *************************************************************/
void CFreqSyncAcq::ProcessDataInternal(CParameter& ReceiverParam)
{
	int			i, j;
	int			iMaxIndex_fxp;
	FXP			rMaxValue_fxp;
	int			iNumDetPeaks_fxp;
	int			iDiffTemp;
	CReal		rLevDiff;
	_BOOLEAN	bNoPeaksLeft_fxp;
	CFRealVector vecrPSDPilPoin_fxp(3);
	FILE* pFile_fxp;

	/*Convert input data to fxp format*/
	for (i = 0; i < iInputBlockSize; i++)
	vecInputData_fxp[i] = (FXP) ((*pvecInputData)[i]);

	if (bAquisition == TRUE)
	{
		/* Do not transfer any data to the next block if no frequency
		   acquisition was successfully done */
		iOutputBlockSize = 0;


		/* Add new symbol in history (shift register) */
		vecrFFTHistory_fxp.AddEnd(vecInputData_fxp, iInputBlockSize);


		/* Start algorithm when history memory is filled -------------------- */
		/* Wait until history memory is filled for the first FFT operation.
		   ("> 1" since, e.g., if we would have only one buffer, we can start
		   immediately) */
		if (iAquisitionCounter > 1)
		{
			/* Decrease counter */
			iAquisitionCounter--;
		}
		else
		{
			/* Copy vector to matlib vector and calculate real-valued FFTW */
			const int iStartIdx = iHistBufSize - iFrAcFFTSize;
			for (i = 0; i < iFrAcFFTSize; i++)
			{
				vecrFFTInput_fxp[i] = vecrFFTHistory_fxp[i + iStartIdx]*(((FXP) (1.0))/((FXP) (6535.0)));
			}

			/* Calculate power spectrum (X = real(F)^2 + imag(F)^2) */
			vecrSqMagFFTOut_fxp = SqMag(rfft(vecrFFTInput_fxp * vecrHammingWin_fxp, FftPlan));


			/* Calculate moving average for better estimate of PSD */
			vvrPSDMovAv_fxp.Add(vecrSqMagFFTOut_fxp);


			/* Wait until we have sufficient data for averaging */
			if (iAverageCounter > 1)
			{
				/* Decrease counter */
				iAverageCounter--;
			}
			else
			{
				/* Get PSD estimate */
				const CFRealVector vecrPSD_fxp(vvrPSDMovAv_fxp.GetAverage());


				/* -------------------------------------------------------------
				   Low pass filtering over frequency axis. We do the filtering
				   from both sides, once from right to left and then from left
				   to the right side. Afterwards, these results are averaged
				   This way, the noise floor is estimated */

// TODO: Introduce offset to debar peak at DC frequency (cause by DC offset of
// sound cards). Set "iStartFilt" in Init() routine!
const int iStartFilt = 0; // <- no offset right now

				/* Reset vectors for intermediate filtered result */
				vecrFiltResLR_fxp.Reset((CFReal) 0);
				vecrFiltResRL_fxp.Reset((CFReal) 0);

				/* From the left edge to the right edge */
				vecrFiltResLR_fxp[iStartFilt] = vecrPSD_fxp[iStartFilt];
				for (i = iStartFilt + 1; i < iHalfBuffer; i++)
				{
					vecrFiltResLR_fxp[i] = (vecrFiltResLR_fxp[i - 1] - vecrPSD_fxp[i]) *
						LAMBDA_FREQ_IIR_FILT_fxp + vecrPSD_fxp[i];
				}

				/* From the right edge to the left edge */
				vecrFiltResRL_fxp[iHalfBuffer - 1] =
					vecrPSD_fxp[iHalfBuffer - 1];

				for (i = iHalfBuffer - 2; i >= iStartFilt; i--)
				{
					vecrFiltResRL_fxp[i] = (vecrFiltResRL_fxp[i + 1] - vecrPSD_fxp[i]) *
						LAMBDA_FREQ_IIR_FILT_fxp + vecrPSD_fxp[i];
				}

				/* Average RL and LR filter outputs */
				vecrFiltRes_fxp = (vecrFiltResLR_fxp + vecrFiltResRL_fxp) * (((FXP) (1.0))/((FXP) (2.0)));



#ifdef _DEBUG_
#if 0
/* Stores curves for PSD estimation and filtering */
FILE* pFile2 = fopen("test/freqacqFilt.dat", "w");
for (i = 0; i < iHalfBuffer; i++)
	fprintf(pFile2, "%e %e\n", vecrPSD[i], vecrFiltRes[i]);
fclose(pFile2);
#endif
#endif

				/* Equalize PSD by "noise floor estimate" */
				for (i = 0; i < iHalfBuffer; i++)
				{
					/* Make sure we do not devide by zero */
					if (vecrFiltRes_fxp[i] != (FXP) 0)
						vecrPSD_fxp[i] *= (((FXP) (1.0))/(vecrFiltRes_fxp[i]));
					else
						vecrPSD_fxp[i] = 0.0;
				}

				/* Correlate known frequency-pilot structure with equalized
				   power spectrum */
				for (i = 0; i < iSearchWinSize; i++)
				{
					vecrPSDPilCor_fxp[i] =
						vecrPSD_fxp[i + veciTableFreqPilots[0]] +
						vecrPSD_fxp[i + veciTableFreqPilots[1]] +
						vecrPSD_fxp[i + veciTableFreqPilots[2]];
				}


				/* Detect peaks --------------------------------------------- */
				/* Get peak indices of detected peaks */
				iNumDetPeaks_fxp = 0;
				for (i = iStartDCSearch; i < iEndDCSearch; i++)
				{
					/* Test peaks against a bound */
					if (vecrPSDPilCor_fxp[i] > rPeakBoundFiltToSig_fxp)
					{
						veciPeakIndex_fxp[iNumDetPeaks_fxp] = i;
						iNumDetPeaks_fxp++;
					}
				}

				/* Check, if at least one peak was detected */
				if (iNumDetPeaks_fxp > 0)
					{
						/* ---------------------------------------------------------
						   The following test shall exclude sinusoid interferers in
						   the received spectrum */
						CVector<int> vecbFlagVec_fxp(iNumDetPeaks_fxp, 1);

						/* Check all detected peaks in the "PSD-domain" if there are
						   at least two peaks with approx the same power at the
						   correct places (positions of the desired pilots) */
						for (i = 0; i < iNumDetPeaks_fxp; i++)
						{
							/* Fill the vector with the values at the desired
							   pilot positions */
							vecrPSDPilPoin_fxp[0] =
								vecrPSD_fxp[veciPeakIndex_fxp[i] + veciTableFreqPilots[0]];
							vecrPSDPilPoin_fxp[1] =
								vecrPSD_fxp[veciPeakIndex_fxp[i] + veciTableFreqPilots[1]];
							vecrPSDPilPoin_fxp[2] =
								vecrPSD_fxp[veciPeakIndex_fxp[i] + veciTableFreqPilots[2]];
							/* Sort, to extract the highest and second highest
							   peak */
							vecrPSDPilPoin_fxp = Sort(vecrPSDPilPoin_fxp);

							/* Debar peak, if it is much higher than second highest
							   peak (most probably a sinusoid interferer). The
							   highest peak is stored at "vecrPSDPilPoin[2]". Also
							   test for lowest peak */
							if ((vecrPSDPilPoin_fxp[1] * (((FXP) (1.0))/vecrPSDPilPoin_fxp[2]) <
								MAX_RAT_PEAKS_AT_PIL_POS_HIGH_fxp) &&
								(vecrPSDPilPoin_fxp[0] * (((FXP) (1.0))/vecrPSDPilPoin_fxp[2]) <
								MAX_RAT_PEAKS_AT_PIL_POS_LOW_fxp))
							{
								/* Reset "good-flag" */
								vecbFlagVec_fxp[i] = 0;
							}
						}


					/* Get maximum ------------------------------------------ */
					/* First, get the first valid peak entry and init the
					   maximum with this value. We also detect, if a peak is
					   left */
					bNoPeaksLeft_fxp = TRUE;
					for (i = 0; i < iNumDetPeaks_fxp; i++)
					{
						if (vecbFlagVec_fxp[i] == 1)
						{
							/* At least one peak is left */
							bNoPeaksLeft_fxp = FALSE;

							/* Init max value */
							iMaxIndex_fxp = veciPeakIndex_fxp[i];
							rMaxValue_fxp = vecrPSDPilCor_fxp[veciPeakIndex_fxp[i]];
						}
					}


					if (bNoPeaksLeft_fxp == FALSE)
					{
						/* Actual maximum detection, take the remaining peak
						   which has the highest value */
						for (i = 0; i < iNumDetPeaks_fxp; i++)
						{
							if ((vecbFlagVec_fxp[i] == 1) &&
								(vecrPSDPilCor_fxp[veciPeakIndex_fxp[i]] >
								rMaxValue_fxp))
							{
								iMaxIndex_fxp = veciPeakIndex_fxp[i];
								rMaxValue_fxp = vecrPSDPilCor_fxp[veciPeakIndex_fxp[i]];
							}
						}




						/* -----------------------------------------------------
						   An acquisition frequency offest estimation was
						   found */
						/* Calculate frequency offset and set global parameter
						   for offset */
						ReceiverParam.rFreqOffsetAcqui =
							(_REAL) iMaxIndex_fxp / iFrAcFFTSize;

						/* Reset acquisition flag */
						bAquisition = FALSE;


						/* Send out the data stored for FFT calculation ----- */
						/* This does not work for bandpass filter. TODO: make
						   this possible for bandpass filter, too */
						if (bUseRecFilter == FALSE)
						{
							iOutputBlockSize = iHistBufSize;

							/* Frequency offset correction */
							const _REAL rNormCurFreqOffsFst = (_REAL) 2.0 * crPi *
								(ReceiverParam.rFreqOffsetAcqui - rInternIFNorm);

							for (i = 0; i < iHistBufSize; i++)
							{
								/* Multiply with exp(j omega t) */
								(*pvecOutputData)[i] = (_REAL) vecrFFTHistory_fxp[i] *
									_COMPLEX(Cos(i * rNormCurFreqOffsFst),
									Sin(-i * rNormCurFreqOffsFst));
							}

							/* Init "exp-step" for regular frequency shift which
							   is used in tracking mode to get contiuous mixing
							   signal */
							cCurExp =
								_COMPLEX(Cos(iHistBufSize * rNormCurFreqOffsFst),
								Sin(-iHistBufSize * rNormCurFreqOffsFst));
						}
					}
				}
			}
		}
	}
	else
	{
		/* If synchronized DRM input stream is used, overwrite the detected
		   frequency offest estimate by the desired frequency, because we know
		   this value */
		if (bSyncInput == TRUE)
		{
			ReceiverParam.rFreqOffsetAcqui =
				(_REAL) ReceiverParam.iIndexDCFreq / ReceiverParam.iFFTSizeN;
		}

		/* Use the same block size as input block size */
		iOutputBlockSize = iInputBlockSize;


		/* Frequency offset correction -------------------------------------- */
		/* Total frequency offset from acquisition and tracking (we calculate
		   the normalized frequency offset) */
		const _REAL rNormCurFreqOffset =
			(_REAL) 2.0 * crPi * (ReceiverParam.rFreqOffsetAcqui +
			ReceiverParam.rFreqOffsetTrack - rInternIFNorm);


		/* New rotation vector for exp() calculation */
		const _COMPLEX cExpStep =
			_COMPLEX(Cos(rNormCurFreqOffset), Sin(rNormCurFreqOffset));

		/* Input data is real, make complex and compensate for frequency
		   offset */
		for (i = 0; i < iOutputBlockSize; i++)
		{
			(*pvecOutputData)[i] = (_REAL)vecInputData_fxp[i] * Conj(cCurExp);

			/* Rotate exp-pointer on step further by complex multiplication with
			   precalculated rotation vector cExpStep. This saves us from
			   calling sin() and cos() functions all the time (iterative
			   calculation of these functions) */
			cCurExp *= cExpStep;
		}


		/* Bandpass filter -------------------------------------------------- */
		if (bUseRecFilter == TRUE)
			BPFilter.Process(*pvecOutputData);
	}
}

void CFreqSyncAcq::InitInternal(CParameter& ReceiverParam)
{
	int			i;
	/* Needed for calculating offset in Hertz in case of synchronized input
	   (for simulation) */
	iFFTSize = ReceiverParam.iFFTSizeN;

	/* We using parameters from robustness mode B as pattern for the desired
	   frequency pilot positions */
	veciTableFreqPilots[0] =
		iTableFreqPilRobModB[0][0] * NUM_BLOCKS_4_FREQ_ACQU;
	veciTableFreqPilots[1] =
		iTableFreqPilRobModB[1][0] * NUM_BLOCKS_4_FREQ_ACQU;
	veciTableFreqPilots[2] =
		iTableFreqPilRobModB[2][0] * NUM_BLOCKS_4_FREQ_ACQU;

	/* Size of FFT */
	iFrAcFFTSize = RMB_FFT_SIZE_N * NUM_BLOCKS_4_FREQ_ACQU;


	/* -------------------------------------------------------------------------
	   Set start- and endpoint of search window for DC carrier after the
	   correlation with the known pilot structure */
	/* Normalize the desired position and window size which are in Hertz */
	const _REAL rNormDesPos = rCenterFreq / SOUNDCRD_SAMPLE_RATE * 2;
	const _REAL rNormHalfWinSize = rWinSize / SOUNDCRD_SAMPLE_RATE;

	/* Length of the half of the spectrum of real input signal (the other half
	   is the same because of the real input signal). We have to consider the
	   Nyquist frequency ("iFrAcFFTSize" is always even!) */
	iHalfBuffer = iFrAcFFTSize / 2 + 1;

	/* Search window is smaller than haft-buffer size because of correlation
	   with pilot positions */
	iSearchWinSize = iHalfBuffer - veciTableFreqPilots[2];

	/* Calculate actual indices of start and end of search window */
	iStartDCSearch =
		(int) Floor((rNormDesPos - rNormHalfWinSize) * iHalfBuffer);
	iEndDCSearch = (int) Ceil((rNormDesPos + rNormHalfWinSize) * iHalfBuffer);

	/* Check range. If out of range -> correct */
	if (!((iStartDCSearch > 0) && (iStartDCSearch < iSearchWinSize)))
		iStartDCSearch = 0;

	if (!((iEndDCSearch > 0) && (iEndDCSearch < iSearchWinSize)))
		iEndDCSearch = iSearchWinSize;

	/* Set bound for ratio between filtered signal to signal. Use a lower bound
	   if the search window is smaller */

	if (((FXP) iEndDCSearch - iStartDCSearch) / iHalfBuffer < (FXP) 0.042)
		rPeakBoundFiltToSig_fxp = PEAK_BOUND_FILT2SIGNAL_0_042_fxp;
	else
		rPeakBoundFiltToSig_fxp = PEAK_BOUND_FILT2SIGNAL_1_fxp;


	/* Init vectors and FFT-plan -------------------------------------------- */
	/* Allocate memory for FFT-histories and init with zeros */
	iHistBufSize = iFrAcFFTSize * NUM_BLOCKS_USED_FOR_AV;
	vecrFFTHistory_fxp.Init(iHistBufSize, (FXP) 0);
	vecrFFTInput_fxp.Init(iFrAcFFTSize);
	vecrSqMagFFTOut_fxp.Init(iHalfBuffer);

	/* Allocate memory for PSD after pilot correlation */
	vecrPSDPilCor_fxp.Init(iHalfBuffer);

	/* Init vectors for filtering in frequency direction */
	vecrFiltResLR_fxp.Init(iHalfBuffer);
	vecrFiltResRL_fxp.Init(iHalfBuffer);
	vecrFiltRes_fxp.Init(iHalfBuffer);

	/* Index memory for detected peaks (assume worst case with the size) */
	veciPeakIndex_fxp.Init(iHalfBuffer);

	/* Init plans for FFT (faster processing of Fft and Ifft commands) */
	FftPlan.Init(iFrAcFFTSize);

	/* Init Hamming window */
	vecrHammingWin.Init(iFrAcFFTSize);
	vecrHammingWin = Hamming(iFrAcFFTSize);
	vecrHammingWin_fxp.Init(iFrAcFFTSize);
	for (i = 0; i < iFrAcFFTSize; i++)
	vecrHammingWin_fxp[i] = (FXP) vecrHammingWin[i];

	/* Init moving average class for SqMag FFT results */
	vvrPSDMovAv_fxp.InitVec(NUM_FFT_RES_AV_BLOCKS, iHalfBuffer);


	/* Frequency correction */
	/* Start with phase null (arbitrary) */
	cCurExp = (_REAL) 1.0;
	rInternIFNorm = (_REAL) ReceiverParam.iIndexDCFreq / iFFTSize;


	/* Init bandpass filter object */
	BPFilter.Init(ReceiverParam.iSymbolBlockSize, VIRTUAL_INTERMED_FREQ,
		ReceiverParam.GetSpectrumOccup(), CDRMBandpassFilt::FT_RECEIVER);


	/* Define block-sizes for input (The output block size is set inside
	   the processing routine, therefore only a maximum block size is set
	   here) */
	iInputBlockSize = ReceiverParam.iSymbolBlockSize;

	/* We have to consider that the next module can take up to two symbols per
	   step. This can be satisfied be multiplying with "3". We also want to ship
	   the whole FFT buffer after finishing the frequency acquisition so that
	   these samples can be reused for synchronization and do not get lost */
	iMaxOutputBlockSize = 3 * ReceiverParam.iSymbolBlockSize + iHistBufSize;

	vecInputData_fxp.Init(iInputBlockSize);
}

void CFreqSyncAcq::SetSearchWindow(_REAL rNewCenterFreq, _REAL rNewWinSize)
{
	/* Set internal parameters */
	rCenterFreq = rNewCenterFreq;
	rWinSize = rNewWinSize;

	/* Set flag to initialize the module to the new parameters */
	SetInitFlag();
}

void CFreqSyncAcq::StartAcquisition()
{
	/* Set flag so that the actual acquisition routine is entered */
	bAquisition = TRUE;

	/* Reset (or init) counters */
	iAquisitionCounter = NUM_BLOCKS_4_FREQ_ACQU;
	iAverageCounter = NUM_FFT_RES_AV_BLOCKS;

	/* Reset FFT-history */
	vecrFFTHistory_fxp.Reset((FXP) 0);
}
