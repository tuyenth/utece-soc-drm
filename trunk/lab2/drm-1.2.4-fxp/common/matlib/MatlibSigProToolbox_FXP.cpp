/******************************************************************************\
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	c++ Mathematic Library (Matlib), signal processing toolbox
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



/* Implementation *************************************************************/

FXP FXP_Sinc (const FXP& rI) 
{
  FXP fSinc;
  FXP cfInvPi;
  FXP fInvX;

  // Sinc(X) = Sin(pi*X)/(pi*X)

  // TODO still need to bound X to (-Pi,Pi)
  //rI = (FXP) rX;

  if ( rI == 0 ) {
    fSinc = (FXP)1;
  } else {
    fInvX = (FXP)1 / rI;
    cfInvPi = (FXP)1 / (FXP)crPi;  // TODO this should be a constant somewhere

    fSinc = FXP_Sin(rI);
    fSinc = fSinc * cfInvPi;
    fSinc = fSinc * fInvX;
  }

  //rSinc = Sin( rX * crPi ) / (rX * crPi);

  return fSinc;

}


CMatlibVector<FXP> Hann_FXP(const int iLen)
{
	int iHalf, i;
	CMatlibVector<FXP> fvRet(iLen, VTY_TEMP);

	if (iLen % 2)
	{
		/* Odd length window */
		iHalf = (iLen + 1) / 2;

		/* Hanning window */
		CMatlibVector<CReal> vecTemp(iHalf);
		CMatlibVector<FXP> w(iHalf);
		for (i = 0; i < iHalf; i++)
			vecTemp[i] =  i;

		w = (CReal) 0.5 * (1 - Cos((CReal) 2.0 * crPi * vecTemp / (iLen - 1)));

		/* Make symmetric window */
		return fvRet.Merge(w, w(iHalf - 1, -1, 1));
	}
	else
	{
		/* Even length window */
		iHalf = iLen / 2;

		/* Hanning window */
		CMatlibVector<CReal> vecTemp(iHalf);
		CMatlibVector<FXP> w(iHalf);
		for (i = 0; i < iHalf; i++)
			vecTemp[i] =  i;

		w = (CReal) 0.5 * (1 - Cos((CReal) 2.0 * crPi * vecTemp / (iLen - 1)));

		/* Make symmetric window */
		return fvRet.Merge(w, w(iHalf, -1, 1));
	}
}

CMatlibVector<FXP> Hamming_FXP(const int iLen)
{
	int iHalf, i;
	CMatlibVector<FXP> fvRet(iLen, VTY_TEMP);

	if (iLen % 2)
	{
		/* Odd length window */
		iHalf = (iLen + 1) / 2;

		/* Hanning window */
		CMatlibVector<CReal> vecTemp(iHalf);
		CMatlibVector<FXP> w(iHalf);
		for (i = 0; i < iHalf; i++)
			vecTemp[i] =  i;

		w = (CReal) 0.54 - (CReal) 0.46 * 
			Cos((CReal) 2.0 * crPi * vecTemp / (iLen - 1));

		/* Make symmetric window */
		return fvRet.Merge(w, w(iHalf - 1, -1, 1));
	}
	else
	{
		/* Even length window */
		iHalf = iLen / 2;

		/* Hanning window */
		CMatlibVector<CReal> vecTemp(iHalf);
		CMatlibVector<FXP> w(iHalf);
		for (i = 0; i < iHalf; i++)
			vecTemp[i] =  i;

		w = (CReal) 0.54 - (CReal) 0.46 * 
			Cos((CReal) 2.0 * crPi * vecTemp / (iLen - 1));

		/* Make symmetric window */
		return fvRet.Merge(w, w(iHalf, -1, 1));
	}
}

