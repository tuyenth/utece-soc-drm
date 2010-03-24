/******************************************************************************\
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	c++ Mathematic Library (Matlib)
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

#ifndef _MATLIB_H_
#define _MATLIB_H_

#include <math.h>
#include "../drm_util/FXP.h"
#include <complex>
using namespace std;
#include "../GlobalDefinitions.h"

/* Definitions ****************************************************************/
/* Two different types: constant and temporary buffer */
enum EVecTy {VTY_CONST, VTY_TEMP};


/* These definitions save a lot of redundant code */
#define _VECOP(TYPE, LENGTH, FCT)	const int iL = LENGTH; \
									CMatlibVector<TYPE> vecRet(iL, VTY_TEMP); \
									for (int i = 0; i < iL; i++) \
										vecRet[i] = FCT; \
									return vecRet

#define _VECOPCL(FCT)				for (int i = 0; i < iVectorLength; i++) \
										operator[](i) FCT; \
									return *this

#define _MATOP(TYPE, LENGTHROW, LENGTHCOL, FCT) \
									const int iRL = LENGTHROW; \
									CMatlibMatrix<TYPE> matRet(iRL, LENGTHCOL, VTY_TEMP); \
									for (int i = 0; i < iRL; i++) \
										matRet[i] = FCT; \
									return matRet

#define _MATOPCL(FCT)				for (int i = 0; i < iRowSize; i++) \
										operator[](i) FCT; \
									return *this


/* In debug mode, test input parameters */
#ifdef _DEBUG_
#define _TESTRNGR(POS)		if ((POS >= iVectorLength) || (POS < 0)) \
								DebugError("MatLibrRead", "POS", POS, \
								"iVectorLength", iVectorLength)
#define _TESTRNGW(POS)		if ((POS >= iVectorLength) || (POS < 0)) \
								DebugError("MatLibrWrite", "POS", POS, \
								"iVectorLength", iVectorLength)
#define _TESTSIZE(INP)		if (INP != iVectorLength) \
								DebugError("SizeCheck", "INP", INP, \
								"iVectorLength", iVectorLength)
#define _TESTRNGRM(POS)		if ((POS >= iRowSize) || (POS < 0)) \
								DebugError("MatLibrReadMatrix", "POS", POS, \
								"iRowSize", iRowSize)
#define _TESTRNGWM(POS)		if ((POS >= iRowSize) || (POS < 0)) \
								DebugError("MatLibrWriteMatrix", "POS", POS, \
								"iRowSize", iRowSize)
#define _TESTSIZEM(INP)		if (INP != iRowSize) \
								DebugError("MatLibOperatorMatrix=()", "INP", INP, \
								"iRowSize", iRowSize)
#else
#define _TESTRNGR(POS)
#define _TESTRNGW(POS)
#define _TESTSIZE(INP)
#define _TESTRNGRM(POS)
#define _TESTRNGWM(POS)
#define _TESTSIZEM(INP)
#endif


/* Some other definitions */
#define For(a, b, c)		for (a = (b); a <= (c); a++) {
#define If(a)				if (a) {
#define Else				} else {
#define End					}


/* Classes ********************************************************************/
/* Prototypes */
template<class T> class			CMatlibVector;
template<class T> class			CMatlibMatrix;

/* Here we can choose the precision of the Matlib calculations */
typedef _REAL			CReal;
typedef complex<CReal>		CComplex;
typedef FXP 			CFix;
typedef complex<FXP>		FComplex;
typedef CMatlibVector<CReal>	CRealVector;
typedef CMatlibVector<FXP>	FXPVector;
typedef CMatlibVector<CComplex>	CComplexVector;
typedef CMatlibVector<FComplex>	FComplexVector;
typedef CMatlibMatrix<CReal>	CRealMatrix;
typedef CMatlibMatrix<FXP>	FXPMatrix;
typedef CMatlibMatrix<CComplex>	CComplexMatrix;
typedef CMatlibMatrix<FComplex>	FComplexMatrix;

typedef FXP                      CFReal;
typedef complex<CFReal>          CFComplex;

typedef CMatlibVector<CFReal>    CFRealVector;
typedef CMatlibVector<CFComplex> CFComplexVector;
typedef CMatlibMatrix<CFReal>    CFRealMatrix;
typedef CMatlibMatrix<CFComplex> CFComplexMatrix;

/******************************************************************************/
/* CMatlibVector class ********************************************************/
/******************************************************************************/
template<class T>
class CMatlibVector
{
public:
	/* Construction, Destruction -------------------------------------------- */
	CMatlibVector() : iVectorLength(0), pData(NULL), eVType(VTY_CONST) {}
	CMatlibVector(const int iNLen, const EVecTy eNTy = VTY_CONST) : 
		iVectorLength(0), pData(NULL), eVType(eNTy) {Init(iNLen);}
	CMatlibVector(const int iNLen, const T tIniVal) : 
		iVectorLength(0), pData(NULL), eVType(VTY_CONST) {Init(iNLen, tIniVal);}
	CMatlibVector(CMatlibVector<T>& vecI);
	CMatlibVector(const CMatlibVector<T>& vecI);
	virtual ~CMatlibVector() {if (pData != NULL) delete[] pData;}

	CMatlibVector(const CMatlibVector<CReal>& fvReal, const CMatlibVector<CReal>& fvImag) : 
		iVectorLength(fvReal.GetSize()), pData(NULL), eVType(VTY_CONST/*VTY_TEMP*/)
	{
		/* Allocate data block for vector */
//		pData = new CComplex[iVectorLength];
               CComplex* pComplex = new CComplex[iVectorLength];
		/* Copy data from real-vectors in complex vector */
		for (int i = 0; i < iVectorLength; i++)
			pComplex[i] = CComplex(fvReal[i], fvImag[i]);
		pData = (T*)((void*) pComplex);	
	}

