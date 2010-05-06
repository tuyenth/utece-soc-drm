/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	
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

#include "fdrmdialog.h"


/* Implementation *************************************************************/
FDRMDialog::FDRMDialog(CDRMReceiver* pNDRMR, QWidget* parent, const char* name,
	bool modal, WFlags f) : pDRMRec(pNDRMR),
	FDRMDialogBase(parent, name, modal, f)
{
	/* Set help text for the controls */
	AddWhatsThisHelp();

#ifdef _WIN32 /* This works only reliable under Windows :-( */
	/* Get window geometry data from DRMReceiver module and apply it */
	const QRect WinGeom(pDRMRec->GeomFdrmdialog.iXPos,
		pDRMRec->GeomFdrmdialog.iYPos,
		pDRMRec->GeomFdrmdialog.iWSize,
		pDRMRec->GeomFdrmdialog.iHSize);

	if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
		setGeometry(WinGeom);
#else /* Under Linux only restore the size */
	resize(pDRMRec->GeomFdrmdialog.iWSize,
		pDRMRec->GeomFdrmdialog.iHSize);
#endif


	/* Set Menu ***************************************************************/
	/* View menu ------------------------------------------------------------ */
	QPopupMenu* EvalWinMenu = new QPopupMenu(this);
	CHECK_PTR(EvalWinMenu);
	EvalWinMenu->insertItem(tr("&Evaluation Dialog..."), this,
		SLOT(OnViewEvalDlg()), CTRL+Key_E);
	EvalWinMenu->insertItem(tr("M&ultimedia Dialog..."), this,
		SLOT(OnViewMultiMediaDlg()), CTRL+Key_U);
	EvalWinMenu->insertItem(tr("S&tations Dialog..."), this,
		SLOT(OnViewStationsDlg()), CTRL+Key_T);
	EvalWinMenu->insertSeparator();
	EvalWinMenu->insertItem(tr("E&xit"), this, SLOT(close()), CTRL+Key_Q);


	/* Settings menu  ------------------------------------------------------- */
	pSettingsMenu = new QPopupMenu(this);
	CHECK_PTR(pSettingsMenu);
	pSettingsMenu->insertItem(tr("&Sound Card Selection"),
		new CSoundCardSelMenu(pDRMRec->GetSoundInterface(), this));
	pSettingsMenu->insertItem(tr("&AM (analog)"), this,
		SLOT(OnSwitchToAM()), CTRL+Key_A);
	pSettingsMenu->insertItem(tr("New &DRM Acquisition"), this,
		SLOT(OnSwitchToDRM()), CTRL+Key_D);


	/* Main menu bar -------------------------------------------------------- */
	pMenu = new QMenuBar(this);
	CHECK_PTR(pMenu);
	pMenu->insertItem(tr("&View"), EvalWinMenu);
	pMenu->insertItem(tr("&Settings"), pSettingsMenu);
	pMenu->insertItem(tr("&?"), new CDreamHelpMenu(this));
	pMenu->setSeparator(QMenuBar::InWindowsStyle);

	/* Now tell the layout about the menu */
	FDRMDialogBaseLayout->setMenuBar(pMenu);


	/* Digi controls */
	/* Reset text */
	TextServiceIDRate->setText("");
	TextServiceLabel->setText("");
	TextServiceAudio->setText("");
	

	/* Init progress bar for input signal level */
	ProgrInputLevel->setRange(-50.0, 0.0);
	ProgrInputLevel->setOrientation(QwtThermo::Vertical, QwtThermo::Left);
	ProgrInputLevel->setFillColor(QColor(0, 190, 0));
	ProgrInputLevel->setAlarmLevel(-12.5);
	ProgrInputLevel->setAlarmColor(QColor(255, 0, 0));

	/* Stations window */
	pStationsDlg = new StationsDlg(pDRMRec, this, tr("Stations"), FALSE,
		Qt::WStyle_MinMax);
	if (pDRMRec->GeomStationsDlg.bVisible == TRUE)
		pStationsDlg->show();
	else
		pStationsDlg->hide();

	/* Evaluation window */
	pSysEvalDlg = new systemevalDlg(pDRMRec, this, tr("System Evaluation"),
		FALSE, Qt::WStyle_MinMax);

	if (pDRMRec->GeomSystemEvalDlg.bVisible == TRUE)
	{
		pSysEvalDlg->show();
		bSysEvalDlgWasVis = TRUE;
	}
	else
	{
		pSysEvalDlg->hide();
		bSysEvalDlgWasVis = FALSE;
	}

	/* Multimedia window */
	pMultiMediaDlg = new MultimediaDlg(pDRMRec, this, tr("Multimedia"), FALSE,
		Qt::WStyle_MinMax);

	if (pDRMRec->GeomMultimediaDlg.bVisible == TRUE)
	{
		pMultiMediaDlg->show();
		bMultMedDlgWasVis = TRUE;
	}
	else
	{
		pMultiMediaDlg->hide();
		bMultMedDlgWasVis = FALSE;
	}

	/* Analog demodulation window */
	pAnalogDemDlg = new AnalogDemDlg(pDRMRec, NULL, tr("Analog Demodulation"),
		FALSE, Qt::WStyle_MinMax);

	if (pDRMRec->GeomAnalogDemDlg.bVisible == TRUE)
		SetReceiverMode(CDRMReceiver::RM_AM);
	else
		SetReceiverMode(CDRMReceiver::RM_DRM);

	/* Enable multimedia */
	pDRMRec->GetParameters()->EnableMultimedia(TRUE);

	/* Init current selected service */
	pDRMRec->GetParameters()->ResetCurSelAudDatServ();
	iCurSelServiceGUI = 0;
	iOldNoServicesGUI = 0;

	PushButtonService1->setOn(TRUE);
	PushButtonService1->setEnabled(FALSE);
	PushButtonService2->setEnabled(FALSE);
	PushButtonService3->setEnabled(FALSE);
	PushButtonService4->setEnabled(FALSE);

	/* Update times for color LEDs */
	CLED_FAC->SetUpdateTime(1500);
	CLED_SDC->SetUpdateTime(1500);
	CLED_MSC->SetUpdateTime(600);

	/* Connect buttons */
	connect(PushButtonService1, SIGNAL(clicked()),
		this, SLOT(OnButtonService1()));
	connect(PushButtonService2, SIGNAL(clicked()),
		this, SLOT(OnButtonService2()));
	connect(PushButtonService3, SIGNAL(clicked()),
		this, SLOT(OnButtonService3()));
	connect(PushButtonService4, SIGNAL(clicked()),
		this, SLOT(OnButtonService4()));

	connect(pAnalogDemDlg, SIGNAL(SwitchToDRM()),
		this, SLOT(OnSwitchToDRM()));
	connect(pAnalogDemDlg, SIGNAL(ViewStationsDlg()),
		this, SLOT(OnViewStationsDlg()));
	connect(pAnalogDemDlg, SIGNAL(Closed()),
		this, SLOT(close()));

	connect(&Timer, SIGNAL(timeout()),
		this, SLOT(OnTimer()));

	/* Disable text message label */
	TextTextMessage->setText("");
	TextTextMessage->hide();

	/* Set timer for real-time controls */
	Timer.start(GUI_CONTROL_UPDATE_TIME);

	/* Update window */
	OnTimer();
}

