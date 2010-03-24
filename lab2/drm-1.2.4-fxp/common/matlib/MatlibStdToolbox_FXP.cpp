
/* Implementation *************************************************************/
FXP Min(const CMatlibVector<FXP>& rvI) 
{ 
  const int iSize = rvI.GetSize(); 
  FXP rMinRet = rvI[0]; 
  for (int i = 1; i < iSize; i++) 
    { 
      if (rvI[i] < rMinRet) 
	rMinRet = rvI[i]; 
    } 
 
  return rMinRet; 
} 


void Min(FXP& rMinVal, int& iMinInd, const CMatlibVector<FXP>& rvI) 
{ 
  const int iSize = rvI.GetSize(); 
  rMinVal = rvI[0]; /* Init actual minimum value */ 
  iMinInd = 0; /* Init index of minimum */ 
  for (int i = 1; i < iSize; i++) 
    { 
      if (rvI[i] < rMinVal) 
	{ 
	  rMinVal = rvI[i]; 
	  iMinInd = i; 
	} 
    } 
} 


FXP Max(const CMatlibVector<FXP>& rvI) 
{ 
  FXP rMaxRet; 
  int iMaxInd; /* Not used by this function */ 
  Max(rMaxRet, iMaxInd, rvI); 
 
  return rMaxRet; 
} 


void Max(FXP& rMaxVal, int& iMaxInd, const CMatlibVector<FXP>& rvI) 
{ 
  const int iSize = rvI.GetSize(); 
  rMaxVal = rvI[0]; /* Init actual maximum value */ 
  iMaxInd = 0; /* Init index of maximum */ 
  for (int i = 1; i < iSize; i++) 
    { 
      if (rvI[i] > rMaxVal) 
	{ 
	  rMaxVal = rvI[i]; 
	  iMaxInd = i; 
	} 
    } 
} 

CMatlibVector<FXP> Sort(const CMatlibVector<FXP>& rvI) 
{ 
  const int iSize = rvI.GetSize(); 
  const int iEnd = iSize - 1; 
  CMatlibVector<FXP> fvRet(iSize, VTY_TEMP); 
 
  /* Copy input vector in output vector */ 
  fvRet = rvI; 
 
  /* Loop through the array one less than its total cell count */ 
  for (int i = 0; i < iEnd; i++) 
    { 
      /* Loop through every cell (value) in array */ 
            for (int j = 0; j < iEnd; j++) 
      	{ 
	  /* Compare the values and switch if necessary */ 
       	  if (fvRet[j] > fvRet[j + 1]) 
	    { 
	      const FXP rSwap = fvRet[j]; 
	      fvRet[j] = fvRet[j + 1]; 
	      fvRet[j + 1] = rSwap; 
	    } 
	} 
    } 
 
  return fvRet; 
}



FXP FXP_Sine (const FXP& fX) 
{
  FXP fNegX;
  FXP fSine;
  FXP fResidue;
  FXP fFactorial;

  int i;

  // The input should be bound by -pi/2 to pi/2
  // Sine(X) = X - X^3/3! + X^5/5! - X^7/7!

  fResidue = fX;

  fSine = fX;
  fNegX = fX * (FXP)(-1);
  fFactorial = (FXP) 2; // Should implement this as a lookup

  //for ( fResidue = fReturn ; fResidue != 0 ; fResidue += fResidueTmp ) {
  //while ( fResidue != 0 ) {
  for ( i = 0 ; (i < 5) && (fResidue != 0) ; i++ ) { // It looks like i<3 works but convergence is not proven, could bound to -pi/4 and pi/4 and use cos for pi/4 to pi/2
    fResidue   = fResidue * fNegX;
    fResidue   = fResidue / fFactorial;
    fFactorial = fFactorial + (FXP) 1;
    fResidue   = fResidue * fX;
    fResidue   = fResidue / fFactorial;
    fFactorial = fFactorial + (FXP) 1;
    fSine      = fSine + fResidue;
  }

  return fSine;
}

FXP FXP_Sin (const FXP& fX) 
{
  FXP fPi;
  FXP fY;
  FXP fSin;

// NOTE: Input is in radians/pi, i.e. radians = fx * pi
// This bounds the input fX from -pi/2 to pi/2 before passing to FXP_CosSin

  fY = ((fX + (FXP)1) * ((FXP)0.5));
  fY = (FXP)2 * (int)fY;
  fY = fX - fY;
  fPi = (FXP)crPi;

  if (fY > (FXP)0.5) {
    fY   = (FXP)1 - fY;
    fSin = FXP_Sine(fY*fPi);
  } else if (fY > (FXP)-0.5) {
    fSin = FXP_Sine(fY*fPi);
  } else {
    fY   = (FXP)1 + fY;
    fSin = FXP_Sine(fY*fPi) * (FXP)(-1);
  }

  return fSin;
}