		// fix point 

	CMatlibVector(const CMatlibVector<FXP>& fvReal, const CMatlibVector<FXP>& fvImag) : 
		iVectorLength(fvReal.GetSize()), pData(NULL), eVType(VTY_CONST/*VTY_TEMP*/)
	{
		/* Allocate data block for vector */
//		pData = new FComplex[iVectorLength];
                FComplex* pComplex =new FComplex[iVectorLength];
		/* Copy data from real-vectors in complex vector */
		for (int i = 0; i < iVectorLength; i++)
			pComplex[i] = FComplex(fvReal[i], fvImag[i]);
		pData = (T*)((void*) pComplex);
	}


	/* Operator[] (Regular indices!!!) */
	inline T& operator[](int const iPos) const
		{_TESTRNGR(iPos); return pData[iPos];}
	inline T& operator[](int const iPos)
		{_TESTRNGW(iPos); return pData[iPos];} // For use as l value

	/* Operator() */
	inline T& operator()(int const iPos) const
		{_TESTRNGR(iPos - 1); return pData[iPos - 1];}
	inline T& operator()(int const iPos)
		{_TESTRNGW(iPos - 1); return pData[iPos - 1];} // For use as l value

	CMatlibVector<T> operator()(const int iFrom, const int iTo) const;
	CMatlibVector<T> operator()(const int iFrom, const int iStep, const int iTo) const;

	inline int GetSize() const {return iVectorLength;}
	void Init(const int iIniLen, const T tIniVal =T(0));
	inline CMatlibVector<T>& Reset(const T tResVal = T(0)) {_VECOPCL(= tResVal);}
	CMatlibVector<T>& PutIn(const int iFrom, const int iTo, CMatlibVector<T>& fvA);
	CMatlibVector<T>& Merge(const CMatlibVector<T>& vecA, T& tB);
	CMatlibVector<T>& Merge(const CMatlibVector<T>& vecA, const CMatlibVector<T>& vecB);
	CMatlibVector<T>& Merge(const CMatlibVector<T>& vecA, const CMatlibVector<T>& vecB,
		const CMatlibVector<T>& vecC);


	/* operator= */
	inline CMatlibVector<T>&		operator=(const CMatlibVector<CReal>& vecI)
		{_TESTSIZE(vecI.GetSize()); _VECOPCL(= vecI[i]);}
	inline CMatlibVector<T>&		operator=(const CMatlibVector<FXP>& vecI)
		{_TESTSIZE(vecI.GetSize()); _VECOPCL(= vecI[i]);}
	inline CMatlibVector<T>&		operator=(const CMatlibVector<int>& vecI)
		{_TESTSIZE(vecI.GetSize()); _VECOPCL(= vecI[i]);}

	inline CMatlibVector<T>&	operator=(const CMatlibVector<CComplex>& vecI)
		{_TESTSIZE(vecI.GetSize()); _VECOPCL(= vecI[i]);}
	inline CMatlibVector<FComplex>& operator=(const CMatlibVector<FComplex>& vecI) 
	  {_TESTSIZE(vecI.GetSize()); _VECOPCL(= vecI[i]);} 


	/* operator*= */
	inline CMatlibVector<T>&		operator*=(const CReal& rI)
		{_VECOPCL(*= rI);}
	inline CMatlibVector<T>&		operator*=(const FXP& rI)
		{_VECOPCL(*= rI);}

	inline CMatlibVector<T>&		operator*=(const CComplex& cI)
		{_VECOPCL(*= cI);}
	inline CMatlibVector<T>& 		operator*=(const FComplex& cI) 
        	{_VECOPCL(*= cI);} 

	inline CMatlibVector<T>&		operator*=(const CMatlibVector<CReal>& vecI)
		{_VECOPCL(*= vecI[i]);}
	inline CMatlibVector<T>&		operator*=(const CMatlibVector<FXP>& vecI)
		{_VECOPCL(*= vecI[i]);}	

	inline CMatlibVector<T>&		operator*=(const CMatlibVector<CComplex>& vecI)
		{_VECOPCL(*= vecI[i]);}
        inline CMatlibVector<T>& 		operator*=(const CMatlibVector<FComplex>& vecI) 
		{_VECOPCL(*= vecI[i]);} 


	/* operator/= */
	inline CMatlibVector<T>&		operator/=(const CReal& rI)
		{_VECOPCL(/= rI);}
	inline CMatlibVector<T>&		operator/=(const FXP& rI)
		{_VECOPCL(/= rI);}

	inline CMatlibVector<T>&		operator/=(const CComplex& cI)
		{_VECOPCL(/= cI);}

	inline CMatlibVector<T>& 		operator/=(const FComplex& cI) 
	{_VECOPCL(/= cI);} 


	inline CMatlibVector<T>&		operator/=(const CMatlibVector<CReal>& vecI)
		{_VECOPCL(/= vecI[i]);}
	inline CMatlibVector<T>&		operator/=(const CMatlibVector<FXP>& vecI)
		{_VECOPCL(/= vecI[i]);}

	inline CMatlibVector<T>&		operator/=(const CMatlibVector<CComplex>& vecI)
		{_VECOPCL(/= vecI[i]);}
	inline CMatlibVector<T>& 		operator/=(const CMatlibVector<FComplex>& vecI) 
		{_VECOPCL(/= vecI[i]);} 

	/* operator+= */
	inline CMatlibVector<T>&		operator+=(const CReal& rI)
		{_VECOPCL(+= rI);}
	inline CMatlibVector<T>&		operator+=(const FXP& rI)
		{_VECOPCL(+= rI);}
	inline CMatlibVector<T>&		operator+=(const int& rI)
		{_VECOPCL(+= rI);}