FDRMDialog::~FDRMDialog()
{
	/* Set window geometry data in DRMReceiver module */
	QRect WinGeom = geometry();

	pDRMRec->GeomFdrmdialog.iXPos = WinGeom.x();
	pDRMRec->GeomFdrmdialog.iYPos = WinGeom.y();
	pDRMRec->GeomFdrmdialog.iHSize = WinGeom.height();
	pDRMRec->GeomFdrmdialog.iWSize = WinGeom.width();

	/* Set "visible" flags for settings */
	pDRMRec->GeomAnalogDemDlg.bVisible = pAnalogDemDlg->isVisible();
	pDRMRec->GeomStationsDlg.bVisible = pStationsDlg->isVisible();

	/* Special treatment for multimedia and systen evaluation dialog since these
	   windows are not used for AM demodulation */
	if (pDRMRec->GetReceiverMode() == CDRMReceiver::RM_AM)
	{
		pDRMRec->GeomSystemEvalDlg.bVisible = bSysEvalDlgWasVis;
		pDRMRec->GeomMultimediaDlg.bVisible = bMultMedDlgWasVis;
	}
	else
	{
		pDRMRec->GeomSystemEvalDlg.bVisible = pSysEvalDlg->isVisible();
		pDRMRec->GeomMultimediaDlg.bVisible = pMultiMediaDlg->isVisible();
	}
}

