/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	DRM Parameters
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

#include "Parameter.h"


// To be replaced by something nicer!!! TODO
#include "DrmReceiver.h"
extern CDRMReceiver	DRMReceiver;


/* Implementation *************************************************************/
void CParameter::ResetServicesStreams()
{
	int i;

	/* Reset everything to possible start values */
	for (i = 0; i < MAX_NUM_SERVICES; i++)
	{
		Service[i].AudioParam.strTextMessage = "";
		Service[i].AudioParam.iStreamID = STREAM_ID_NOT_USED;
		Service[i].AudioParam.eAudioCoding = AC_AAC;
		Service[i].AudioParam.eSBRFlag = SB_NOT_USED;
		Service[i].AudioParam.eAudioSamplRate = AS_24KHZ;
		Service[i].AudioParam.bTextflag = FALSE;
		Service[i].AudioParam.bEnhanceFlag = FALSE;
		Service[i].AudioParam.eAudioMode = AM_MONO;
		Service[i].AudioParam.iCELPIndex = 0;
		Service[i].AudioParam.bCELPCRC = FALSE;
		Service[i].AudioParam.eHVXCRate = HR_2_KBIT;
		Service[i].AudioParam.bHVXCCRC = FALSE;

		Service[i].DataParam.iStreamID = STREAM_ID_NOT_USED;
		Service[i].DataParam.ePacketModInd = PM_PACKET_MODE;
		Service[i].DataParam.eDataUnitInd = DU_SINGLE_PACKETS;
		Service[i].DataParam.iPacketID = 0;
		Service[i].DataParam.iPacketLen = 0;
		Service[i].DataParam.eAppDomain = AD_DRM_SPEC_APP;

		Service[i].iServiceID = SERV_ID_NOT_USED;
		Service[i].eCAIndication = CA_NOT_USED;
		Service[i].iLanguage = 0;
		Service[i].eAudDataFlag = SF_AUDIO;
		Service[i].iServiceDescr = 0;
		Service[i].strLabel = "";
	}

	for (i = 0; i < MAX_NUM_STREAMS; i++)
	{
		Stream[i].iLenPartA = 0;
		Stream[i].iLenPartB = 0;
	}
}

int CParameter::GetNumActiveServices()
{
	int iNumAcServ = 0;

	for (int i = 0; i < MAX_NUM_SERVICES; i++)
	{
		if (Service[i].IsActive())
			iNumAcServ++;
	}

	return iNumAcServ;
}

void CParameter::GetActiveServices(CVector<int>& veciActServ)
{
	CVector<int> vecbServices(MAX_NUM_SERVICES, 0);

	/* Init return vector */
	veciActServ.Init(0);

	/* Get active services */
	int iNumServices = 0;
	for (int i = 0; i < MAX_NUM_SERVICES; i++)
	{
		if (Service[i].IsActive())
		{
			/* A service is active, add ID to vector */
			veciActServ.Add(i);
			iNumServices++;
		}
	}
}

void CParameter::GetActiveStreams(CVector<int>& veciActStr)
{
	int					i;
	int					iNumStreams;
	CVector<int>		vecbStreams(MAX_NUM_STREAMS, 0);

	/* Determine which streams are active */
	for (i = 0; i < MAX_NUM_SERVICES; i++)
	{
		if (Service[i].IsActive())
		{
			/* Audio stream */
			if (Service[i].AudioParam.iStreamID != STREAM_ID_NOT_USED)
				vecbStreams[Service[i].AudioParam.iStreamID] = 1;

			/* Data stream */
			if (Service[i].DataParam.iStreamID != STREAM_ID_NOT_USED)
				vecbStreams[Service[i].DataParam.iStreamID] = 1;
		}
	}

	/* Now, count streams */
	iNumStreams = 0;
	for (i = 0; i < MAX_NUM_STREAMS; i++)
		if (vecbStreams[i] == 1)
			iNumStreams++;

	/* Now that we know how many streams are active, dimension vector */
	veciActStr.Init(iNumStreams);

	/* Store IDs of active streams */
	iNumStreams = 0;
	for (i = 0; i < MAX_NUM_STREAMS; i++)
	{
		if (vecbStreams[i] == 1)
		{
			veciActStr[iNumStreams] = i;
			iNumStreams++;
		}
	}
}