	inline CMatlibVector<T>&		operator+=(const CComplex& cI)
		{_VECOPCL(+= cI);}
	inline CMatlibVector<T>&	 	operator+=(const FComplex& cI) 
        	{_VECOPCL(+= cI);} 


	inline CMatlibVector<T>&		operator+=(const CMatlibVector<CReal>& vecI)
		{_VECOPCL(+= vecI[i]);}
	inline CMatlibVector<T>&		operator+=(const CMatlibVector<FXP>& vecI)
		{_VECOPCL(+= vecI[i]);}
	inline CMatlibVector<T>&		operator+=(const CMatlibVector<int>& vecI)
		{_VECOPCL(+= vecI[i]);}

	inline CMatlibVector<T>&		operator+=(const CMatlibVector<CComplex>& vecI)
		{_VECOPCL(+= vecI[i]);}
        inline CMatlibVector<T>& 		operator+=(const CMatlibVector<FComplex>& vecI) 
	{_VECOPCL(+= vecI[i]);} 


	/* operator-= */
	inline CMatlibVector<T>&		operator-=(const CReal& rI)
		{_VECOPCL(-= rI);}
	inline CMatlibVector<T>&		operator-=(const FXP& rI)
		{_VECOPCL(-= rI);}
	inline CMatlibVector<T>&		operator-=(const int& rI)
		{_VECOPCL(-= rI);}

	inline CMatlibVector<T>&		operator-=(const CComplex& cI)
		{_VECOPCL(-= cI);}
	inline CMatlibVector<T>& 		operator-=(const FComplex& cI) 
	{_VECOPCL(-= cI);} 


	inline CMatlibVector<T>&		operator-=(const CMatlibVector<CReal>& vecI)
		{_VECOPCL(-= vecI[i]);}
	inline CMatlibVector<T>&		operator-=(const CMatlibVector<FXP>& vecI)
		{_VECOPCL(-= vecI[i]);}
	inline CMatlibVector<T>&		operator-=(const CMatlibVector<int>& vecI)
		{_VECOPCL(-= vecI[i]);}

	inline CMatlibVector<T>&		operator-=(const CMatlibVector<CComplex>& vecI)
		{_VECOPCL(-= vecI[i]);}
        inline CMatlibVector<T>& 		operator-=(const CMatlibVector<FComplex>& vecI) 
	{_VECOPCL(-= vecI[i]);} 


	//protected:
	EVecTy	eVType;
	int		iVectorLength;
	T*		pData;
};


/* Help functions *************************************************************/
template<class T> inline
int Size(const CMatlibVector<T>& vecI) {return vecI.GetSize();}
template<class T> inline
int Length(const CMatlibVector<T>& vecI) {return vecI.GetSize();}

/* operator* ---------------------------------------------------------------- */
inline CMatlibVector<CComplex> // cv, cv
	operator*(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] * cvB[i]);}
inline CMatlibVector<FComplex> // cv, cv 
        operator*(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] * cvB[i]);} 


inline CMatlibVector<CReal> // rv, rv
	operator*(const CMatlibVector<CReal>& rvA, const CMatlibVector<CReal>& rvB)
	{_VECOP(CReal, rvA.GetSize(), rvA[i] * rvB[i]);}
inline CMatlibVector<FXP> // rv, rv
	operator*(const CMatlibVector<FXP>& rvA, const CMatlibVector<FXP>& rvB)
	{_VECOP(FXP, rvA.GetSize(), rvA[i] * rvB[i]);}


inline CMatlibVector<CComplex> // cv, rv
	operator*(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CReal>& rvB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] * rvB[i]);}
inline CMatlibVector<FComplex> // cv, rv
	operator*(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FXP>& rvB)
	{_VECOP(FComplex, cvA.GetSize(), cvA[i] * rvB[i]);}

inline CMatlibVector<CComplex> // rv, cv
	operator*(const CMatlibVector<CReal>& rvB, const CMatlibVector<CComplex>& cvA)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] * rvB[i]);}
inline CMatlibVector<FComplex> // rv, cv 
        operator*(const CMatlibVector<FXP>& rvB, const CMatlibVector<FComplex>& cvA) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] * rvB[i]);} 


template<class T> inline
CMatlibVector<T> // Tv, r
	operator*(const CMatlibVector<T>& vecA, const CReal& rB)
	{_VECOP(T, vecA.GetSize(), vecA[i] * rB);}
template<class T> inline
CMatlibVector<T> // Tv, r 
        operator*(const CMatlibVector<T>& vecA, const FXP& rB) 
        {_VECOP(T, vecA.GetSize(), vecA[i] * rB);} 


template<class T> inline
CMatlibVector<T> // r, Tv
	operator*(const CReal& rA, const CMatlibVector<T>& vecB)
	{_VECOP(T, vecB.GetSize(), rA * vecB[i]);}
template<class T> inline 
CMatlibVector<T> // r, Tv 
        operator*(const FXP& rA, const CMatlibVector<T>& vecB) 
        {_VECOP(T, vecB.GetSize(), rA * vecB[i]);} 


inline CMatlibVector<CComplex> // cv, c
	operator*(const CMatlibVector<CComplex>& cvA, const CComplex& cB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] * cB);}
inline CMatlibVector<FComplex> // cv, c 
        operator*(const CMatlibVector<FComplex>& cvA, const FComplex& cB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] * cB);} 


inline CMatlibVector<CComplex> // c, cv
	operator*(const CComplex& cA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvB.GetSize(), cA * cvB[i]);}
inline CMatlibVector<FComplex> // c, cv 
        operator*(const FComplex& cA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvB.GetSize(), cA * cvB[i]);} 


/* operator/ ---------------------------------------------------------------- */