void FDRMDialog::OnTimer()
{
	/* Input level meter */
	ProgrInputLevel->setValue(pDRMRec->GetReceiver()->GetLevelMeter());

	/* Check if receiver does receive a DRM signal */
	if ((pDRMRec->GetReceiverState() == CDRMReceiver::AS_WITH_SIGNAL) &&
		(pDRMRec->GetReceiverMode() == CDRMReceiver::RM_DRM))
	{
		/* Receiver does receive a DRM signal ------------------------------- */
		/* First get current selected services */
		const int iCurSelAudioServ =
			pDRMRec->GetParameters()->GetCurSelAudioService();
		const int iCurSelDataServ =
			pDRMRec->GetParameters()->GetCurSelDataService();

		/* If selected service is audio and text message is true */
		if ((pDRMRec->GetParameters()->Service[iCurSelAudioServ].
			eAudDataFlag == CParameter::SF_AUDIO) &&
			(pDRMRec->GetParameters()->Service[iCurSelAudioServ].
			AudioParam.bTextflag == TRUE))
		{
			/* Activate text window */
			TextTextMessage->show();

			/* Text message of current selected audio service 
			   (UTF-8 decoding) */
			TextTextMessage->setText(QString().fromUtf8(QCString(
				pDRMRec->GetParameters()->Service[iCurSelAudioServ].AudioParam.
				strTextMessage.c_str())));
		}
		else
		{
			/* Deactivate text window */
			TextTextMessage->hide();

			/* Clear Text */
			TextTextMessage->setText("");
		}

		/* Check whether service parameters were not transmitted yet */
		if (pDRMRec->GetParameters()->Service[iCurSelAudioServ].IsActive())
		{
			/* Service label (UTF-8 encoded string -> convert) */
			TextServiceLabel->setText(QString().fromUtf8(QCString(
				pDRMRec->GetParameters()->Service[iCurSelAudioServ].
				strLabel.c_str())));

			TextServiceIDRate->setText(SetBitrIDStr(iCurSelAudioServ));

			/* Audio informations digi-string */
			TextServiceAudio->setText(SetServParamStr(iCurSelAudioServ));
		}
		else
		{
			TextServiceLabel->setText(tr("No Service"));

			TextServiceIDRate->setText("");
			TextServiceAudio->setText("");
		}


		/* Update service selector ------------------------------------------ */
		/* Make sure a possible service was selected. If not, correct */
		if ((!pDRMRec->GetParameters()->Service[iCurSelServiceGUI].IsActive()) ||
			(!((iCurSelServiceGUI == iCurSelAudioServ) ||
			(iCurSelServiceGUI == iCurSelDataServ))))
		{
			/* Reset checks */
			PushButtonService1->setOn(FALSE);
			PushButtonService2->setOn(FALSE);
			PushButtonService3->setOn(FALSE);
			PushButtonService4->setOn(FALSE);

			/* Set right flag */
			switch (iCurSelAudioServ)
			{
			case 0:
				PushButtonService1->setOn(TRUE);
				iCurSelServiceGUI = 0;
				break;

			case 1:
				PushButtonService2->setOn(TRUE);
				iCurSelServiceGUI = 1;
				break;

			case 2:
				PushButtonService3->setOn(TRUE);
				iCurSelServiceGUI = 2;
				break;

			case 3:
				PushButtonService4->setOn(TRUE);
				iCurSelServiceGUI = 3;
				break;
			}
		}


		/* Service selector ------------------------------------------------- */
		QString strSpace = "   |   ";

		/* Enable only so many number of channel switches as present in the
		   stream */
		const int iNumServices = pDRMRec->GetParameters()->GetTotNumServices();

		QString m_StaticService[MAX_NUM_SERVICES] = {"", "", "", ""};

		/* Reset all buttons only if number of services has changed */
		if (iOldNoServicesGUI != iNumServices)
		{
			PushButtonService1->setEnabled(FALSE);
			PushButtonService2->setEnabled(FALSE);
			PushButtonService3->setEnabled(FALSE);
			PushButtonService4->setEnabled(FALSE);
		}
		iOldNoServicesGUI = iNumServices;

		for (int i = 0; i < MAX_NUM_SERVICES; i++)
		{
			/* Check, if service is used */
			if (pDRMRec->GetParameters()->Service[i].IsActive())
			{
				/* Do UTF-8 to string converion with the label strings */
				QString strLabel = QString().fromUtf8(QCString(pDRMRec->
					GetParameters()->Service[i].strLabel.c_str()));

				/* Print out label in bold letters (rich text). Problem, if 
				   html tags are used in the label: FIXME */
				m_StaticService[i] = "<b>" + strLabel + 
					"</b>" + strSpace + SetServParamStr(i);

				/* Bit-rate (only show if greater than 0) */
				const _REAL rBitRate =
					pDRMRec->GetParameters()->GetBitRateKbps(i, FALSE);

				if (rBitRate > (_REAL) 0.0)
				{
					m_StaticService[i] += " (" +
						QString().setNum(rBitRate, 'f', 2) + " kbps)";
				}

				/* Show, if a multimedia stream is connected to this service */
				if ((pDRMRec->GetParameters()->Service[i].
					eAudDataFlag == CParameter::SF_AUDIO) && 
					(pDRMRec->GetParameters()->Service[i].
					DataParam.iStreamID != STREAM_ID_NOT_USED))
				{
					m_StaticService[i] += tr(" + MM");

					/* Bit-rate of connected data stream */
					m_StaticService[i] += " (" + QString().setNum(pDRMRec->
						GetParameters()->GetBitRateKbps(i, TRUE), 'f', 2) +
						" kbps)";
				}

				switch (i)
				{
				case 0:
					PushButtonService1->setEnabled(TRUE);
					break;

				case 1:
					PushButtonService2->setEnabled(TRUE);
					break;

				case 2:
					PushButtonService3->setEnabled(TRUE);
					break;

				case 3:
					PushButtonService4->setEnabled(TRUE);
					break;
				}
			}
		}

		/* Set texts */
		TextMiniService1->setText(m_StaticService[0]);
		TextMiniService2->setText(m_StaticService[1]);
		TextMiniService3->setText(m_StaticService[2]);
		TextMiniService4->setText(m_StaticService[3]);
	}
	else
	{
		/* No signal is currently received ---------------------------------- */
		/* Disable service buttons and associated labels */
		PushButtonService1->setEnabled(FALSE);
		PushButtonService2->setEnabled(FALSE);
		PushButtonService3->setEnabled(FALSE);
		PushButtonService4->setEnabled(FALSE);
		TextMiniService1->setText("");
		TextMiniService2->setText("");
		TextMiniService3->setText("");
		TextMiniService4->setText("");

		/* Main text labels */
		TextServiceAudio->setText("");
		TextServiceIDRate->setText("");

		/* Hide text message label */
		TextTextMessage->hide();
		TextTextMessage->setText("");

		TextServiceLabel->setText(tr("Scanning..."));
	}
}