_REAL CParameter::GetBitRateKbps(const int iServiceID, const _BOOLEAN bAudData)
{
	int iNoBitsPerFrame;
			
	/* Init lengths to zero in case the stream is not yet assigned */
	int iLenPartA = 0;
	int iLenPartB = 0;

	/* First, check if audio or data service and get lengths */
	if (Service[iServiceID].eAudDataFlag == SF_AUDIO)
	{
		/* Check if we want to get the data stream connected to an audio
		   stream */
		if (bAudData == TRUE)
		{
			if (Service[iServiceID].DataParam.iStreamID != STREAM_ID_NOT_USED)
			{
				iLenPartA =
					Stream[Service[iServiceID].DataParam.iStreamID].iLenPartA;

				iLenPartB =
					Stream[Service[iServiceID].DataParam.iStreamID].iLenPartB;
			}
		}
		else
		{
			if (Service[iServiceID].AudioParam.iStreamID != STREAM_ID_NOT_USED)
			{
				iLenPartA =
					Stream[Service[iServiceID].AudioParam.iStreamID].iLenPartA;

				iLenPartB =
					Stream[Service[iServiceID].AudioParam.iStreamID].iLenPartB;
			}
		}
	}
	else
	{
		if (Service[iServiceID].DataParam.iStreamID != STREAM_ID_NOT_USED)
		{
			iLenPartA =
				Stream[Service[iServiceID].DataParam.iStreamID].iLenPartA;

			iLenPartB =
				Stream[Service[iServiceID].DataParam.iStreamID].iLenPartB;
		}
	}

	/* Total length in bits */
	iNoBitsPerFrame = (iLenPartA + iLenPartB) * SIZEOF__BYTE;

	/* We have 3 frames with time duration of 1.2 seconds. Bit rate should be
	   returned in kbps (/ 1000) */
	return (_REAL) iNoBitsPerFrame * 3 / 1.2 / 1000;
}

_REAL CParameter::PartABLenRatio(const int iServiceID)
{
	int iLenA = 0;
	int iLenB = 0;

	/* Get the length of protection part A and B */
	if (Service[iServiceID].eAudDataFlag == SF_AUDIO)
	{
		/* Audio service */
		if (Service[iServiceID].AudioParam.iStreamID != STREAM_ID_NOT_USED)
		{
			iLenA = Stream[Service[iServiceID].AudioParam.iStreamID].iLenPartA;
			iLenB = Stream[Service[iServiceID].AudioParam.iStreamID].iLenPartB;
		}
	}
	else
	{
		/* Data service */
		if (Service[iServiceID].DataParam.iStreamID != STREAM_ID_NOT_USED)
		{
			iLenA = Stream[Service[iServiceID].DataParam.iStreamID].iLenPartA;
			iLenB = Stream[Service[iServiceID].DataParam.iStreamID].iLenPartB;
		}
	}

	const int iTotLen = iLenA + iLenB;

	if (iTotLen != 0)
		return (_REAL) iLenA / iTotLen;
	else
		return (_REAL) 0.0;
}

void CParameter::InitCellMapTable(const ERobMode eNewWaveMode,
								  const ESpecOcc eNewSpecOcc)
{
	/* Set new values and make table */
	eRobustnessMode = eNewWaveMode;
	eSpectOccup = eNewSpecOcc;
	MakeTable(eRobustnessMode, eSpectOccup);
}

_BOOLEAN CParameter::SetWaveMode(const ERobMode eNewWaveMode)
{
	/* First check if spectrum occupancy and robustness mode pair is defined */
	if ((
		(eNewWaveMode == RM_ROBUSTNESS_MODE_C) || 
		(eNewWaveMode == RM_ROBUSTNESS_MODE_D)
		) && !(
		(eSpectOccup == SO_3) ||
		(eSpectOccup == SO_5)
		))
	{
		/* Set spectrum occupance to a valid parameter */
		eSpectOccup = SO_3;
	}

	/* Store new value in reception log */
	ReceptLog.SetRobMode(eNewWaveMode);

	/* Apply changes only if new paramter differs from old one */
	if (eRobustnessMode != eNewWaveMode)
	{
		/* Set new value */
		eRobustnessMode = eNewWaveMode;

		/* This parameter change provokes update of table */
		MakeTable(eRobustnessMode, eSpectOccup);

		/* Set init flags */
		DRMReceiver.InitsForWaveMode();

		/* Signal that parameter has changed */
		return TRUE;
	}
	else
		return FALSE;
}

