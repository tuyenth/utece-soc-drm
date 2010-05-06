/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See Parameter.cpp
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

#if !defined(PARAMETER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
#define PARAMETER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_

#include "GlobalDefinitions.h"
#include "ofdmcellmapping/CellMappingTable.h"
#include "matlib/Matlib.h"


/* Classes ********************************************************************/
class CParameter : public CCellMappingTable
{
public:
	CParameter() : bRunThread(FALSE), Stream(MAX_NUM_STREAMS), iChanEstDelay(0),
		bUsingMultimedia(TRUE),	iCurSelAudioService(0), iCurSelDataService(0)
		{}
	virtual ~CParameter() {}

	/* Enumerations --------------------------------------------------------- */
	/* CA: CA system */
	enum ECACond {CA_USED, CA_NOT_USED};

	/* SF: Service Flag */
	enum ETyOServ {SF_AUDIO, SF_DATA};

	/* AS: AFS in SDC is valid or not */
	enum EAFSVali {AS_VALID, AS_NOT_VALID};

	/* PM: Packet Mode */
	enum EPackMod {PM_SYNCHRON_STR_MODE, PM_PACKET_MODE};

	/* DU: Data Unit */
	enum EDatUnit {DU_SINGLE_PACKETS, DU_DATA_UNITS};

	/* AD: Application Domain */
	enum EApplDomain {AD_DRM_SPEC_APP, AD_DAB_SPEC_APP, AD_OTHER_SPEC_APP};

	/* AC: Audio Coding */
	enum EAudCod {AC_AAC, AC_CELP, AC_HVXC};

	/* SB: SBR */
	enum ESBRFlag {SB_NOT_USED, SB_USED};

	/* AM: Audio Mode */
	enum EAudMode {AM_MONO, AM_P_STEREO, AM_STEREO};

	/* HR: HVXC Rate */
	enum EHVXCRate {HR_2_KBIT, HR_4_KBIT};

	/* AS: Audio Sampling rate */
	enum EAudSamRat {AS_8_KHZ, AS_12KHZ, AS_16KHZ, AS_24KHZ};

	/* CS: Coding Scheme */
	enum ECodScheme {CS_1_SM, CS_2_SM, CS_3_SM, CS_3_HMSYM, CS_3_HMMIX};

	/* SI: Symbol Interleaver */
	enum ESymIntMod {SI_LONG, SI_SHORT};

	/* CT: Channel Type */
	enum EChanType {CT_MSC, CT_SDC, CT_FAC};

	/* ST: Simulation Type */
	enum ESimType {ST_NONE, ST_BITERROR, ST_MSECHANEST, ST_BER_IDEALCHAN, ST_SYNC_PARAM};


	/* Classes -------------------------------------------------------------- */
	class CAudioParam
	{
	public:
		CAudioParam() : strTextMessage("") {}

		/* Text-message */
		string		strTextMessage; /* Max length is (8 * 16 Bytes) */	

		int			iStreamID; /* Stream Id of the stream which carries the audio service */

		EAudCod		eAudioCoding; /* This field indicated the source coding system */
		ESBRFlag	eSBRFlag; /* SBR flag */
		EAudSamRat	eAudioSamplRate; /* Audio sampling rate */
		_BOOLEAN	bTextflag; /* Indicates whether a text message is present or not */
		_BOOLEAN	bEnhanceFlag; /* Enhancement flag */


		/* For AAC: Mono, LC Stereo, Stereo --------------------------------- */
		EAudMode	eAudioMode; /* Audio mode */


		/* For CELP --------------------------------------------------------- */
		int			iCELPIndex; /* This field indicates the CELP bit rate index */
		_BOOLEAN	bCELPCRC; /* This field indicates whether the CRC is used or not*/


		/* For HVXC --------------------------------------------------------- */
		EHVXCRate	eHVXCRate; /* This field indicates the rate of the HVXC */
		_BOOLEAN	bHVXCCRC; /* This field indicates whether the CRC is used or not */

/* TODO: Copy operator. Now, default copy operator is used! */

