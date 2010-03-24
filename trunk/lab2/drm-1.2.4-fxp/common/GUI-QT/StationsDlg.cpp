/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2004
 *
 * Author(s):
 *	Volker Fischer, Stephane Fillod, Tomi Manninen
 *
 * 5/15/2005 Andrea Russo
 *	- added preview
 * 5/25/2005 Andrea Russo
 *	- added "days" column in stations list view
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

#include "StationsDlg.h"

#if !defined(HAVE_RIG_PARSE_MODE) && defined(HAVE_LIBHAMLIB)
extern "C"
{
	extern rmode_t parse_mode(const char *);
	extern vfo_t parse_vfo(const char *);
	extern setting_t parse_func(const char *);
	extern setting_t parse_level(const char *);
	extern setting_t parse_parm(const char *);
	extern const char* strstatus(enum rig_status_e);
}
# define rig_parse_mode parse_mode
# define rig_parse_vfo parse_vfo
# define rig_parse_func parse_func
# define rig_parse_level parse_level
# define rig_parse_parm parse_parm
# define rig_strstatus strstatus
#endif


/* Implementation *************************************************************/
void CDRMSchedule::ReadStatTabFromFile(const ESchedMode eNewSchM)
{
	const int	iMaxLenName = 256;
	char		cName[iMaxLenName];
	int			iFileStat;
	_BOOLEAN	bReadOK = TRUE;
	FILE*		pFile;

	/* Save new mode */
	eSchedMode = eNewSchM;

	/* Open file and init table for stations */
	StationsTable.Init(0);

	switch (eNewSchM)
	{
	case SM_DRM:
		pFile = fopen(DRMSCHEDULE_INI_FILE_NAME, "r");
		break;

	case SM_ANALOG:
		pFile = fopen(AMSCHEDULE_INI_FILE_NAME, "r");
		break;
	}

	/* Check if opening of file was successful */
	if (pFile == 0)
		return;

	fgets(cName, iMaxLenName, pFile); /* Remove "[DRMSchedule]" */
	do
	{
		CStationsItem StationsItem;

		/* Start stop time */
		int iStartTime, iStopTime;
		iFileStat = fscanf(pFile, "StartStopTimeUTC=%04d-%04d\n",
			&iStartTime, &iStopTime);

		if (iFileStat != 2)
			bReadOK = FALSE;
		else
		{
			StationsItem.SetStartTimeNum(iStartTime);
			StationsItem.SetStopTimeNum(iStopTime);
		}

		/* Days */
		/* Init days with the "irregular" marker in case no valid string could
		   be read */
		string strNewDaysFlags = FLAG_STR_IRREGULAR_TRANSM;

		iFileStat = fscanf(pFile, "Days[SMTWTFS]=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
		{
			/* Check for length of input string (must be 7) */
			const string strTMP = cName;
			if (strTMP.length() == 7)
				strNewDaysFlags = strTMP;
		}

		/* Frequency */
		iFileStat = fscanf(pFile, "Frequency=%d\n", &StationsItem.iFreq);
		if (iFileStat != 1)
			bReadOK = FALSE;

		/* Target */
		iFileStat = fscanf(pFile, "Target=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
			StationsItem.strTarget = cName;

		/* Power */
		iFileStat = fscanf(pFile, "Power=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
			StationsItem.rPower = QString(cName).toFloat();

		/* Name of the station */
		iFileStat = fscanf(pFile, "Programme=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
			StationsItem.strName = cName;

		/* Language */
		iFileStat = fscanf(pFile, "Language=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
			StationsItem.strLanguage = cName;

		/* Site */
		iFileStat = fscanf(pFile, "Site=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
			StationsItem.strSite = cName;

		/* Country */
		iFileStat = fscanf(pFile, "Country=%255[^\n|^\r]\n", cName);
		if (iFileStat != 1)
			fscanf(pFile, "\n");
		else
			StationsItem.strCountry = cName;

		iFileStat = fscanf(pFile, "\n");

		/* Check for error before applying data */
		if (bReadOK == TRUE)
		{
			/* Set "days flag string" and generate strings for displaying active
			   days */
			StationsItem.SetDaysFlagString(strNewDaysFlags);

			/* Add new item in table */
			StationsTable.Add(StationsItem);
		}
	} while (!((iFileStat == EOF) || (bReadOK == FALSE)));

	fclose(pFile);
}

CDRMSchedule::StationState CDRMSchedule::CheckState(const int iPos)
{
	/* Get system time */
	time_t ltime;
	time(&ltime);

	if (IsActive(iPos, ltime) == TRUE)
		return IS_ACTIVE;
	else
	{
		/* Station is not active, check preview condition */
		if (iSecondsPreview > 0)
		{
			if (IsActive(iPos, ltime + iSecondsPreview) == TRUE)
				return IS_PREVIEW;
			else
				return IS_INACTIVE;
		}
		else
			return IS_INACTIVE;
	}
}

_BOOLEAN CDRMSchedule::IsActive(const int iPos, const time_t ltime)
{
	/* Calculate time in UTC */
	struct tm* gmtCur = gmtime(&ltime);
	const time_t lCurTime = mktime(gmtCur);

	/* Get stop time */
	struct tm* gmtStop = gmtime(&ltime);
	gmtStop->tm_hour = StationsTable[iPos].iStopHour;
	gmtStop->tm_min = StationsTable[iPos].iStopMinute;
	const time_t lStopTime = mktime(gmtStop);

	/* Get start time */
	struct tm* gmtStart = gmtime(&ltime);
	gmtStart->tm_hour = StationsTable[iPos].iStartHour;
	gmtStart->tm_min = StationsTable[iPos].iStartMinute;
	const time_t lStartTime = mktime(gmtStart);

	/* Check, if stop time is on next day */
	_BOOLEAN bSecondDay = FALSE;
	if (lStopTime < lStartTime)
	{
		/* Check, if we are at the first or the second day right now */
		if (lCurTime < lStopTime)
		{
			/* Second day. Increase day count */
			gmtCur->tm_wday++;

			/* Check that value is valid (range 0 - 6) */
			if (gmtCur->tm_wday > 6)
				gmtCur->tm_wday = 0;

			/* Set flag */
			bSecondDay = TRUE;
		}
	}

	/* Check day
	   tm_wday: day of week (0 - 6; Sunday = 0). "strDaysFlags" are coded with
	   pseudo binary representation. A one signalls that day is active. The most
	   significant 1 is the sunday, then followed the monday and so on. */
	if ((StationsTable[iPos].strDaysFlags[gmtCur->tm_wday] ==
		CHR_ACTIVE_DAY_MARKER) ||
		/* Check also for special case: days are 0000000. This is reserved for
		   DRM test transmissions or irregular transmissions. We define here
		   that these stations are transmitting every day */
		(StationsTable[iPos].strDaysFlags == FLAG_STR_IRREGULAR_TRANSM))
	{
		/* Check time interval */
		if (lStopTime > lStartTime)
		{
			if ((lCurTime >= lStartTime) && (lCurTime < lStopTime))
				return TRUE;
		}
		else
		{
			if (bSecondDay == FALSE)
			{
				/* First day. Only check if we are after start time */
				if (lCurTime >= lStartTime)
					return TRUE;
			}
			else
			{
				/* Second day. Only check if we are before stop time */
				if (lCurTime < lStopTime)
					return TRUE;
			}
		}
	}

	return FALSE;
}

void CStationsItem::SetDaysFlagString(const string strNewDaysFlags)
{
	/* Set internal "days flag" string and "show days" string */
	strDaysFlags = strNewDaysFlags;
	strDaysShow = "";

	/* Init days string vector */
	const QString strDayDef [] =
	{
		QObject::tr("Sun"),
		QObject::tr("Mon"),
		QObject::tr("Tue"),
		QObject::tr("Wed"),
		QObject::tr("Thu"),
		QObject::tr("Fri"),
		QObject::tr("Sat")
	};

	/* First test for day constellations which allow to apply special names */
	if (strDaysFlags == FLAG_STR_IRREGULAR_TRANSM)
		strDaysShow = QObject::tr("irregular").latin1();
	else if (strDaysFlags == "1111111")
		strDaysShow = QObject::tr("daily").latin1();
	else if (strDaysFlags == "1111100")
		strDaysShow = QObject::tr("from Sun to Thu").latin1();
	else if (strDaysFlags == "1111110")
		strDaysShow = QObject::tr("from Sun to Fri").latin1();
	else if (strDaysFlags == "0111110")
		strDaysShow = QObject::tr("from Mon to Fri").latin1();
	else if (strDaysFlags == "0111111")
		strDaysShow = QObject::tr("from Mon to Sat").latin1();
	else
	{
		/* No special name could be applied, just list all active days */
		for (int i = 0; i < 7; i++)
		{
			/* Check if day is active */
			if (strDaysFlags[i] == CHR_ACTIVE_DAY_MARKER)
			{
				/* Set commas in between the days, to not set a comma at
				   the beginning */
				if (strDaysShow != "") 
					strDaysShow += ",";

				/* Add current day */
				strDaysShow += strDayDef[i].latin1();
			}
		}
	}
}

StationsDlg::StationsDlg(CDRMReceiver* pNDRMR, QWidget* parent,
	const char* name, bool modal, WFlags f) : vecpListItems(0),
	CStationsDlgBase(parent, name, modal, f), pDRMRec(pNDRMR)
{
	/* Set help text for the controls */
	AddWhatsThisHelp();

#ifdef _WIN32 /* This works only reliable under Windows :-( */
	/* Get window geometry data from DRMReceiver module and apply it */
	const QRect WinGeom(pDRMRec->GeomStationsDlg.iXPos,
		pDRMRec->GeomStationsDlg.iYPos,
		pDRMRec->GeomStationsDlg.iWSize,
		pDRMRec->GeomStationsDlg.iHSize);

	if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
		setGeometry(WinGeom);
#else /* Under Linux only restore the size */
	resize(pDRMRec->GeomStationsDlg.iWSize,
		pDRMRec->GeomStationsDlg.iHSize);
#endif

	/* Define size of the bitmaps */
	const int iXSize = 13;
	const int iYSize = 13;

	/* Create bitmaps */
	BitmCubeGreen.resize(iXSize, iYSize);
	BitmCubeGreen.fill(QColor(0, 255, 0));
	BitmCubeYellow.resize(iXSize, iYSize);
	BitmCubeYellow.fill(QColor(255, 255, 0));
	BitmCubeRed.resize(iXSize, iYSize);
	BitmCubeRed.fill(QColor(255, 0, 0));
	BitmCubeOrange.resize(iXSize, iYSize);
	BitmCubeOrange.fill(QColor(255, 128, 0));

#ifdef HAVE_LIBHAMLIB
	/* Init progress bar for input s-meter */
	ProgrSigStrength->setRange(S_METER_THERMO_MIN, S_METER_THERMO_MAX);
	ProgrSigStrength->setOrientation(QwtThermo::Horizontal, QwtThermo::Top);
	ProgrSigStrength->setAlarmLevel(S_METER_THERMO_ALARM);
	ProgrSigStrength->setAlarmColor(QColor(255, 0, 0));
	ProgrSigStrength->setScale(S_METER_THERMO_MIN, S_METER_THERMO_MAX, 10.0);
	EnableSMeter(FALSE); /* disable for initialization */
#else
	/* s-meter only implemented for hamlib */
	ProgrSigStrength->hide();
	TextLabelSMeter->hide();
#endif

	/* Clear list box for file names and set up columns */
	ListViewStations->clear();

	/* We assume that one column is already there */
	ListViewStations->setColumnText(0, tr("Station Name"));
	ListViewStations->addColumn(tr("Time [UTC]"));
	ListViewStations->addColumn(tr("Frequency [kHz]"));
	ListViewStations->addColumn(tr("Target"));
	ListViewStations->addColumn(tr("Power [kW]"));
	ListViewStations->addColumn(tr("Country"));
	ListViewStations->addColumn(tr("Site"));
	ListViewStations->addColumn(tr("Language"));
	ListViewStations->addColumn(tr("Days"));

	/* Load the current schedule from file and initialize list view */
	LoadSchedule(CDRMSchedule::SM_DRM);

	/* Set up frequency selector control (QWTCounter control) */
	QwtCounterFrequency->setRange(0.0, 30000.0, 1.0);
	QwtCounterFrequency->setNumButtons(3); /* Three buttons on each side */
	QwtCounterFrequency->setIncSteps(QwtCounter::Button1, 1); /* Increment */
	QwtCounterFrequency->setIncSteps(QwtCounter::Button2, 10);
	QwtCounterFrequency->setIncSteps(QwtCounter::Button3, 100);

	/* Init with current setting in log file */
	QwtCounterFrequency->
		setValue(pDRMRec->GetParameters()->ReceptLog.GetFrequency());

	/* Init UTC time shown with a label control */
	SetUTCTimeLabel();


	/* Set Menu ***************************************************************/
	/* View menu ------------------------------------------------------------ */
	pViewMenu = new QPopupMenu(this);
	CHECK_PTR(pViewMenu);
	pViewMenu->insertItem(tr("Show &only active stations"), this,
		SLOT(OnShowStationsMenu(int)), 0, 0);
	pViewMenu->insertItem(tr("Show &all stations"), this,
		SLOT(OnShowStationsMenu(int)), 0, 1);

	/* Set stations in list view which are active right now */
	bShowAll = FALSE;
	pViewMenu->setItemChecked(0, TRUE);


	/* Stations Preview menu ------------------------------------------------ */
	pPreviewMenu = new QPopupMenu(this);
	CHECK_PTR(pPreviewMenu);
	pPreviewMenu->insertItem(tr("&Disabled"), this,
		SLOT(OnShowPreviewMenu(int)), 0, 0);
	pPreviewMenu->insertItem(tr("&5 minutes"), this,
		SLOT(OnShowPreviewMenu(int)), 0, 1);
	pPreviewMenu->insertItem(tr("&15 minutes"), this,
		SLOT(OnShowPreviewMenu(int)), 0, 2);
	pPreviewMenu->insertItem(tr("&30 minutes"), this,
		SLOT(OnShowPreviewMenu(int)), 0, 3);

	/* Set stations preview */
	/* Retrive the setting saved into the .ini file */
	DRMSchedule.SetSecondsPreview(pDRMRec->iSecondsPreview);
	switch (DRMSchedule.GetSecondsPreview())
	{
	case NUM_SECONDS_PREV_5MIN:
		pPreviewMenu->setItemChecked(1, TRUE);
		break;

	case NUM_SECONDS_PREV_15MIN:
		pPreviewMenu->setItemChecked(2, TRUE);
		break;

	case NUM_SECONDS_PREV_30MIN:
		pPreviewMenu->setItemChecked(3, TRUE);
		break;

	default: /* case 0: */
		pPreviewMenu->setItemChecked(0, TRUE);
		break;
	}

	pViewMenu->insertSeparator();
	pViewMenu->insertItem(tr("Stations &preview"),pPreviewMenu);


	SetStationsView();


#ifdef HAVE_LIBHAMLIB
	/* Remote menu  --------------------------------------------------------- */
	pRemoteMenu = new QPopupMenu(this);
	CHECK_PTR(pRemoteMenu);

	pRemoteMenuOther = new QPopupMenu(this);
	CHECK_PTR(pRemoteMenuOther);

	/* Init vector for storing the model IDs with zero length */
	veciModelID.Init(0);

	/* Add menu entry "none" */
	pRemoteMenu->insertItem(tr("None"), this, SLOT(OnRemoteMenu(int)), 0, 0);
	veciModelID.Add(0); /* ID 0 for "none" */

	/* Number of supported rigs */
	const int iNumRigs = pDRMRec->GetHamlib()->GetNumHamModels();

	/* Add menu entries */
	_BOOLEAN bCheckWasSet = FALSE;
	for (int i = 0; i < iNumRigs; i++)
	{
		/* Get rig details */
		const CHamlib::SDrRigCaps CurSDRiCa =
			pDRMRec->GetHamlib()->GetHamModel(i);

		/* Store model ID */
		veciModelID.Add(CurSDRiCa.iModelID);
		const int iCurModIDIdx = veciModelID.Size() - 1;

		/* Create menu objects which belong to an action group. We hope that
		   QT takes care of all the new objects and deletes them... */
		if (CurSDRiCa.bIsSpecRig == TRUE)
		{
			/* Main rigs */
			pRemoteMenu->insertItem(
				/* Set menu string. Should look like:
				   [ID] Manuf. Model */
				"[" + QString().setNum(CurSDRiCa.iModelID) + "] " +
				CurSDRiCa.strManufacturer + " " +
				CurSDRiCa.strModelName,
				this, SLOT(OnRemoteMenu(int)), 0, iCurModIDIdx);

			/* Check for checking */
			if (pDRMRec->GetHamlib()->GetHamlibModelID() == CurSDRiCa.iModelID)
			{
				pRemoteMenu->setItemChecked(iCurModIDIdx, TRUE);
				bCheckWasSet = TRUE;
			}
		}
		else
		{
			/* "Other" menu */
			pRemoteMenuOther->insertItem(
				/* Set menu string. Should look like:
				   [ID] Manuf. Model (status) */
				"[" + QString().setNum(CurSDRiCa.iModelID) + "] " +
				CurSDRiCa.strManufacturer + " " +
				CurSDRiCa.strModelName +
				" (" + rig_strstatus(CurSDRiCa.eRigStatus) +
				")",
				this, SLOT(OnRemoteMenu(int)), 0, iCurModIDIdx);

			/* Check for checking */
			if (pDRMRec->GetHamlib()->GetHamlibModelID() == CurSDRiCa.iModelID)
			{
				pRemoteMenuOther->setItemChecked(iCurModIDIdx, TRUE);
				bCheckWasSet = TRUE;
			}
		}
	}

	/* Add "other" menu */
	pRemoteMenu->insertItem(tr("Other"), pRemoteMenuOther);

	/* If no rig was selected, set check to "none" */
	if (bCheckWasSet == FALSE)
		pRemoteMenu->setItemChecked(0, TRUE);

	/* Separator */
	pRemoteMenu->insertSeparator();


	/* COM port selection --------------------------------------------------- */
	/* Toggle action for com port selection menu entries */
	agCOMPortSel = new QActionGroup(this, "Com port", TRUE);

	pacMenuCOM1 = new QAction("COM1", "COM1", 0, agCOMPortSel, 0, TRUE);
	pacMenuCOM2 = new QAction("COM2", "COM2", 0, agCOMPortSel, 0, TRUE);
	pacMenuCOM3 = new QAction("COM3", "COM3", 0, agCOMPortSel, 0, TRUE);
	pacMenuCOM4 = new QAction("COM4", "COM4", 0, agCOMPortSel, 0, TRUE);
	pacMenuCOM5 = new QAction("COM5", "COM5", 0, agCOMPortSel, 0, TRUE);

	/* Add COM port selection menu group to remote menu */
	agCOMPortSel->addTo(pRemoteMenu);

	/* Try to get the COM port number from the hamlib configure string */
	if (pDRMRec->GetHamlib()->GetHamlibConf() == HAMLIB_CONF_COM1)
		pacMenuCOM1->setOn(TRUE);

	if (pDRMRec->GetHamlib()->GetHamlibConf() == HAMLIB_CONF_COM2)
		pacMenuCOM2->setOn(TRUE);

	if (pDRMRec->GetHamlib()->GetHamlibConf() == HAMLIB_CONF_COM3)
		pacMenuCOM3->setOn(TRUE);

	if (pDRMRec->GetHamlib()->GetHamlibConf() == HAMLIB_CONF_COM4)
		pacMenuCOM4->setOn(TRUE);

	if (pDRMRec->GetHamlib()->GetHamlibConf() == HAMLIB_CONF_COM5)
		pacMenuCOM5->setOn(TRUE);


	/* Other settings ------------------------------------------------------- */
	/* Separator */
	pRemoteMenu->insertSeparator();

	/* Enable s-meter */
	const int iSMeterMenuID = pRemoteMenu->insertItem(tr("Enable S-Meter"),
		this, SLOT(OnSMeterMenu(int)), 0);

	/* S-meter settings */
	pRemoteMenu->setItemChecked(iSMeterMenuID, pDRMRec->bEnableSMeter);
	EnableSMeter(pDRMRec->bEnableSMeter);

	/* Separator */
	pRemoteMenu->insertSeparator();

	/* Enable special settings for rigs */
	const int iModRigMenuID = pRemoteMenu->insertItem(tr("With DRM "
		"Modification"), this, SLOT(OnModRigMenu(int)), 0);

	/* Set check */
	pRemoteMenu->setItemChecked(iModRigMenuID,
		pDRMRec->GetHamlib()->GetEnableModRigSettings());
#endif


	/* Update menu ---------------------------------------------------------- */
	QPopupMenu* pUpdateMenu = new QPopupMenu(this);
	CHECK_PTR(pUpdateMenu);
	pUpdateMenu->insertItem(tr("&Get Update..."), this, SLOT(OnGetUpdate()));


	/* Main menu bar -------------------------------------------------------- */
	QMenuBar* pMenu = new QMenuBar(this);
	CHECK_PTR(pMenu);
	pMenu->insertItem(tr("&View"), pViewMenu);
#ifdef HAVE_LIBHAMLIB
	pMenu->insertItem(tr("&Remote"), pRemoteMenu);
#endif
	pMenu->insertItem(tr("&Update"), pUpdateMenu); /* String "Udate" used below */
	pMenu->setSeparator(QMenuBar::InWindowsStyle);

	/* Now tell the layout about the menu */
	CStationsDlgBaseLayout->setMenuBar(pMenu);


	/* Register the network protokol (ftp). This is needed for the DRMSchedule
	   download */
	QNetworkProtocol::registerNetworkProtocol("ftp",
		new QNetworkProtocolFactory<QFtp>);


	/* Connections ---------------------------------------------------------- */
#ifdef HAVE_LIBHAMLIB
	/* Action group */
	connect(agCOMPortSel, SIGNAL(selected(QAction*)),
		this, SLOT(OnComPortMenu(QAction*)));
#endif

	connect(&TimerList, SIGNAL(timeout()),
		this, SLOT(OnTimerList()));
	connect(&TimerUTCLabel, SIGNAL(timeout()),
		this, SLOT(OnTimerUTCLabel()));
	connect(&TimerSMeter, SIGNAL(timeout()),
		this, SLOT(OnTimerSMeter()));

	connect(ListViewStations, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(OnListItemClicked(QListViewItem*)));
	connect(&UrlUpdateSchedule, SIGNAL(finished(QNetworkOperation*)),
		this, SLOT(OnUrlFinished(QNetworkOperation*)));

	connect(QwtCounterFrequency, SIGNAL(valueChanged(double)),
		this, SLOT(OnFreqCntNewValue(double)));


	/* Set up timers */
	TimerList.start(GUI_TIMER_LIST_VIEW_STAT); /* Stations list */
	TimerUTCLabel.start(GUI_TIMER_UTC_TIME_LABEL);
}

StationsDlg::~StationsDlg()
{
	/* Set window geometry data in DRMReceiver module */
	QRect WinGeom = geometry();

	pDRMRec->GeomStationsDlg.iXPos = WinGeom.x();
	pDRMRec->GeomStationsDlg.iYPos = WinGeom.y();
	pDRMRec->GeomStationsDlg.iHSize = WinGeom.height();
	pDRMRec->GeomStationsDlg.iWSize = WinGeom.width();

	/* Store preview settings */
	pDRMRec->iSecondsPreview = DRMSchedule.GetSecondsPreview();
}

void StationsDlg::SetUTCTimeLabel()
{
	/* Get current UTC time */
	time_t ltime;
	time(&ltime);
	struct tm* gmtCur = gmtime(&ltime);

	/* Generate time in format "UTC 12:00" */
	QString strUTCTime = QString().sprintf("%02d:%02d UTC",
		gmtCur->tm_hour, gmtCur->tm_min);

	/* Only apply if time label does not show the correct time */
	if (TextLabelUTCTime->text().compare(strUTCTime))
		TextLabelUTCTime->setText(strUTCTime);
}

void StationsDlg::OnShowStationsMenu(int iID)
{
	/* Show only active stations if ID is 0, else show all */
	if (iID == 0)
		bShowAll = FALSE;
	else
		bShowAll = TRUE;

	/* Update list view */
	SetStationsView();

	/* Taking care of checks in the menu */
	pViewMenu->setItemChecked(0, 0 == iID);
	pViewMenu->setItemChecked(1, 1 == iID);
}

void StationsDlg::OnShowPreviewMenu(int iID)
{
	switch (iID)
	{
	case 1:
		DRMSchedule.SetSecondsPreview(NUM_SECONDS_PREV_5MIN);
		break;

	case 2:
		DRMSchedule.SetSecondsPreview(NUM_SECONDS_PREV_15MIN);
		break;

	case 3:
		DRMSchedule.SetSecondsPreview(NUM_SECONDS_PREV_30MIN);
		break;

	default: /* case 0: */
		DRMSchedule.SetSecondsPreview(0);
		break;
	}

	/* Update list view */
	SetStationsView();

	/* Taking care of checks in the menu */
	pPreviewMenu->setItemChecked(0, 0 == iID);
	pPreviewMenu->setItemChecked(1, 1 == iID);
	pPreviewMenu->setItemChecked(2, 2 == iID);
	pPreviewMenu->setItemChecked(3, 3 == iID);
}

void StationsDlg::OnGetUpdate()
{
	if (QMessageBox::information(this, tr("Dream Schedule Update"),
		tr("Dream tries to download the newest DRM schedule\nfrom "
		"www.drm-dx.de (powered by Klaus Schneider).\nYour computer "
		"must be connected to the internet.\n\nThe current file "
		"DRMSchedule.ini will be overwritten.\nDo you want to "
		"continue?"),
		QMessageBox::Yes, QMessageBox::No) == 3 /* Yes */)
	{
		/* Try to download the current schedule. Copy the file to the
		   current working directory (which is "QDir().absFilePath(NULL)") */
		UrlUpdateSchedule.copy(QString(DRM_SCHEDULE_UPDATE_FILE),
			QString(QDir().absFilePath(NULL)));
	}
}

void StationsDlg::OnUrlFinished(QNetworkOperation* pNetwOp)
{
	/* Check that pointer points to valid object */
	if (pNetwOp)
	{
		if (pNetwOp->state() == QNetworkProtocol::StFailed)
		{
			/* Something went wrong -> stop all network operations */
			UrlUpdateSchedule.stop();

			/* Notify the user of the failure */
			QMessageBox::information(this, "Dream",
				tr("Update failed. The following things may caused the "
				"failure:\n"
				"\t- the internet connection was not set up properly\n"
				"\t- the server www.drm-dx.de is currently not available\n"
				"\t- the file 'DRMSchedule.ini' could not be written"),
				QMessageBox::Ok);
		}

		/* We are interested in the state of the final put function */
		if (pNetwOp->operation() == QNetworkProtocol::OpPut)
		{
			if (pNetwOp->state() == QNetworkProtocol::StDone)
			{
				/* Notify the user that update was successful */
#ifdef _WIN32
				QMessageBox::warning(this, "Dream", tr("Update successful.\n"
					"Due to network problems with the Windows version of QT, "
					"the Dream software must be restarted after a DRMSchedule "
					"update.\nPlease exit Dream now."),
					tr("Ok"));
#else
				QMessageBox::information(this, "Dream",
					tr("Update successful."), QMessageBox::Ok);
#endif

				/* Read updated ini-file */
				LoadSchedule(CDRMSchedule::SM_DRM);
			}
		}
	}
}

void StationsDlg::showEvent(QShowEvent* pEvent)
{
	/* If number of stations is zero, we assume that the ini file is missing */
	if (DRMSchedule.GetStationNumber() == 0)
	{
		if (DRMSchedule.GetSchedMode() == CDRMSchedule::SM_DRM)
		{
			QMessageBox::information(this, "Dream", tr("The file "
				DRMSCHEDULE_INI_FILE_NAME
				" could not be found or contains no data.\nNo "
				"stations can be displayed.\nTry to download this file by "
				"using the 'Update' menu."), QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(this, "Dream", tr("The file "
				AMSCHEDULE_INI_FILE_NAME
				" could not be found or contains no data.\nNo "
				"stations can be displayed."), QMessageBox::Ok);
		}
	}
}

void StationsDlg::OnTimerList()
{
	/* Update list view */
	SetStationsView();
}

QString MyListViewItem::key(int column, bool ascending) const
{
	/* Reimplement "key()" function to get correct sorting behaviour */
	if ((column == 2) || (column == 4))
	{
		/* These columns are filled with numbers. Some items may have numbers
		   after the comma, therefore multiply with 10000 (which moves the
		   numbers in front of the comma). Afterwards append zeros at the
		   beginning so that positive integer numbers are sorted correctly */
		return QString(QString().setNum((long int)
			(text(column).toFloat() * 10000.0))).rightJustify(20, '0');
	}
    else
		return QListViewItem::key(column, ascending);
}

void StationsDlg::LoadSchedule(CDRMSchedule::ESchedMode eNewSchM)
{
	/* Lock mutex for modifying the vecpListItems */
	ListItemsMutex.Lock();

	/* Delete all old list view items (it is important that the vector
	   "vecpListItems" was initialized to 0 at creation of the global object
	   otherwise this may cause an segmentation fault) */
	for (int i = 0; i < vecpListItems.Size(); i++)
	{
		if (vecpListItems[i] != NULL)
			delete vecpListItems[i];
	}

	/* Read initialization file */
	DRMSchedule.ReadStatTabFromFile(eNewSchM);

	/* Init vector for storing the pointer to the list view items */
	vecpListItems.Init(DRMSchedule.GetStationNumber(), NULL);

	/* Set sorting behaviour of the list */
	switch (eNewSchM)
	{
	case CDRMSchedule::SM_DRM:
		/* Sort list by transmit power (5th column), most powerful on top */
		ListViewStations->setSorting(4, FALSE);
		break;

	case CDRMSchedule::SM_ANALOG:
		/* Sort list by station name (1th column) */
		ListViewStations->setSorting(0, TRUE);
		break;
	}

	/* Unlock BEFORE calling the stations view update because in this function
	   the mutex is locked, too! */
	ListItemsMutex.Unlock();

	/* Update list view */
	SetStationsView();
}

void StationsDlg::SetStationsView()
{
	/* Set lock because of list view items. These items could be changed
	   by another thread */
	ListItemsMutex.Lock();

	const int iNumStations = DRMSchedule.GetStationNumber();
	_BOOLEAN bListHastChanged = FALSE;

	/* Add new item for each station in list view */
	for (int i = 0; i < iNumStations; i++)
	{
		if (!((bShowAll == FALSE) &&
			(DRMSchedule.CheckState(i) == CDRMSchedule::IS_INACTIVE)))
		{
			/* Only insert item if it is not already in the list */
			if (vecpListItems[i] == NULL)
			{
				/* Get power of the station. We have to do a special treatment
				   here, because we want to avoid having a "0" in the list when
				   a "?" was in the schedule-ini-file */
				const _REAL rPower = DRMSchedule.GetItem(i).rPower;

				QString strPower;
				if (rPower == (_REAL) 0.0)
					strPower = "?";
				else
					strPower.setNum(rPower);

				/* Generate new list item with all necessary column entries */
				vecpListItems[i] = new MyListViewItem(ListViewStations,
					DRMSchedule.GetItem(i).strName.c_str()     /* name */,
					QString().sprintf("%04d-%04d",
					DRMSchedule.GetItem(i).GetStartTimeNum(),
					DRMSchedule.GetItem(i).GetStopTimeNum())   /* time */,
					QString().setNum(DRMSchedule.GetItem(i).iFreq) /* freq. */,
					DRMSchedule.GetItem(i).strTarget.c_str()   /* target */,
					strPower                                   /* power */,
					DRMSchedule.GetItem(i).strCountry.c_str()  /* country */,
					DRMSchedule.GetItem(i).strSite.c_str()     /* site */,
					DRMSchedule.GetItem(i).strLanguage.c_str() /* language */);

				/* Show list of days */
				vecpListItems[i]->setText(8,
					DRMSchedule.GetItem(i).strDaysShow.c_str());

				/* Insert this new item in list. The item object is destroyed by
				   the list view control when this is destroyed */
				ListViewStations->insertItem(vecpListItems[i]);

				/* Set flag for sorting the list */
				bListHastChanged = TRUE;
			}

			/* Check, if station is currently transmitting. If yes, set
			   special pixmap */
			if (DRMSchedule.CheckState(i) == CDRMSchedule::IS_ACTIVE)
			{
				/* Check for "special case" transmissions */
				if (DRMSchedule.GetItem(i).strDaysFlags ==
					FLAG_STR_IRREGULAR_TRANSM)
				{
					vecpListItems[i]->setPixmap(0, BitmCubeYellow);
				}
				else
					vecpListItems[i]->setPixmap(0, BitmCubeGreen);
			}
			else
			{
				if (DRMSchedule.CheckState(i) == CDRMSchedule::IS_PREVIEW)
					vecpListItems[i]->setPixmap(0, BitmCubeOrange);
				else
					vecpListItems[i]->setPixmap(0, BitmCubeRed);
			}
		}
		else
		{
			/* Delete this item since it is not used anymore */
			if (vecpListItems[i] != NULL)
			{
				/* If one deletes a menu item in QT list view, it is
				   automaticall removed from the list and the list gets
				   repainted */
				delete vecpListItems[i];

				/* Reset pointer so we can distinguish if it is used or not */
				vecpListItems[i] = NULL;

				/* Set flag for sorting the list */
				bListHastChanged = TRUE;
			}
		}
	}

	/* Sort the list if items have changed */
	if (bListHastChanged == TRUE)
		ListViewStations->sort();

	ListItemsMutex.Unlock();
}

void StationsDlg::OnFreqCntNewValue(double dVal)
{
#ifdef HAVE_LIBHAMLIB
	/* Set frequency to front-end */
	pDRMRec->GetHamlib()->SetFrequency((int) dVal);
#endif

	/* Set selected frequency in log file class */
	pDRMRec->GetParameters()->ReceptLog.SetFrequency((int) dVal);
}

void StationsDlg::OnListItemClicked(QListViewItem* item)
{
	/* Check that it is a valid item (!= 0) */
	if (item)
	{
		/* Third text of list view item is frequency -> text(2)
		   Set value in frequency counter control QWT. Setting this parameter
		   will emit a "value changed" signal which sets the new frequency.
		   Therefore, here is no call to "SetFrequency()" needed. Also, the
		   frequency is set in the log file, therefore here is no
		   "ReceptLog.SetFrequency()" needed, too */
		QwtCounterFrequency->setValue(QString(item->text(2)).toInt());

		/* Now tell the receiver that the frequency has changed */
		switch (DRMSchedule.GetSchedMode())
		{
		case CDRMSchedule::SM_DRM:
			pDRMRec->SetReceiverMode(CDRMReceiver::RM_DRM);
			break;

		case CDRMSchedule::SM_ANALOG:
			pDRMRec->SetReceiverMode(CDRMReceiver::RM_AM);
			break;
		}
	}
}

void StationsDlg::OnSMeterMenu(int iID)
{
#ifdef HAVE_LIBHAMLIB
	if (pRemoteMenu->isItemChecked(iID))
	{
		pRemoteMenu->setItemChecked(iID, FALSE);
		pDRMRec->bEnableSMeter = FALSE;
	}
	else
	{
		pRemoteMenu->setItemChecked(iID, TRUE);
		pDRMRec->bEnableSMeter = TRUE;
	}

	/* Only try to enable s-meter if it is not ID 0 ("none") */
	if (pDRMRec->GetHamlib()->GetHamlibModelID() != 0)
		EnableSMeter(pDRMRec->bEnableSMeter);
#endif
}

void StationsDlg::OnModRigMenu(int iID)
{
#ifdef HAVE_LIBHAMLIB
	if (pRemoteMenu->isItemChecked(iID))
	{
		pRemoteMenu->setItemChecked(iID, FALSE);
		pDRMRec->GetHamlib()->SetEnableModRigSettings(FALSE);
	}
	else
	{
		pRemoteMenu->setItemChecked(iID, TRUE);
		pDRMRec->GetHamlib()->SetEnableModRigSettings(TRUE);
	}
#endif
}

void StationsDlg::OnRemoteMenu(int iID)
{
#ifdef HAVE_LIBHAMLIB
	/* Take care of check */
	for (int i = 0; i < veciModelID.Size(); i++)
	{
		/* We don't care here that not all IDs are in each menu. If there is a
		   non-valid ID for the menu item, there is simply nothing done */
		pRemoteMenu->setItemChecked(i, i == iID);
		pRemoteMenuOther->setItemChecked(i, i == iID);
	}

	/* Set ID */
	pDRMRec->GetHamlib()->SetHamlibModelID(veciModelID[iID]);

	/* If model is changed, update s-meter because new rig might have support
	   for it. Only try to enable s-meter if it is not ID 0 ("none") */
	if (iID != 0)
		EnableSMeter(pDRMRec->bEnableSMeter);
#endif
}

void StationsDlg::OnComPortMenu(QAction* action)
{
#ifdef HAVE_LIBHAMLIB
	/* We cannot use the switch command for the non constant expressions here */
	if (action == pacMenuCOM1)
		pDRMRec->GetHamlib()->SetHamlibConf(HAMLIB_CONF_COM1);

	if (action == pacMenuCOM2)
		pDRMRec->GetHamlib()->SetHamlibConf(HAMLIB_CONF_COM2);

	if (action == pacMenuCOM3)
		pDRMRec->GetHamlib()->SetHamlibConf(HAMLIB_CONF_COM3);

	if (action == pacMenuCOM4)
		pDRMRec->GetHamlib()->SetHamlibConf(HAMLIB_CONF_COM4);

	if (action == pacMenuCOM5)
		pDRMRec->GetHamlib()->SetHamlibConf(HAMLIB_CONF_COM5);
#endif
}

void StationsDlg::OnTimerSMeter()
{
#ifdef HAVE_LIBHAMLIB
	/* Get current s-meter value */
	_REAL rCurSigStr;
	const CHamlib::ESMeterState eSMState =
		pDRMRec->GetHamlib()->GetSMeter(rCurSigStr);

	/* If a time-out happened, do not update s-meter anymore (disable it) */
	if (eSMState != CHamlib::SS_VALID)
		EnableSMeter(FALSE);
	else
		ProgrSigStrength->setValue(rCurSigStr);
#endif
}

void StationsDlg::EnableSMeter(const _BOOLEAN bStatus)
{
	/* Both, GUI "enabled" and hamlib "enabled" must be fullfilled before
	   s-meter is used */
	if ((bStatus == TRUE) && (pDRMRec->bEnableSMeter == TRUE))
	{
		/* Init progress bar for input s-meter */
		ProgrSigStrength->setAlarmEnabled(TRUE);
		ProgrSigStrength->setValue(S_METER_THERMO_MIN);
		ProgrSigStrength->setFillColor(QColor(0, 190, 0));

		ProgrSigStrength->setEnabled(TRUE);
		TextLabelSMeter->setEnabled(TRUE);

		TimerSMeter.start(GUI_TIMER_S_METER);
	}
	else
	{
		/* Set s-meter control in "disabled" status */
		ProgrSigStrength->setAlarmEnabled(FALSE);
		ProgrSigStrength->setValue(S_METER_THERMO_MAX);
		ProgrSigStrength->setFillColor(palette().disabled().light());

		ProgrSigStrength->setEnabled(FALSE);
		TextLabelSMeter->setEnabled(FALSE);

		TimerSMeter.stop();
	}
}

void StationsDlg::AddWhatsThisHelp()
{
	/* Stations List */
	QWhatsThis::add(ListViewStations,
		tr("<b>Stations List:</b> In the stations list "
		"view all DRM stations which are stored in the DRMSchedule.ini file "
		"are shown. It is possible to show only active stations by changing a "
		"setting in the 'view' menu. The color of the cube on the left of a "
		"menu item shows the current status of the DRM transmission. A green "
		"box shows that the transmission takes place right now, a "
		"yellow cube shows that this is a test transmission and with a "
		"red cube it is shown that the transmission is offline.<br>"
		"If the stations preview is active an orange box shows the stations "
		"that will be active.<br>"
		"The list can be sorted by clicking on the headline of the "
		"column.<br>By clicking on a menu item, a remote front-end can "
		"be automatically switched to the current frequency and the "
		"Dream software is reset to a new acquisition (to speed up the "
		"synchronization process). Also, the log-file frequency edit "
		"is automatically updated."));

	/* Frequency Counter */
	QWhatsThis::add(QwtCounterFrequency,
		tr("<b>Frequency Counter:</b> The current frequency "
		"value can be changed by using this counter. The tuning steps are "
		"100 kHz for the  buttons with three arrows, 10 kHz for the "
		"buttons with two arrows and 1 kHz for the buttons having only "
		"one arrow. By keeping the button pressed, the values are "
		"increased / decreased automatically."));

	/* UTC time label */
	QWhatsThis::add(TextLabelUTCTime,
		tr("<b>UTC Time:</b> Shows the current Coordinated "
		"Universal Time (UTC) which is also known as Greenwich Mean Time "
		"(GMT)."));

#ifdef HAVE_LIBHAMLIB
	/* S-meter */
	const QString strSMeter =
		tr("<b>Signal-Meter:</b> Shows the signal strength "
		"level in dB relative to S9.<br>Note that not all "
		"front-ends controlled by hamlib support this feature. If the s-meter "
		"is not available, the controls are disabled.");

	QWhatsThis::add(TextLabelSMeter, strSMeter);
	QWhatsThis::add(ProgrSigStrength, strSMeter);
#endif
}