CMatlibVector<FXP> Nuttallwin_FXP(const int iLen)
{
	CMatlibVector<FXP> fvRet(iLen, VTY_TEMP);

	/* Nuttall coefficients */
	const FXP rA0 = (FXP) 0.3635819;
	const FXP rA1 = (FXP) 0.4891775;
	const FXP rA2 = (FXP) 0.1365995;
	const FXP rA3 = (FXP) 0.0106411;

	const FXP rArg = (FXP) 2.0 * crPi / (iLen - 1);

	for (int i = 0; i < iLen; i++)
	{
		fvRet[i] = rA0 - rA1 * Cos(rArg * i) +
			rA2 * Cos(rArg * i * 2) - rA3 * Cos(rArg * i * 3);
	}

	return fvRet;
}

CMatlibVector<FXP> Bartlett_FXP(const int iLen)
{
	const int iHalf = (int) Ceil((FXP) iLen / 2);
	CMatlibVector<FXP> fvHalfWin(iHalf);
	CMatlibVector<FXP> fvRet(iLen, VTY_TEMP);

	for (int i = 0; i < iHalf; i++)
		fvHalfWin[i] = (FXP) 2.0 * i / (iLen - 1);

	/* Build complete output vector depending on odd or even input length */
	if (iLen % 2)
		fvRet.Merge(fvHalfWin, fvHalfWin(iHalf - 1, -1, 1)); /* Odd */
	else
		fvRet.Merge(fvHalfWin, fvHalfWin(iHalf, -1, 1)); /* Even */

	return fvRet;
}

CMatlibVector<FXP> Triang_FXP(const int iLen)
{
	const int iHalf = (int) Ceil((FXP) iLen / 2);
	CMatlibVector<FXP> fvHalfWin(iHalf);
	CMatlibVector<FXP> fvRet(iLen, VTY_TEMP);

	/* Build complete output vector depending on odd or even input length */
	if (iLen % 2)
	{
		for (int i = 0; i < iHalf; i++)
			fvHalfWin[i] = (FXP) 2.0 * (i + 1) / (iLen + 1);

		fvRet.Merge(fvHalfWin, fvHalfWin(iHalf - 1, -1, 1)); /* Odd */
	}
	else
	{
		for (int i = 0; i < iHalf; i++)
			fvHalfWin[i] = ((FXP) 2.0 * (i + 1) - 1) / iLen;

		fvRet.Merge(fvHalfWin, fvHalfWin(iHalf, -1, 1)); /* Even */
	}

	return fvRet;
}

CMatlibVector<FXP> Kaiser(const int iLen, const FXP rBeta)
{
	FXP		rX;
	const int	iIsOdd = iLen % 2;
	const int	n = (iLen + 1) / 2; /* Half vector size, round up */
	CMatlibVector<FXP> fvRet(iLen);
	CMatlibVector<FXP> fvW(n);

	const FXP rNorm = Abs(Besseli((FXP) 0.0, rBeta));
	const FXP rXind = (iLen - 1) * (iLen - 1);

	if (iIsOdd == 0)
		rX = (FXP) 0.5;
	else
		rX = (FXP) 0.0;

	for (int i = 0; i < n; i++)
	{
		fvW[i] = Besseli((FXP) 0.0, rBeta * Sqrt((FXP) 1.0 -
			(FXP) 4.0 * rX * rX / rXind)) / rNorm;
		rX += (FXP) 1.0;
	}

	/* Symmetrical window */
	fvRet.Merge(fvW(n, -1, iIsOdd + 1), fvW);

	return Abs(fvRet);
}

FXP Besseli(const FXP rNu, const FXP rZ)
{
	const FXP	rEp = (FXP) 10e-9; /* Define accuracy */
	const FXP	rY = rZ / (FXP) 2.0;
	FXP		rReturn = (FXP) 1.0;
	FXP		rD = (FXP) 1.0;
	FXP		rS = (FXP) 1.0;

#ifdef _DEBUG_
	/* Only nu = 0 is supported right now! */
	if (rNu != (FXP) 0.0)
	{
		DebugError("MatLibr: Besseli function", "The nu = ", rNu, \
			" is not supported, only nu = ", 0);
	}
#endif

	for (int i = 1; i <= 25 && rReturn * rEp <= rS; i++)
	{
		rD *= rY / i;
		rS = rD * rD;
		rReturn += rS;
	}

	return rReturn;
}