void CParameter::SetSpectrumOccup(ESpecOcc eNewSpecOcc)
{
	/* First check if spectrum occupancy and robustness mode pair is defined */
	if ((
		(eRobustnessMode == RM_ROBUSTNESS_MODE_C) || 
		(eRobustnessMode == RM_ROBUSTNESS_MODE_D)
		) && !(
		(eNewSpecOcc == SO_3) ||
		(eNewSpecOcc == SO_5)
		))
	{
		/* Set spectrum occupance to a valid parameter */
		eNewSpecOcc = SO_3;
	}

	/* Apply changes only if new paramter differs from old one */
	if (eSpectOccup != eNewSpecOcc)
	{
		/* Set new value */
		eSpectOccup = eNewSpecOcc;

		/* This parameter change provokes update of table */
		MakeTable(eRobustnessMode, eSpectOccup);

		/* Set init flags */
		DRMReceiver.InitsForSpectrumOccup();
	}
}

void CParameter::SetStreamLen(const int iStreamID, const int iNewLenPartA,
							  const int iNewLenPartB)
{
	/* Apply changes only if parameters have changed */
	if ((Stream[iStreamID].iLenPartA != iNewLenPartA) ||
		(Stream[iStreamID].iLenPartB != iNewLenPartB))
	{
		/* Use new parameters */
		Stream[iStreamID].iLenPartA = iNewLenPartA;
		Stream[iStreamID].iLenPartB = iNewLenPartB;

		/* Set init flags */
		DRMReceiver.InitsForMSC();
	}
}

void CParameter::SetNumDecodedBitsMSC(const int iNewNumDecodedBitsMSC)
{
	/* Apply changes only if parameters have changed */
	if (iNewNumDecodedBitsMSC != iNumDecodedBitsMSC)
	{
		iNumDecodedBitsMSC = iNewNumDecodedBitsMSC;

		/* Set init flags */
		DRMReceiver.InitsForMSCDemux();
	}
}

void CParameter::SetNumDecodedBitsSDC(const int iNewNumDecodedBitsSDC)
{
	/* Apply changes only if parameters have changed */
	if (iNewNumDecodedBitsSDC != iNumSDCBitsPerSFrame)
	{
		iNumSDCBitsPerSFrame = iNewNumDecodedBitsSDC;

		/* Set init flags */
		DRMReceiver.InitsForNoDecBitsSDC();
	}
}

void CParameter::SetNumBitsHieraFrTot(const int iNewNumBitsHieraFrTot)
{
	/* Apply changes only if parameters have changed */
	if (iNewNumBitsHieraFrTot != iNumBitsHierarchFrameTotal)
	{
		iNumBitsHierarchFrameTotal = iNewNumBitsHieraFrTot;

		/* Set init flags */
		DRMReceiver.InitsForMSCDemux();
	}
}

void CParameter::SetNumAudioDecoderBits(const int iNewNumAudioDecoderBits)
{
	/* Apply changes only if parameters have changed */
	if (iNewNumAudioDecoderBits != iNumAudioDecoderBits)
	{
		iNumAudioDecoderBits = iNewNumAudioDecoderBits;

		/* Set init flags */
		DRMReceiver.InitsForAudParam();
	}
}

void CParameter::SetNumDataDecoderBits(const int iNewNumDataDecoderBits)
{
	/* Apply changes only if parameters have changed */
	if (iNewNumDataDecoderBits != iNumDataDecoderBits)
	{
		iNumDataDecoderBits = iNewNumDataDecoderBits;

		/* Set init flags */
		DRMReceiver.InitsForDataParam();
	}
}

void CParameter::SetMSCProtLev(const CMSCProtLev NewMSCPrLe,
							   const _BOOLEAN bWithHierarch)
{
	_BOOLEAN bParamersHaveChanged = FALSE;

	if ((NewMSCPrLe.iPartA != MSCPrLe.iPartA) ||
		(NewMSCPrLe.iPartB != MSCPrLe.iPartB))
	{
		MSCPrLe.iPartA = NewMSCPrLe.iPartA;
		MSCPrLe.iPartB = NewMSCPrLe.iPartB;

		bParamersHaveChanged = TRUE;
	}

	/* Apply changes only if parameters have changed */
	if (bWithHierarch == TRUE)
	{
		if (NewMSCPrLe.iHierarch != MSCPrLe.iHierarch)
		{
			MSCPrLe.iHierarch = NewMSCPrLe.iHierarch;
		
			bParamersHaveChanged = TRUE;
		}
	}

	/* In case parameters have changed, set init flags */
	if (bParamersHaveChanged == TRUE)
		DRMReceiver.InitsForMSC();

	/* Set new protection levels in reception log file */
	ReceptLog.SetProtLev(MSCPrLe);
}