void FDRMDialog::SetReceiverMode(const CDRMReceiver::ERecMode eNewReMo)
{
	const _BOOLEAN bModeHastChanged = pDRMRec->GetReceiverMode() != eNewReMo;

	/* Set mode in receiver object */
	pDRMRec->SetReceiverMode(eNewReMo);

	/* Make sure correct evaluation dialog is shown */
	switch (eNewReMo)
	{
	case CDRMReceiver::RM_DRM:
		/* For DRM mode, always show main window */
		show();

		pAnalogDemDlg->hide();

		/* Recover visibility state (only if mode has changed) */
		if (bModeHastChanged)
		{
			if (bSysEvalDlgWasVis == TRUE)
				pSysEvalDlg->show();

			if (bMultMedDlgWasVis == TRUE)
				pMultiMediaDlg->show();
		}

		/* Load correct schedule */
		pStationsDlg->LoadSchedule(CDRMSchedule::SM_DRM);
		break;

	case CDRMReceiver::RM_AM:
		/* Main window is not needed, hide it. If Multimedia window was open,
		   hide it. Make sure analog demodulation dialog is visible */
		hide();

		/* Store visibility state */
		bSysEvalDlgWasVis = pSysEvalDlg->isVisible();
		bMultMedDlgWasVis = pMultiMediaDlg->isVisible();

		pSysEvalDlg->hide();
		pMultiMediaDlg->hide();

		pAnalogDemDlg->show();

		/* Load correct schedule */
		pStationsDlg->LoadSchedule(CDRMSchedule::SM_ANALOG);
		break;
	}
}

