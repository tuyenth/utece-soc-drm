/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	DRM simulation script
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

#include "DrmSimulation.h"

#ifndef _WIN32
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif


/* Implementation *************************************************************/
void CDRMSimulation::SimScript()
{
	int									i;
	int									iNumItMLC;
	_REAL								rSNRCnt;
	FILE*								pFileSimRes;
	CVector<_REAL>						vecrMSE;
	string								strSimFile;
	string								strSpecialRemark;
	CChannelEstimation::ETypeIntFreq	eChanEstFreqIntType;
	CChannelEstimation::ETypeIntTime	eChanEstTimIntType;


	/**************************************************************************\
	* Simulation settings vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*
	\**************************************************************************/
	/* Choose which type of simulation, if you choose "ST_NONE", the regular
	   application will be started */
	Param.eSimType = CParameter::ST_MSECHANEST;
	Param.eSimType = CParameter::ST_SYNC_PARAM; /* Check "SetSyncInput()"s! */
	Param.eSimType = CParameter::ST_BITERROR;
	Param.eSimType = CParameter::ST_BER_IDEALCHAN;
	Param.eSimType = CParameter::ST_NONE; /* No simulation, regular GUI */


	if (Param.eSimType != CParameter::ST_NONE)
	{
		Param.iDRMChannelNum = 8;
Param.iChan8Doppler = 2; /* Hz (integer value!) */

		rStartSNR = (_REAL) 15;
		rEndSNR = (_REAL) 25;
		rStepSNR = (_REAL) 1.0;
		strSpecialRemark = "refideal";

		/* Length of simulation */
//		iSimTime = 100;
		iSimNumErrors = 100000;


		iNumItMLC = 1; /* Number of iterations for MLC */

		/* The associated code rate is R = 0,6 and the modulation is 64-QAM */
		Param.MSCPrLe.iPartB = 1;
		Param.eMSCCodingScheme = CParameter::CS_3_SM;


// TEST (for schnieter)
//Param.eMSCCodingScheme = CParameter::CS_2_SM;
//Param.eSDCCodingScheme = CParameter::CS_1_SM;



		eChanEstFreqIntType = CChannelEstimation::FWIENER;//FDFTFILTER;//FLINEAR;
		eChanEstTimIntType = CChannelEstimation::TWIENER;//TLINEAR;


		/* Define which synchronization algorithms we want to use */
		/* In case of bit error simulations, a synchronized DRM data stream is
		   used. Set corresponding modules to synchronized mode */
		InputResample.SetSyncInput(TRUE);
		FreqSyncAcq.SetSyncInput(TRUE);
		SyncUsingPil.SetSyncInput(TRUE);
		TimeSync.SetSyncInput(TRUE);


		if (Param.iDRMChannelNum < 3)
		{
			Param.InitCellMapTable(RM_ROBUSTNESS_MODE_A, SO_2);
			Param.eSymbolInterlMode = CParameter::SI_SHORT;
		}
		else if (Param.iDRMChannelNum == 8)
		{
			/* Special setting for channel 8 */
			Param.InitCellMapTable(RM_ROBUSTNESS_MODE_B, SO_0);
			Param.eSymbolInterlMode = CParameter::SI_LONG;
		}
		else
		{
			Param.InitCellMapTable(RM_ROBUSTNESS_MODE_B, SO_3);
			Param.eSymbolInterlMode = CParameter::SI_LONG;
		}


// TEST -> for robustness mode detection
//Param.InitCellMapTable(RM_ROBUSTNESS_MODE_B, SO_3);


	/**************************************************************************\
	* Simulation settings ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
	\**************************************************************************/


		/* Apply settings --------------------------------------------------- */
		/* Set channel estimation interpolation type */
		ChannelEstimation.SetFreqInt(eChanEstFreqIntType);
		ChannelEstimation.SetTimeInt(eChanEstTimIntType);

		/* Number of iterations for MLC */
		MSCMLCDecoder.SetNumIterations(iNumItMLC);


		/* Set the simulation priority to lowest possible value */
#ifdef _WIN32
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#else
# ifdef HAVE_UNISTD_H
		/* re-nice the process to highest possible value -> 19 */
		nice(19);

		/* Try to get hostname */
		char chHostName[255];
		if (gethostname(chHostName, size_t(255)) == 0)
		{
			/* Append it to the file name of simulation output */
			strSpecialRemark += "_";
			strSpecialRemark += chHostName;
		}
# endif
#endif

		/* Set simulation time or number of errors. Slighty different convetion
		   for __SIMTIME file name */
		if (iSimTime != 0)
		{
			GenSimData.SetSimTime(iSimTime, 
				SimFileName(Param, strSpecialRemark, TRUE));
		}
		else
		{
			GenSimData.SetNumErrors(iSimNumErrors, 
				SimFileName(Param, strSpecialRemark, TRUE));
		}

		/* Set file name for simulation results output */
		strSimFile = string(SIM_OUT_FILES_PATH) +
			SimFileName(Param, strSpecialRemark, FALSE) + string(".dat");

		printf("%s\n", strSimFile.c_str()); /* Show name directly */

		/* Open file for storing simulation results */
		pFileSimRes = fopen(strSimFile.c_str(), "w");

		/* If opening of file was not successful, exit simulation */
		if (pFileSimRes == NULL)
			exit(1);

		/* Main SNR loop */
		for (rSNRCnt = rStartSNR; rSNRCnt <= rEndSNR; rSNRCnt += rStepSNR)
		{
			/* Set SNR in global struct and run simulation */
			Param.SetNominalSNRdB(rSNRCnt);
			Run();

			/* Store results in file */
			switch (Param.eSimType)
			{
			case CParameter::ST_MSECHANEST:
				/* After the simulation get results */
				IdealChanEst.GetResults(vecrMSE);

				/* Store results in a file */
				for (i = 0; i < vecrMSE.Size(); i++)
					fprintf(pFileSimRes, "%e ", vecrMSE[i]);
				fprintf(pFileSimRes, "\n"); /* New line */
				break;

			case CParameter::ST_SYNC_PARAM:
				/* Save results in the file */
				fprintf(pFileSimRes, "%e %e\n", rSNRCnt, Param.rSyncTestParam);
				break;

			default:
				/* Save results in the file */
				fprintf(pFileSimRes, "%e %e\n", rSNRCnt, Param.rBitErrRate);
				break;
			}

			/* Make sure results are actually written in the file. In case the
			   simulation machine crashes, at least the last results are
			   preserved */
			fflush(pFileSimRes);
		}

		/* Close simulation results file afterwards */
		fclose(pFileSimRes);

		/* At the end of the simulation, exit the application */
		exit(1);
	}
}