void CParameter::SetAudioParam(const int iShortID,
							   const CAudioParam NewAudParam)
{
	/* Apply changes only if parameters have changed */
	//if (Service[iShortID].AudioParam != NewAudParam)
	{
		Service[iShortID].AudioParam = NewAudParam;

		/* Set init flags */
		DRMReceiver.InitsForAudParam();
	}
}

void CParameter::SetDataParam(const int iShortID, const CDataParam NewDataParam)
{
	/* Apply changes only if parameters have changed */
	//if (Service[iShortID].DataParam != NewDataParam)
	{
		Service[iShortID].DataParam = NewDataParam;

		/* Set init flags */
		DRMReceiver.InitsForDataParam();
	}
}

void CParameter::SetInterleaverDepth(const ESymIntMod eNewDepth)
{
	if (eSymbolInterlMode != eNewDepth)
	{
		eSymbolInterlMode = eNewDepth;

		/* Set init flags */
		DRMReceiver.InitsForInterlDepth();
	}
}

void CParameter::SetMSCCodingScheme(const ECodScheme eNewScheme)
{
	if (eMSCCodingScheme != eNewScheme)
	{
		eMSCCodingScheme = eNewScheme;

		/* Set init flags */
		DRMReceiver.InitsForMSCCodSche();
	}

	/* Set new coding scheme in reception log */
	ReceptLog.SetMSCScheme(eNewScheme);
}

void CParameter::SetSDCCodingScheme(const ECodScheme eNewScheme)
{
	if (eSDCCodingScheme != eNewScheme)
	{
		eSDCCodingScheme = eNewScheme;

		/* Set init flags */
		DRMReceiver.InitsForSDCCodSche();
	}
}

void CParameter::SetCurSelAudioService(const int iNewService)
{
	/* Change the current selected audio service ID only if the new ID does
	   contain an audio service. If not, keep the old ID. In that case it is
	   possible to select a "data-only" service and still listen to the audio of
	   the last selected service */
	if ((iCurSelAudioService != iNewService) &&
		(Service[iNewService].AudioParam.iStreamID != STREAM_ID_NOT_USED))
	{
		iCurSelAudioService = iNewService;

		/* Set init flags */
		DRMReceiver.InitsForMSCDemux();
	}
}

void CParameter::SetCurSelDataService(const int iNewService)
{
	/* Change the current selected data service ID only if the new ID does
	   contain a data service. If not, keep the old ID. In that case it is
	   possible to select a "data-only" service and click back to an audio
	   service to be able to decode data service and listen to audio at the
	   same time */
	if ((iCurSelDataService != iNewService) &&
		(Service[iNewService].DataParam.iStreamID != STREAM_ID_NOT_USED))
	{
		iCurSelDataService = iNewService;

		/* Set init flags */
		DRMReceiver.InitsForMSCDemux();
	}
}

void CParameter::EnableMultimedia(const _BOOLEAN bFlag)
{
	if (bUsingMultimedia != bFlag)
	{
		bUsingMultimedia = bFlag;

		/* Set init flags */
		DRMReceiver.InitsForMSCDemux();
	}
}

void CParameter::SetNumOfServices(const int iNNumAuSe, const int iNNumDaSe)
{
	/* Check whether number of activated services is not greater than the
	   number of services signalled by the FAC because it can happen that
	   a false CRC check (it is only a 8 bit CRC) of the FAC block
	   initializes a wrong service */
	if (GetNumActiveServices() > iNNumAuSe + iNNumDaSe)
	{
		/* Reset services and streams and set flag for init modules */
		ResetServicesStreams();
		DRMReceiver.InitsForMSCDemux();
	}

	if ((iNumAudioService != iNNumAuSe) || (iNumDataService != iNNumDaSe))
	{
		iNumAudioService = iNNumAuSe;
		iNumDataService = iNNumDaSe;

		/* Set init flags */
		DRMReceiver.InitsForMSCDemux();
	}
}

void CParameter::SetAudDataFlag(const int iServID, const ETyOServ iNewADaFl)
{
	if (Service[iServID].eAudDataFlag != iNewADaFl)
	{
		Service[iServID].eAudDataFlag = iNewADaFl;

		/* Set init flags */
		DRMReceiver.InitsForMSC();
	}
}