		/* This function is needed for detection changes in the class */
		_BOOLEAN operator!=(const CAudioParam AudioParam)
		{
			if (iStreamID != AudioParam.iStreamID) return TRUE;
			if (eAudioCoding != AudioParam.eAudioCoding) return TRUE;
			if (eSBRFlag != AudioParam.eSBRFlag) return TRUE;
			if (eAudioSamplRate != AudioParam.eAudioSamplRate) return TRUE;
			if (bTextflag != AudioParam.bTextflag) return TRUE;
			if (bEnhanceFlag != AudioParam.bEnhanceFlag) return TRUE;

			switch (AudioParam.eAudioCoding)
			{
			case AC_AAC:
				if (eAudioMode != AudioParam.eAudioMode) return TRUE;
				break;

			case AC_CELP:
				if (bCELPCRC != AudioParam.bCELPCRC) return TRUE;
				if (iCELPIndex != AudioParam.iCELPIndex) return TRUE;
				break;

			case AC_HVXC:
				if (eHVXCRate != AudioParam.eHVXCRate) return TRUE;
				if (bHVXCCRC != AudioParam.bHVXCCRC) return TRUE;
				break;
			}
			return FALSE;
		}
	};

	class CDataParam
	{
	public:
		int			iStreamID; /* Stream Id of the stream which carries the data service */

		EPackMod	ePacketModInd; /* Packet mode indicator */


		/* In case of packet mode ------------------------------------------- */
		EDatUnit	eDataUnitInd; /* Data unit indicator */
		int			iPacketID; /* Packet Id (2 bits) */
		int			iPacketLen; /* Packet length */

		// "DAB specified application" not yet implemented!!!
		EApplDomain eAppDomain; /* Application domain */
		int			iUserAppIdent; /* User application identifier, only DAB */

/* TODO: Copy operator. Now, default copy operator is used! */

		/* This function is needed for detection changes in the class */
		_BOOLEAN operator!=(const CDataParam DataParam)
		{
			if (iStreamID != DataParam.iStreamID) return TRUE;
			if (ePacketModInd != DataParam.ePacketModInd) return TRUE;
			if (DataParam.ePacketModInd == PM_PACKET_MODE)
			{
				if (eDataUnitInd != DataParam.eDataUnitInd) return TRUE;
				if (iPacketID != DataParam.iPacketID) return TRUE;
				if (iPacketLen != DataParam.iPacketLen) return TRUE;
				if (eAppDomain != DataParam.eAppDomain) return TRUE;
				if (DataParam.eAppDomain == AD_DAB_SPEC_APP)
					if (iUserAppIdent != DataParam.iUserAppIdent) return TRUE;
			}
			return FALSE;
		}
	};

	class CService
	{
	public:
		CService() : strLabel("") {}

		_BOOLEAN IsActive() {return iServiceID != SERV_ID_NOT_USED;}

		uint32_t	iServiceID;
		ECACond		eCAIndication;
		int			iLanguage;
		ETyOServ	eAudDataFlag;
		int			iServiceDescr;

		/* Label of the service */
		string		strLabel;

		/* Audio parameters */
		CAudioParam	AudioParam;

		/* Data parameters */
		CDataParam	DataParam;
	};

	class CStream
	{
	public:
		CStream() : iLenPartA(0), iLenPartB(0) {}

		int	iLenPartA; /* Data length for part A */
		int	iLenPartB; /* Data length for part B */

		_BOOLEAN operator!=(const CStream Stream)
		{
			if (iLenPartA != Stream.iLenPartA) return TRUE;
			if (iLenPartB != Stream.iLenPartB) return TRUE;
			return FALSE;
		}
	};

	class CMSCProtLev
	{
	public:
		int	iPartA; /* MSC protection level for part A */
		int	iPartB; /* MSC protection level for part B */
		int	iHierarch; /* MSC protection level for hierachical frame */

		CMSCProtLev& operator=(const CMSCProtLev& NewMSCProtLev)
		{
			iPartA = NewMSCProtLev.iPartA;
			iPartB = NewMSCProtLev.iPartB;
			iHierarch = NewMSCProtLev.iHierarch;
			return *this; 
		}
	};

	/* Alternative frequency signalling class */
	class CAltFreqSign
	{
	public:
		CAltFreqSign() {Reset();}

