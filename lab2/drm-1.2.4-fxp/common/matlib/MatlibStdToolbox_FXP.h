/* Helpfunctions **************************************************************/
inline FXP			Min(const FXP& rA, const FXP& rB)
								{return rA < rB ? rA : rB;}
inline CMatlibVector<FXP>	Min(const CMatlibVector<FXP>& rvA, const CMatlibVector<FXP>& rvB)
								{_VECOP(FXP, rvA.GetSize(), Min(rvA[i], rvB[i]));}
FXP				Min(const CMatlibVector<FXP>& rvI);

void				Min(FXP& rMinVal /* out */, int& iMinInd /* out */,
								const CMatlibVector<FXP>& rvI /* in */);

inline FXP			Min(const FXP& r1, const FXP& r2, const FXP& r3, const FXP& r4)
								{return Min(Min(Min(r1, r2), r3), r4);}
inline FXP			Min(const FXP& r1, const FXP& r2, const FXP& r3, const FXP& r4,
								const FXP& r5, const FXP& r6, const FXP& r7, const FXP& r8)
								{return Min(Min(Min(Min(Min(Min(Min(r1, r2), r3), r4), r5), r6), r7), r8);}


inline FXP			Max(const FXP& rA, const FXP& rB)
								{return rA > rB ? rA : rB;}

inline CMatlibVector<FXP>	Max(const CMatlibVector<FXP>& rvA, const CMatlibVector<FXP>& rvB)
								{_VECOP(FXP, rvA.GetSize(), Max(rvA[i], rvB[i]));}

FXP				Max(const CMatlibVector<FXP>& rvI); 


void				Max(FXP& rMaxVal /* out */, int& iMaxInd /* out */,
								const CMatlibVector<FXP>& rvI /* in */);

inline FXP			Max(const FXP& r1, const FXP& r2, const FXP& r3)
								{return Max(Max(r1, r2), r3);}
inline FXP			Max(const FXP& r1, const FXP& r2, const FXP& r3, const FXP& r4,
								const FXP& r5, const FXP& r6, const FXP& r7)
								{return Max(Max(Max(Max(Max(Max(r1, r2), r3), r4), r5), r6), r7);}

inline CMatlibVector<FXP>	Ones_FXP(const int iLen)      	{_VECOP(FXP, iLen,  1);}
inline CMatlibVector<FXP>	Zeros_FXP(const int iLen)       {_VECOP(FXP, iLen,  0);}


inline FXP			Real(const _FCOMPLEX& cI) {return cI.real();}
inline CMatlibVector<FXP>	Real(const CMatlibVector<FComplex>& cvI)
								{_VECOP(FXP, cvI.GetSize(), Real(cvI[i]));}

inline FXP			Imag(const _FCOMPLEX& cI) {return cI.imag();}
inline CMatlibVector<FXP>	Imag(const CMatlibVector<FComplex>& cvI)
								{_VECOP(FXP, cvI.GetSize(), Imag(cvI[i]));}

inline FComplex 		Conj(const _FCOMPLEX& cI) {return conj(cI);}
inline CMatlibVector<FComplex>	Conj(const CMatlibVector<FComplex>& cvI)
									{_VECOP(FComplex, cvI.GetSize(), Conj(cvI[i]));}
inline CMatlibMatrix<FComplex>	Conj(const CMatlibMatrix<FComplex>& cmI)
									{_MATOP(FComplex, cmI.GetRowSize(), cmI.GetColSize(), Conj(cmI[i]));}


/* Absolute and angle (argument) functions */
inline FXP			Abs(const FXP& rI)   {return fabs(rI);}
inline CMatlibVector<FXP>	Abs(const CMatlibVector<FXP>& fvI)
								{_VECOP(FXP, fvI.GetSize(), Abs(fvI[i]));}

inline FXP			Abs(const FComplex& cI) {return abs(cI);}
inline CMatlibVector<FXP>	Abs(const CMatlibVector<FComplex>& cvI)
								{_VECOP(FXP, cvI.GetSize(), Abs(cvI[i]));}

inline FXP			Angle(const FComplex& cI) {return arg(cI);}
inline CMatlibVector<FXP>	Angle(const CMatlibVector<FComplex>& cvI)
								{_VECOP(FXP, cvI.GetSize(), Angle(cvI[i]));}


/* Trigonometric functions */
// NOTE: Input is in radians/pi, i.e. radians = fx * pi
FXP      			FXP_Sin(const FXP& fX);
FXP      			FXP_Cos(const FXP& fX);


/* Mean, variance and standard deviation */


/* Rounding functions */

CMatlibVector<FXP>		Sort(const CMatlibVector<FXP>& rvI); 
CMatlibVector<int>		Sort(const CMatlibVector<int>& rvI);

/* Matrix inverse */
CMatlibMatrix<FComplex>         Inv(const CMatlibMatrix<FComplex>& matrI); 


/* Identity matrix */
CMatlibMatrix<FXP>		Eye_FXP(const int iLen); 

CMatlibMatrix<FComplex>         Diag(const CMatlibVector<FComplex>& cvI); 


FXP				Trace(const CMatlibMatrix<FXP>& rmI); 

/* Matrix transpose */
CMatlibMatrix<FComplex>         Transp(const CMatlibMatrix<FComplex>& cmI); 
inline 
CMatlibMatrix<FComplex>         TranspH(const CMatlibMatrix<FComplex>& cmI) 
                                                                {return Conj(Transp(cmI));} /* With conjugate complex */ 


/* Fourier transformations (also included: real FFT) */
CMatlibVector<FComplex>		Fft(const CMatlibVector<FComplex>& cvI, const CFftPlans& FftPlans = CFftPlans());
CMatlibVector<FComplex>		Ifft(const CMatlibVector<FComplex>& cvI, const CFftPlans& FftPlans = CFftPlans());
CMatlibVector<FComplex>		rfft(const CMatlibVector<FXP>& fvI, const CFftPlans& FftPlans = CFftPlans());
CMatlibVector<FXP>		rifft(const CMatlibVector<FComplex>& cvI, const CFftPlans& FftPlans = CFftPlans());

CMatlibVector<FXP>		FftFilt(const CMatlibVector<FComplex>& rvH,
									const CMatlibVector<FXP>& rvI,
									CMatlibVector<FXP>& rvZ,
									const CFftPlans& FftPlans = CFftPlans());

/* Numerical integration */
typedef FComplex(MATLIB_CALLBACK_QAUD_FXP)(FXP rX); /* Callback function definition */
FComplex					Quad(MATLIB_CALLBACK_QAUD_FXP f, const FXP a,
								 const FXP b, const FXP errorBound = 1.e-6);

/* Implementation **************************************************************
   (the implementation of template classes must be in the header file!) */
