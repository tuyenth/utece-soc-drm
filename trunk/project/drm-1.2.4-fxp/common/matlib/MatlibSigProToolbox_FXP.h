/* Helpfunctions **************************************************************/
/* Randomize functions */
CMatlibVector<FXP>	Randn_FXP(const int iLen);
inline
CMatlibVector<FXP>	Rand_FXP(const int iLen)
							{_VECOP(FXP, iLen, (FXP) rand() / RAND_MAX);}


/* Window functions */
CMatlibVector<FXP>	Hann_FXP(const int iLen);
CMatlibVector<FXP>	Hamming_FXP(const int iLen);
CMatlibVector<FXP>	Nuttallwin_FXP(const int iLen);
CMatlibVector<FXP>	Bartlett_FXP(const int iLen);
CMatlibVector<FXP>	Triang_FXP(const int iLen);
CMatlibVector<FXP>	Kaiser(const int iLen, const FXP rBeta);


/* Bessel function */
FXP			Besseli(const FXP rNu, const FXP rZ);


/* Filter data with a recursive (IIR) or nonrecursive (FIR) filter */
CMatlibVector<FXP>	Filter(const CMatlibVector<FXP>& fvB, 
			   const CMatlibVector<FXP>& fvA, 
			   const CMatlibVector<FXP>& fvX, 
			   CMatlibVector<FXP>& fvZ);


/* Levinson durbin recursion */
CMatlibVector<FXP>	Levinson(const CMatlibVector<FXP>& vecrRx, 
				 const CMatlibVector<FXP>& vecrB);
CMatlibVector<FComplex>	Levinson(const CMatlibVector<FComplex>& veccRx, 
				 const CMatlibVector<FComplex>& veccB);


/* Sinc-function */
FXP			FXP_Sinc(const FXP& rI);  /* true FXP approximation */
/* Skipped as they are not going to work in FXP.
inline FXP		Sinc(const FXP& rI)       
							{return rI == (FXP) 0.0 ? (FXP) 1.0 : (FXP) sin(crPi * (_REAL)rI) / (crPi * (_REAL)rI);}
inline
CMatlibVector<FXP>	Sinc(const CMatlibVector<FXP>& fvI)
							{_VECOP(FXP, fvI.GetSize(), Sinc(fvI[i]));}
*/

/* My own functions --------------------------------------------------------- */
/* Lowpass filter design using windowing method */
CMatlibVector<FXP>	FirLP(const FXP rNormBW,
			  const CMatlibVector<FXP>& rvWin);

/* Complex FIR filter with decimation */
CMatlibVector<FComplex>	FirFiltDec(const CMatlibVector<FComplex>& cvB,
				   const CMatlibVector<FComplex>& cvX,
				   CMatlibVector<FComplex>& cvZ,
				   const int iDecFact);

/* Squared magnitude */
inline FXP		SqMag(const FComplex& cI)
							{return cI.real() * cI.real() + cI.imag() * cI.imag();}
inline FXP		SqMag(const FXP& rI)
							{return rI * rI;}
inline
CMatlibVector<FXP>	SqMag(const CMatlibVector<FComplex>& veccI)
							{_VECOP(FXP, veccI.GetSize(), SqMag(veccI[i]));}
inline
CMatlibVector<FXP>	SqMag(const CMatlibVector<FXP>& vecrI)
							{_VECOP(FXP, vecrI.GetSize(), SqMag(vecrI[i]));}

/* One pole recursion (first order IIR)
   y_n = lambda * y_{n - 1} + (1 - lambda) * x_n */

inline void		IIR1(FXP& rY, const FXP& rX, const FXP rLambda)
							{rY = rLambda * (rY - rX) + rX;}

inline void		IIR1(FComplex& cY, const FComplex& cX, const FXP rLambda)
							{cY = rLambda * (cY - cX) + cX;}
 
inline void		IIR1(CMatlibVector<FXP>& rY,
			 const CMatlibVector<FXP>& rX,
			 const FXP rLambda)
{
	const int iSize = rY.GetSize();

	for (int i = 0; i < iSize; i++)
		IIR1(rY[i], rX[i], rLambda);
}

/* Two-sided one pole recursion */
inline void		IIR1TwoSided(FXP& rY, const FXP& rX,
			 const FXP rLamUp, const FXP rLamDown)
							{rX > rY ? IIR1(rY, rX, rLamUp) : IIR1(rY, rX, rLamDown);}

/* Get lambda for one-pole recursion from time constant */
inline FXP		IIR1Lam(const FXP& rTau, const FXP& rFs)
							{return exp((FXP) -1.0 / (rTau * rFs));}