		class CAltFreq
		{
		public:
			CAltFreq() {Reset();}
			CAltFreq(const CAltFreq& nAF) :
				veciFrequencies(nAF.veciFrequencies),
				veciServRestrict(nAF.veciServRestrict),
				bIsSyncMultplx(nAF.bIsSyncMultplx),
				iRegionID(nAF.iRegionID), iScheduleID(nAF.iScheduleID) {}

			CAltFreq& operator=(const CAltFreq& nAF)
			{
				veciFrequencies.Init(nAF.veciFrequencies.Size());
				veciFrequencies = nAF.veciFrequencies;

				veciServRestrict.Init(nAF.veciServRestrict.Size());
				veciServRestrict = nAF.veciServRestrict;

				bIsSyncMultplx = nAF.bIsSyncMultplx;
				iRegionID = nAF.iRegionID;
				iScheduleID = nAF.iScheduleID;
				return *this; 
			}

			_BOOLEAN operator==(const CAltFreq& nAF)
			{
				int i;

				/* Vector sizes */
				if (veciFrequencies.Size() != nAF.veciFrequencies.Size()) return FALSE;
				if (veciServRestrict.Size() != nAF.veciServRestrict.Size()) return FALSE;

				/* Vector contents */
				for (i = 0; i < veciFrequencies.Size(); i++)
					if (veciFrequencies[i] != nAF.veciFrequencies[i]) return FALSE;
				for (i = 0; i < veciServRestrict.Size(); i++)
					if (veciServRestrict[i] != nAF.veciServRestrict[i]) return FALSE;

				if (bIsSyncMultplx != nAF.bIsSyncMultplx) return FALSE;
				if (iRegionID != nAF.iRegionID) return FALSE;
				if (iScheduleID != nAF.iScheduleID) return FALSE;
				return TRUE;
			}

			void Reset()
			{
				veciFrequencies.Init(0);
				veciServRestrict.Init(MAX_NUM_SERVICES, 0);
				bIsSyncMultplx = FALSE;
				iRegionID = iScheduleID = 0;
			}

			CVector<int>	veciFrequencies;
			CVector<int>	veciServRestrict;
			_BOOLEAN		bIsSyncMultplx;
			int				iRegionID;
			int				iScheduleID;
		};

		void Reset() {vecAltFreq.Init(0); bVersionFlag = FALSE;}

		CVector<CAltFreq>	vecAltFreq;
		_BOOLEAN			bVersionFlag;
	} AltFreqSign;

	void			ResetServicesStreams();
	void			GetActiveServices(CVector<int>& veciActServ);
	void			GetActiveStreams(CVector<int>& veciActStr);
	int				GetNumActiveServices();
	void			InitCellMapTable(const ERobMode eNewWaveMode, const ESpecOcc eNewSpecOcc);

	void			SetNumDecodedBitsMSC(const int iNewNumDecodedBitsMSC);
	void			SetNumDecodedBitsSDC(const int iNewNumDecodedBitsSDC);
	void			SetNumBitsHieraFrTot(const int iNewNumBitsHieraFrTot);
	void			SetNumAudioDecoderBits(const int iNewNumAudioDecoderBits);
	void			SetNumDataDecoderBits(const int iNewNumDataDecoderBits);

	_BOOLEAN		SetWaveMode(const ERobMode eNewWaveMode);
	ERobMode		GetWaveMode() const {return eRobustnessMode;}

	void			SetCurSelAudioService(const int iNewService);
	int				GetCurSelAudioService() const {return iCurSelAudioService;}
	void			SetCurSelDataService(const int iNewService);
	int				GetCurSelDataService() const {return iCurSelDataService;}

	void			ResetCurSelAudDatServ() {iCurSelAudioService = 0; iCurSelDataService = 0;}

	void			EnableMultimedia(const _BOOLEAN bFlag);
	_BOOLEAN		GetEnableMultimedia() const {return bUsingMultimedia;}

	_REAL			GetDCFrequency() const {return SOUNDCRD_SAMPLE_RATE *
						(rFreqOffsetAcqui + rFreqOffsetTrack);}
	_REAL			GetSampFreqEst() const {return rResampleOffset;}

	_REAL			GetBitRateKbps(const int iServiceID, const _BOOLEAN bAudData);
	_REAL			PartABLenRatio(const int iServiceID);