FXP FXP_Cosi (const FXP& fX) 
{
  FXP  fNegX;
  FXP fCosi;
  FXP fResidue;
  FXP fFactorial;

  int i;

  // NOTE: Input is in radians/pi, i.e. radians = fx * pi
  // The input should be bound by -pi/2 to pi/2
  // Cosi(X) = 1 - X^2/2! + X^4/4! - X^6/6!

  fResidue = fX;

  fCosi = (FXP) 1;
  fNegX = fX * (FXP)(-1);
  fFactorial = (FXP) 2; // Should implement this as a lookup

  //for ( fResidue = fReturn ; fResidue != 0 ; fResidue += fResidueTmp ) {
  //while ( fResidue != 0 ) {
  for ( i = 0 ; (i < 5) && (fResidue != 0); i++ ) {
    fResidue   = fResidue * fNegX;
    fResidue   = fResidue / fFactorial;
    fFactorial = fFactorial + (FXP) 1;
    fCosi      = fCosi + fResidue;
    fResidue   = fResidue * fX;
    fResidue   = fResidue / fFactorial;
    fFactorial = fFactorial + (FXP) 1;
  }

  return fCosi;
}


FXP FXP_Cos (const FXP& fX) {
  FXP fPi;
  FXP fY;
  FXP fCos;

// This bounds the input fX from -pi/2 to pi/2 before passing to FXP_Cosi
// TODO Get to work on the range of -PI,PI, now is -1,1

  fY = (fX + (FXP)1) * ((FXP)0.5);
  fY = (FXP)2 * (int)fY;
  fY = fX - fY;
  fPi = (FXP)crPi;   // TODO need a FXP_crPi global

  if (fY > (FXP)  0.5) {
    fY   = (FXP)1 - fY;
    fCos = (FXP)(-1)*FXP_Cosi(fY*fPi);
  } else if (fY > (FXP) -0.5) {
    fCos = FXP_Cosi(fY*fPi);
  } else {
    fY   = (FXP)1 + fY;
    fCos = (FXP)(-1)*FXP_Cosi(fY*fPi);
  }

  return fCos;

}



CMatlibMatrix<FXP> Eye_FXP(const int iLen) 
{ 
  CMatlibMatrix<FXP> matrRet(iLen, iLen, VTY_TEMP); 
 
  /* Set all values except of the diagonal to zero, diagonal entries = 1 */ 
  for (int i = 0; i < iLen; i++) 
    { 
      for (int j = 0; j < iLen; j++) 
	{ 
	  if (i == j) 
	    matrRet[i][j] = (FXP) 1.0; 
	  else 
	    matrRet[i][j] = (FXP) 0.0; 
	} 
    } 
 
  return matrRet; 
} 

CMatlibMatrix<FComplex> Diag(const CMatlibVector<FComplex>& cvI) 
{ 
  const int iSize = cvI.GetSize(); 
  CMatlibMatrix<FComplex> matcRet(iSize, iSize, VTY_TEMP); 
 
  /* Set the diagonal to the values of the input vector */ 
  for (int i = 0; i < iSize; i++) 
    { 
      for (int j = 0; j < iSize; j++) 
	{ 
	  if (i == j) 
	    matcRet[i][j] = cvI[i]; 
	  else 
	    matcRet[i][j] = (FXP) 0.0; 
	} 
    } 
 
  return matcRet; 
} 


FXP Trace(const CMatlibMatrix<FXP>& rmI) 
{ 
  const int iSize = rmI.GetRowSize(); /* matrix must be square */ 
  FXP rReturn = (FXP) 0.0; 
 
  for (int i = 0; i < iSize; i++) 
    rReturn += rmI[i][i]; 
 
  return rReturn; 
} 

CMatlibMatrix<FComplex> Transp(const CMatlibMatrix<FComplex>& cmI) 
{ 
  const int iRowSize = cmI.GetRowSize(); 
  const int iColSize = cmI.GetColSize(); 
 
  /* Swaped row and column size due to transpose operation */ 
  CMatlibMatrix<FComplex> matcRet(iColSize, iRowSize, VTY_TEMP); 
 
  /* Transpose matrix */ 
  for (int i = 0; i < iRowSize; i++) 
    { 
      for (int j = 0; j < iColSize; j++) 
	matcRet[j][i] = cmI[i][j]; 
    } 
 
  return matcRet; 
} 


