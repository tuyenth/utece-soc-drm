/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2004
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Implements:
 *	- Signal level meter
 *	- Bandpass filter
 *	- Modified Julian Date
 *	- Reverberation effect
 *	- Hamlib interface
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

#if !defined(UTILITIES_H__3B0BA660_CA63_4344_B3452345D31912__INCLUDED_)
#define UTILITIES_H__3B0BA660_CA63_4344_B3452345D31912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "Vector.h"
#include "../matlib/Matlib.h"
#include <iostream>

#ifdef HAVE_LIBHAMLIB
# include <hamlib/rig.h>
#endif


/* Definitions ****************************************************************/
#define	METER_FLY_BACK					15

#ifdef HAVE_LIBHAMLIB
/* Config string for com-port selection is different in Windows and Linux */
# ifdef _WIN32
#  define HAMLIB_CONF_COM1				"rig_pathname=COM1"
#  define HAMLIB_CONF_COM2				"rig_pathname=COM2"
#  define HAMLIB_CONF_COM3				"rig_pathname=COM3"
#  define HAMLIB_CONF_COM4				"rig_pathname=COM4"
#  define HAMLIB_CONF_COM5				"rig_pathname=COM5"
# else
#  define HAMLIB_CONF_COM1				"rig_pathname=/dev/ttyS0"
#  define HAMLIB_CONF_COM2				"rig_pathname=/dev/ttyS1"
#  define HAMLIB_CONF_COM3				"rig_pathname=/dev/ttyS2"
#  define HAMLIB_CONF_COM4				"rig_pathname=/dev/ttyS3"
#  define HAMLIB_CONF_COM5				"rig_pathname=/dev/ttyUSB0"
# endif
#endif


/* Classes ********************************************************************/
/* Signal level meter ------------------------------------------------------- */
class CSignalLevelMeter
{
public:
	CSignalLevelMeter() : rCurLevel((_REAL) 0.0) {}
	virtual ~CSignalLevelMeter() {}

	void Init(_REAL rStartVal) {rCurLevel = Abs(rStartVal);}
	void Update(const _REAL rVal);
	void Update(const CVector<_REAL> vecrVal);
	void Update(const CVector<_SAMPLE> vecsVal);
	_REAL Level();

protected:
	_REAL rCurLevel;
};


/* Bandpass filter ---------------------------------------------------------- */
class CDRMBandpassFilt
{
public:
	enum EFiltType {FT_TRANSMITTER, FT_RECEIVER};

	void Init(const int iNewBlockSize, const _REAL rOffsetHz,
		const ESpecOcc eSpecOcc, const EFiltType eNFiTy);
	void Process(CVector<_COMPLEX>& veccData);

protected:
	int				iBlockSize;

	CComplexVector	cvecDataTmp;

	CRealVector		rvecZReal; /* State memory real part */
	CRealVector		rvecZImag; /* State memory imaginary part */
	CRealVector		rvecDataReal;
	CRealVector		rvecDataImag;
	CFftPlans		FftPlanBP;
	CComplexVector	cvecB;
};


/* Modified Julian Date ----------------------------------------------------- */
class CModJulDate
{
public:
	CModJulDate() : iYear(0), iDay(0), iMonth(0) {}
	CModJulDate(const uint32_t iModJulDate) {Set(iModJulDate);}

	void Set(const uint32_t iModJulDate);

	int GetYear() {return iYear;}
	int GetDay() {return iDay;}
	int GetMonth() {return iMonth;}

protected:
	int iYear, iDay, iMonth;
};


/* Audio Reverbration ------------------------------------------------------- */
class CAudioReverb
{
public:
	CAudioReverb(const CReal rT60 = (CReal) 1.0);

	void Clear();
	CReal ProcessSample(const CReal rLInput, const CReal rRInput);

protected:
	void setT60(const CReal rT60);
	_BOOLEAN isPrime(const int number);

	CFIFO<int>	allpassDelays_[3];
	CFIFO<int>	combDelays_[4];
	CReal		allpassCoefficient_;
	CReal		combCoefficient_[4];
};


#ifdef HAVE_LIBHAMLIB
/* Hamlib interface --------------------------------------------------------- */
class CHamlib
{
public:
	enum ESMeterState {SS_VALID, SS_NOTVALID, SS_TIMEOUT};