void CParameter::SetServID(const int iServID, const uint32_t iNewServID)
{
	if (Service[iServID].iServiceID != iNewServID)
	{
		Service[iServID].iServiceID = iNewServID;

		/* Set init flags */
		DRMReceiver.InitsForMSC();
	}
}


/* Implementaions for simulation -------------------------------------------- */
void CParameter::CRawSimData::Add(uint32_t iNewSRS) 
{
	/* Attention, function does not take care of overruns, data will be
	   lost if added to a filled shift register! */
	if (iCurWritePos < ciMaxDelBlocks) 
		veciShRegSt[iCurWritePos++] = iNewSRS;
}

uint32_t CParameter::CRawSimData::Get() 
{
	/* We always use the first value of the array for reading and do a
	   shift of the other data by adding a arbitrary value (0) at the
	   end of the whole shift register */
	uint32_t iRet = veciShRegSt[0];
	veciShRegSt.AddEnd(0);
	iCurWritePos--;

	return iRet;
}

_REAL CParameter::GetSysSNRdBPilPos() const
{
/*
	Get system SNR in dB for the pilot positions. Since the average power of
	the pilots is higher than the data cells, the SNR is also higher at these
	positions compared to the total SNR of the DRM signal.
*/
	return (_REAL) 10.0 * log10(pow((_REAL) 10.0, rSysSimSNRdB / 10) /
		rAvPowPerSymbol * rAvScatPilPow * (_REAL) iNumCarrier);
}

_REAL CParameter::GetNominalSNRdB()
{
	/* Convert SNR from system bandwidth to nominal bandwidth */
	return (_REAL) 10.0 * log10(pow((_REAL) 10.0, rSysSimSNRdB / 10) *
		GetSysToNomBWCorrFact());
}

void CParameter::SetNominalSNRdB(const _REAL rSNRdBNominal)
{
	/* Convert SNR from nominal bandwidth to system bandwidth */
	rSysSimSNRdB = (_REAL) 10.0 * log10(pow((_REAL) 10.0, rSNRdBNominal / 10) /
		GetSysToNomBWCorrFact());
}

_REAL CParameter::GetSysToNomBWCorrFact()
{
	_REAL rNomBW;

	/* Nominal bandwidth as defined in the DRM standard */
	switch (eSpectOccup)
	{
	case SO_0:
		rNomBW = (_REAL) 4500.0; // Hz
		break;

	case SO_1:
		rNomBW = (_REAL) 5000.0; // Hz
		break;

	case SO_2:
		rNomBW = (_REAL) 9000.0; // Hz
		break;

	case SO_3:
		rNomBW = (_REAL) 10000.0; // Hz
		break;

	case SO_4:
		rNomBW = (_REAL) 18000.0; // Hz
		break;

	case SO_5:
		rNomBW = (_REAL) 20000.0; // Hz
		break;

	default:
		rNomBW = (_REAL) 10000.0; // Hz
		break;
	}

	/* Calculate system bandwidth (N / T_u) */
	const _REAL rSysBW = (_REAL) iNumCarrier /
		iFFTSizeN * SOUNDCRD_SAMPLE_RATE;

	return rSysBW / rNomBW;
}


/* Reception log implementation --------------------------------------------- */
CParameter::CReceptLog::CReceptLog() : iNumAACFrames(10), pFileLong(NULL),
	pFileShort(NULL), iFrequency(0), strAdditText(""), strLatitude(""),
	strLongitude(""), bDelayedLogStart(FALSE), iSecDelLogStart(0)
{
	ResetLog(TRUE);
	ResetLog(FALSE);
}

void CParameter::CReceptLog::SetDelLogStart(const int iSecDel)
{
	/* If the parameter is 0, no delayed log file shall be used */
	if (iSecDel > 0)
	{
		bDelayedLogStart = TRUE;
		iSecDelLogStart = iSecDel;
	}
}

void CParameter::CReceptLog::ResetLog(const _BOOLEAN bIsLong)
{
	if (bIsLong == TRUE)
	{
		bSyncOK = TRUE;
		bFACOk = TRUE;
		bMSCOk = TRUE;

		/* Invalidate flags for initialization */
		bSyncOKValid = FALSE;
		bFACOkValid = FALSE;
		bMSCOkValid = FALSE;

		/* Reset total number of checked CRCs and number of CRC ok */
		iNumCRCMSCLong = 0;
		iNumCRCOkMSCLong = 0;

		rCurSNR = (_REAL) 0.0;
	}
	else
	{
		iNumCRCOkFAC = 0;
		iNumCRCOkMSC = 0;
		iNumSNR = 0;
		rAvSNR = (_REAL) 0.0;
	}
}