inline CMatlibVector<CComplex> // cv, cv
	operator/(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] / cvB[i]);}
inline CMatlibVector<FComplex> // cv, cv 
        operator/(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] / cvB[i]);} 


inline CMatlibVector<CReal> // rv, rv
	operator/(const CMatlibVector<CReal>& rvA, const CMatlibVector<CReal>& rvB)
	{_VECOP(CReal, rvA.GetSize(), rvA[i] / rvB[i]);}
inline CMatlibVector<FXP> // rv, rv 
        operator/(const CMatlibVector<FXP>& rvA, const CMatlibVector<FXP>& rvB) 
        {_VECOP(FXP, rvA.GetSize(), rvA[i] / rvB[i]);} 


inline CMatlibVector<CComplex> // cv, rv
	operator/(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CReal>& rvB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] / rvB[i]);}
inline CMatlibVector<FComplex> // cv, rv 
        operator/(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FXP>& rvB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] / rvB[i]);} 



inline CMatlibVector<CComplex> // rv, cv
	operator/(const CMatlibVector<CReal>& rvA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, rvA.GetSize(), rvA[i] / cvB[i]);}
inline CMatlibVector<FComplex> // rv, cv 
        operator/(const CMatlibVector<FXP>& rvA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, rvA.GetSize(), rvA[i] / cvB[i]);} 


template<class T> inline
CMatlibVector<T> // Tv, r
	operator/(const CMatlibVector<T>& vecA, const CReal& rB)
	{_VECOP(T, vecA.GetSize(), vecA[i] / rB);}
template<class T> inline 
CMatlibVector<T> // Tv, r 
        operator/(const CMatlibVector<T>& vecA, const FXP& rB) 
        {_VECOP(T, vecA.GetSize(), vecA[i] / rB);} 


template<class T> inline
CMatlibVector<T> // r, Tv
	operator/(const CReal& rA, const CMatlibVector<T>& vecB)
	{_VECOP(T, vecB.GetSize(), rA / vecB[i]);}
template<class T> inline 
CMatlibVector<T> // r, Tv 
        operator/(const FXP& rA, const CMatlibVector<T>& vecB) 
        {_VECOP(T, vecB.GetSize(), rA / vecB[i]);} 


inline CMatlibVector<CComplex> // cv, c
	operator/(const CMatlibVector<CComplex>& cvA, const CComplex& cB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] / cB);}
inline CMatlibVector<FComplex> // cv, c 
        operator/(const CMatlibVector<FComplex>& cvA, const FComplex& cB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] / cB);} 

inline CMatlibVector<CComplex> // c, cv
	operator/(const CComplex& cA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvB.GetSize(), cA / cvB[i]);}
inline CMatlibVector<FComplex> // c, cv 
        operator/(const FComplex& cA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvB.GetSize(), cA / cvB[i]);} 


/* operator+ ---------------------------------------------------------------- */
inline CMatlibVector<CComplex> // cv, cv
	operator+(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CComplex>& cvB) 
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] + cvB[i]);}
inline CMatlibVector<FComplex> // cv, cv 
        operator+(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FComplex>& cvB)  
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] + cvB[i]);} 


inline CMatlibVector<CReal> // rv, rv
	operator+(const CMatlibVector<CReal>& rvA, const CMatlibVector<CReal>& rvB) 
	{_VECOP(CReal, rvA.GetSize(), rvA[i] + rvB[i]);}
inline CMatlibVector<FXP> // rv, rv 
        operator+(const CMatlibVector<FXP>& rvA, const CMatlibVector<FXP>& rvB)  
        {_VECOP(FXP, rvA.GetSize(), rvA[i] + rvB[i]);} 


inline CMatlibVector<CComplex> // cv, rv
	operator+(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CReal>& rvB) 
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] + rvB[i]);}
inline CMatlibVector<FComplex> // cv, rv 
        operator+(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FXP>& rvB)  
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] + rvB[i]);} 


inline CMatlibVector<CComplex> // rv, cv
	operator+(const CMatlibVector<CReal>& rvA, const CMatlibVector<CComplex>& cvB) 
	{_VECOP(CComplex, rvA.GetSize(), rvA[i] + cvB[i]);}
inline CMatlibVector<FComplex> // rv, cv 
        operator+(const CMatlibVector<FXP>& rvA, const CMatlibVector<FComplex>& cvB)  
        {_VECOP(FComplex, rvA.GetSize(), rvA[i] + cvB[i]);} 


template<class T> inline
CMatlibVector<T> // Tv, r
	operator+(const CMatlibVector<T>& vecA, const CReal& rB)
	{_VECOP(T, vecA.GetSize(), vecA[i] + rB);}
template<class T> inline 
CMatlibVector<T> // Tv, r 
        operator+(const CMatlibVector<T>& vecA, const FXP& rB) 
        {_VECOP(T, vecA.GetSize(), vecA[i] + rB);} 

template<class T> inline
CMatlibVector<T> // r, Tv
	operator+(const CReal& rA, const CMatlibVector<T>& vecB)
	{_VECOP(T, vecB.GetSize(), rA + vecB[i]);}
template<class T> inline 
CMatlibVector<T> // r, Tv 
        operator+(const FXP& rA, const CMatlibVector<T>& vecB) 
        {_VECOP(T, vecB.GetSize(), rA + vecB[i]);} 


inline CMatlibVector<CComplex> // cv, c
	operator+(const CMatlibVector<CComplex>& cvA, const CComplex& cB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] + cB);}
inline CMatlibVector<FComplex> // cv, c 
        operator+(const CMatlibVector<FComplex>& cvA, const FComplex& cB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] + cB);} 

inline CMatlibVector<CComplex> // c, cv
	operator+(const CComplex& cA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvB.GetSize(), cA + cvB[i]);}