CMatlibVector<FXP> Randn_FXP(const int iLen)
{
	/* Add some constant distributed random processes together */
	_VECOP(FXP, iLen, 
		(FXP) ((((FXP) 
		rand() + rand() + rand() + rand() + rand() + rand() + rand()) 
		/ RAND_MAX - 0.5) * /* sqrt(3) * 2 / sqrt(7) */ 1.3093));
}

CMatlibVector<FXP> Filter(const CMatlibVector<FXP>& fvB,
							const CMatlibVector<FXP>& fvA,
							const CMatlibVector<FXP>& fvX,
							CMatlibVector<FXP>& fvZ)
{
	int						m, n, iLenCoeff;
	const int				iSizeA = fvA.GetSize();
	const int				iSizeB = fvB.GetSize();
	const int				iSizeX = fvX.GetSize();
	const int				iSizeZ = fvZ.GetSize();
	CMatlibVector<FXP>	fvY(iSizeX, VTY_TEMP);
	CMatlibVector<FXP>	fvANew, fvBNew;

	if ((iSizeA == 1) && (fvA[0] == (FXP) 1.0))
	{
		/* FIR filter ------------------------------------------------------- */
		const int				iSizeXNew = iSizeX + iSizeZ;
		CMatlibVector<FXP>	rvXNew(iSizeXNew);

		/* Add old values to input vector */
		rvXNew.Merge(fvZ, fvX);

		/* Actual convolution */
		for (m = 0; m < iSizeX; m++)
		{
			fvY[m] = (FXP) 0.0;

			for (n = 0; n < iSizeB; n++)
				fvY[m] += fvB[n] * rvXNew[m + iSizeB - n - 1];
		}

		/* Save last samples in state vector */
		fvZ = rvXNew(iSizeXNew - iSizeZ + 1, iSizeXNew);
	}
	else
	{
		/* IIR filter ------------------------------------------------------- */
		/* Length of coefficients */
		iLenCoeff = (int) Max((int) iSizeB, (int) iSizeA);

		/* Make fvB and fvA the same length (zero padding) */
		if (iSizeB > iSizeA)
		{
			fvBNew.Init(iSizeB);
			fvANew.Init(iSizeB);

			fvBNew = fvB;
			fvANew.Merge(fvA, Zeros_FXP(iSizeB - iSizeA));
		}
		else
		{
			fvBNew.Init(iSizeA);
			fvANew.Init(iSizeA);

			fvANew = fvA;
			fvBNew.Merge(fvB, Zeros_FXP(iSizeA - iSizeB));
		}

		/* Filter is implemented as a transposed direct form II structure */
		for (m = 0; m < iSizeX; m++)
		{
			/* y(m) = (b(1) x(m) + z_1(m - 1)) / a(1) */
			fvY[m] = (fvBNew[0] * fvX[m] + fvZ[0]) / fvANew[0];

			for (n = 1; n < iLenCoeff; n++)
			{
				/* z_{n - 2}(m) = b(n - 1) x(m) + z_{n - 1}(m - 1) -
				   a(n - 1) y(m) */
				fvZ[n - 1] = fvBNew[n] * fvX[m] + fvZ[n] - fvANew[n] * fvY[m];
			}
		}
	}

	return fvY;
}

CMatlibVector<FXP> FirLP(const FXP rNormBW,
						   const CMatlibVector<FXP>& rvWin)
{
/*
	Lowpass filter design using windowing method
*/
	const int				iLen = rvWin.GetSize();
	const int				iHalfLen = (int) Floor((FXP) iLen / 2);
	CMatlibVector<FXP>	fvRet(iLen, VTY_TEMP);

	/* Generate truncuated ideal response */
	for (int i = 0; i < iLen; i++)
		fvRet[i] = rNormBW * Sinc(rNormBW * (i - iHalfLen));

	/* Apply window */
	fvRet *= rvWin;

	return fvRet;
}