CMatlibMatrix<FComplex> Inv(const CMatlibMatrix<FComplex>& matrI)
{
/*
	Parts of the following code are taken from Ptolemy
	(http://ptolemy.eecs.berkeley.edu/)

	The input matrix must be square, this is NOT checked here!
*/
	FComplex	temp;
	int			row, col, i;

	const int iSize = matrI.GetColSize();
	CMatlibMatrix<FComplex> matrRet(iSize, iSize, VTY_TEMP);

	/* Make a working copy of input matrix */
	CMatlibMatrix<FComplex> work(matrI);

	/* Set result to be the identity matrix */
	matrRet = Eye_FXP(iSize);

	for (i = 0; i < iSize; i++) 
	{
		/* Check that the element in (i,i) is not zero */
		if ((Real(work[i][i]) == 0) && (Imag(work[i][i]) == 0))
		{
			/* Swap with a row below this one that has a non-zero element
			   in the same column */
			for (row = i + 1; row < iSize; row++)
			{
				if ((Real(work[i][i]) != 0) || (Imag(work[i][i]) != 0))
					break;
			}

// TEST
if (row == iSize)
{
printf("couldn't invert matrix, possibly singular.\n");
	matrRet = Eye_FXP(iSize);
	return matrRet;
}

			/* Swap rows */
			for (col = 0; col < iSize; col++)
			{
				temp = work[i][col];
				work[i][col] = work[row][col];
				work[row][col] = temp;
				temp = matrRet[i][col];
				matrRet[i][col] = matrRet[row][col];
				matrRet[row][col] = temp;
			}
		}

		/* Divide every element in the row by element (i,i) */
		temp = work[i][i];
		for (col = 0; col < iSize; col++)
		{
			work[i][col] /= temp;
			matrRet[i][col] /= temp;
		}

		/* Zero out the rest of column i */
		for (row = 0; row < iSize; row++)
		{
			if (row != i)
			{
				temp = work[row][i];
				for (col = iSize - 1; col >= 0; col--)
				{
					work[row][col] -= (temp * work[i][col]);
					matrRet[row][col] -= (temp * matrRet[i][col]);
				}
			}
		}
	}

	return matrRet;
}

//CMatlibVector<FXP> rifft(const CMatlibVector<FComplex>& cvI, 
//			   const CFftPlans& FftPlans) 
//{ 
  /* 
        This function only works with EVEN N! 
  */ 
/*  int                     i; 
  CFftPlans*      pCurPlan; 
  fftw_real*      pFftwRealIn; 
  fftw_real*      pFftwRealOut; 
*/
//  const int       iShortLength(cvI.GetSize() - 1); /* Nyquist frequency! */ 
/*  const int       iLongLength(iShortLength * 2); 
 
  CMatlibVector<FXP> fvReturn(iLongLength, VTY_TEMP); 
*/ 
  /* If input vector is too short, return */ 
  //  if (iShortLength <= 0) 
  //  return fvReturn; 
 
  /* Check, if plans are already created, else: create it */ 
  /*  if (!FftPlans.IsInitialized()) 
    { 
      pCurPlan = new CFftPlans; 
      pCurPlan->Init(iLongLength); 
    } 
  else 
  { */
      /* Ugly, but ok: We transform "const" object in "non constant" object 
                   since we KNOW that the original object is not constant since it 
                   was already initialized! */ 
/*      pCurPlan = (CFftPlans*) &FftPlans; 
    } 
 
  pFftwRealIn = pCurPlan->pFftwRealIn; 
  pFftwRealOut = pCurPlan->pFftwRealOut; 
 */
  /* Now build half-complex-vector */ 
  /*  pFftwRealIn[0] = cvI[0].real(); 
  for (i = 1; i < iShortLength; i++) 
    { 
      pFftwRealIn[i] = cvI[i].real(); 
      pFftwRealIn[iLongLength - i] = cvI[i].imag(); 
    } 
  */
  /* Nyquist frequency */ 
  //  pFftwRealIn[iShortLength] = cvI[iShortLength].real();  
 
  /* Actual fftw call */ 
  //  rfftw_one(pCurPlan->RFFTPlBackw, pFftwRealIn, pFftwRealOut); 
 
  /* Scale output vector */ 
  /*  const FXP scale = (FXP) 1.0 / iLongLength; 
  for (i = 0; i < iLongLength; i++)  
        fvReturn[i] = pFftwRealOut[i] * scale; 
 
 if (!FftPlans.IsInitialized()) 
   delete pCurPlan; 
 
 return fvReturn; 
} 
  */