inline CMatlibVector<FComplex> // c, cv 
        operator+(const FComplex& cA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvB.GetSize(), cA + cvB[i]);} 


/* operator- ---------------------------------------------------------------- */
inline CMatlibVector<CComplex> // cv, cv
	operator-(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] - cvB[i]);}
inline CMatlibVector<FComplex> // cv, cv 
        operator-(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] - cvB[i]);} 

inline CMatlibVector<CReal> // rv, rv
	operator-(const CMatlibVector<CReal>& rvA, const CMatlibVector<CReal>& rvB)
	{_VECOP(CReal, rvA.GetSize(), rvA[i] - rvB[i]);}
inline CMatlibVector<FXP> // rv, rv 
        operator-(const CMatlibVector<FXP>& rvA, const CMatlibVector<FXP>& rvB) 
        {_VECOP(FXP, rvA.GetSize(), rvA[i] - rvB[i]);} 


inline CMatlibVector<CComplex> // cv, rv
	operator-(const CMatlibVector<CComplex>& cvA, const CMatlibVector<CReal>& rvB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] - rvB[i]);}
inline CMatlibVector<FComplex> // cv, rv 
        operator-(const CMatlibVector<FComplex>& cvA, const CMatlibVector<FXP>& rvB) 
        {_VECOP(FComplex, cvA.GetSize(), cvA[i] - rvB[i]);} 

inline CMatlibVector<CComplex> // rv, cv
	operator-(const CMatlibVector<CReal>& rvA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, rvA.GetSize(), rvA[i] - cvB[i]);}
inline CMatlibVector<FComplex> // rv, cv 
       operator-(const CMatlibVector<FXP>& rvA, const CMatlibVector<FComplex>& cvB) 
       {_VECOP(FComplex, rvA.GetSize(), rvA[i] - cvB[i]);} 


template<class T> inline
CMatlibVector<T> // Tv, r
	operator-(const CMatlibVector<T>& vecA, const CReal& rB)
	{_VECOP(T, vecA.GetSize(), vecA[i] - rB);}
template<class T> inline 
CMatlibVector<T> // Tv, r 
        operator-(const CMatlibVector<T>& vecA, const FXP& rB) 
        {_VECOP(T, vecA.GetSize(), vecA[i] - rB);} 

template<class T> inline
CMatlibVector<T> // r, Tv
	operator-(const CReal& rA, const CMatlibVector<T>& vecB)
	{_VECOP(T, vecB.GetSize(), rA - vecB[i]);}
template<class T> inline 
CMatlibVector<T> // r, Tv 
        operator-(const FXP& rA, const CMatlibVector<T>& vecB) 
        {_VECOP(T, vecB.GetSize(), rA - vecB[i]);} 


inline CMatlibVector<CComplex> // cv, c
	operator-(const CMatlibVector<CComplex>& cvA, const CComplex& cB)
	{_VECOP(CComplex, cvA.GetSize(), cvA[i] - cB);}
inline CMatlibVector<FComplex> // cv, c 
       operator-(const CMatlibVector<FComplex>& cvA, const FComplex& cB) 
       {_VECOP(FComplex, cvA.GetSize(), cvA[i] - cB);} 

inline CMatlibVector<CComplex> // c, cv
	operator-(const CComplex& cA, const CMatlibVector<CComplex>& cvB)
	{_VECOP(CComplex, cvB.GetSize(), cA - cvB[i]);}
inline CMatlibVector<FComplex> // c, cv 
        operator-(const FComplex& cA, const CMatlibVector<FComplex>& cvB) 
        {_VECOP(FComplex, cvB.GetSize(), cA - cvB[i]);} 


/* Implementation **************************************************************
   (the implementation of template classes must be in the header file!) */
template<class T>
CMatlibVector<T>::CMatlibVector(CMatlibVector<T>& vecI) :
	iVectorLength(vecI.GetSize()), pData(NULL), eVType(VTY_CONST/*VTY_TEMP*/)
{
	/* The copy constructor for the constant vector is a real copying
	   task. But in the case of a temporary buffer only the pointer
	   of the temporary buffer is used. The buffer of the temporary
	   vector is then destroyed!!! Therefore the usage of "VTY_TEMP"
	   should be done if the vector IS NOT USED IN A FUNCTION CALL, 
	   otherwise this vector will be destroyed afterwards (if the 
	   function argument is not declared with "&") */
	if (iVectorLength > 0)
	{
		if (vecI.eVType == VTY_CONST)
		{
			/* Allocate data block for vector */
			pData = new T[iVectorLength];

			/* Copy vector */
			for (int i = 0; i < iVectorLength; i++)
				pData[i] = vecI[i];
		}
		else
		{
			/* We can define the copy constructor as a destroying operator of
			   the input vector for performance reasons. This
			   saves us from always copy the entire vector */
			/* Take data pointer from input vector (steal it) */
			pData = vecI.pData;

			/* Destroy other vector (temporary vectors only) */
			vecI.pData = NULL;
		}
	}
}

/* Copy constructor for constant Matlib vectors */
template<class T>
CMatlibVector<T>::CMatlibVector(const CMatlibVector<T>& vecI) : 
	iVectorLength(vecI.GetSize()), pData(NULL), eVType(VTY_CONST)
{
	if (iVectorLength > 0)
	{
		/* Allocate data block for vector */
		pData = new T[iVectorLength];

		/* Copy vector */
		for (int i = 0; i < iVectorLength; i++)
			pData[i] = vecI[i];
	}
}

template<class T>
void CMatlibVector<T>::Init(const int iIniLen, const T tIniVal)
{
	iVectorLength = iIniLen;

	/* Allocate data block for vector */
	if (iVectorLength > 0)
	{
		if (pData != NULL)
			delete[] pData;

		pData = new T[iVectorLength];

		/* Init with init value */
		for (int i = 0; i < iVectorLength; i++)
			pData[i] = tIniVal;
	}
}