CMatlibVector<FComplex> FirFiltDec(const CMatlibVector<FComplex>& cvB,
								   const CMatlibVector<FComplex>& cvX,
								   CMatlibVector<FComplex>& cvZ,
								   const int iDecFact)
{
	int			m, n, iCurPos;
	const int	iSizeX = cvX.GetSize();
	const int	iSizeZ = cvZ.GetSize();
	const int	iSizeB = cvB.GetSize();
	const int	iSizeXNew = iSizeX + iSizeZ;
	const int	iSizeFiltHist = iSizeB - 1;

	int iNewLenZ;
	int iDecSizeY;

	if (iSizeFiltHist >= iSizeXNew)
	{
		 /* Special case if no new output can be calculated */
		iDecSizeY = 0;

		iNewLenZ = iSizeXNew;
	}
	else
	{
		/* Calculate the number of output bits which can be generated from the
		   provided input vector */
		iDecSizeY = 
				(int) (((FXP) iSizeXNew - iSizeFiltHist - 1) / iDecFact + 1);

		/* Since the input vector length must not be a multiple of "iDecFact",
		   some input bits will be unused. To store this number, the size of
		   the state vector "Z" is adapted */
		iNewLenZ = iSizeFiltHist - 
			(iDecSizeY * iDecFact - (iSizeXNew - iSizeFiltHist));
	}

	CMatlibVector<FComplex>	cvY(iDecSizeY, VTY_TEMP);
	CMatlibVector<FComplex>	cvXNew(iSizeXNew);

	/* Add old values to input vector */
	cvXNew.Merge(cvZ, cvX);

	/* FIR filter */
	for (m = 0; m < iDecSizeY; m++)
	{
		iCurPos = m * iDecFact + iSizeFiltHist;

		cvY[m] = (FXP) 0.0;

		for (n = 0; n < iSizeB; n++)
			cvY[m] += cvB[n] * cvXNew[iCurPos - n];
	}

	/* Save last samples in state vector */
	cvZ.Init(iNewLenZ);
	cvZ = cvXNew(iSizeXNew - iNewLenZ + 1, iSizeXNew);

	return cvY;
}

CMatlibVector<FXP> Levinson(const CMatlibVector<FXP>& vecrRx,
			  const CMatlibVector<FXP>& vecrB)
{
/* 
	The levinson recursion [S. Haykin]

	This algorithm solves the following equations:
	Rp ap = ep u1,
	Rp Xp = b, where Rp is a Toepliz-matrix of vector prRx and b = prB 
	is an arbitrary correlation-vector. The Result is ap = prA.

	Parts of the following code are taken from Ptolemy
	(http://ptolemy.eecs.berkeley.edu/)
*/
	const int	iLength = vecrRx.GetSize();
	FXPVector vecrX(iLength, VTY_TEMP);

	FXP		rGamma;
	FXP		rGammaCap;
	FXP		rDelta;
	FXP		rE;
	FXP		rQ;
	int			i, j;
	FXPVector vecraP(iLength);
	FXPVector vecrA(iLength);

	/* Initialize the recursion --------------------------------------------- */
	// (a) First coefficient is always unity
	vecrA[0] = (FXP) 1.0;
	vecraP[0] = (FXP) 1.0;

	// (b) 
	vecrX[0] = vecrB[0] / vecrRx[0];

	// (c) Initial prediction error is simply the zero-lag of
	// of the autocorrelation, or the signal power estimate.
	rE = vecrRx[0];


	/* Main loop ------------------------------------------------------------ */
	// The order recurrence
	for (j = 0; j < iLength - 1; j++)
	{
		const int iNextInd = j + 1;

		// (a) Compute the new gamma
		rGamma = vecrRx[iNextInd];
		for (i = 1; i < iNextInd; i++) 
			rGamma += vecrA[i] * vecrRx[iNextInd - i];

		// (b), (d) Compute and output the reflection coefficient
		// (which is also equal to the last AR parameter)
		vecrA[j + 1] = rGammaCap = - rGamma / rE;

		// (c)
		for (i = 1; i < iNextInd; i++) 
			vecraP[i] = vecrA[i] + rGammaCap * vecrA[iNextInd - i];

		// Swap a and aP for next order recurrence
		for (i = 1; i < iNextInd; i++)
			vecrA[i] = vecraP[i];

		// (e) Update the prediction error power
		rE = rE * ((FXP) 1.0 - rGammaCap * rGammaCap);

		// (f)
		rDelta = (FXP) 0.0;
		for (i = 0; i < iNextInd; i++) 
			rDelta += vecrX[i] * vecrRx[iNextInd - i];

		// (g), (i) 
		vecrX[iNextInd] = rQ = (vecrB[iNextInd] - rDelta) / rE;

		// (h)
		for (i = 0; i < iNextInd; i++) 
			vecrX[i] = vecrX[i] + rQ * vecrA[iNextInd - i];
	}

	return vecrX;
}

