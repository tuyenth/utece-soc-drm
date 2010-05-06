/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2004
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
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

#include "Utilities.h"


/* Implementation *************************************************************/
/******************************************************************************\
* Signal level meter                                                           *
\******************************************************************************/
void CSignalLevelMeter::Update(const _REAL rVal)
{
	/* Search for maximum. Decrease this max with time */
	/* Decrease max with time */
	if (rCurLevel >= METER_FLY_BACK)
		rCurLevel -= METER_FLY_BACK;
	else
	{
		if ((rCurLevel <= METER_FLY_BACK) && (rCurLevel > 1))
			rCurLevel -= 2;
	}

	/* Search for max */
	const _REAL rCurAbsVal = Abs(rVal);
	if (rCurAbsVal > rCurLevel)
		rCurLevel = rCurAbsVal;
}

void CSignalLevelMeter::Update(const CVector<_REAL> vecrVal)
{
	/* Do the update for entire vector */
	const int iVecSize = vecrVal.Size();
	for (int i = 0; i < iVecSize; i++)
		Update(vecrVal[i]);
}

void CSignalLevelMeter::Update(const CVector<_SAMPLE> vecsVal)
{
	/* Do the update for entire vector, convert to real */
	const int iVecSize = vecsVal.Size();
	for (int i = 0; i < iVecSize; i++)
		Update((_REAL) vecsVal[i]);
}

_REAL CSignalLevelMeter::Level()
{
	const _REAL rNormMicLevel = rCurLevel / _MAXSHORT;

	/* Logarithmic measure */
	if (rNormMicLevel > 0)
		return (_REAL) 20.0 * log10(rNormMicLevel);
	else
		return RET_VAL_LOG_0;
}


/******************************************************************************\
* Bandpass filter                                                              *
\******************************************************************************/
void CDRMBandpassFilt::Process(CVector<_COMPLEX>& veccData)
{
	int	i;

	/* Copy CVector data in CMatlibVector */
	for (i = 0; i < iBlockSize; i++)
		cvecDataTmp[i] = veccData[i];

	/* Apply FFT filter */
	cvecDataTmp = CComplexVector(
		FftFilt(cvecB, Real(cvecDataTmp), rvecZReal, FftPlanBP),
		FftFilt(cvecB, Imag(cvecDataTmp), rvecZImag, FftPlanBP));

	/* Copy CVector data in CMatlibVector */
	for (i = 0; i < iBlockSize; i++)
		veccData[i] = cvecDataTmp[i];
}