/* FftPlans implementation -------------------------------------------------- */
CMatlibVector<FComplex> Fft(const CMatlibVector<FComplex>& cvI,
							const CFftPlans& FftPlans)
{
	int				i;
	CFftPlans*		pCurPlan;
	fftw_complex*	pFftwComplexIn;
	fftw_complex*	pFftwComplexOut;

	const int				n(cvI.GetSize());

	CMatlibVector<FComplex>	cvReturn(n, VTY_TEMP);

	/* If input vector has zero length, return */
	if (n == 0)
		return cvReturn;

	/* Check, if plans are already created, else: create it */
	if (!FftPlans.IsInitialized())
	{
		pCurPlan = new CFftPlans;
		pCurPlan->Init(n);
	}
	else
	{
		/* Ugly, but ok: We transform "const" object in "non constant" object
		   since we KNOW that the original object is not constant since it
		   was already initialized! */
		pCurPlan = (CFftPlans*) &FftPlans;
	}

	pFftwComplexIn = pCurPlan->pFftwComplexIn;
	pFftwComplexOut = pCurPlan->pFftwComplexOut;

	/* fftw (Homepage: http://www.fftw.org/) */
	for (i = 0; i < n; i++)
	{
		pFftwComplexIn[i].re = cvI[i].real();
		pFftwComplexIn[i].im = cvI[i].imag();
	}

	/* Actual fftw call */
	fftw_one(pCurPlan->FFTPlForw, pFftwComplexIn, pFftwComplexOut);

	for (i = 0; i < n; i++)
		cvReturn[i] = FComplex(pFftwComplexOut[i].re, pFftwComplexOut[i].im);

	if (!FftPlans.IsInitialized())
		delete pCurPlan;

	return cvReturn;
}

CMatlibVector<FComplex> Ifft(const CMatlibVector<FComplex>& cvI,
							 const CFftPlans& FftPlans)
{
	int				i;
	CFftPlans*		pCurPlan;
	fftw_complex*	pFftwComplexIn;
	fftw_complex*	pFftwComplexOut;

	const int		n(cvI.GetSize());

	CMatlibVector<FComplex>	cvReturn(n, VTY_TEMP);

	/* If input vector has zero length, return */
	if (n == 0)
		return cvReturn;

	/* Check, if plans are already created, else: create it */
	if (!FftPlans.IsInitialized())
	{
		pCurPlan = new CFftPlans;
		pCurPlan->Init(n);
	}
	else
	{
		/* Ugly, but ok: We transform "const" object in "non constant" object
		   since we KNOW that the original object is not constant since it
		   was already initialized! */
		pCurPlan = (CFftPlans*) &FftPlans;
	}

	pFftwComplexIn = pCurPlan->pFftwComplexIn;
	pFftwComplexOut = pCurPlan->pFftwComplexOut;

	/* fftw (Homepage: http://www.fftw.org/) */
	for (i = 0; i < n; i++)
	{
		pFftwComplexIn[i].re = cvI[i].real();
		pFftwComplexIn[i].im = cvI[i].imag();
	}

	/* Actual fftw call */
	fftw_one(pCurPlan->FFTPlBackw, pFftwComplexIn, pFftwComplexOut);
	
	const FXP scale = (FXP) 1.0 / n;
	for (i = 0; i < n; i++)
	{
	  cvReturn[i] = FComplex(FXP(pFftwComplexOut[i].re) * scale,
				 FXP(pFftwComplexOut[i].im) * scale);
	}

	if (!FftPlans.IsInitialized())
		delete pCurPlan;

	return cvReturn;
}

CMatlibVector<FComplex> rfft(const CMatlibVector<FXP>& fvI,
							 const CFftPlans& FftPlans)
{
	int			i;
	CFftPlans*	pCurPlan;
	fftw_real*	pFftwRealIn;
	fftw_real*	pFftwRealOut;

	const int	iSizeI = fvI.GetSize();
	const int	iLongLength(iSizeI);
	const int	iShortLength(iLongLength / 2);
	const int	iUpRoundShortLength((iLongLength + 1) / 2);
	
	CMatlibVector<FComplex>	cvReturn(iShortLength
		/* Include Nyquist frequency in case of even N */ + 1, VTY_TEMP);

	/* If input vector has zero length, return */
	if (iLongLength == 0)
		return cvReturn;

	/* Check, if plans are already created, else: create it */
	if (!FftPlans.IsInitialized())
	{
		pCurPlan = new CFftPlans;
		pCurPlan->Init(iLongLength);
	}
	else
	{
		/* Ugly, but ok: We transform "const" object in "non constant" object
		   since we KNOW that the original object is not constant since it
		   was already initialized! */
		pCurPlan = (CFftPlans*) &FftPlans;
	}

	pFftwRealIn = pCurPlan->pFftwRealIn;
	pFftwRealOut = pCurPlan->pFftwRealOut;

	/* fftw (Homepage: http://www.fftw.org/) */
	for (i = 0; i < iSizeI; i++)
		pFftwRealIn[i] = fvI[i];

	/* Actual fftw call */
	rfftw_one(pCurPlan->RFFTPlForw, pFftwRealIn, pFftwRealOut);

	/* Now build complex output vector */
	/* Zero frequency */
	cvReturn[0] = FXP(pFftwRealOut[0]);
	for (i = 1; i < iUpRoundShortLength; i++)
		cvReturn[i] = FComplex(pFftwRealOut[i], pFftwRealOut[iLongLength - i]);

	/* If N is even, include Nyquist frequency */
	if (iLongLength % 2 == 0)
		cvReturn[iShortLength] = FXP(pFftwRealOut[iShortLength]);

	if (!FftPlans.IsInitialized())
		delete pCurPlan;

	return cvReturn;
}