void CParameter::CReceptLog::ResetTransParams()
{
	/* Reset transmission parameters */
	eCurMSCScheme = CParameter::CS_3_SM;
	eCurRobMode = RM_NO_MODE_DETECTED;
	CurProtLev.iPartA = 0;
	CurProtLev.iPartB = 0;
	CurProtLev.iHierarch = 0;
}

void CParameter::CReceptLog::SetSync(const _BOOLEAN bCRCOk)
{
	if (bLogActivated == TRUE)
	{
		Mutex.Lock();

		/* If one of the syncs were wrong in one second, set to false */
		if (bCRCOk == FALSE)
			bSyncOK = FALSE;

		/* Validate sync flag */
		bSyncOKValid = TRUE;

		Mutex.Unlock();
	}
}

void CParameter::CReceptLog::SetFAC(const _BOOLEAN bCRCOk)
{
	if (bLogActivated == TRUE)
	{
		Mutex.Lock();

		if (bCRCOk == TRUE)
			iNumCRCOkFAC++;
		else
			bFACOk = FALSE;

		/* Validate FAC flag */
		bFACOkValid = TRUE;

		Mutex.Unlock();
	}
}

void CParameter::CReceptLog::SetMSC(const _BOOLEAN bCRCOk)
{
	if (bLogActivated == TRUE)
	{
		Mutex.Lock();

		/* Count for total number of MSC cells in a certain period of time */
		iNumCRCMSCLong++;

		if (bCRCOk == TRUE)
		{
			iNumCRCOkMSC++;
			iNumCRCOkMSCLong++; /* Increase number of CRCs which are ok */
		}
		else
			bMSCOk = FALSE;

		/* Validate MSC flag */
		bMSCOkValid = TRUE;

		Mutex.Unlock();
	}
}

void CParameter::CReceptLog::SetSNR(const _REAL rNewCurSNR)
{
	if (bLogActivated == TRUE)
	{
		Mutex.Lock();

		/* Set parameter for long log file version */
		rCurSNR = rNewCurSNR;

		iNumSNR++;

		/* Average SNR values */
		rAvSNR += rNewCurSNR;

		/* Set minimum and maximum of SNR */
		if (rNewCurSNR > rMaxSNR)
			rMaxSNR = rNewCurSNR;
		if (rNewCurSNR < rMinSNR)
			rMinSNR = rNewCurSNR;

		Mutex.Unlock();
	}
}

void CParameter::CReceptLog::SetNumAAC(const int iNewNum)
{
	if (iNumAACFrames != iNewNum)
	{
		/* Set the number of AAC frames in one block */
		iNumAACFrames = iNewNum;

		ResetLog(TRUE);
		ResetLog(FALSE);
	}
}

void CParameter::CReceptLog::SetLog(const _BOOLEAN bLog)
{
	bLogActivated = bLog;

	/* Open or close the file */
	if (bLogActivated == TRUE)
	{
		Mutex.Lock();

		/* Init long and short version of log file. Open output file, write
		   header and reset log file parameters */
		/* Short */
		pFileShort = fopen("DreamLog.txt", "a");
		SetLogHeader(pFileShort, FALSE);
		ResetLog(FALSE);
		iTimeCntShort = 0;

		/* Long */
		pFileLong = fopen("DreamLogLong.csv", "a");
		SetLogHeader(pFileLong, TRUE);
		ResetLog(TRUE);

		/* Init time with current time. The time function returns the number of
		   seconds elapsed since midnight (00:00:00), January 1, 1970,
		   coordinated universal time, according to the system clock */
		time(&TimeCntLong);

		/* Init maximum and mininum value of SNR */
		rMaxSNR = 0;
		rMinSNR = 1000; /* Init with high value */

		Mutex.Unlock();
	}
	else
	{
		/* Close both types of log files */
		CloseFile(pFileLong, TRUE);
		CloseFile(pFileShort, FALSE);
	}
}