template<class T> inline
CMatlibVector<T> CMatlibVector<T>::operator()(const int iFrom,
						  const int iTo) const
{
	/* This is also capable of "wrap around" blocks (if the value in "iFrom" is
	   larger then the "iTo" value) */
	int			i;
	const int	iStartVal = iFrom - 1;

	if (iFrom > iTo)
	{
		/* Wrap around case */
		CMatlibVector<T> vecRet(iVectorLength - iStartVal + iTo, VTY_TEMP);

		int iCurPos = 0;

		for (i = iStartVal; i < iVectorLength; i++)
			vecRet[iCurPos++] = operator[](i);

		for (i = 0; i < iTo; i++)
			vecRet[iCurPos++] = operator[](i);

		return vecRet;
	}
	else
	{
		CMatlibVector<T> vecRet(iTo - iStartVal, VTY_TEMP);

		for (i = iStartVal; i < iTo; i++)
			vecRet[i - iStartVal] = operator[](i);

		return vecRet;
	}
}

template<class T> inline
CMatlibVector<T> CMatlibVector<T>::operator()(const int iFrom,
						  const int iStep,
						  const int iTo) const
{
	CMatlibVector<T> vecRet(abs(iTo - iFrom) / abs(iStep) + 1, VTY_TEMP);
	int iOutPos = 0;
	int i;

	if (iFrom > iTo)
	{
		const int iEnd = iTo - 2;
		for (i = iFrom - 1; i > iEnd; i += iStep)
		{
			vecRet[iOutPos] = operator[](i);
			iOutPos++;
		}
	}
	else
	{
		for (i = iFrom - 1; i < iTo; i += iStep)
		{
			vecRet[iOutPos] = operator[](i);
			iOutPos++;
		}
	}

	return vecRet;
}

template<class T> inline
CMatlibVector<T>& CMatlibVector<T>::PutIn(const int iFrom,
					  const int iTo,
					  CMatlibVector<T>& vecI)
{
	const int iStart = iFrom - 1;
	const int iEnd = iTo - iStart;

	for (int i = 0; i < iEnd; i++)
		operator[](i + iStart) = vecI[i];

	return *this;
}

template<class T> inline
CMatlibVector<T>& CMatlibVector<T>::Merge(const CMatlibVector<T>& vecA, T& tB)
{
	const int iSizeA = vecA.GetSize();

	for (int i = 0; i < iSizeA; i++)
		operator[](i) = vecA[i];
	
	operator[](iSizeA) = tB;

	return *this;
}

template<class T> inline
CMatlibVector<T>& CMatlibVector<T>::Merge(const CMatlibVector<T>& vecA,
					  const CMatlibVector<T>& vecB)
{
	int i;
	const int iSizeA = vecA.GetSize();
	const int iSizeB = vecB.GetSize();

	/* Put first vector */
	for (i = 0; i < iSizeA; i++)
		operator[](i) = vecA[i];
	
	/* Put second vector behind the first one, both
	   together must have length of *this */
	for (i = 0; i < iSizeB; i++)
		operator[](i + iSizeA) = vecB[i];

	return *this;
}

template<class T> inline
CMatlibVector<T>& CMatlibVector<T>::Merge(const CMatlibVector<T>& vecA,
					  const CMatlibVector<T>& vecB,
					  const CMatlibVector<T>& vecC)
{
	int i;
	const int iSizeA = vecA.GetSize();
	const int iSizeB = vecB.GetSize();
	const int iSizeC = vecC.GetSize();
	const int iSizeAB = iSizeA + iSizeB;

	/* Put first vector */
	for (i = 0; i < iSizeA; i++)
		operator[](i) = vecA[i];
	
	/* Put second vector behind the first one */
	for (i = 0; i < iSizeB; i++)
		operator[](i + iSizeA) = vecB[i];

	/* Put third vector behind previous put vectors */
	for (i = 0; i < iSizeC; i++)
		operator[](i + iSizeAB) = vecC[i];

	return *this;
}


/******************************************************************************/
/* CMatlibMatrix class ********************************************************/
/******************************************************************************/
/*
	We define: Matrix[row][column]
*/

template<class T>
class CMatlibMatrix
{
public:
	/* Construction, Destruction -------------------------------------------- */
	CMatlibMatrix() : iRowSize(0), ppData(NULL), eVType(VTY_CONST) {}
	CMatlibMatrix(const int iNRowLen, const int iNColLen,
		const EVecTy eNTy = VTY_CONST) : 
		iRowSize(0), ppData(NULL), eVType(eNTy) {Init(iNRowLen, iNColLen, T(0));}
	CMatlibMatrix(const int iNRowLen, const int iNColLen, const T tIniVal) : 
		iRowSize(0), ppData(NULL), eVType(VTY_CONST)
		{Init(iNRowLen, iNColLen, tIniVal);}
	CMatlibMatrix(const CMatlibMatrix<T>& matI);

	virtual ~CMatlibMatrix() {if (ppData != NULL) delete[] ppData;}

	void Init(int iNRowLen, int iNColLen, T tIniVal = T(0));
	inline int GetRowSize() const {return iRowSize;}
	inline int GetColSize() const
		{if (iRowSize > 0) return ppData[0].GetSize(); else return 0;}

	/* Operator[] (Regular indices!!!) */
	inline CMatlibVector<T>& operator[](int const iPos) const
		{_TESTRNGRM(iPos); return ppData[iPos];}
	inline CMatlibVector<T>& operator[](int const iPos)
		{_TESTRNGWM(iPos); return ppData[iPos];} // For use as l value