void CDRMBandpassFilt::Init(const int iNewBlockSize, const _REAL rOffsetHz,
							const ESpecOcc eSpecOcc, const EFiltType eNFiTy)
{
	CReal rMargin;

	/* Set internal parameter */
	iBlockSize = iNewBlockSize;

	/* Init temporary vector */
	cvecDataTmp.Init(iBlockSize);

	/* Choose correct filter for chosen DRM bandwidth. Also, adjust offset
	   frequency for different modes. E.g., 5 kHz mode is on the right side
	   of the DC frequency */
	CReal rNormCurFreqOffset;
	CReal rBPFiltBW; /* Band-pass filter bandwidth */

	/* Negative margin for receiver filter for better interferer rejection */
	if (eNFiTy == FT_TRANSMITTER)
		rMargin = (CReal) 300.0; /* Hz */
	else
		rMargin = (CReal) -200.0; /* Hz */

	switch (eSpecOcc)
	{
	case SO_0:
		rBPFiltBW = ((CReal) 4500.0 + rMargin) / SOUNDCRD_SAMPLE_RATE;

		/* Completely on the right side of DC */
		rNormCurFreqOffset =
			(rOffsetHz + (CReal) 2190.0) / SOUNDCRD_SAMPLE_RATE;
		break;

	case SO_1:
		rBPFiltBW = ((CReal) 5000.0 + rMargin) / SOUNDCRD_SAMPLE_RATE;

		/* Completely on the right side of DC */
		rNormCurFreqOffset =
			(rOffsetHz + (CReal) 2440.0) / SOUNDCRD_SAMPLE_RATE;
		break;

	case SO_2:
		rBPFiltBW = ((CReal) 9000.0 + rMargin) / SOUNDCRD_SAMPLE_RATE;

		/* Centered */
		rNormCurFreqOffset = rOffsetHz / SOUNDCRD_SAMPLE_RATE;
		break;

	case SO_3:
		rBPFiltBW = ((CReal) 10000.0 + rMargin) / SOUNDCRD_SAMPLE_RATE;

		/* Centered */
		rNormCurFreqOffset = rOffsetHz / SOUNDCRD_SAMPLE_RATE;
		break;

	case SO_4:
		rBPFiltBW = ((CReal) 18000.0 + rMargin) / SOUNDCRD_SAMPLE_RATE;

		/* Main part on the right side of DC */
		rNormCurFreqOffset =
			(rOffsetHz + (CReal) 4500.0) / SOUNDCRD_SAMPLE_RATE;
		break;

	case SO_5:
		rBPFiltBW = ((CReal) 20000.0 + rMargin) / SOUNDCRD_SAMPLE_RATE;

		/* Main part on the right side of DC */
		rNormCurFreqOffset =
			(rOffsetHz + (CReal) 5000.0) / SOUNDCRD_SAMPLE_RATE;
		break;
	}

	/* FFT plan is initialized with the long length */
	FftPlanBP.Init(iBlockSize * 2);

	/* State memory (init with zeros) and data vector */
	rvecZReal.Init(iBlockSize, (CReal) 0.0);
	rvecZImag.Init(iBlockSize, (CReal) 0.0);
	rvecDataReal.Init(iBlockSize);
	rvecDataImag.Init(iBlockSize);

	/* "+ 1" because of the Nyquist frequency (filter in frequency domain) */
	cvecB.Init(iBlockSize + 1);

	/* Actual filter design */
	CRealVector vecrFilter(iBlockSize);
	vecrFilter = FirLP(rBPFiltBW, Nuttallwin(iBlockSize));

	/* Copy actual filter coefficients. It is important to initialize the
	   vectors with zeros because we also do a zero-padding */
	CRealVector rvecB(2 * iBlockSize, (CReal) 0.0);

	/* Modulate filter to shift it to the correct IF frequency */
	for (int i = 0; i < iBlockSize; i++)
	{
		rvecB[i] =
			vecrFilter[i] * Cos((CReal) 2.0 * crPi * rNormCurFreqOffset * i);
	}

	/* Transformation in frequency domain for fft filter */
	cvecB = rfft(rvecB, FftPlanBP);
}


/******************************************************************************\
* Modified Julian Date                                                         *
\******************************************************************************/
void CModJulDate::Set(const uint32_t iModJulDate)
{
	uint32_t	iZ, iA, iAlpha, iB, iC, iD, iE;
	_REAL		rJulDate, rF;

	/* Definition of the Modified Julian Date */
	rJulDate = (_REAL) iModJulDate + 2400000.5;

	/* Get "real" date out of Julian Date
	   (Taken from "http://mathforum.org/library/drmath/view/51907.html") */
	// 1. Add .5 to the JD and let Z = integer part of (JD+.5) and F the
	// fractional part F = (JD+.5)-Z
	iZ = (uint32_t) (rJulDate + (_REAL) 0.5);
	rF = (rJulDate + (_REAL) 0.5) - iZ;

	// 2. If Z < 2299161, take A = Z
	// If Z >= 2299161, calculate alpha = INT((Z-1867216.25)/36524.25)
	// and A = Z + 1 + alpha - INT(alpha/4).
	if (iZ < 2299161)
		iA = iZ;
	else
	{
		iAlpha = (int) (((_REAL) iZ - (_REAL) 1867216.25) / (_REAL) 36524.25);
		iA = iZ + 1 + iAlpha - (int) ((_REAL) iAlpha / (_REAL) 4.0);
	}

	// 3. Then calculate:
	// B = A + 1524
	// C = INT( (B-122.1)/365.25)
	// D = INT( 365.25*C )
	// E = INT( (B-D)/30.6001 )
	iB = iA + 1524;
	iC = (int) (((_REAL) iB - (_REAL) 122.1) / (_REAL) 365.25);
	iD = (int) ((_REAL) 365.25 * iC);
	iE = (int) (((_REAL) iB - iD) / (_REAL) 30.6001);

	// The day of the month dd (with decimals) is:
	// dd = B - D - INT(30.6001*E) + F
	iDay = iB - iD - (int) ((_REAL) 30.6001 * iE);// + rF;

	// The month number mm is:
	// mm = E - 1, if E < 13.5
	// or
	// mm = E - 13, if E > 13.5
	if ((_REAL) iE < 13.5)
		iMonth = iE - 1;
	else
		iMonth = iE - 13;

	// The year yyyy is:
	// yyyy = C - 4716   if m > 2.5
	// or
	// yyyy = C - 4715   if m < 2.5
	if ((_REAL) iMonth > 2.5)
		iYear = iC - 4716;
	else
		iYear = iC - 4715;
}