void CParameter::CReceptLog::SetLogHeader(FILE* pFile, const _BOOLEAN bIsLong)
{
	time_t		ltime;
	struct tm*	today;

	/* Get time and date */
	time(&ltime);
	today = gmtime(&ltime); /* Should be UTC time */

	if (pFile != NULL)
	{
		if (bIsLong != TRUE)
		{
			/* Beginning of new table (similar to standard DRM log file) */
			fprintf(pFile, "\n>>>>\nDream\nSoftware Version %s\n", VERSION);

			fprintf(pFile, "Starttime (UTC)  %d-%02d-%02d %02d:%02d:%02d\n",
				today->tm_year + 1900, today->tm_mon + 1, today->tm_mday,
				today->tm_hour, today->tm_min, today->tm_sec);

			fprintf(pFile, "Frequency        ");
			if (iFrequency != 0)
				fprintf(pFile, "%d kHz", iFrequency);
			
			fprintf(pFile, "\nLatitude         %7s", strLatitude.c_str());
			fprintf(pFile, "\nLongitude        %7s", strLongitude.c_str());

			/* Write additional text */
			if (strAdditText != "")
				fprintf(pFile, "\n%s\n\n", strAdditText.c_str());
			else
				fprintf(pFile, "\n\n");

			fprintf(pFile, "MINUTE  SNR     SYNC    AUDIO     TYPE\n");
		}
		else
		{
#ifdef _DEBUG_
			/* In case of debug mode, use more paramters */
			fprintf(pFile, "FREQ/MODE/QAM PL:ABH,       DATE,       TIME,    "
				"SNR, SYNC, FAC, MSC, AUDIO, AUDIOOK, DOPPLER, DELAY,  "
				"DC-FREQ, SAMRATEOFFS\n");
#else
			/* The long version of log file has different header */
			fprintf(pFile, "FREQ/MODE/QAM PL:ABH,       DATE,       TIME,    "
				"SNR, SYNC, FAC, MSC, AUDIO, AUDIOOK, DOPPLER, DELAY\n");
#endif
		}

		fflush(pFile);
	}
}

void CParameter::CReceptLog::CloseFile(FILE* pFile, const _BOOLEAN bIsLong)
{
	if (pFile != NULL)
	{
		if (bIsLong == TRUE)
		{
			/* Long log file ending */
			fprintf(pFile, "\n\n");
		}
		else
		{
			/* Set min and max values of SNR. Check values first */
			if (rMaxSNR < rMinSNR)
			{
				/* It seems that no SNR value was set, set both max and min
				   to 0 */
				rMaxSNR = 0;
				rMinSNR = 0;
			}
			fprintf(pFile, "\nSNR min: %4.1f, max: %4.1f\n", rMinSNR, rMaxSNR);

			/* Short log file ending */
			fprintf(pFile, "\nCRC: \n");
			fprintf(pFile, "<<<<\n\n");
		}

		fclose(pFile);

		pFile = NULL;
	}
}