void FDRMDialog::OnButtonService1()
{
	if (PushButtonService1->isOn())
	{
		/* Set all other buttons up */
		if (PushButtonService2->isOn()) PushButtonService2->setOn(FALSE);
		if (PushButtonService3->isOn()) PushButtonService3->setOn(FALSE);
		if (PushButtonService4->isOn()) PushButtonService4->setOn(FALSE);

		SetService(0);
	}
	else
		PushButtonService1->setOn(TRUE);
}

void FDRMDialog::OnButtonService2()
{
	if (PushButtonService2->isOn())
	{
		/* Set all other buttons up */
		if (PushButtonService1->isOn()) PushButtonService1->setOn(FALSE);
		if (PushButtonService3->isOn()) PushButtonService3->setOn(FALSE);
		if (PushButtonService4->isOn()) PushButtonService4->setOn(FALSE);

		SetService(1);
	}
	else
		PushButtonService2->setOn(TRUE);

}

void FDRMDialog::OnButtonService3()
{
	if (PushButtonService3->isOn())
	{
		/* Set all other buttons up */
		if (PushButtonService1->isOn()) PushButtonService1->setOn(FALSE);
		if (PushButtonService2->isOn()) PushButtonService2->setOn(FALSE);
		if (PushButtonService4->isOn()) PushButtonService4->setOn(FALSE);

		SetService(2);
	}
	else
		PushButtonService3->setOn(TRUE);
}

void FDRMDialog::OnButtonService4()
{
	if (PushButtonService4->isOn())
	{
		/* Set all other buttons up */
		if (PushButtonService1->isOn()) PushButtonService1->setOn(FALSE);
		if (PushButtonService2->isOn()) PushButtonService2->setOn(FALSE);
		if (PushButtonService3->isOn()) PushButtonService3->setOn(FALSE);

		SetService(3);
	}
	else
		PushButtonService4->setOn(TRUE);
}

void FDRMDialog::SetService(int iNewServiceID)
{
	pDRMRec->GetParameters()->SetCurSelAudioService(iNewServiceID);
	pDRMRec->GetParameters()->SetCurSelDataService(iNewServiceID);
	iCurSelServiceGUI = iNewServiceID;

	/* If service is only data service, activate multimedia window */
	if (pDRMRec->GetParameters()->Service[iNewServiceID].eAudDataFlag ==
		CParameter::SF_DATA)
	{
		OnViewMultiMediaDlg();
	}
}

void FDRMDialog::OnViewEvalDlg()
{
	if (pDRMRec->GetReceiverMode() == CDRMReceiver::RM_DRM)
	{
		/* Show evauation window in DRM mode */
		pSysEvalDlg->show();
	}
	else
	{
		/* Show AM demodulation window in AM mode */
		pAnalogDemDlg->show();
	}
}

void FDRMDialog::OnViewMultiMediaDlg()
{
	/* Show evaluation window */
	pMultiMediaDlg->show();
}

void FDRMDialog::OnViewStationsDlg()
{
	/* Show evauation window */
	pStationsDlg->show();
}