	/* Operator() */
	inline CMatlibVector<T>& operator()(int const iPos) const 
		{_TESTRNGRM(iPos - 1); return ppData[iPos - 1];}
	inline CMatlibVector<T>& operator()(int const iPos) 
		{_TESTRNGWM(iPos - 1); return ppData[iPos - 1];} // For use as l value

	CMatlibMatrix<T> operator()(const int iRowFrom, const int iRowTo,
		const int iColFrom, const int iColTo) const;

	/* operator= */
	inline CMatlibMatrix<T>& operator=(const CMatlibMatrix<CReal>& matI) 
		{_TESTSIZEM(matI.GetRowSize()); _MATOPCL(= matI[i]);}
	inline CMatlibMatrix<T>& operator=(const CMatlibMatrix<FXP>& matI)  
	  {_TESTSIZEM(matI.GetRowSize()); _MATOPCL(= matI[i]);} 
	
	inline CMatlibMatrix<CComplex>& operator=(const CMatlibMatrix<CComplex>& matI) 
		{_TESTSIZEM(matI.GetRowSize()); _MATOPCL(= matI[i]);}
        inline CMatlibMatrix<FComplex>& operator=(const CMatlibMatrix<FComplex>& matI)  
	  {_TESTSIZEM(matI.GetRowSize()); _MATOPCL(= matI[i]);} 

	/* operator*= */
	inline CMatlibMatrix<T>& operator*=(const CReal& rI)
		{_MATOPCL(*= rI);}
	inline CMatlibMatrix<T>& operator*=(const FXP& rI) 
        	{_MATOPCL(*= rI);} 

        inline CMatlibMatrix<CComplex>& operator*=(const CComplex& cI)
		{_MATOPCL(*= cI);}
	inline CMatlibMatrix<FComplex>& operator*=(const FComplex& cI) 
        	{_MATOPCL(*= cI);} 

	/* operator/= */
	inline CMatlibMatrix<T>& operator/=(const CReal& rI)
		{_MATOPCL(/= rI);}
	inline CMatlibMatrix<T>& operator/=(const FXP& rI) 
	        {_MATOPCL(/= rI);} 

	inline CMatlibMatrix<CComplex>& operator/=(const CComplex& cI)
		{_MATOPCL(/= cI);}
	inline CMatlibMatrix<FComplex>& operator/=(const FComplex& cI) 
	        {_MATOPCL(/= cI);} 

	//protected:
	EVecTy				eVType;
	int					iRowSize;
	CMatlibVector<T>*	ppData;
};

#if __GNUC__ == 3
template class CMatlibVector<FComplex>;
#endif


/* Help functions *************************************************************/
/* operator+ */
inline CMatlibMatrix<CComplex> // cm, cm
operator+(const CMatlibMatrix<CComplex>& cmA, const CMatlibMatrix<CComplex>& cmB)
{
	const int iRowSizeA = cmA.GetRowSize();
	const int iColSizeA = cmA.GetColSize();
	CMatlibMatrix<CComplex> matRet(iRowSizeA, iColSizeA, VTY_TEMP);

	for (int j = 0; j < iRowSizeA; j++)
		for (int i = 0; i < iColSizeA; i++)
			matRet[j][i] = cmA[j][i] + cmB[j][i];

	return matRet;
}

inline CMatlibMatrix<FComplex> // cm, cm 
operator+(const CMatlibMatrix<FComplex>& cmA, const CMatlibMatrix<FComplex>& cmB) 
{ 
  const int iRowSizeA = cmA.GetRowSize(); 
  const int iColSizeA = cmA.GetColSize(); 
  CMatlibMatrix<FComplex> matRet(iRowSizeA, iColSizeA, VTY_TEMP); 
 
  for (int j = 0; j < iRowSizeA; j++) 
    for (int i = 0; i < iColSizeA; i++) 
      matRet[j][i] = cmA[j][i] + cmB[j][i]; 
 
  return matRet; 
} 

/* operator- */
inline CMatlibMatrix<CComplex> // cm, cm
operator-(const CMatlibMatrix<CComplex>& cmA, const CMatlibMatrix<CComplex>& cmB)
{
	const int iRowSizeA = cmA.GetRowSize();
	const int iColSizeA = cmA.GetColSize();
	CMatlibMatrix<CComplex> matRet(iRowSizeA, iColSizeA, VTY_TEMP);

	for (int j = 0; j < iRowSizeA; j++)
		for (int i = 0; i < iColSizeA; i++)
			matRet[j][i] = cmA[j][i] - cmB[j][i];

	return matRet;
}
inline CMatlibMatrix<FComplex> // cm, cm 
operator-(const CMatlibMatrix<FComplex>& cmA, const CMatlibMatrix<FComplex>& cmB) 
{ 
        const int iRowSizeA = cmA.GetRowSize(); 
        const int iColSizeA = cmA.GetColSize(); 
        CMatlibMatrix<FComplex> matRet(iRowSizeA, iColSizeA, VTY_TEMP); 
 
        for (int j = 0; j < iRowSizeA; j++) 
                for (int i = 0; i < iColSizeA; i++) 
                         matRet[j][i] = cmA[j][i] - cmB[j][i]; 
 
        return matRet; 
} 