CMatlibVector<FComplex> Levinson(const CMatlibVector<FComplex>& veccRx,
								 const CMatlibVector<FComplex>& veccB)
{
/* 
	The levinson recursion [S. Haykin]
	COMPLEX version!

	This algorithm solves the following equations:
	Rp ap = ep u1,
	Rp Xp = b, where Rp is a Toepliz-matrix of vector prRx and b = prB 
	is an arbitrary correlation-vector. The Result is ap = prA.

	Parts of the following code are taken from Ptolemy
	(http://ptolemy.eecs.berkeley.edu/)
*/
	const int		iLength = veccRx.GetSize();
	FComplexVector	veccX(iLength, VTY_TEMP);

	FComplex		cGamma;
	FComplex		cGammaCap;
	FComplex		cDelta;
	FXP			rE;
	FComplex		cQ;
	int				i, j;
	FComplexVector	veccaP(iLength);
	FComplexVector	veccA(iLength);

	/* Initialize the recursion --------------------------------------------- */
	// (a) First coefficient is always unity
	veccA[0] = (FXP) 1.0;
	veccaP[0] = (FXP) 1.0;

	// (b) 
	veccX[0] = veccB[0] / veccRx[0];

	// (c) Initial prediction error is simply the zero-lag of
	// of the autocorrelation, or the signal power estimate.
	rE = Real(veccRx[0]);


	/* Main loop ------------------------------------------------------------ */
	// The order recurrence
	for (j = 0; j < iLength - 1; j++)
	{
		const int iNextInd = j + 1;

		// (a) Compute the new gamma
		cGamma = veccRx[iNextInd];
		for (i = 1; i < iNextInd; i++) 
			cGamma += veccA[i] * veccRx[iNextInd - i];

		// (b), (d) Compute and output the reflection coefficient
		// (which is also equal to the last AR parameter)
		veccA[iNextInd] = cGammaCap = - cGamma / rE;

		// (c)
		for (i = 1; i < iNextInd; i++) 
			veccaP[i] = veccA[i] + cGammaCap * Conj(veccA[iNextInd - i]);

		// Swap a and aP for next order recurrence
		for (i = 1; i < iNextInd; i++)
			veccA[i] = veccaP[i];

		// (e) Update the prediction error power
		rE = rE * ((FXP) 1.0 - SqMag(cGammaCap));

		// (f)
		cDelta = (FXP) 0.0;
		for (i = 0; i < iNextInd; i++) 
			cDelta += veccX[i] * veccRx[iNextInd - i];

		// (g), (i) 
		veccX[iNextInd] = cQ = (veccB[iNextInd] - cDelta) / rE;

		// (h)
		for (i = 0; i < iNextInd; i++) 
			veccX[i] = veccX[i] + cQ * Conj(veccA[iNextInd - i]);
	}

	return veccX;
}


