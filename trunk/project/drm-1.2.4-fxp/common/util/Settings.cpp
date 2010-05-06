/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2004
 *
 * Author(s):
 *	Volker Fischer, Tomi Manninen, Stephane Fillod, Robert Kesterson
 *
 * Description:
 *
 * 10/03/2003 Tomi Manninen / OH2BNS
 *  - Initial (basic) code for command line argument parsing (argv)
 * 04/15/2004 Tomi Manninen, Stephane Fillod
 *  - Hamlib
 * 07/27/4002
 *  - included stlini routines written by Robert Kesterson
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

#include "Settings.h"


/* Implementation *************************************************************/
_BOOLEAN CSettings::Load(int argc, char** argv)
{
	/* First load settings from init-file and then parse command line arguments.
	   The command line arguments overwrite settings in init-file! */
	ReadIniFile();

	return ParseArguments(argc, argv);
}

void CSettings::Save()
{
	/* Just write settings in init-file */
	WriteIniFile();
}


/* Read and write init-file ***************************************************/
void CSettings::ReadIniFile()
{
	int			iValue;
	_BOOLEAN	bValue;

	/* Load data from init-file */
	INIFile ini = LoadIni(DREAM_INIT_FILE_NAME);


	/* Receiver ------------------------------------------------------------- */
	/* Flip spectrum flag */
	if (GetFlagIniSet(ini, "Receiver", "flipspectrum", bValue) == TRUE)
		pDRMRec->GetReceiver()->SetFlippedSpectrum(bValue);


	/* Mute audio flag */
	if (GetFlagIniSet(ini, "Receiver", "muteaudio", bValue) == TRUE)
		pDRMRec->GetWriteData()->MuteAudio(bValue);


	/* Reverberation flag */
	if (GetFlagIniSet(ini, "Receiver", "reverb", bValue) == TRUE)
		pDRMRec->GetAudSorceDec()->SetReverbEffect(bValue);


	/* Bandpass filter flag */
	if (GetFlagIniSet(ini, "Receiver", "filter", bValue) == TRUE)
		pDRMRec->GetFreqSyncAcq()->SetRecFilter(bValue);


	/* Modified metrics flag */
	if (GetFlagIniSet(ini, "Receiver", "modmetric", bValue) == TRUE)
		pDRMRec->GetChanEst()->SetIntCons(bValue);


	/* Sound In device */
	if (GetNumericIniSet(ini, "Receiver", "snddevin", 0, MAX_NUM_SND_DEV, iValue) == TRUE)
		pDRMRec->GetSoundInterface()->SetInDev(iValue);


	/* Sound Out device */
	if (GetNumericIniSet(ini, "Receiver", "snddevout", 0, MAX_NUM_SND_DEV, iValue) == TRUE)
		pDRMRec->GetSoundInterface()->SetOutDev(iValue);


	/* Number of iterations for MLC setting */
	if (GetNumericIniSet(ini, "Receiver", "mlciter", 0, MAX_NUM_MLC_IT, iValue) == TRUE)
		pDRMRec->GetMSCMLC()->SetNumIterations(iValue);


#ifdef USE_QT_GUI
	/* Logfile -------------------------------------------------------------- */
	/* Start log file flag */
	if (GetNumericIniSet(ini, "Logfile", "startlog", 0, MAX_SEC_LOG_FI_START, iValue) == TRUE)
		pDRMRec->GetParameters()->ReceptLog.SetDelLogStart(iValue);


	/* Frequency for log file */
	if (GetNumericIniSet(ini, "Logfile", "frequency", 0, MAX_FREQ_LOG_FILE, iValue) == TRUE)
		pDRMRec->GetParameters()->ReceptLog.SetFrequency(iValue);


	/* Latitude string for log file */
	pDRMRec->GetParameters()->ReceptLog.SetLatitude(
		GetIniSetting(ini, "Logfile", "latitude"));


	/* Longitude string for log file */
	pDRMRec->GetParameters()->ReceptLog.SetLongitude(
		GetIniSetting(ini, "Logfile", "longitude"));


	/* Seconds for preview into Stations Dialog if zero then inactive */
	if (GetNumericIniSet(ini, "Stations dialog", "preview", 0, MAX_NUM_SEC_PREVIEW, iValue) == TRUE)
		pDRMRec->iSecondsPreview = iValue;


	/* Window geometry ------------------------------------------------------ */
	/* Main window */
	if (GetNumericIniSet(ini, "Window geometry", "mainxpos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomFdrmdialog.iXPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "mainypos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomFdrmdialog.iYPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "mainhsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomFdrmdialog.iHSize = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "mainwsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomFdrmdialog.iWSize = iValue;

	/* System evaluation window */
	if (GetNumericIniSet(ini, "Window geometry", "sysevxpos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomSystemEvalDlg.iXPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "sysevypos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomSystemEvalDlg.iYPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "sysevhsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomSystemEvalDlg.iHSize = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "sysevwsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomSystemEvalDlg.iWSize = iValue;
	if (GetFlagIniSet(ini, "Window geometry", "sysevvis", bValue) == TRUE)
		pDRMRec->GeomSystemEvalDlg.bVisible = bValue;

	/* Multimedia window */
	if (GetNumericIniSet(ini, "Window geometry", "multdlgxpos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomMultimediaDlg.iXPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "multdlgypos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomMultimediaDlg.iYPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "multdlghsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomMultimediaDlg.iHSize = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "multdlgwsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomMultimediaDlg.iWSize = iValue;
	if (GetFlagIniSet(ini, "Window geometry", "multdlgvis", bValue) == TRUE)
		pDRMRec->GeomMultimediaDlg.bVisible = bValue;

	/* Stations dialog */
	if (GetNumericIniSet(ini, "Window geometry", "statdlgxpos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomStationsDlg.iXPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "statdlgypos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomStationsDlg.iYPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "statdlghsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomStationsDlg.iHSize = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "statdlgwsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomStationsDlg.iWSize = iValue;
	if (GetFlagIniSet(ini, "Window geometry", "statdlgvis", bValue) == TRUE)
		pDRMRec->GeomStationsDlg.bVisible = bValue;

	/* Analog demodulation dialog */
	if (GetNumericIniSet(ini, "Window geometry", "analdemxpos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomAnalogDemDlg.iXPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "analdemypos", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomAnalogDemDlg.iYPos = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "analdemhsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomAnalogDemDlg.iHSize = iValue;
	if (GetNumericIniSet(ini, "Window geometry", "analdemwsize", 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
		pDRMRec->GeomAnalogDemDlg.iWSize = iValue;
	if (GetFlagIniSet(ini, "Window geometry", "analdemvis", bValue) == TRUE)
		pDRMRec->GeomAnalogDemDlg.bVisible = bValue;

	/* Chart windows */
	int iNumChartWin = 0;
	if (GetNumericIniSet(ini, "Window geometry", "numchartwin", 0, MAX_NUM_CHART_WIN_EV_DLG, iValue) == TRUE)
		iNumChartWin = iValue;

	pDRMRec->GeomChartWindows.Init(iNumChartWin);
	for (int i = 0; i < iNumChartWin; i++)
	{
		/* Convert number to string */
		char chNumTmpLong[256];

		sprintf(chNumTmpLong, "chwin%dxpos", i);
		if (GetNumericIniSet(ini, "Window geometry", chNumTmpLong, 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
			pDRMRec->GeomChartWindows[i].iXPos = iValue;

		sprintf(chNumTmpLong, "chwin%dypos", i);
		if (GetNumericIniSet(ini, "Window geometry", chNumTmpLong, 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
			pDRMRec->GeomChartWindows[i].iYPos = iValue;

		sprintf(chNumTmpLong, "chwin%dhsize", i);
		if (GetNumericIniSet(ini, "Window geometry", chNumTmpLong, 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
			pDRMRec->GeomChartWindows[i].iHSize = iValue;

		sprintf(chNumTmpLong, "chwin%dwsize", i);
		if (GetNumericIniSet(ini, "Window geometry", chNumTmpLong, 0, MAX_WIN_GEOM_VAL, iValue) == TRUE)
			pDRMRec->GeomChartWindows[i].iWSize = iValue;

		sprintf(chNumTmpLong, "chwin%dtype", i);
		if (GetNumericIniSet(ini, "Window geometry", chNumTmpLong, 0, MAX_IND_CHART_TYPES, iValue) == TRUE)
			pDRMRec->GeomChartWindows[i].iType = iValue;
	}


	/* Other ---------------------------------------------------------------- */
	/* Color scheme main plot */
	if (GetNumericIniSet(ini, "GUI", "colorscheme", 0, MAX_COLOR_SCHEMES_VAL, iValue) == TRUE)
		pDRMRec->iMainPlotColorStyle = iValue;

	/* System evaluation dialog plot type. Maximum value is the last element
	   in the plot type enum! */
	if (GetNumericIniSet(ini, "GUI", "sysevplottype", 0, CDRMPlot::NONE_OLD, iValue) == TRUE)
		pDRMRec->iSysEvalDlgPlotType = iValue;
#endif


#ifdef HAVE_LIBHAMLIB
	/* Hamlib --------------------------------------------------------------- */
	/* Hamlib Model ID */
	if (GetNumericIniSet(ini, "Hamlib",	"hamlib-model", 0, MAX_ID_HAMLIB, iValue) == TRUE)
		pDRMRec->GetHamlib()->SetHamlibModelID(iValue);

	/* Hamlib configuration string */
	pDRMRec->GetHamlib()->SetHamlibConf(GetIniSetting(ini, "Hamlib", "hamlib-config"));

# ifdef USE_QT_GUI
	/* Enable s-meter flag */
	if (GetFlagIniSet(ini, "Hamlib", "ensmeter", bValue) == TRUE)
		pDRMRec->bEnableSMeter = bValue;
# endif

	/* Enable DRM modified receiver flag */
	if (GetFlagIniSet(ini, "Hamlib", "enmodrig", bValue) == TRUE)
		pDRMRec->GetHamlib()->SetEnableModRigSettings(bValue);
#endif
}

void CSettings::WriteIniFile()
{
	INIFile ini;

	/* Receiver ------------------------------------------------------------- */
	/* Flip spectrum flag */
	SetFlagIniSet(ini, "Receiver", "flipspectrum",
		pDRMRec->GetReceiver()->GetFlippedSpectrum());


	/* Mute audio flag */
	SetFlagIniSet(ini, "Receiver", "muteaudio",
		pDRMRec->GetWriteData()->GetMuteAudio());


	/* Reverberation */
	SetFlagIniSet(ini, "Receiver", "reverb",
		pDRMRec->GetAudSorceDec()->GetReverbEffect());


	/* Bandpass filter flag */
	SetFlagIniSet(ini, "Receiver", "filter",
		pDRMRec->GetFreqSyncAcq()->GetRecFilter());


	/* Modified metrics flag */
	SetFlagIniSet(ini, "Receiver", "modmetric",
		pDRMRec->GetChanEst()->GetIntCons());


	/* Sound In device */
	SetNumericIniSet(ini, "Receiver", "snddevin",
		pDRMRec->GetSoundInterface()->GetInDev());


	/* Sound Out device */
	SetNumericIniSet(ini, "Receiver", "snddevout",
		pDRMRec->GetSoundInterface()->GetOutDev());


	/* Number of iterations for MLC setting */
	SetNumericIniSet(ini, "Receiver", "mlciter",
		pDRMRec->GetMSCMLC()->GetInitNumIterations());


#ifdef USE_QT_GUI
	/* Logfile -------------------------------------------------------------- */
	/* Start log file delayed */
	SetNumericIniSet(ini, "Logfile", "startlog",
		pDRMRec->GetParameters()->ReceptLog.GetDelLogStart());


	/* Frequency for log file */
	SetNumericIniSet(ini, "Logfile", "frequency",
		pDRMRec->GetParameters()->ReceptLog.GetFrequency());


	/* Latitude string for log file */
	PutIniSetting(ini, "Logfile", "latitude",
		pDRMRec->GetParameters()->ReceptLog.GetLatitude().c_str());


	/* Longitude string for log file */
	PutIniSetting(ini, "Logfile", "longitude",
		pDRMRec->GetParameters()->ReceptLog.GetLongitude().c_str());


	/* Seconds for preview into Stations Dialog if zero then inactive */
	SetNumericIniSet(ini, "Stations dialog", "preview",
		pDRMRec->iSecondsPreview);


	/* Window geometry ------------------------------------------------------ */
	/* Main window */
	SetNumericIniSet(ini, "Window geometry", "mainxpos", pDRMRec->GeomFdrmdialog.iXPos);
	SetNumericIniSet(ini, "Window geometry", "mainypos", pDRMRec->GeomFdrmdialog.iYPos);
	SetNumericIniSet(ini, "Window geometry", "mainhsize", pDRMRec->GeomFdrmdialog.iHSize);
	SetNumericIniSet(ini, "Window geometry", "mainwsize", pDRMRec->GeomFdrmdialog.iWSize);

	/* System evaluation window */
	SetNumericIniSet(ini, "Window geometry", "sysevxpos", pDRMRec->GeomSystemEvalDlg.iXPos);
	SetNumericIniSet(ini, "Window geometry", "sysevypos", pDRMRec->GeomSystemEvalDlg.iYPos);
	SetNumericIniSet(ini, "Window geometry", "sysevhsize", pDRMRec->GeomSystemEvalDlg.iHSize);
	SetNumericIniSet(ini, "Window geometry", "sysevwsize", pDRMRec->GeomSystemEvalDlg.iWSize);
	SetFlagIniSet(ini, "Window geometry", "sysevvis", pDRMRec->GeomSystemEvalDlg.bVisible);

	/* Multimedia window */
	SetNumericIniSet(ini, "Window geometry", "multdlgxpos", pDRMRec->GeomMultimediaDlg.iXPos);
	SetNumericIniSet(ini, "Window geometry", "multdlgypos", pDRMRec->GeomMultimediaDlg.iYPos);
	SetNumericIniSet(ini, "Window geometry", "multdlghsize", pDRMRec->GeomMultimediaDlg.iHSize);
	SetNumericIniSet(ini, "Window geometry", "multdlgwsize", pDRMRec->GeomMultimediaDlg.iWSize);
	SetFlagIniSet(ini, "Window geometry", "multdlgvis", pDRMRec->GeomMultimediaDlg.bVisible);

	/* Stations dialog */
	SetNumericIniSet(ini, "Window geometry", "statdlgxpos", pDRMRec->GeomStationsDlg.iXPos);
	SetNumericIniSet(ini, "Window geometry", "statdlgypos", pDRMRec->GeomStationsDlg.iYPos);
	SetNumericIniSet(ini, "Window geometry", "statdlghsize", pDRMRec->GeomStationsDlg.iHSize);
	SetNumericIniSet(ini, "Window geometry", "statdlgwsize", pDRMRec->GeomStationsDlg.iWSize);
	SetFlagIniSet(ini, "Window geometry", "statdlgvis", pDRMRec->GeomStationsDlg.bVisible);

	/* Analog demodulation dialog */
	SetNumericIniSet(ini, "Window geometry", "analdemxpos", pDRMRec->GeomAnalogDemDlg.iXPos);
	SetNumericIniSet(ini, "Window geometry", "analdemypos", pDRMRec->GeomAnalogDemDlg.iYPos);
	SetNumericIniSet(ini, "Window geometry", "analdemhsize", pDRMRec->GeomAnalogDemDlg.iHSize);
	SetNumericIniSet(ini, "Window geometry", "analdemwsize", pDRMRec->GeomAnalogDemDlg.iWSize);
	SetFlagIniSet(ini, "Window geometry", "analdemvis", pDRMRec->GeomAnalogDemDlg.bVisible);

	/* Chart windows */
	const int iNumChartWin = pDRMRec->GeomChartWindows.Size();
	SetNumericIniSet(ini, "Window geometry", "numchartwin", iNumChartWin);

	for (int i = 0; i < iNumChartWin; i++)
	{
		/* Convert number to string */
		char chNumTmpLong[256];

		sprintf(chNumTmpLong, "chwin%dxpos", i);
		SetNumericIniSet(ini, "Window geometry", chNumTmpLong, pDRMRec->GeomChartWindows[i].iXPos);

		sprintf(chNumTmpLong, "chwin%dypos", i);
		SetNumericIniSet(ini, "Window geometry", chNumTmpLong, pDRMRec->GeomChartWindows[i].iYPos);

		sprintf(chNumTmpLong, "chwin%dhsize", i);
		SetNumericIniSet(ini, "Window geometry", chNumTmpLong, pDRMRec->GeomChartWindows[i].iHSize);

		sprintf(chNumTmpLong, "chwin%dwsize", i);
		SetNumericIniSet(ini, "Window geometry", chNumTmpLong, pDRMRec->GeomChartWindows[i].iWSize);

		sprintf(chNumTmpLong, "chwin%dtype", i);
		SetNumericIniSet(ini, "Window geometry", chNumTmpLong, pDRMRec->GeomChartWindows[i].iType);
	}


	/* Other ---------------------------------------------------------------- */
	/* Color scheme main plot */
	SetNumericIniSet(ini, "GUI", "colorscheme", pDRMRec->iMainPlotColorStyle);

	/* System evaluation dialog plot type */
	SetNumericIniSet(ini, "GUI", "sysevplottype", pDRMRec->iSysEvalDlgPlotType);
#endif


#ifdef HAVE_LIBHAMLIB
	/* Hamlib --------------------------------------------------------------- */
	/* Hamlib Model ID */
	SetNumericIniSet(ini, "Hamlib", "hamlib-model",
		pDRMRec->GetHamlib()->GetHamlibModelID());

	/* Hamlib configuration string */
	PutIniSetting(ini, "Hamlib", "hamlib-config", pDRMRec->GetHamlib()->GetHamlibConf().c_str());

# ifdef USE_QT_GUI
	/* Enable s-meter flag */
	SetFlagIniSet(ini, "Hamlib", "ensmeter", pDRMRec->bEnableSMeter);
# endif

	/* Enable DRM modified receiver flag */
	SetFlagIniSet(ini, "Hamlib", "enmodrig", pDRMRec->GetHamlib()->GetEnableModRigSettings());
#endif


	/* Save settings in init-file */
	SaveIni(ini, DREAM_INIT_FILE_NAME);
}

_BOOLEAN CSettings::GetNumericIniSet(INIFile& theINI, string strSection,
									 string strKey, int iRangeStart,
									 int iRangeStop, int& iValue)
{
	/* Init return value */
	_BOOLEAN bReturn = FALSE;

	const string strGetIni =
		GetIniSetting(theINI, strSection.c_str(), strKey.c_str());

	/* Check if it is a valid parameter */
	if (!strGetIni.empty())
	{
		iValue = atoi(strGetIni.c_str());

		/* Check range */
		if ((iValue >= iRangeStart) && (iValue <= iRangeStop))
			bReturn = TRUE;
	}

	return bReturn;
}

void CSettings::SetNumericIniSet(INIFile& theINI, string strSection,
								 string strKey, int iValue)
{
	char cString[256];

	sprintf(cString, "%d", iValue);
	PutIniSetting(theINI, strSection.c_str(), strKey.c_str(), cString);
}

_BOOLEAN CSettings::GetFlagIniSet(INIFile& theINI, string strSection,
								  string strKey, _BOOLEAN& bValue)
{
	/* Init return value */
	_BOOLEAN bReturn = FALSE;

	const string strGetIni =
		GetIniSetting(theINI, strSection.c_str(), strKey.c_str());

	if (!strGetIni.empty())
	{
		if (atoi(strGetIni.c_str()))
			bValue = TRUE;
		else
			bValue = FALSE;

		bReturn = TRUE;
	}

	return bReturn;
}

void CSettings::SetFlagIniSet(INIFile& theINI, string strSection, string strKey,
							  _BOOLEAN bValue)
{
	if (bValue == TRUE)
		PutIniSetting(theINI, strSection.c_str(), strKey.c_str(), "1");
	else
		PutIniSetting(theINI, strSection.c_str(), strKey.c_str(), "0");
}


/* Command line argument parser ***********************************************/
_BOOLEAN CSettings::ParseArguments(int argc, char** argv)
{
	_BOOLEAN	bIsReceiver = TRUE;
	_REAL		rArgument;
	string		strArgument;
	_REAL		rFreqAcSeWinSize = (_REAL) 0.0;
	_REAL		rFreqAcSeWinCenter = (_REAL) 0.0;

	/* QT docu: argv()[0] is the program name, argv()[1] is the first
	   argument and argv()[argc()-1] is the last argument.
	   Start with first argument, therefore "i = 1" */
	for (int i = 1; i < argc; i++)
	{
		/* DRM transmitter mode flag ---------------------------------------- */
		if (GetFlagArgument(argc, argv, i, "-t", "--transmitter") == TRUE)
		{
			bIsReceiver = FALSE;
			continue;
		}

		
		/* Flip spectrum flag ----------------------------------------------- */
		if (GetFlagArgument(argc, argv, i, "-p", "--flipspectrum") == TRUE)
		{
			pDRMRec->GetReceiver()->SetFlippedSpectrum(TRUE);
			continue;
		}


		/* Mute audio flag -------------------------------------------------- */
		if (GetFlagArgument(argc, argv, i, "-m", "--muteaudio") == TRUE)
		{
			pDRMRec->GetWriteData()->MuteAudio(TRUE);
			continue;
		}


		/* Bandpass filter flag --------------------------------------------- */
		if (GetFlagArgument(argc, argv, i, "-F", "--filter") == TRUE)
		{
			pDRMRec->GetFreqSyncAcq()->SetRecFilter(TRUE);
			continue;
		}


		/* Modified metrics flag -------------------------------------------- */
		if (GetFlagArgument(argc, argv, i, "-D", "--modmetric") == TRUE)
		{
			pDRMRec->GetChanEst()->SetIntCons(TRUE);
			continue;
		}


		/* Sound In device -------------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-I", "--snddevin", 0,
			MAX_NUM_SND_DEV, rArgument) == TRUE)
		{
			pDRMRec->GetSoundInterface()->SetInDev((int) rArgument);
			continue;
		}


		/* Sound Out device ------------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-O", "--snddevout", 0,
			MAX_NUM_SND_DEV, rArgument) == TRUE)
		{
			pDRMRec->GetSoundInterface()->SetOutDev((int) rArgument);
			continue;
		}


		/* Do not use sound card, read from file ---------------------------- */
		if (GetStringArgument(argc, argv, i, "-f", "--fileio",
			strArgument) == TRUE)
		{
			pDRMRec->SetReadDRMFromFile(strArgument);
			continue;
		}


		/* Write output data to file as WAV --------------------------------- */
		if (GetStringArgument(argc, argv, i, "-w", "--writewav",
			strArgument) == TRUE)
		{
			pDRMRec->GetWriteData()-> StartWriteWaveFile(strArgument);
			continue;
		}

		
		/* Number of iterations for MLC setting ----------------------------- */
		if (GetNumericArgument(argc, argv, i, "-i", "--mlciter", 0,
			MAX_NUM_MLC_IT, rArgument) == TRUE)
		{
			pDRMRec->GetMSCMLC()->SetNumIterations((int) rArgument);
			continue;
		}


		/* Sample rate offset start value ----------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-s", "--sampleoff",
			MIN_SAM_OFFS_INI, MAX_SAM_OFFS_INI,	rArgument) == TRUE)
		{
			pDRMRec->SetInitResOff(rArgument);
			continue;
		}


		/* Frequency acquisition search window size ------------------------- */
		if (GetNumericArgument(argc, argv, i, "-S", "--fracwinsize", 0,
			MAX_FREQ_AQC_SE_WIN_SI, rArgument) == TRUE)
		{
			rFreqAcSeWinSize = rArgument;
			continue;
		}


		/* Frequency acquisition search window center ----------------------- */
		if (GetNumericArgument(argc, argv, i, "-E", "--fracwincent", 0,
			MAX_FREQ_AQC_SE_WIN_CEN, rArgument) == TRUE)
		{
			rFreqAcSeWinCenter = rArgument;
			continue;
		}


		/* Input channel selection ------------------------------------------ */
		if (GetNumericArgument(argc, argv, i, "-c", "--inchansel", 0,
			MAX_VAL_IN_CHAN_SEL, rArgument) == TRUE)
		{
			switch ((int) rArgument)
			{
			case 0:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_LEFT_CHAN);
				break;

			case 1:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_RIGHT_CHAN);
				break;

			case 2:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_MIX_CHAN);
				break;

			case 3:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_IQ_POS);
				break;

			case 4:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_IQ_NEG);
				break;

			case 5:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_IQ_POS_ZERO);
				break;

			case 6:
				pDRMRec->GetReceiver()->
					SetInChanSel(CReceiveData::CS_IQ_NEG_ZERO);
				break;
			}
			continue;
		}


		/* Output channel selection ----------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-u", "--outchansel", 0,
			MAX_VAL_OUT_CHAN_SEL, rArgument) == TRUE)
		{
			switch ((int) rArgument)
			{
			case 0:
				pDRMRec->GetWriteData()->
					SetOutChanSel(CWriteData::CS_BOTH_BOTH);
				break;

			case 1:
				pDRMRec->GetWriteData()->
					SetOutChanSel(CWriteData::CS_LEFT_LEFT);
				break;

			case 2:
				pDRMRec->GetWriteData()->
					SetOutChanSel(CWriteData::CS_RIGHT_RIGHT);
				break;

			case 3:
				pDRMRec->GetWriteData()->
					SetOutChanSel(CWriteData::CS_LEFT_MIX);
				break;

			case 4:
				pDRMRec->GetWriteData()->
					SetOutChanSel(CWriteData::CS_RIGHT_MIX);
				break;
			}
			continue;
		}


#ifdef USE_QT_GUI /* QThread needed for log file timing */
		/* Start log file flag ---------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-l", "--startlog", 1,
			MAX_SEC_LOG_FI_START, rArgument) == TRUE)
		{
			pDRMRec->GetParameters()->ReceptLog.
				SetDelLogStart((int) rArgument);
			continue;
		}


		/* Frequency for log file ------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-r", "--frequency", 0,
			MAX_FREQ_LOG_FILE, rArgument) == TRUE)
		{
			pDRMRec->GetParameters()->ReceptLog.
				SetFrequency((int) rArgument);
			continue;
		}


		/* Latitude string for log file ------------------------------------- */
		if (GetStringArgument(argc, argv, i, "-a", "--latitude",
			strArgument) == TRUE)
		{
			pDRMRec->GetParameters()->ReceptLog.SetLatitude(strArgument);
			continue;
		}


		/* Longitude string for log file ------------------------------------ */
		if (GetStringArgument(argc, argv, i, "-o", "--longitude",
			strArgument) == TRUE)
		{
			pDRMRec->GetParameters()->ReceptLog.SetLongitude(strArgument);
			continue;
		}


		/* Color scheme main plot ------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-y", "--colorstyle", 0,
			MAX_COLOR_SCHEMES_VAL, rArgument) == TRUE)
		{
			pDRMRec->iMainPlotColorStyle = (int) rArgument;
			continue;
		}


		/* MDI out address -------------------------------------------------- */
		if (GetStringArgument(argc, argv, i, "--mdioutadr", "--mdioutadr",
			strArgument) == TRUE)
		{
			pDRMRec->GetMDI()->SetNetwOutAddr(strArgument);
			continue;
		}


		/* MDI in port number ----------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "--mdiinport", "--mdiinport", 0,
			MAX_MDI_PORT_IN_NUM, rArgument) == TRUE)
		{
			pDRMRec->GetMDI()->SetNetwInPort((int) rArgument);
			continue;
		}
#endif

#ifdef HAVE_LIBHAMLIB
		/* Hamlib Model ID -------------------------------------------------- */
		if (GetStringArgument(argc, argv, i, "-C", "--hamlib-config",
			strArgument) == TRUE)
		{
			pDRMRec->GetHamlib()->SetHamlibConf(strArgument);
			continue;
		}


		/* Hamlib config string --------------------------------------------- */
		if (GetNumericArgument(argc, argv, i, "-M", "--hamlib-model", 0,
			MAX_ID_HAMLIB, rArgument) == TRUE)
		{
			pDRMRec->GetHamlib()->SetHamlibModelID((int) rArgument);
			continue;
		}


# ifdef USE_QT_GUI
		/* Enable s-meter flag ---------------------------------------------- */
		if (GetFlagArgument(argc, argv, i, "-T", "--ensmeter") == TRUE)
		{
			pDRMRec->bEnableSMeter = TRUE;
			continue;
		}
# endif
#endif


		/* Help (usage) flag ------------------------------------------------ */
		if ((!strcmp(argv[i], "--help")) ||
			(!strcmp(argv[i], "-h")) ||
			(!strcmp(argv[i], "-?")))
		{
			const string strHelp = UsageArguments(argv);

#if defined(_WIN32)
			MessageBox(NULL, strHelp.c_str(), "Dream",
				MB_SYSTEMMODAL | MB_OK | MB_ICONINFORMATION);
#else
			cerr << strHelp;
#endif

			exit(1);
		}


		/* Unknown option --------------------------------------------------- */
		cerr << argv[0] << ": ";
		cerr << "Unknown option '" << argv[i] << "' -- use '--help' for help"
			<< endl;

		exit(1);
	}

	/* Set parameters for frequency acquisition search window if needed */
	if (rFreqAcSeWinSize != (_REAL) 0.0)
	{
		if (rFreqAcSeWinCenter == (_REAL) 0.0)
		{
			/* If no center was specified, set default parameter (in the
			   middle of the available spectrum) */
			rFreqAcSeWinCenter = (_REAL) SOUNDCRD_SAMPLE_RATE / 4;
		}

		/* Set new parameters */
		pDRMRec->GetFreqSyncAcq()->SetSearchWindow(rFreqAcSeWinCenter,
			rFreqAcSeWinSize);
	}

	return bIsReceiver;
}

string CSettings::UsageArguments(char** argv)
{
// TODO: Use macro definitions for help text, too (instead of hard-coded numbers)!

	return
		"Usage: " + string(argv[0]) + " [option] [argument]\n\n"
		"Recognized options:\n\n"
		"  -t, --transmitter           DRM transmitter mode\n"
		"  -p, --flipspectrum          flip input spectrum\n"
		"  -i <n>, --mlciter <n>       number of MLC iterations\n"
		"                              allowed range: 0...4\n"
		"                              default: 1\n"
		"  -s <r>, --sampleoff <r>     sample rate offset initial value [Hz]\n"
		"                              allowed range: -200.0...200.0\n"
		"  -m, --muteaudio             mute audio output\n"
		"  -f <s>, --fileio <s>        disable sound card,\n"
		"                              use file <s> instead\n"
		"  -w <s>, --writewav <s>      write output to wave file\n"
		"  -S <r>, --fracwinsize <r>   freq. acqu. search window size [Hz]\n"
		"  -E <r>, --fracwincent <r>   freq. acqu. search window center [Hz]\n"
		"  -F, --filter                apply bandpass filter\n"
		"  -D, --modmetric             enable modified metrics\n"
		"  -c <n>, --inchansel <n>     input channel selection\n"
		"                              0: left channel\n"
		"                              1: right channel\n"
		"                              2: mix both channels (default)\n"
		"                              3: I / Q input positive\n"
		"                              4: I / Q input negative\n"
		"                              5: I / Q input positive (0 Hz IF)\n"
		"                              6: I / Q input negative (0 Hz IF)\n"
		"  -u <n>, --outchansel <n>    output channel selection\n"
		"                              0: L -> L, R -> R (default)\n"
		"                              1: L -> L, R muted\n"
		"                              2: L muted, R -> R\n"
		"                              3: mix -> L, R muted\n"
		"                              4: L muted, mix -> R\n"

#ifdef USE_QT_GUI
		"  -r <n>, --frequency <n>     set frequency [kHz] for log file\n"
		"  -a <s>, --latitude <s>      set latitude string for log file\n"
		"  -o <s>, --longitude <s>     set longitude string for log file\n"
		"  -l <n>, --startlog <n>      start log file (delayed by\n"
		"                              <n> seconds)\n"
		"                              allowed range: 1...3600\n"
		"  -y <n>, --colorscheme <n>   set color scheme for main plot\n"
		"                              0: blue-white (default)\n"
		"                              1: green-black\n"
		"                              2: black-grey\n"
		"  --mdioutadr <s>             MDI out network address\n"
		"                              format [IP#]:[port]\n"
		"  --mdiinport <n>             set MDI in port number\n"
#endif

		"  -I <n>, --snddevin <n>      set sound in device\n"
		"  -O <n>, --snddevout <n>     set sound out device\n"

#ifdef HAVE_LIBHAMLIB
		"  -M <n>, --hamlib-model <n>  set Hamlib radio model ID\n"
		"  -C <s>, --hamlib-config <s> set Hamlib config parameter\n"
		"  -T, --ensmeter              enable S-Meter\n"
#endif

		"\n  -h, -?, --help             this help text\n\n"
		"Example: " + string(argv[0]) +
		" -p --sampleoff -0.23 -i 2 "
#ifdef USE_QT_GUI
		"-r 6140 -a 50°13\\'N -o 8°34\\'E --mdioutadr 127.0.0.1:3002"
#endif
		"\n\n";
}

_BOOLEAN CSettings::GetFlagArgument(int argc, char** argv, int& i,
									string strShortOpt, string strLongOpt)
{
	if ((!strShortOpt.compare(argv[i])) || (!strLongOpt.compare(argv[i])))
		return TRUE;
	else
		return FALSE;
}

_BOOLEAN CSettings::GetStringArgument(int argc, char** argv, int& i,
									  string strShortOpt, string strLongOpt,
									  string& strArg)
{
	if ((!strShortOpt.compare(argv[i])) || (!strLongOpt.compare(argv[i])))
	{
		if (++i >= argc)
		{
			cerr << argv[0] << ": ";
			cerr << "'" << strLongOpt << "' needs a string argument" << endl;
			exit(1);
		}

		strArg = argv[i];

		return TRUE;
	}
	else
		return FALSE;
}

_BOOLEAN CSettings::GetNumericArgument(int argc, char** argv, int& i,
									   string strShortOpt, string strLongOpt,
									   _REAL rRangeStart, _REAL rRangeStop,
									   _REAL& rValue)
{
	if ((!strShortOpt.compare(argv[i])) || (!strLongOpt.compare(argv[i])))
	{
		if (++i >= argc)
		{
			cerr << argv[0] << ": ";
			cerr << "'" << strLongOpt << "' needs a numeric argument between "
				<< rRangeStart << " and " << rRangeStop << endl;
			exit(1);
		}

		char *p;
		rValue = strtod(argv[i], &p);
		if (*p || rValue < rRangeStart || rValue > rRangeStop)
		{
			cerr << argv[0] << ": ";
			cerr << "'" << strLongOpt << "' needs a numeric argument between "
				<< rRangeStart << " and " << rRangeStop << endl;
			exit(1);
		}

		return TRUE;
	}
	else
		return FALSE;
}


/* INI File routines using the STL ********************************************/
/* The following code was taken from "INI File Tools (STLINI)" written by
   Robert Kesterson in 1999. The original files are stlini.cpp and stlini.h.
   The homepage is http://robertk.com/source

   Copyright August 18, 1999 by Robert Kesterson */

#ifdef _MSC_VER
/* These pragmas are to quiet VC++ about the expanded template identifiers
   exceeding 255 chars. You won't be able to see those variables in a debug
   session, but the code will run normally */
#pragma warning (push)
#pragma warning (disable : 4786 4503)
#endif

string CSettings::GetIniSetting(CSettings::INIFile& theINI, const char* section,
								const char* key, const char* defaultval)
{
	string result(defaultval);
	INIFile::iterator iSection = theINI.find(string(section));

	if (iSection != theINI.end())
	{
		INISection::iterator apair = iSection->second.find(string(key));

		if (apair != iSection->second.end())
			result = apair->second;
	}

	return result;
}

void CSettings::PutIniSetting(CSettings::INIFile &theINI, const char *section,
							  const char *key, const char *value)
{
	INIFile::iterator		iniSection;
	INISection::iterator	apair;
	
	if ((iniSection = theINI.find(string(section))) == theINI.end())
	{
		/* No such section? Then add one */
		INISection newsection;
		if (key)
		{
				//pair<string, string, allocator<string>, allocator<string> > a;//(string(key), string(value));
				//pair<string, string > a;//(string(key), string(value));
			newsection.insert(
					//a);
				pair<const string, string >(string(key), string(value)));
				
		}

		theINI.insert(
			pair<const string, INISection >(string(section), newsection));
	}
	else if (key)
	{	
		/* Found section, make sure key isn't in there already, 
		   if it is, just drop and re-add */
		apair = iniSection->second.find(string(key));
		if (apair != iniSection->second.end())
			iniSection->second.erase(apair);

		iniSection->second.insert(
			pair<const string, string >(string(key), string(value)));
	}
}

CSettings::INIFile CSettings::LoadIni(const char* filename)
{
	INIFile			theINI;
	char			*value, *temp;
	string			section;
	char			buffer[MAX_INI_LINE];
	//fstream	file(filename, std::ios::in);
	ifstream	file(filename);
	
	while (file.good())
	{
		memset(buffer, 0, sizeof(buffer));
		file.getline(buffer, sizeof(buffer));

		if ((temp = strchr(buffer, '\n')))
			*temp = '\0'; /* Cut off at newline */

		if ((temp = strchr(buffer, '\r')))
			*temp = '\0'; /* Cut off at linefeeds */

		if ((buffer[0] == '[') && (temp = strrchr(buffer, ']')))
		{   /* if line is like -->   [section name] */
			*temp = '\0'; /* Chop off the trailing ']' */
			section = &buffer[1];
			PutIniSetting(theINI, &buffer[1]); /* Start new section */
		}
		else if (buffer[0] && (value = strchr(buffer, '=')))
		{
			/* Assign whatever follows = sign to value, chop at "=" */
			*value++ = '\0';

			/* And add both sides to INISection */
			PutIniSetting(theINI, section.c_str(), buffer, value);
		}
		else if (buffer[0])
		{
			/* Must be a comment or something */
			PutIniSetting(theINI, section.c_str(), buffer, "");
		}
	}
	return theINI;
}

void CSettings::SaveIni(CSettings::INIFile &theINI, const char* filename)
{
	_BOOLEAN bFirstSection = TRUE; /* Init flag */

	ofstream file(filename);
	if(!file.good())
		return;
	
	/* Just iterate the hashes and values and dump them to a file */
	INIFile::iterator section = theINI.begin();
	while (section != theINI.end())
	{
		if (section->first > "")
		{
			if (bFirstSection == TRUE)
			{
				/* Don't put a newline at the beginning of the first section */
				file << "[" << section->first << "]" << endl;

				/* Reset flag */
				bFirstSection = FALSE;
			}
			else
				file << endl << "[" << section->first << "]" << endl;
		}

		INISection::iterator pair = section->second.begin();
	
		while (pair != section->second.end())
		{
			if (pair->second > "")
				file << pair->first << "=" << pair->second << endl;
			else
				file << pair->first << "=" << endl;
			pair++;
		}
		section++;
	}
	file.close();
}

/* Return true or false depending on whether the first string is less than the
   second */
bool CSettings::StlIniCompareStringNoCase::operator()(const string& x,
													  const string& y) const
{
#ifdef WIN32
	return (stricmp(x.c_str(), y.c_str()) < 0) ? true : false;
#else
#ifdef strcasecmp
	return (strcasecmp(x.c_str(), y.c_str()) < 0) ? true : false;
#else
	unsigned	nCount = 0;
	int			nResult = 0;
	const char	*p1 = x.c_str();
	const char	*p2 = y.c_str();

	while (*p1 && *p2)
	{
		nResult = toupper(*p1) - toupper(*p2);
		if (nResult != 0)
			break;
		p1++;
		p2++;
		nCount++;
	}
	if (nResult == 0)
	{
		if (*p1 && !*p2)
			nResult = -1;
		if (!*p1 && *p2)
			nResult = 1;
	}
	if (nResult < 0)
		return true;
	return false;
#endif /* strcasecmp */
#endif
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