/* operator* */
inline CMatlibVector<CComplex> // cm, cv
operator*(const CMatlibMatrix<CComplex>& cmA, const CMatlibVector<CComplex>& cvB)
{
	const int iRowSizeA = cmA.GetRowSize();
	const int iSizeB = cvB.GetSize();
	CMatlibVector<CComplex> vecRet(iSizeB, VTY_TEMP);

	for (int j = 0; j < iRowSizeA; j++)
	{
		vecRet[j] = (CReal) 0.0;

		for (int i = 0; i < iSizeB; i++)
			vecRet[j] += cmA[j][i] * cvB[i];
	}

	return vecRet;
}
inline CMatlibVector<FComplex> // cm, cv 
operator*(const CMatlibMatrix<FComplex>& cmA, const CMatlibVector<FComplex>& cvB) 
{ 
        const int iRowSizeA = cmA.GetRowSize(); 
        const int iSizeB = cvB.GetSize(); 
        CMatlibVector<FComplex> vecRet(iSizeB, VTY_TEMP); 
 
        for (int j = 0; j < iRowSizeA; j++) 
        { 
               vecRet[j] = (FXP) 0.0; 
 
               for (int i = 0; i < iSizeB; i++) 
	               vecRet[j] += cmA[j][i] * cvB[i]; 
        } 
 
        return vecRet; 
} 


/* operator* */
inline CMatlibMatrix<CComplex> // cm, cm
operator*(const CMatlibMatrix<CComplex>& cmA, const CMatlibMatrix<CComplex>& cmB)
{
	const int iRowSizeA = cmA.GetRowSize();
	const int iRowSizeB = cmB.GetRowSize();
	const int iColSizeB = cmB.GetColSize();
	CMatlibMatrix<CComplex> matRet(iRowSizeA, iColSizeB, VTY_TEMP);

	for (int k = 0; k < iColSizeB; k++)
	{
		for (int j = 0; j < iRowSizeA; j++)
		{
			matRet[j][k] = (CReal) 0.0;

			for (int i = 0; i < iRowSizeB; i++)
				matRet[j][k] += cmA[j][i] * cmB[i][k];
		}
	}

	return matRet;
}


inline CMatlibMatrix<FComplex> // cm, cm 
operator*(const CMatlibMatrix<FComplex>& cmA, const CMatlibMatrix<FComplex>& cmB) 
{ 
  const int iRowSizeA = cmA.GetRowSize(); 
  const int iRowSizeB = cmB.GetRowSize(); 
  const int iColSizeB = cmB.GetColSize(); 
  CMatlibMatrix<FComplex> matRet(iRowSizeA, iColSizeB, VTY_TEMP); 
 
  for (int k = 0; k < iColSizeB; k++) 
    { 
      for (int j = 0; j < iRowSizeA; j++) 
	{ 
	  matRet[j][k] = (FXP) 0.0; 
 
	  for (int i = 0; i < iRowSizeB; i++) 
	    matRet[j][k] += cmA[j][i] * cmB[i][k]; 
	} 
    } 
 
  return matRet; 
} 

/* operator* */
inline CMatlibMatrix<CComplex> // c, cm
operator*(const CComplex& cA, const CMatlibMatrix<CComplex>& cmB)
{
	const int iRowSizeB = cmB.GetRowSize();
	const int iColSizeB = cmB.GetColSize();
	CMatlibMatrix<CComplex> matRet(iRowSizeB, iColSizeB, VTY_TEMP);

	for (int k = 0; k < iColSizeB; k++)
	{
		for (int j = 0; j < iRowSizeB; j++)
			matRet[j][k] = cA * cmB[j][k];
	}

	return matRet;
}
inline CMatlibMatrix<FComplex> // c, cm 
operator*(const FComplex& cA, const CMatlibMatrix<FComplex>& cmB) 
{ 
  const int iRowSizeB = cmB.GetRowSize(); 
  const int iColSizeB = cmB.GetColSize(); 
  CMatlibMatrix<FComplex> matRet(iRowSizeB, iColSizeB, VTY_TEMP); 
 
  for (int k = 0; k < iColSizeB; k++) 
    { 
      for (int j = 0; j < iRowSizeB; j++) 
	matRet[j][k] = cA * cmB[j][k]; 
    } 
 
  return matRet; 
} 
 


/* Implementation **************************************************************
   (the implementation of template classes must be in the header file!) */
template<class T>
CMatlibMatrix<T>::CMatlibMatrix(const CMatlibMatrix<T>& matI) : 
	iRowSize(matI.GetRowSize()), ppData(NULL), eVType(VTY_CONST)
{
	if (iRowSize > 0)
	{
		/* Allocate data block for vector */
		ppData = new CMatlibVector<T>[iRowSize];

		/* Init column vectors and copy */
		for (int i = 0; i < iRowSize; i++)
		{
			ppData[i].Init(matI.GetColSize());

			/* Copy entire vector */
			ppData[i] = matI[i];
		}
	}
}



template<class T>
void CMatlibMatrix<T>::Init(int iNRowLen, int iNColLen, T tIniVal)
{
	iRowSize = iNRowLen;

	/* Allocate data block for vector */
	if (iRowSize > 0)
	{
		if (ppData != NULL)
			delete[] ppData;

		ppData = new CMatlibVector<T>[iRowSize];

		/* Init column vectors and set to init value */
		for (int i = 0; i < iRowSize; i++)
			ppData[i].Init(iNColLen, tIniVal);
	}
}

template<class T> inline
CMatlibMatrix<T> CMatlibMatrix<T>::operator()(const int iRowFrom, const int iRowTo,
											  const int iColFrom, const int iColTo) const
{
	const int iStartRow = iRowFrom - 1;
	const int iStartCol = iColFrom - 1;
	CMatlibMatrix<T> matRet(iRowTo - iStartRow, iColTo - iStartCol, VTY_TEMP);

	for (int j = iStartRow; j < iRowTo; j++)
		for (int i = iStartCol; i < iColTo; i++)
			matRet[j - iStartRow][i - iStartCol] = operator[](j)[i];

	return matRet;
}


/* Include toolboxes after all type definitions */
#include "MatlibStdToolbox.h"
#include "MatlibSigProToolbox.h"


#endif	/* _MATLIB_H_ */