	/* Parameters controlled by FAC ----------------------------------------- */
	void			SetInterleaverDepth(const ESymIntMod eNewDepth);
	ESymIntMod		GetInterleaverDepth() {return eSymbolInterlMode;}

	void			SetMSCCodingScheme(const ECodScheme eNewScheme);
	void			SetSDCCodingScheme(const ECodScheme eNewScheme);

	void			SetSpectrumOccup(ESpecOcc eNewSpecOcc);
	ESpecOcc		GetSpectrumOccup() const {return eSpectOccup;}

	void			SetNumOfServices(const int iNNumAuSe, const int iNNumDaSe);
	int				GetTotNumServices()
						{return iNumAudioService + iNumDataService;}

	void			SetAudDataFlag(const int iServID, const ETyOServ iNewADaFl);
	void			SetServID(const int iServID, const uint32_t iNewServID);


	/* Symbol interleaver mode (long or short interleaving) */
	ESymIntMod		eSymbolInterlMode; 

	ECodScheme		eMSCCodingScheme; /* MSC coding scheme */
	ECodScheme		eSDCCodingScheme; /* SDC coding scheme */


	int				iNumAudioService;
	int				iNumDataService;


	/* Parameters controlled by SDC ----------------------------------------- */
	void SetAudioParam(const int iShortID, const CAudioParam NewAudParam);
	CAudioParam GetAudioParam(const int iShortID)
		{return Service[iShortID].AudioParam;}
	void SetDataParam(const int iShortID, const CDataParam NewDataParam);
	CDataParam GetDataParam(const int iShortID)
		{return Service[iShortID].DataParam;}

	void SetMSCProtLev(const CMSCProtLev NewMSCPrLe, const _BOOLEAN bWithHierarch);
	void SetStreamLen(const int iStreamID, const int iNewLenPartA, const int iNewLenPartB);

	/* Protection levels for MSC */
	CMSCProtLev			MSCPrLe;

	CVector<CStream>	Stream;
	CService			Service[MAX_NUM_SERVICES];

	/* These values are used to set input and output block sizes of some
	   modules */
	int					iNumBitsHierarchFrameTotal;
	int					iNumDecodedBitsMSC;
	int					iNumSDCBitsPerSFrame; /* Number of SDC bits per super-frame */
	int					iNumAudioDecoderBits; /* Number of input bits for audio module */
	int					iNumDataDecoderBits; /* Number of input bits for data decoder module */

	/* Date */
	int					iYear;
	int					iMonth;
	int					iDay;

	/* UTC (hours and minutes) */
	int					iUTCHour;
	int					iUTCMin;
	


	/* Identifies the current frame. This parameter is set by FAC */
	int					iFrameIDTransm;
	int					iFrameIDReceiv;


	/* Synchronization ------------------------------------------------------ */
	_REAL				rFreqOffsetAcqui;
	_REAL				rFreqOffsetTrack;

	_REAL				rResampleOffset;

	int					iTimingOffsTrack;


	/* Reception log -------------------------------------------------------- */
	class CReceptLog
	{
	public:
		CReceptLog();
		virtual ~CReceptLog() {CloseFile(pFileLong, TRUE);
			CloseFile(pFileShort, FALSE);}

		void SetFAC(const _BOOLEAN bCRCOk);
		void SetMSC(const _BOOLEAN bCRCOk);
		void SetSync(const _BOOLEAN bCRCOk);
		void SetSNR(const _REAL rNewCurSNR);
		void SetNumAAC(const int iNewNum);
		void SetLog(const _BOOLEAN bLog);
		void SetLogHeader(FILE* pFile, const _BOOLEAN bIsLong);
		void SetFrequency(const int iNewFreq) {iFrequency = iNewFreq;}
		int GetFrequency() {return iFrequency;}
		void SetLatitude(const string strLat) {strLatitude = strLat;}
		string GetLatitude() {return strLatitude;}
		void SetLongitude(const string strLon) {strLongitude = strLon;}
		string GetLongitude() {return strLongitude;}
		void SetAdditText(const string strNewTxt) {strAdditText = strNewTxt;}
		void WriteParameters(const _BOOLEAN bIsLong);
		void SetDelLogStart(const int iSecDel);
		_BOOLEAN IsDelLogStart() {return bDelayedLogStart;}
		int GetDelLogStart() {return iSecDelLogStart;}