	CHamlib();
	virtual ~CHamlib();

	class SDrRigCaps
	{
	public:
		SDrRigCaps() : iModelID(0), strManufacturer(""), strModelName(""),
			eRigStatus(RIG_STATUS_ALPHA), bIsSpecRig(FALSE) {}
		SDrRigCaps(rig_model_t tNID, QString strNMan, QString strNModN,
			rig_status_e eNSt, _BOOLEAN bNSRI) : iModelID(tNID),
			eRigStatus(eNSt), strManufacturer(strNMan), strModelName(strNModN),
			bIsSpecRig(bNSRI) {}
		SDrRigCaps(const SDrRigCaps& nSDRC) : iModelID(nSDRC.iModelID),
			strManufacturer(nSDRC.strManufacturer),
			strModelName(nSDRC.strModelName),
			eRigStatus(nSDRC.eRigStatus), bIsSpecRig(nSDRC.bIsSpecRig) {}

		inline SDrRigCaps& operator=(const SDrRigCaps& cNew)
		{
			iModelID = cNew.iModelID;
			strManufacturer = cNew.strManufacturer;
			strModelName = cNew.strModelName;
			eRigStatus = cNew.eRigStatus;
			bIsSpecRig = cNew.bIsSpecRig;
			return *this;
		}

		rig_model_t		iModelID;
		QString			strManufacturer;
		QString			strModelName;
		rig_status_e	eRigStatus;
		_BOOLEAN		bIsSpecRig;
	};

	_BOOLEAN		SetFrequency(const int iFreqkHz);
	ESMeterState	GetSMeter(_REAL& rCurSigStr);

	int				GetNumHamModels() {return veccapsHamlibModels.Size();}
	SDrRigCaps		GetHamModel(const int iIdx)
						{return veccapsHamlibModels[iIdx];}

	void			SetHamlibModelID(const int iNewM);
	int				GetHamlibModelID() const {return iHamlibModelID;}

	void			SetHamlibConf(const string strNewC);
	string			GetHamlibConf() const {return strHamlibConf;}

	void			SetEnableModRigSettings(const _BOOLEAN bNSM);
	_BOOLEAN		GetEnableModRigSettings() const {return bModRigSettings;}

protected:
	class CSpecDRMRig
	{
	public:
		CSpecDRMRig() : iModelID(0), strDRMSetMod(""), strDRMSetNoMod(""),
			iFreqOffs(0) {}
		CSpecDRMRig(const CSpecDRMRig& nSpec) :
			iModelID(nSpec.iModelID), strDRMSetMod(nSpec.strDRMSetMod),
			strDRMSetNoMod(nSpec.strDRMSetNoMod), iFreqOffs(nSpec.iFreqOffs) {}
		CSpecDRMRig(rig_model_t newID, string sSet, int iNFrOff,
			string sModSet) : iModelID(newID), strDRMSetNoMod(sSet),
			iFreqOffs(iNFrOff), strDRMSetMod(sModSet) {}

		rig_model_t	iModelID; /* Model ID for hamlib */
		string		strDRMSetMod; /* Special DRM settings (modified) */
		string		strDRMSetNoMod; /* Special DRM settings (not mod.) */
		int			iFreqOffs; /* Frequency offset */
	};

	CVector<CSpecDRMRig>	vecSpecDRMRigs;
	CVector<SDrRigCaps>		veccapsHamlibModels;

	void SortHamlibModelList(CVector<SDrRigCaps>& veccapsHamlMod);
	void EnableSMeter(const _BOOLEAN bStatus);

	static int			PrintHamlibModelList(const struct rig_caps* caps,
											 void* data);
	_BOOLEAN			CheckForSpecDRMFE(const rig_model_t iID, int& iIndex);

	RIG*				pRig;
	_BOOLEAN			bSMeterIsSupported;
	_BOOLEAN			bModRigSettings;
	rig_model_t			iHamlibModelID;
	string				strHamlibConf;
	int					iFreqOffset;
};
#endif


#endif // !defined(UTILITIES_H__3B0BA660_CA63_4344_B3452345D31912__INCLUDED_)