void CParameter::CReceptLog::WriteParameters(const _BOOLEAN bIsLong)
{
	//try
	{
		if (bLogActivated == TRUE)
		{
			Mutex.Lock();

			if (bIsLong == TRUE)
			{
				/* Log LONG ------------------------------------------------- */
				int			iSyncInd, iFACInd, iMSCInd;
				struct tm*	TimeNow;

				if ((bSyncOK == TRUE) && (bSyncOKValid == TRUE))
					iSyncInd = 1;
				else
					iSyncInd = 0;

				if ((bFACOk == TRUE) && (bFACOkValid == TRUE))
					iFACInd = 1;
				else
					iFACInd = 0;

				if ((bMSCOk == TRUE) && (bMSCOkValid == TRUE))
					iMSCInd = 1;
				else
					iMSCInd = 0;

				TimeNow = gmtime(&TimeCntLong); /* Should be UTC time */

				/* Get parameters for delay and Doppler. In case the receiver is
				   not synchronized, set parameters to zero */
				_REAL rDoppler = (_REAL) 0.0;
				_REAL rDelay = (_REAL) 0.0;
				if (DRMReceiver.GetReceiverState() ==
					CDRMReceiver::AS_WITH_SIGNAL)
				{
					rDelay = DRMReceiver.GetChanEst()->GetMinDelay();
					if (DRMReceiver.GetChanEst()->GetSigma(rDoppler) == FALSE)
						rDoppler = (_REAL) 0.0;
				}

				/* Get robustness mode string */
				char chRobMode;
				switch (eCurRobMode)
				{
				case RM_ROBUSTNESS_MODE_A:
					chRobMode = 'A';
					break;

				case RM_ROBUSTNESS_MODE_B:
					chRobMode = 'B';
					break;

				case RM_ROBUSTNESS_MODE_C:
					chRobMode = 'C';
					break;

				case RM_ROBUSTNESS_MODE_D:
					chRobMode = 'D';
					break;

				case RM_NO_MODE_DETECTED:
					chRobMode = 'X';
					break;
				}

				/* Get MSC scheme */
				int iCurMSCSc;
				switch (eCurMSCScheme)
				{
				case CParameter::CS_3_SM:
					iCurMSCSc = 0;
					break;

				case CParameter::CS_3_HMMIX:
					iCurMSCSc = 1;
					break;

				case CParameter::CS_3_HMSYM:
					iCurMSCSc = 2;
					break;

				case CParameter::CS_2_SM:
					iCurMSCSc = 3;
					break;
				}

				/* Copy protection levels */
				int iCurProtLevPartA = CurProtLev.iPartA;
				int iCurProtLevPartB = CurProtLev.iPartB;
				int iCurProtLevPartH = CurProtLev.iHierarch;

				/* Only show mode if FAC CRC was ok */
				if (iFACInd == 0)
				{
					chRobMode = 'X';
					iCurMSCSc = 0;
					iCurProtLevPartA = 0;
					iCurProtLevPartB = 0;
					iCurProtLevPartH = 0;
				}

#ifdef _DEBUG_
				/* Some more parameters in debug mode */
				fprintf(pFileLong,
					" %5d/%c%d%d%d%d        , %d-%02d-%02d, %02d:%02d:%02d.0, "
					"%6.2f,    %1d,   %1d,   %1d,   %3d,     %3d,   %5.2f, "
					"%5.2f, %8.2f,       %5.2f\n",
					iFrequency,	chRobMode, iCurMSCSc, iCurProtLevPartA,
					iCurProtLevPartB, iCurProtLevPartH,
					TimeNow->tm_year + 1900, TimeNow->tm_mon + 1,
					TimeNow->tm_mday, TimeNow->tm_hour, TimeNow->tm_min,
					TimeNow->tm_sec, rCurSNR, iSyncInd, iFACInd, iMSCInd,
					iNumCRCMSCLong, iNumCRCOkMSCLong,
					rDoppler, rDelay,
					DRMReceiver.GetParameters()->GetDCFrequency(),
					DRMReceiver.GetParameters()->GetSampFreqEst());
#else
				/* This data can be read by Microsoft Excel */
				fprintf(pFileLong,
					" %5d/%c%d%d%d%d        , %d-%02d-%02d, %02d:%02d:%02d.0, "
					"%6.2f,    %1d,   %1d,   %1d,   %3d,     %3d,   %5.2f, "
					"%5.2f\n",
					iFrequency,	chRobMode, iCurMSCSc, iCurProtLevPartA,
					iCurProtLevPartB, iCurProtLevPartH,
					TimeNow->tm_year + 1900, TimeNow->tm_mon + 1,
					TimeNow->tm_mday, TimeNow->tm_hour, TimeNow->tm_min,
					TimeNow->tm_sec, rCurSNR, iSyncInd, iFACInd, iMSCInd,
					iNumCRCMSCLong, iNumCRCOkMSCLong,
					rDoppler, rDelay);
#endif
			}
			else
			{
				/* Log SHORT ------------------------------------------------ */ 
				int iAverageSNR, iTmpNumAAC;

				/* Avoid division by zero */
				if (iNumSNR == 0)
					iAverageSNR = 0;
				else
					iAverageSNR = (int) Round(rAvSNR / iNumSNR);

				/* If no sync, do not print number of AAC frames. If the number
				   of correct FAC CRCs is lower than 10%, we assume that
				   receiver is not synchronized */
				if (iNumCRCOkFAC < 15)
					iTmpNumAAC = 0;
				else
					iTmpNumAAC = iNumAACFrames;

				fprintf(pFileShort, "  %04d   %2d      %3d  %4d/%02d        0",
					iTimeCntShort, iAverageSNR, iNumCRCOkFAC,
					iNumCRCOkMSC, iTmpNumAAC);

				fprintf(pFileShort, "\n"); /* New line */
			}

			fflush(pFileLong);
			fflush(pFileShort);

			ResetLog(bIsLong);

			if (bIsLong == TRUE)
			{
				/* This is a time_t type variable. It contains the number of
				   seconds from a certain defined date. We simply increment
				   this number for the next second instance */
				TimeCntLong++;
			}
			else
				iTimeCntShort++;

			Mutex.Unlock();
		}
	}

	//catch (...)
	//{
		///* To prevent errors if user views the file during reception */
	//}
}