		void ResetTransParams();
		void SetMSCScheme(const ECodScheme eNewMCS) {eCurMSCScheme = eNewMCS;}
		void SetRobMode(const ERobMode eNewRM) {eCurRobMode = eNewRM;}
		void SetProtLev(const CMSCProtLev eNPL) {CurProtLev = eNPL;}

	protected:
		void ResetLog(const _BOOLEAN bIsLong);
		void CloseFile(FILE* pFile, const _BOOLEAN bIsLong);
		int				iNumSNR;
		int				iNumCRCOkFAC, iNumCRCOkMSC;
		int				iNumCRCOkMSCLong, iNumCRCMSCLong;
		int				iNumAACFrames, iTimeCntShort;
		time_t			TimeCntLong;
		_BOOLEAN		bSyncOK, bFACOk, bMSCOk;
		_BOOLEAN		bSyncOKValid, bFACOkValid, bMSCOkValid;
		int				iFrequency;
		_REAL			rAvSNR, rCurSNR;
		_REAL			rMaxSNR, rMinSNR;
		_BOOLEAN		bLogActivated;
		FILE*			pFileLong;
		FILE*			pFileShort;
		string			strAdditText;
		string			strLatitude;
		string			strLongitude;
		_BOOLEAN		bDelayedLogStart;
		int				iSecDelLogStart;

		ERobMode		eCurRobMode;
		ECodScheme		eCurMSCScheme;
		CMSCProtLev		CurProtLev;

		CMutex			Mutex;
	} ReceptLog;


	/* Simulation ----------------------------------------------------------- */
	ESimType			eSimType;

	int					iDRMChannelNum;
	int					iChan8Doppler;
	_REAL				rBitErrRate;
	_REAL				rSyncTestParam; /* For any other simulations, used
										   with "ST_SYNC_PARAM" type */
	int					iNumBitErrors;
	int					iChanEstDelay;

	int					iNumTaps;
	int					iPathDelay[MAX_NUM_TAPS_DRM_CHAN];
	_REAL				rGainCorr;
	int					iOffUsfExtr;

	void SetNominalSNRdB(const _REAL rSNRdBNominal);
	_REAL GetNominalSNRdB();
	void SetSystemSNRdB(const _REAL rSNRdBSystem) {rSysSimSNRdB = rSNRdBSystem;}
	_REAL GetSystemSNRdB() const {return rSysSimSNRdB;}
	_REAL GetSysSNRdBPilPos() const;

	/* Simulation raw-data management. We have to implement a shift register
	   with varying size. We do that by adding a variable for storing the
	   current write position. */
	class CRawSimData
	{
	/* We have to implement a shift register with varying size. We do that
	   by adding a variable for storing the current write position. We use
	   always the first value of the array for reading and do a shift of the
	   other data by adding a arbitrary value (0) at the end of the whole
	   shift register */
	public:
		/* Here, the maximal size of the shift register is set */
		CRawSimData() : ciMaxDelBlocks(50), iCurWritePos(0) 
			{veciShRegSt.Init(ciMaxDelBlocks);}

		void Add(uint32_t iNewSRS);
		uint32_t Get();

		void Reset() {iCurWritePos = 0;}

	protected:
		/* Max number of delayed blocks */
		int							ciMaxDelBlocks;
		CShiftRegister<uint32_t>	veciShRegSt;
		int							iCurWritePos;
	} RawSimDa;


	/* General -------------------------------------------------------------- */
	_REAL				GetSysToNomBWCorrFact();
	_BOOLEAN			bRunThread;
	_BOOLEAN			bUsingMultimedia;

protected:
	_REAL				rSysSimSNRdB;

	/* Current selected audio service for processing */
	int					iCurSelAudioService;
	int					iCurSelDataService;

	ERobMode			eRobustnessMode; /* E.g.: Mode A, B, C or D */
	ESpecOcc			eSpectOccup;
};


#endif // !defined(PARAMETER_H__3B0BA660_CA63_4344_BB2B_23E7A0D31912__INCLUDED_)