/******************************************************************************\
* Audio Reverberation                                                          *
\******************************************************************************/
/*
	The following code is based on "JCRev: John Chowning's reverberator class"
	by Perry R. Cook and Gary P. Scavone, 1995 - 2004
	which is in "The Synthesis ToolKit in C++ (STK)"
	http://ccrma.stanford.edu/software/stk

	Original description:
	This class is derived from the CLM JCRev function, which is based on the use
	of networks of simple allpass and comb delay filters. This class implements
	three series allpass units, followed by four parallel comb filters, and two
	decorrelation delay lines in parallel at the output.
*/
CAudioReverb::CAudioReverb(const CReal rT60)
{
	/* Delay lengths for 44100 Hz sample rate */
	int lengths[9] = {1777, 1847, 1993, 2137, 389, 127, 43, 211, 179};
	const CReal scaler = (CReal) SOUNDCRD_SAMPLE_RATE / 44100.0;

	int delay, i;
	if (scaler != 1.0)
	{
		for (i = 0; i < 9; i++)
		{
			delay = (int) Floor(scaler * lengths[i]);

			if ((delay & 1) == 0)
				delay++;

			while (!isPrime(delay))
				delay += 2;

			lengths[i] = delay;
		}
	}

	for (i = 0; i < 3; i++)
		allpassDelays_[i].Init(lengths[i + 4]);

	for (i = 0; i < 4; i++)
		combDelays_[i].Init(lengths[i]);

	setT60(rT60);
	allpassCoefficient_ = (CReal) 0.7;
	Clear();
}

_BOOLEAN CAudioReverb::isPrime(const int number)
{
/*
	Returns true if argument value is prime. Taken from "class Effect" in
	"STK abstract effects parent class".
*/
	if (number == 2)
		return TRUE;

	if (number & 1)
	{
		for (int i = 3; i < (int) Sqrt((CReal) number) + 1; i += 2)
		{
			if ((number % i) == 0)
				return FALSE;
		}

		return TRUE; /* prime */
	}
	else
		return FALSE; /* even */
}

void CAudioReverb::Clear()
{
	/* Reset and clear all internal state */
	allpassDelays_[0].Reset(0);
	allpassDelays_[1].Reset(0);
	allpassDelays_[2].Reset(0);
	combDelays_[0].Reset(0);
	combDelays_[1].Reset(0);
	combDelays_[2].Reset(0);
	combDelays_[3].Reset(0);
}

void CAudioReverb::setT60(const CReal rT60)
{
	/* Set the reverberation T60 decay time */
	for (int i = 0; i < 4; i++)
	{
		combCoefficient_[i] = pow((CReal) 10.0, (-3.0 * combDelays_[i].Size() /
			(rT60 * SOUNDCRD_SAMPLE_RATE)));
	}
}

CReal CAudioReverb::ProcessSample(const CReal rLInput, const CReal rRInput)
{
	/* Compute one output sample */
	CReal temp, temp0, temp1, temp2;

	/* Mix stereophonic input signals to mono signal (since the maximum value of
	   the input signal is 0.5 * max due to the current implementation in
	   AudioSourceDecoder.cpp, we cannot get an overrun) */
	const CReal input = rLInput + rRInput;

	temp = allpassDelays_[0].Get();
	temp0 = allpassCoefficient_ * temp;
	temp0 += input;
	allpassDelays_[0].Add((int) temp0);
	temp0 = -(allpassCoefficient_ * temp0) + temp;

	temp = allpassDelays_[1].Get();
	temp1 = allpassCoefficient_ * temp;
	temp1 += temp0;
	allpassDelays_[1].Add((int) temp1);
	temp1 = -(allpassCoefficient_ * temp1) + temp;

	temp = allpassDelays_[2].Get();
	temp2 = allpassCoefficient_ * temp;
	temp2 += temp1;
	allpassDelays_[2].Add((int) temp2);
	temp2 = -(allpassCoefficient_ * temp2) + temp;

	const CReal temp3 = temp2 + (combCoefficient_[0] * combDelays_[0].Get());
	const CReal temp4 = temp2 + (combCoefficient_[1] * combDelays_[1].Get());
	const CReal temp5 = temp2 + (combCoefficient_[2] * combDelays_[2].Get());
	const CReal temp6 = temp2 + (combCoefficient_[3] * combDelays_[3].Get());

	combDelays_[0].Add((int) temp3);
	combDelays_[1].Add((int) temp4);
	combDelays_[2].Add((int) temp5);
	combDelays_[3].Add((int) temp6);

	return temp3 + temp4 + temp5 + temp6;
}