CMatlibVector<FXP> rifft(const CMatlibVector<FComplex>& cvI,
						   const CFftPlans& FftPlans)
{
/*
	This function only works with EVEN N!
*/
	int			i;
	CFftPlans*	pCurPlan;
	fftw_real*	pFftwRealIn;
	fftw_real*	pFftwRealOut;

	const int	iShortLength(cvI.GetSize() - 1); /* Nyquist frequency! */
	const int	iLongLength(iShortLength * 2);

	CMatlibVector<FXP> fvReturn(iLongLength, VTY_TEMP);

	/* If input vector is too short, return */
	if (iShortLength <= 0)
		return fvReturn;

	/* Check, if plans are already created, else: create it */
	if (!FftPlans.IsInitialized())
	{
		pCurPlan = new CFftPlans;
		pCurPlan->Init(iLongLength);
	}
	else
	{
		/* Ugly, but ok: We transform "const" object in "non constant" object
		   since we KNOW that the original object is not constant since it
		   was already initialized! */
		pCurPlan = (CFftPlans*) &FftPlans;
	}

	pFftwRealIn = pCurPlan->pFftwRealIn;
	pFftwRealOut = pCurPlan->pFftwRealOut;

	/* Now build half-complex-vector */
	pFftwRealIn[0] = cvI[0].real();
	for (i = 1; i < iShortLength; i++)
	{
		pFftwRealIn[i] = cvI[i].real();
		pFftwRealIn[iLongLength - i] = cvI[i].imag();
	}
	/* Nyquist frequency */
	pFftwRealIn[iShortLength] = cvI[iShortLength].real(); 

	/* Actual fftw call */
	rfftw_one(pCurPlan->RFFTPlBackw, pFftwRealIn, pFftwRealOut);

	/* Scale output vector */
	const FXP scale = (FXP) 1.0 / iLongLength;
	for (i = 0; i < iLongLength; i++) 
	  fvReturn[i] = FXP(pFftwRealOut[i]) * scale;

	if (!FftPlans.IsInitialized())
		delete pCurPlan;

	return fvReturn;
}

CMatlibVector<FXP> FftFilt(const CMatlibVector<FComplex>& rvH,
							 const CMatlibVector<FXP>& rvI,
							 CMatlibVector<FXP>& rvZ,
							 const CFftPlans& FftPlans)
{
/*
	This function only works with EVEN N!
*/
	CFftPlans*				pCurPlan;
	const int				iL(rvH.GetSize() - 1); /* Nyquist frequency! */
	const int				iL2(2 * iL);
	CMatlibVector<FXP>	rvINew(iL2);
	CMatlibVector<FXP>	rvOutTMP(iL2);

	/* Check, if plans are already created, else: create it */
	if (!FftPlans.IsInitialized())
	{
		pCurPlan = new CFftPlans;
		pCurPlan->Init(iL2);
	}
	else
	{
		/* Ugly, but ok: We transform "const" object in "non constant" object
		   since we KNOW that the original object is not constant since it
		   was already initialized! */
		pCurPlan = (CFftPlans*) &FftPlans;
	}

	/* Update history of input signal */
	rvINew.Merge(rvZ, rvI);

	rvOutTMP = rifft(rfft(rvINew, FftPlans) * rvH, FftPlans);
 
	/* Save old input signal vector for next block */
	rvZ = rvI;

	/* Cut out correct samples (to get from cyclic convolution to linear
	   convolution) */
	return rvOutTMP(iL + 1, iL2);
}


/* FftPlans implementation -------------------------------------------------- */

