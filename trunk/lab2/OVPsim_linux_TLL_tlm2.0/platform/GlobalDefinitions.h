/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Global definitions
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

#if !defined(DEF_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define DEF_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

//#pragma warning(disable:4996)

#include "FXP.h"

#include <complex>
#if defined(__ARMCC__) || defined(__CC_ARM)
#else
using namespace std; /* Because of the library: "complex" */
#endif

/*
#include <string>
#include <stdio.h>
#include <math.h>
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
*/

#include "TableDRMGlobal.h"


/* Definitions ****************************************************************/
/* When you define the following flag, a directory called
   "test" MUST EXIST in the windows directory (or linux directory if you use
   Linux)! */
//#define _DEBUG_
#undef _DEBUG_

#ifndef VERSION
# define VERSION						"1.2.4"
#endif


//#ifdef _WIN32 /* For Windows set flags here, otherwise it is set by configure */
///* Define whether using GUI or non-GUI receiver */
//# define USE_QT_GUI
////# undef USE_QT_GUI
//
///* Activate or disable faad2 library (AAC decoding) */
//# define USE_FAAD2_LIBRARY
////# undef USE_FAAD2_LIBRARY
//
//# define USE_FAAC_LIBRARY
////# undef USE_FAAC_LIBRARY
//
//# define HAVE_JOURNALINE
////# undef HAVE_JOURNALINE
//
//# define HAVE_LIBFREEIMAGE
////# undef HAVE_LIBFREEIMAGE
//
//# define HAVE_LIBHAMLIB
////# undef HAVE_LIBHAMLIB
//
///* Define if you have Hamlib >= 1.2.1 */
//# define HAVE_RIG_PARSE_MODE			1
//#endif


/* Choose algorithms -------------------------------------------------------- */
/* There are two algorithms available for frequency offset estimation for
   tracking mode: Using frequency pilots or the guard-interval correlation. In
   case of guard-interval correlation (which will be chosen if this macro is
   defined), the Hilbert filter in TimeSync must be used all the time -> more
   CPU usage. Also, the frequency tracking range is smaller */
#undef USE_FRQOFFS_TRACK_GUARDCORR

/* The sample rate offset estimation can be done using the frequency pilots or
   the movement of the estimated impulse response. Defining this macro will
   enable the frequency pilot based estimation. Simulations showed that this
   method is more vulnerable to bad channel situations */
#undef USE_SAMOFFS_TRACK_FRE_PIL

/* Using max-log MAP decoder. A lot more memory and CPU is needed for this
   method. This is just for showing the potential of an improved decoding
   method and should not be activated for the "regular" version of Dream */
#undef USE_MAX_LOG_MAP

/* This method tries to speed up the audio output after a re-synchronization
   when long symbol interleaver is used. We work with erasure symbols to mark
   data which is not yet received. We hope that the channel decoder can still
   decode audio even if not all data is yet received to fill the interleaver
   history */
#define USE_ERASURE_FOR_FASTER_ACQ


/* Define the application specific data-types ------------------------------- */
typedef	double						_REAL;
typedef	complex<_REAL>					_COMPLEX;
typedef short						_SAMPLE;
typedef unsigned char					_BYTE;
typedef bool						_BOOLEAN;

/* Fixed-point types */
typedef FXP						_FREAL;
typedef complex<_FREAL>					_FCOMPLEX;

typedef int             _INT;
typedef complex<_INT>   _COMPLEX_I;
typedef complex<_INT>    IComplex;

// bool seems not to work with linux TODO: Fix Me!
typedef unsigned char/*bool*/			_BINARY;
#if 0
#if defined(_WIN32)
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
#else
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# else
//typedef unsigned long uint32_t;
//typedef unsigned int uint16_t;
# endif
#endif
#endif
#endif

/* Define type-specific information */
#define SIZEOF__BYTE					8
#define _MAXSHORT						32767
#define _MAXREAL						((_REAL) 3.4e38) /* Max for float */
#define _MAXFREAL						_MAXSHORT	//cj
#ifdef USE_ERASURE_FOR_FASTER_ACQ
/* Use max-value for showing that this is an erasure */ 
//cj # define ERASURE_TAG_VALUE				_MAXREAL
# define ERASURE_TAG_VALUE				_MAXFREAL	// cj
#endif


/* MAP ---------------------------------------------------------------------- */
#ifdef USE_MAX_LOG_MAP
typedef _REAL							_DECISION;
# define ML_SOFT_INF_MAX_VALUE			((_DECISION) 1e10)
inline _BINARY ExtractBit(_DECISION dD) {return dD > 0 ? 1 : 0;}
inline _DECISION BitToSoft(_BINARY biB) {return biB == 0 ? -1.0 : 1.0;}
#else
typedef _BINARY							_DECISION;
#define ExtractBit(a)					(a)
#define BitToSoft(a)					(a)
#endif


/* Definitions for window message system ------------------------------------ */
typedef unsigned int					_MESSAGE_IDENT;
#define MS_FAC_CRC						1	/* MS: Message */
#define MS_SDC_CRC						2
#define MS_MSC_CRC						3
#define MS_FRAME_SYNC					4
#define MS_TIME_SYNC					5
#define MS_IOINTERFACE					6
#define MS_RESET_ALL					7
#define MS_MOT_OBJ_STAT					8