#ifdef HAVE_LIBHAMLIB
/******************************************************************************\
* Hamlib interface                                                             *
\******************************************************************************/
/*
	This code is based on patches and example code from Tomi Manninen and
	Stephane Fillod (developer of hamlib)
*/
CHamlib::CHamlib() : pRig(NULL), bModRigSettings(FALSE) ,iHamlibModelID(0),
	bSMeterIsSupported(FALSE), strHamlibConf(HAMLIB_CONF_COM1)
{
	/* Special DRM front-end list */
	vecSpecDRMRigs.Init(0);

#ifdef RIG_MODEL_G303
	/* Winradio G3 */
	vecSpecDRMRigs.Add(CSpecDRMRig(RIG_MODEL_G303,
		"l_ATT=0,l_AGC=3", 0,
		"l_ATT=0,l_AGC=3"));
#endif

#ifdef RIG_MODEL_AR7030
	/* AOR 7030 */
	vecSpecDRMRigs.Add(CSpecDRMRig(RIG_MODEL_AR7030,
		"m_CW=9500,l_IF=-4200,l_AGC=3", 5 /* kHz frequency offset */,
		"l_AGC=3"));
#endif

#ifdef RIG_MODEL_ELEKTOR304
	/* Elektor 3/04 */
	vecSpecDRMRigs.Add(CSpecDRMRig(RIG_MODEL_ELEKTOR304, "", 0, ""));
#endif

#ifdef RIG_MODEL_NRD535
	/* JRC NRD 535 */
	vecSpecDRMRigs.Add(CSpecDRMRig(RIG_MODEL_NRD535,
		"l_CWPITCH=-5000,m_CW=12000,l_IF=-2000,l_AGC=3" /* AGC=slow */,
		3 /* kHz frequency offset */,
		"l_AGC=3"));
#endif

#ifdef RIG_MODEL_RX320
	/* TenTec RX320D */
	vecSpecDRMRigs.Add(CSpecDRMRig(RIG_MODEL_RX320,
		"l_AF=1,l_AGC=3,m_AM=6000", 0,
		"l_AGC=3"));
#endif

#ifdef RIG_MODEL_RX340
	/* TenTec RX340D */
	vecSpecDRMRigs.Add(CSpecDRMRig(RIG_MODEL_RX340,
		"l_AF=1,m_USB=16000,l_AGC=3,l_IF=2000",
		-12 /* kHz frequency offset */,
		"l_AGC=3"));
#endif

	/* Load all available front-end remotes in hamlib library */
	rig_load_all_backends();

	/* Get all models which are available. First, the vector for storing the
	   data has to be initialized with zero length! A call-back function is
	   called to return the different rigs */
	veccapsHamlibModels.Init(0);
	rig_list_foreach(PrintHamlibModelList, this);

	/* Sort list */
	SortHamlibModelList(veccapsHamlibModels);
}

CHamlib::~CHamlib()
{
	if (pRig != NULL)
	{
		/* close everything */
		rig_close(pRig);
		rig_cleanup(pRig);
	}
}

int CHamlib::PrintHamlibModelList(const struct rig_caps* caps,
								  void* data)
{
	/* Access data members of class through pointer ((CHamlib*) data).
	   Store new model in string vector. Use only relevant information */
	int iIndex;
	const _BOOLEAN bSpecRigIdx = ((CHamlib*) data)->
		CheckForSpecDRMFE(caps->rig_model, iIndex);

	((CHamlib*) data)->veccapsHamlibModels.Add(
		SDrRigCaps(caps->rig_model,	caps->mfg_name, caps->model_name,
		caps->status, bSpecRigIdx));

	return 1; /* !=0, we want them all! */
}