QString	FDRMDialog::SetServParamStr(int iServiceID)
{
	QString strReturn;

	if (pDRMRec->GetParameters()->Service[iServiceID].
		eAudDataFlag == CParameter::SF_AUDIO)
	{
		/* Audio service ---------------------------------------------------- */
		/* Audio coding */
		switch (pDRMRec->GetParameters()->Service[iServiceID].
			AudioParam.eAudioCoding)
		{
		case CParameter::AC_AAC:	
			strReturn = "AAC(";
			break;

		case CParameter::AC_CELP:
			strReturn = "Celp(";
			break;

		case CParameter::AC_HVXC:
			strReturn = "HVXC(";
			break;
		}

		/* Sample rate */
		switch (pDRMRec->GetParameters()->Service[iServiceID].
			AudioParam.eAudioSamplRate)
		{
		case CParameter::AS_8_KHZ:	
			strReturn += "8 kHz)";
			break;

		case CParameter::AS_12KHZ:	
			strReturn += "12 kHz)";
			break;

		case CParameter::AS_16KHZ:	
			strReturn += "16 kHz)";
			break;

		case CParameter::AS_24KHZ:	
			strReturn += "24 kHz)";
			break;
		}

		/* SBR */
		if (pDRMRec->GetParameters()->Service[iServiceID].
			AudioParam.eSBRFlag == CParameter::SB_USED)
		{
			strReturn += "+SBR";
		}

		/* Mono-Stereo */
		switch (pDRMRec->GetParameters()->
			Service[iServiceID].AudioParam.eAudioMode)
		{
			case CParameter::AM_MONO:
				strReturn += " Mono";
				break;

			case CParameter::AM_P_STEREO:
				strReturn += " P-Stereo";
				break;

			case CParameter::AM_STEREO:
				strReturn += " Stereo";
				break;
		}

		/* Language */
		strReturn += " / ";
		strReturn += strTableLanguageCode[pDRMRec->GetParameters()->
			Service[iServiceID].iLanguage].c_str();

		/* Program type */
		strReturn += " / ";
		strReturn += strTableProgTypCod[pDRMRec->GetParameters()->
			Service[iServiceID].iServiceDescr].c_str();
	}
	else
	{
		/* Data service ----------------------------------------------------- */
		strReturn = "Data Service: ";

		if (pDRMRec->GetParameters()->Service[iServiceID].DataParam.
			ePacketModInd == CParameter::PM_PACKET_MODE)
		{
			if (pDRMRec->GetParameters()->Service[iServiceID].DataParam.
				eAppDomain == CParameter::AD_DAB_SPEC_APP)
			{
				switch (pDRMRec->GetParameters()->Service[iServiceID].
					DataParam.iUserAppIdent)
				{
				case 1:
					strReturn += "Dynamic labels";
					break;

				case 2:
					strReturn += "MOT Slideshow";
					break;

				case 3:
					strReturn += "MOT Broadcast Web Site";
					break;

				case 4:
					strReturn += "TPEG";
					break;

				case 5:
					strReturn += "DGPS";
					break;

				case 0x44A: /* Journaline */
					strReturn += "NewsService Journaline";
					break;
				}
			}
			else
				strReturn += tr("Unknown Service");
		}
		else
			strReturn += tr("Unknown Service");
	}

	return strReturn;
}

QString	FDRMDialog::SetBitrIDStr(int iServiceID)
{
	/* Bit-rate */
	QString strServIDBitrate = tr("Bit Rate:") + QString().setNum(pDRMRec->
		GetParameters()->GetBitRateKbps(iServiceID, FALSE), 'f', 2) +
		tr(" kbps");

	/* Equal or unequal error protection */
	const _REAL rPartABLenRat =
		pDRMRec->GetParameters()->PartABLenRatio(iServiceID);

	if (rPartABLenRat != (_REAL) 0.0)
	{
		/* Print out the percentage of part A length to total length */
		strServIDBitrate += " UEP (" +
			QString().setNum(rPartABLenRat * 100, 'f', 1) + " %)";
	}
	else
	{
		/* If part A is zero, equal error protection (EEP) is used */
		strServIDBitrate += " EEP";
	}

	/* Service ID */
	strServIDBitrate += " / ID:";
	strServIDBitrate += QString().setNum((long) pDRMRec->GetParameters()->
		Service[iServiceID].iServiceID);

	return strServIDBitrate;
}

void FDRMDialog::customEvent(QCustomEvent* Event)
{
	if (Event->type() == QEvent::User + 11)
	{
		int iMessType = ((DRMEvent*) Event)->iMessType;
		int iStatus = ((DRMEvent*) Event)->iStatus;

		if (iMessType == MS_MOT_OBJ_STAT)
			pMultiMediaDlg->SetStatus(iMessType, iStatus);
		else
		{
			pSysEvalDlg->SetStatus(iMessType, iStatus);

			switch(iMessType)
			{
			case MS_FAC_CRC:
				CLED_FAC->SetLight(iStatus);
				break;

			case MS_SDC_CRC:
				CLED_SDC->SetLight(iStatus);
				break;

			case MS_MSC_CRC:
				CLED_MSC->SetLight(iStatus);
				break;

			case MS_RESET_ALL:
				CLED_FAC->Reset();
				CLED_SDC->Reset();
				CLED_MSC->Reset();
				break;
			}
		}
	}
}