#define GUI_CONTROL_UPDATE_TIME			500	/* Milliseconds */
#define GUI_CONTROL_UPDATE_TIME_FAST	250	/* Milliseconds */


/* Global enumerations ------------------------------------------------------ */
#if 0
enum ESpecOcc {SO_0, SO_1, SO_2, SO_3, SO_4, SO_5}; /* SO: Spectrum Occupancy */
enum ERobMode {RM_ROBUSTNESS_MODE_A, RM_ROBUSTNESS_MODE_B,
		RM_ROBUSTNESS_MODE_C, RM_ROBUSTNESS_MODE_D,
		RM_NO_MODE_DETECTED}; /* RM: Robustness Mode */
#endif

/* Constants ---------------------------------------------------------------- */
//const _REAL crPi = ((_REAL) 3.14159265358979323846);
//const _FREAL crfPi = FXP(205887, 0);


/* Define a number for the case: log10(0), which would lead to #inf */
#define RET_VAL_LOG_0					((_REAL) -200.0)


/* Standard definitions */
#define	TRUE							1
#define FALSE							0


/* Classes ********************************************************************/
/* For metric */
#if 0
class CDistance
{
public:
	/* Distance towards 0 or towards 1 */
	_REAL rTow0;
	_REAL rTow1;
};
#endif
class CFDistance
{
public:
	/* Distance towards 0 or towards 1 */
	_FREAL rTow0;
	_FREAL rTow1;
};

#if 0
/* Viterbi needs information of equalized received signal and channel */
class CEquSig
{
public:
	CEquSig() : cSig(_COMPLEX((_REAL) 0.0, (_REAL) 0.0)), rChan((_REAL) 0.0) {}
	CEquSig(const _COMPLEX cNS, const _REAL rNC) : cSig(cNS), rChan(rNC) {}

	_COMPLEX	cSig; /* Actual signal */
	_REAL		rChan; /* Channel power at this cell */
};
class CFEquSig
{
public:
	CFEquSig() : cSig(_FCOMPLEX((_FREAL) 0, (_FREAL) 0)), rChan((_FREAL) 0) {}
	CFEquSig(const _FCOMPLEX cNS, const _FREAL rNC) : cSig(cNS), rChan(rNC) {}

	_FCOMPLEX	cSig; /* Actual signal */
	_FREAL		rChan; /* Channel power at this cell */
};

/* Mutex object to access data safely from different threads */
/* QT mutex */
#ifdef USE_QT_GUI
#include <qthread.h>
class CMutex
{
public:
	void Lock() {Mutex.lock();}
	void Unlock() {Mutex.unlock();}

protected:
	QMutex Mutex;
};
#else
/* No GUI and no threads, we do not need mutex in this case */
class CMutex
{
public:
	void Lock() {}
	void Unlock() {}
};
#endif

class CGenErr
{
public:
	CGenErr(string strNE) : strError(strNE) {}
	string strError;
};

// FIXME something nicer than using "MAX_NUM_TAPS_DRM_CHAN"
/* For simulation, data from channel simulation */
#define MAX_NUM_TAPS_DRM_CHAN			4
template<class T> class CChanSimData
{
public:
	T					tIn; /* Channel input data */
	T					tOut; /* Output of the channel (with noise) */
	T					tRef; /* Channel reference signal (without noise) */
	_COMPLEX			veccTap[MAX_NUM_TAPS_DRM_CHAN]; /* Tap gains */
	_COMPLEX			veccTapBackw[MAX_NUM_TAPS_DRM_CHAN];
};
typedef CChanSimData<_REAL>		CChanSimDataMod; /* OFDM modulated signals */
typedef CChanSimData<_COMPLEX>	CChanSimDataDemod; /* Demodulated signals */

/* Path for simulation output and status files */
#define SIM_OUT_FILES_PATH				"test/"


/* Prototypes for global functions ********************************************/
/* Posting a window message */
void PostWinMessage(const _MESSAGE_IDENT MessID, const int iMessageParam = 0);

/* Debug error handling */
void DebugError(const char* pchErDescr, const char* pchPar1Descr,
				const double dPar1, const char* pchPar2Descr,
				const double dPar2);

void ErrorMessage(string strErrorString);


/* Global functions ***********************************************************/
/* Converting _REAL to _SAMPLE */
inline _SAMPLE Real2Sample(const _REAL rInput)
{
	/* Lower bound */
	if (rInput < -_MAXSHORT)
		return -_MAXSHORT;

	/* Upper bound */
	if (rInput > _MAXSHORT)
		return _MAXSHORT;

	return (_SAMPLE) rInput;
}
inline _SAMPLE Real2Sample(const _FREAL rInput)
{
	/* Lower bound */
	if (rInput < -_MAXSHORT)
		return -_MAXSHORT;

	/* Upper bound */
	if (rInput > _MAXSHORT)
		return _MAXSHORT;

	return (_SAMPLE) rInput;
}
inline _SAMPLE Real2Sample(const int rInput)
{
	        /* Lower bound */
        if (rInput < -_MAXSHORT)
                return -_MAXSHORT;

						        /* Upper bound */
        if (rInput > _MAXSHORT)
                return _MAXSHORT;

        return (_SAMPLE) rInput;
}
#endif 
#endif // !defined(DEF_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