_BOOLEAN CHamlib::CheckForSpecDRMFE(const rig_model_t iID, int& iIndex)
{
	_BOOLEAN bIsSpecialDRMrig = FALSE;
	const int iVecSize = vecSpecDRMRigs.Size();

	/* Check for special DRM front-end */
	for (int i = 0; i < iVecSize; i++)
	{
		if (vecSpecDRMRigs[i].iModelID == iID)
		{
			bIsSpecialDRMrig = TRUE;
			iIndex = i;
		}
	}

	return bIsSpecialDRMrig;
}

void CHamlib::SortHamlibModelList(CVector<SDrRigCaps>& veccapsHamlMod)
{
	/* Loop through the array one less than its total cell count */
	const int iEnd = veccapsHamlMod.Size() - 1;

	for (int i = 0; i < iEnd; i++)
	{
		for (int j = 0; j < iEnd; j++)
		{
			/* Compare the values and switch if necessary */
			if (veccapsHamlMod[j].iModelID >
				veccapsHamlMod[j + 1].iModelID)
			{
				const SDrRigCaps instSwap = veccapsHamlMod[j];
				veccapsHamlMod[j] = veccapsHamlMod[j + 1];
				veccapsHamlMod[j + 1] = instSwap;
			}
		}
	}
}

_BOOLEAN CHamlib::SetFrequency(const int iFreqkHz)
{
	_BOOLEAN bSucceeded = FALSE;

	/* Check if rig was opend properly */
	if (pRig != NULL)
	{
		/* Set frequency (consider frequency offset and conversion
		   from kHz to Hz by "* 1000") */
		if (rig_set_freq(pRig, RIG_VFO_CURR, (iFreqkHz + iFreqOffset) * 1000)
			== RIG_OK)
		{
			bSucceeded = TRUE;
		}
	}

	return bSucceeded;
}

CHamlib::ESMeterState CHamlib::GetSMeter(_REAL& rCurSigStr)
{
	ESMeterState eRetVal = SS_NOTVALID;
	rCurSigStr = (_REAL) 0.0;

	if ((pRig != NULL) && (bSMeterIsSupported == TRUE))
	{
		value_t tVal;
		const int iHamlibRetVal =
			rig_get_level(pRig, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &tVal);
 
		if (!iHamlibRetVal)
		{
			rCurSigStr = (_REAL) tVal.i;
			eRetVal = SS_VALID;
		}

		/* If a time-out happened, do not update s-meter anymore (disable it) */
		if (iHamlibRetVal == -RIG_ETIMEOUT)
		{
			bSMeterIsSupported = FALSE;
			eRetVal = SS_TIMEOUT;
		}
	}

	return eRetVal;
}

void CHamlib::SetEnableModRigSettings(const _BOOLEAN bNSM)
{
	if (bModRigSettings != bNSM)
	{
		/* Set internal parameter */
		bModRigSettings = bNSM;

		/* Hamlib must be re-initialized with new parameter */
		SetHamlibModelID(iHamlibModelID);
	}
}

void CHamlib::SetHamlibConf(const string strNewC)
{
	if (strHamlibConf.compare(strNewC))
	{
		/* Set internal parameter */
		strHamlibConf = strNewC;

		/* Hamlib must be re-initialized with new parameter */
		SetHamlibModelID(iHamlibModelID);
	}
}