string CDRMSimulation::SimFileName(CParameter& Param, string strAddInf,
								   _BOOLEAN bWithSNR)
{
/* 
	File naming convention:
	BER: Bit error rate simulation
	MSE: MSE for channel estimation
	BERIDEAL: Bit error rate simulation with ideal channel estimation
	SYNC: Simulation for a synchronization paramter (usually variance)

	B3: Robustness mode and spectrum occupancy
	CH5: Channel number
	It: Number of iterations in MLC decoder
	PL: Protection level of channel code
	"64SM": Modulation
	"T50": Length of simulation

	example: BERIDEAL_CH8_B0_It1_PL1_64SM_T50_MyRemark
*/
	char chNumTmpLong[256];
	string strFileName = "";

	/* What type of simulation ---------------------------------------------- */
	switch (Param.eSimType)
	{
	case CParameter::ST_BITERROR:
		strFileName += "BER_";
		break;
	case CParameter::ST_MSECHANEST:
		strFileName += "MSE_";
		break;
	case CParameter::ST_BER_IDEALCHAN:
		strFileName += "BERIDEAL_";
		break;
	case CParameter::ST_SYNC_PARAM:
		strFileName += "SYNC_";
		break;
	}


	/* Channel -------------------------------------------------------------- */
	/* In case of channel 8 also write Doppler frequency in file name */
	if (Param.iDRMChannelNum == 8)
	{
		sprintf(chNumTmpLong, "CH%d_%dHz_",
			Param.iDRMChannelNum, Param.iChan8Doppler);
	}
	else
		sprintf(chNumTmpLong, "CH%d_", Param.iDRMChannelNum);

	strFileName += chNumTmpLong;


	/* Robustness mode and spectrum occupancy ------------------------------- */
	switch (Param.GetWaveMode())
	{
	case RM_ROBUSTNESS_MODE_A:
		strFileName += "A";
		break;
	case RM_ROBUSTNESS_MODE_B:
		strFileName += "B";
		break;
	case RM_ROBUSTNESS_MODE_C:
		strFileName += "C";
		break;
	case RM_ROBUSTNESS_MODE_D:
		strFileName += "D";
		break;
	}
	switch (Param.GetSpectrumOccup())
	{
	case SO_0:
		strFileName += "0_";
		break;
	case SO_1:
		strFileName += "1_";
		break;
	case SO_2:
		strFileName += "2_";
		break;
	case SO_3:
		strFileName += "3_";
		break;
	case SO_4:
		strFileName += "4_";
		break;
	case SO_5:
		strFileName += "5_";
		break;
	}


	/* Channel estimation method -------------------------------------------- */
	/* In case of BER simulation, print out channel estimation methods used */
	if (Param.eSimType == CParameter::ST_BITERROR)
	{
		/* Time direction */
		switch (ChannelEstimation.GetTimeInt())
		{
		case CChannelEstimation::TLINEAR:
			strFileName += "Tl";
			break;

		case CChannelEstimation::TWIENER:
			strFileName += "Tw";
			break;
		}

		/* Frequency direction */
		switch (ChannelEstimation.GetFreqInt())
		{
		case CChannelEstimation::FLINEAR:
			strFileName += "Fl_";
			break;

		case CChannelEstimation::FDFTFILTER:
			strFileName += "Fd_";
			break;

		case CChannelEstimation::FWIENER:
			strFileName += "Fw_";
			break;
		}
	}


	/* Number of iterations in MLC decoder ---------------------------------- */
	sprintf(chNumTmpLong, "It%d_", MSCMLCDecoder.GetInitNumIterations());
	strFileName += chNumTmpLong;


	/* Protection level part B ---------------------------------------------- */
	sprintf(chNumTmpLong, "PL%d_", Param.MSCPrLe.iPartB);
	strFileName += chNumTmpLong;


	/* MSC coding scheme ---------------------------------------------------- */
	switch (Param.eMSCCodingScheme)
	{
	case CParameter::CS_2_SM:
		strFileName += "16SM_";
		break;

	case CParameter::CS_3_SM:
		strFileName += "64SM_";
		break;

	case CParameter::CS_3_HMMIX:
		strFileName += "64MIX_";
		break;

	case CParameter::CS_3_HMSYM:
		strFileName += "64SYM_";
		break;
	}


	/* Number of error events or simulation time ---------------------------- */
	int iCurNum;
	string strMultPl = "_";
	if (iSimTime != 0)
	{
		strFileName += "T"; /* T -> time */
		iCurNum = iSimTime;
	}
	else
	{
		strFileName += "E"; /* E -> errors */
		iCurNum = iSimNumErrors;
	}
	
	if (iCurNum / 1000 > 0)
	{
		strMultPl = "K_";
		iCurNum /= 1000;

		if (iCurNum / 1000 > 0)
		{
			strMultPl = "M_";
			iCurNum /= 1000;
		}
	}

	sprintf(chNumTmpLong, "%d", iCurNum);
	strFileName += chNumTmpLong;
	strFileName += strMultPl;


	/* SNR range (optional) ------------------------------------------------- */
	if (bWithSNR == TRUE)
	{
		strFileName += "SNR";
		sprintf(chNumTmpLong, "%.1f-", rStartSNR);
		strFileName += chNumTmpLong;
		sprintf(chNumTmpLong, "%.1f-", rStepSNR);
		strFileName += chNumTmpLong;
		sprintf(chNumTmpLong, "%.1f_", rEndSNR);
		strFileName += chNumTmpLong;
	}

	/* Special remark */
	if (!strAddInf.empty())
		strFileName += strAddInf;

	return strFileName;
}