void FDRMDialog::AddWhatsThisHelp()
{
/*
	This text was taken from the only documentation of Dream software
*/
	/* Text Message */
	QWhatsThis::add(TextTextMessage,
		tr("<b>Text Message:</b> On the top right the text "
		"message label is shown. This label only appears when an actual text "
		"message is transmitted. If the current service does not transmit a "
		"text message, the label will be invisible."));

	/* Input Level */
	const QString strInputLevel =
		tr("<b>Input Level:</b> The input level meter shows "
		"the relative input signal peak level in dB. If the level is too high, "
		"the meter turns from green to red. The red region should be avoided "
		"since overload causes distortions which degrade the reception "
		"performance. Too low levels should be avoided too, since in this case "
		"the Signal-to-Noise Ratio (SNR) degrades.");

	QWhatsThis::add(TextLabelInputLevel, strInputLevel);
	QWhatsThis::add(ProgrInputLevel, strInputLevel);

	/* Status LEDs */
	const QString strStatusLEDS =
		tr("<b>Status LEDs:</b> The three status LEDs show "
		"the current CRC status of the three logical channels of a DRM stream. "
		"These LEDs are the same as the top LEDs on the Evaluation Dialog.");

	QWhatsThis::add(TextLabelStatusLEDs, strStatusLEDS);
	QWhatsThis::add(CLED_MSC, strStatusLEDS);
	QWhatsThis::add(CLED_SDC, strStatusLEDS);
	QWhatsThis::add(CLED_FAC, strStatusLEDS);

	/* Station Label and Info Display */
	const QString strStationLabelOther =
		tr("<b>Station Label and Info Display:</b> In the "
		"big label with the black background the station label and some other "
		"information about the current selected service is displayed. The red "
		"text on the top shows the audio compression format (e.g. AAC), the "
		"sample rate of the core coder without SBR (e.g. 24 kHz), if SBR is "
		"used and what audio mode is used (mono, stereo, P-stereo -> "
		"low-complexity or parametric stereo). In case SBR is used, the actual "
		"sample rate is twice the sample rate of the core AAC decoder. The "
		"next two types of information are the language and the program type "
		"of the service (e.g. German / News).<br>The big "
		"turquoise text in the middle is the station label. This label may "
		"appear later than the red text since this information is transmitted "
		"in a different logical channel of a DRM stream.<br>The "
		"turquoise text on the bottom shows the gross bit-rate in kbits per "
		"second of the current selected service. The abbreviations EEP and "
		"UEP stand for Equal Error Protection and Unequal Error Protection. "
		"UEP is a feature of DRM for a graceful degradation of the decoded "
		"audio signal in case of a bad reception situation. UEP means that "
		"some parts of the audio is higher protected and some parts are lower "
		"protected (the ratio of higher protected part length to total length "
		"is shown in the brackets). On the right, the ID number connected with "
		"this service is shown.");

	QWhatsThis::add(TextServiceAudio, strStationLabelOther);
	QWhatsThis::add(TextServiceLabel, strStationLabelOther);
	QWhatsThis::add(TextServiceIDRate, strStationLabelOther);

	/* Service Selectors */
	const QString strServiceSel =
		tr("<b>Service Selectors:</b> In a DRM stream up to "
		"four services can be carried. The service type can either be audio, "
		"data or audio and data. If a data service is selected, the Multimedia "
		"Dialog will automatically show up. On the right of each service "
		"selection button a short description of the service is shown. If a "
		"service is an audio and data service, a \"+ MM\" is added to this "
		"text. If a service is an audio and data service and this service "
		"is selected, by opening the Multimedia Dialog, the data can be viewed "
		"while the audio is still playing.");

	QWhatsThis::add(PushButtonService1, strServiceSel);
	QWhatsThis::add(PushButtonService2, strServiceSel);
	QWhatsThis::add(PushButtonService3, strServiceSel);
	QWhatsThis::add(PushButtonService4, strServiceSel);
	QWhatsThis::add(TextMiniService1, strServiceSel);
	QWhatsThis::add(TextMiniService2, strServiceSel);
	QWhatsThis::add(TextMiniService3, strServiceSel);
	QWhatsThis::add(TextMiniService4, strServiceSel);

	/* Dream Logo */
	QWhatsThis::add(PixmapLabelDreamLogo,
		tr("<b>Dream Logo:</b> This is the official logo "
		"of the Dream software."));
}