void CHamlib::SetHamlibModelID(const int iNewM)
{
	int ret;

	/* Set value for current selected model ID */
	iHamlibModelID = (rig_model_t) iNewM;

	/* Init frequency offset */
	iFreqOffset = 0;

try
{
	/* If rig was already open, close it first */
	if (pRig != NULL)
	{
		/* Close everything */
		rig_close(pRig);
		rig_cleanup(pRig);
	}

	if (iHamlibModelID == 0)
		throw CGenErr("No rig model ID selected.");

	/* Init rig */
	pRig = rig_init(iHamlibModelID);
	if (pRig == NULL)
		throw CGenErr("Initialization of hamlib failed.");

	/* Config setup */
	char *p_dup, *p, *q, *n;
	for (p = p_dup = strdup(strHamlibConf.c_str()); p && *p != '\0'; p = n)
	{
		if ((q = strchr(p, '=')) == NULL)
		{
			rig_cleanup(pRig);
			pRig = NULL;

			throw CGenErr("Malformatted config string.");
		}
		*q++ = '\0';

		if ((n = strchr(q, ',')) != NULL)
			*n++ = '\0';

		ret = rig_set_conf(pRig, rig_token_lookup(pRig, p), q);
		if (ret != RIG_OK)
		{
			rig_cleanup(pRig);
			pRig = NULL;

			throw CGenErr("Rig set conf failed.");
		}
	}
	if (p_dup)
		free(p_dup);

	/* Open rig */
	if (ret = rig_open(pRig) != RIG_OK)
	{
		/* Fail! */
		rig_cleanup(pRig);
		pRig = NULL;

		throw CGenErr("Rig open failed.");
	}

	/* Ignore result, some rigs don't have support for this */
	rig_set_powerstat(pRig, RIG_POWER_ON);

	/* Check for special DRM front-end selection */
	int iIndex;
	if (CheckForSpecDRMFE(iHamlibModelID, iIndex) == TRUE)
	{
		/* Get correct parameter string */
		string strSet;
		if (bModRigSettings == TRUE)
			strSet = vecSpecDRMRigs[iIndex].strDRMSetMod;
		else
		{
			strSet = vecSpecDRMRigs[iIndex].strDRMSetNoMod;

			/* Additionally, set frequency offset for this special rig */
			iFreqOffset = vecSpecDRMRigs[iIndex].iFreqOffs;
		}

		/* Parse special settings */
		char *p_dup, *p, *q, *n;
		for (p = p_dup = strdup(strSet.c_str()); p && *p != '\0'; p = n)
		{
			if ((q = strchr(p, '=')) == NULL)
			{
				/* Malformatted config string */
				rig_cleanup(pRig);
				pRig = NULL;

				throw CGenErr("Malformatted config string.");
			}
			*q++ = '\0';

			if ((n = strchr(q, ',')) != NULL)
				*n++ = '\0';

			rmode_t mode;
			setting_t setting;
			value_t val;

			if (p[0] == 'm' && (mode = rig_parse_mode(p + 2)) != RIG_MODE_NONE)
			{
				ret = rig_set_mode(pRig, RIG_VFO_CURR, mode, atoi(q));
				if (ret != RIG_OK)
				{
					cerr << "Rig set mode failed: " << rigerror(ret) <<
					endl;
				}
			}
			else if (p[0] == 'l' && (setting = rig_parse_level(p + 2)) !=
				RIG_LEVEL_NONE)
			{
				if (RIG_LEVEL_IS_FLOAT(setting))
					val.f = atof(q);
				else
					val.i = atoi(q);

				ret = rig_set_level(pRig, RIG_VFO_CURR, setting, val);
				if (ret != RIG_OK)
				{
					cerr << "Rig set level failed: " << rigerror(ret) <<
					endl;
				}
			}
			else if (p[0] == 'f' && (setting = rig_parse_func(p + 2)) !=
				RIG_FUNC_NONE)
			{
				ret = rig_set_func(pRig, RIG_VFO_CURR, setting, atoi(q));
				if (ret != RIG_OK)
				{
					cerr << "Rig set func failed: " << rigerror(ret) <<
					endl;
				}
			}
			else if (p[0] == 'p' && (setting = rig_parse_parm(p + 2)) !=
				RIG_PARM_NONE)
			{
				if (RIG_PARM_IS_FLOAT(setting))
					val.f = atof(q);
				else
					val.i = atoi(q);

				ret = rig_set_parm(pRig, setting, val);
				if (ret != RIG_OK)
				{
					cerr << "Rig set parm failed: " << rigerror(ret) <<
					endl;
				}
			}
			else
				cerr << "Rig unknown setting: " << p << "=" << q << endl;
		}
		if (p_dup)
			free(p_dup);
	}

	/* Check if s-meter capabilities are available */
	if (pRig != NULL)
	{
		/* Check if s-meter can be used. Disable GUI control if not */
		if (rig_has_get_level(pRig, RIG_LEVEL_STRENGTH))
			bSMeterIsSupported = TRUE;
		else
			bSMeterIsSupported = FALSE;
	}
}

catch (CGenErr GenErr)
{
	/* Print error message */
	cerr << GenErr.strError << endl;

	/* Disable s-meter */
	bSMeterIsSupported = FALSE;
}
}
#endif
