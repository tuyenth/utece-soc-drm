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

#include "TransmDlg.h"


TransmDialog::TransmDialog(QWidget* parent, const char* name, bool modal,
						   WFlags f)
	: TransmDlgBase(parent, name, modal, f), bIsStarted(FALSE),
	vecstrTextMessage(1) /* 1 for new text */, iIDCurrentText(0)
{
	int i;

	/* Set help text for the controls */
	AddWhatsThisHelp();

	/* Set controls to custom behavior */
	MultiLineEditTextMessage->setWordWrap(QMultiLineEdit::WidgetWidth);
	MultiLineEditTextMessage->setEdited(FALSE);
	ComboBoxTextMessage->insertItem(tr("new"), 0);
	UpdateMSCProtLevCombo();

	/* Init controls with default settings */
	ButtonStartStop->setText(tr("&Start"));

	/* Init progress bar for input signal level */
	ProgrInputLevel->setRange(-50.0, 0.0);
	ProgrInputLevel->setOrientation(QwtThermo::Horizontal, QwtThermo::Bottom);
	ProgrInputLevel->setFillColor(QColor(0, 190, 0));
	ProgrInputLevel->setAlarmLevel(-5.0);
	ProgrInputLevel->setAlarmColor(QColor(255, 0, 0));

	/* Init progress bar for current transmitted picture */
	ProgressBarCurPict->setTotalSteps(100);
	ProgressBarCurPict->setProgress(0);
	TextLabelCurPict->setText("");

	/* Output mode (real valued, I / Q or E / P) */
	switch (TransThread.DRMTransmitter.GetTransData()->GetIQOutput())
	{
	case CTransmitData::OF_REAL_VAL:
		RadioButtonOutReal->setChecked(TRUE);
		break;

	case CTransmitData::OF_IQ_POS:
		RadioButtonOutIQPos->setChecked(TRUE);
		break;

	case CTransmitData::OF_IQ_NEG:
		RadioButtonOutIQNeg->setChecked(TRUE);
		break;

	case CTransmitData::OF_EP:
		RadioButtonOutEP->setChecked(TRUE);
		break;
	}

	/* Robustness mode */
	switch (TransThread.DRMTransmitter.GetParameters()->GetWaveMode())
	{
	case RM_ROBUSTNESS_MODE_A:
		RadioButtonRMA->setChecked(TRUE);
		break;

	case RM_ROBUSTNESS_MODE_B:
		RadioButtonRMB->setChecked(TRUE);
		break;

	case RM_ROBUSTNESS_MODE_C:
		RadioButtonRMC->setChecked(TRUE);
		break;

	case RM_ROBUSTNESS_MODE_D:
		RadioButtonRMD->setChecked(TRUE);
		break;
	}

	/* Bandwidth */
	switch (TransThread.DRMTransmitter.GetParameters()->GetSpectrumOccup())
	{
	case SO_0:
		RadioButtonBandwidth45->setChecked(TRUE);
		break;

	case SO_1:
		RadioButtonBandwidth5->setChecked(TRUE);
		break;

	case SO_2:
		RadioButtonBandwidth9->setChecked(TRUE);
		break;

	case SO_3:
		RadioButtonBandwidth10->setChecked(TRUE);
		break;

	case SO_4:
		RadioButtonBandwidth18->setChecked(TRUE);
		break;

	case SO_5:
		RadioButtonBandwidth20->setChecked(TRUE);
		break;
	}

	/* Set button group IDs */
	ButtonGroupBandwidth->insert(RadioButtonBandwidth45, 0);
	ButtonGroupBandwidth->insert(RadioButtonBandwidth5, 1);
	ButtonGroupBandwidth->insert(RadioButtonBandwidth9, 2);
	ButtonGroupBandwidth->insert(RadioButtonBandwidth10, 3);
	ButtonGroupBandwidth->insert(RadioButtonBandwidth18, 4);
	ButtonGroupBandwidth->insert(RadioButtonBandwidth20, 5);

	/* MSC interleaver mode */
	ComboBoxMSCInterleaver->insertItem(tr("2 s (Long Interleaving)"), 0);
	ComboBoxMSCInterleaver->insertItem(tr("400 ms (Short Interleaving)"), 1);

	switch (TransThread.DRMTransmitter.GetParameters()->eSymbolInterlMode)
	{
	case CParameter::SI_LONG:
		ComboBoxMSCInterleaver->setCurrentItem(0);
		break;

	case CParameter::SI_SHORT:
		ComboBoxMSCInterleaver->setCurrentItem(1);
		break;
	}

	/* MSC Constellation Scheme */
	ComboBoxMSCConstellation->insertItem(tr("SM 16-QAM"), 0);
	ComboBoxMSCConstellation->insertItem(tr("SM 64-QAM"), 1);

// These modes should not be used right now, TODO
//	ComboBoxMSCConstellation->insertItem(tr("HMsym 64-QAM"), 2);
//	ComboBoxMSCConstellation->insertItem(tr("HMmix 64-QAM"), 3);

	switch (TransThread.DRMTransmitter.GetParameters()->eMSCCodingScheme)
	{
	case CParameter::CS_2_SM:
		ComboBoxMSCConstellation->setCurrentItem(0);
		break;

	case CParameter::CS_3_SM:
		ComboBoxMSCConstellation->setCurrentItem(1);
		break;

//	case CParameter::CS_3_HMSYM:
//		ComboBoxMSCConstellation->setCurrentItem(2);
//		break;
//
//	case CParameter::CS_3_HMMIX:
//		ComboBoxMSCConstellation->setCurrentItem(3);
//		break;
	}

	/* SDC Constellation Scheme */
	ComboBoxSDCConstellation->insertItem(tr("4-QAM"), 0);
	ComboBoxSDCConstellation->insertItem(tr("16-QAM"), 1);

	switch (TransThread.DRMTransmitter.GetParameters()->eSDCCodingScheme)
	{
	case CParameter::CS_1_SM:
		ComboBoxSDCConstellation->setCurrentItem(0);
		break;

	case CParameter::CS_2_SM:
		ComboBoxSDCConstellation->setCurrentItem(1);
		break;
	}

	/* Service parameters --------------------------------------------------- */
	/* Service label */
	LineEditServiceLabel->setText(TransThread.DRMTransmitter.
		GetParameters()->Service[0].strLabel.c_str());

	/* Service ID */
	LineEditServiceID->setText(QString().setNum((int) TransThread.
		DRMTransmitter.GetParameters()->Service[0].iServiceID));

	/* Language */
	for (i = 0; i < LEN_TABLE_LANGUAGE_CODE; i++)
		ComboBoxLanguage->insertItem(strTableLanguageCode[i].c_str(), i);

	ComboBoxLanguage->setCurrentItem(TransThread.DRMTransmitter.
		GetParameters()->Service[0].iLanguage);

	/* Program type */
	for (i = 0; i < LEN_TABLE_PROG_TYPE_CODE; i++)
		ComboBoxProgramType->insertItem(strTableProgTypCod[i].c_str(), i);

	ComboBoxProgramType->setCurrentItem(TransThread.DRMTransmitter.
		GetParameters()->Service[0].iServiceDescr);

	/* Sound card IF */
	LineEditSndCrdIF->setText(QString().number(
		TransThread.DRMTransmitter.GetCarOffset(), 'f', 2));

	/* Clear list box for file names and set up columns */
	ListViewFileNames->clear();

	/* We assume that one column is already there */
	ListViewFileNames->setColumnText(0, "File Name");
	ListViewFileNames->addColumn("Size [KB]");
	ListViewFileNames->addColumn("Full Path");

	/* Disable other three services */
	TabWidgetServices->setTabEnabled(tabService2, FALSE);
	TabWidgetServices->setTabEnabled(tabService3, FALSE);
	TabWidgetServices->setTabEnabled(tabService4, FALSE);
	CheckBoxEnableService->setChecked(TRUE);
	CheckBoxEnableService->setEnabled(FALSE);


	/* Let this service be an audio service for initialization */
	/* Set audio enable check box */
	CheckBoxEnableAudio->setChecked(TRUE);
	EnableAudio(TRUE);
	CheckBoxEnableData->setChecked(FALSE);
	EnableData(FALSE);


	/* Add example text message at startup ---------------------------------- */
	/* Activate text message */
	EnableTextMessage(TRUE);
	CheckBoxEnableTextMessage->setChecked(TRUE);

	/* Add example text in internal container */
	vecstrTextMessage.Add(tr("Dream DRM Transmitter\x0B\x0AThis is a test "
		"transmission").latin1());

	/* Insert item in combo box, display text and set item to our text */
	ComboBoxTextMessage->insertItem(QString().setNum(1), 1);
	ComboBoxTextMessage->setCurrentItem(1);
	iIDCurrentText = 1;
	MultiLineEditTextMessage->insertLine(vecstrTextMessage[1].c_str());

	/* Now make sure that the text message flag is activated in global struct */
	TransThread.DRMTransmitter.GetParameters()->
		Service[0].AudioParam.bTextflag = TRUE;


	/* Enable all controls */
	EnableAllControlsForSet();


	/* Set Menu ***************************************************************/
	/* Settings menu  ------------------------------------------------------- */
	pSettingsMenu = new QPopupMenu(this);
	CHECK_PTR(pSettingsMenu);
	pSettingsMenu->insertItem(tr("&Sound Card Selection"),
		new CSoundCardSelMenu(TransThread.DRMTransmitter.
		GetSoundInterface(), this));

	/* Main menu bar */
	pMenu = new QMenuBar(this);
	CHECK_PTR(pMenu);
	pMenu->insertItem(tr("&Settings"), pSettingsMenu);
	pMenu->insertItem(tr("&?"), new CDreamHelpMenu(this));
	pMenu->setSeparator(QMenuBar::InWindowsStyle);

	/* Now tell the layout about the menu */
	TransmDlgBaseLayout->setMenuBar(pMenu);


	/* Connections ---------------------------------------------------------- */
	connect(ButtonStartStop, SIGNAL(clicked()),
		this, SLOT(OnButtonStartStop()));
	connect(PushButtonAddText, SIGNAL(clicked()),
		this, SLOT(OnPushButtonAddText()));
	connect(PushButtonClearAllText, SIGNAL(clicked()),
		this, SLOT(OnButtonClearAllText()));
	connect(PushButtonAddFile, SIGNAL(clicked()),
		this, SLOT(OnPushButtonAddFileName()));
	connect(PushButtonClearAllFileNames, SIGNAL(clicked()),
		this, SLOT(OnButtonClearAllFileNames()));
	connect(CheckBoxEnableTextMessage, SIGNAL(toggled(bool)),
		this, SLOT(OnToggleCheckBoxEnableTextMessage(bool)));
	connect(CheckBoxEnableAudio, SIGNAL(toggled(bool)),
		this, SLOT(OnToggleCheckBoxEnableAudio(bool)));
	connect(CheckBoxEnableData, SIGNAL(toggled(bool)),
		this, SLOT(OnToggleCheckBoxEnableData(bool)));

	/* Combo boxes */
	connect(ComboBoxMSCInterleaver, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxMSCInterleaverHighlighted(int)));
	connect(ComboBoxMSCConstellation, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxMSCConstellationHighlighted(int)));
	connect(ComboBoxSDCConstellation, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxSDCConstellationHighlighted(int)));
	connect(ComboBoxLanguage, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxLanguageHighlighted(int)));
	connect(ComboBoxProgramType, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxProgramTypeHighlighted(int)));
	connect(ComboBoxTextMessage, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxTextMessageHighlighted(int)));
	connect(ComboBoxMSCProtLev, SIGNAL(highlighted(int)),
		this, SLOT(OnComboBoxMSCProtLevHighlighted(int)));

	/* Button groups */
	connect(ButtonGroupRobustnessMode, SIGNAL(clicked(int)),
		this, SLOT(OnRadioRobustnessMode(int)));
	connect(ButtonGroupBandwidth, SIGNAL(clicked(int)),
		this, SLOT(OnRadioBandwidth(int)));
	connect(ButtonGroupBandwidth, SIGNAL(clicked(int)),
		this, SLOT(OnRadioBandwidth(int)));
	connect(ButtonGroupOutput, SIGNAL(clicked(int)),
		this, SLOT(OnRadioOutput(int)));

	/* Line edits */
	connect(LineEditServiceLabel, SIGNAL(textChanged(const QString&)),
		this, SLOT(OnTextChangedServiceLabel(const QString&)));
	connect(LineEditServiceID, SIGNAL(textChanged(const QString&)),
		this, SLOT(OnTextChangedServiceID(const QString&)));
	connect(LineEditSndCrdIF, SIGNAL(textChanged(const QString&)),
		this, SLOT(OnTextChangedSndCrdIF(const QString&)));

	connect(&Timer, SIGNAL(timeout()), 
		this, SLOT(OnTimer()));


	/* Set timer for real-time controls */
	Timer.start(GUI_CONTROL_UPDATE_TIME);
}

TransmDialog::~TransmDialog()
{
	/* Stop transmitter */
	if (bIsStarted == TRUE)
		TransThread.Stop();
}

void TransmDialog::OnTimer()
{
	/* Set value for input level meter (only in "start" mode) */
	if (bIsStarted == TRUE)
	{
		ProgrInputLevel->
			setValue(TransThread.DRMTransmitter.GetReadData()->GetLevelMeter());

		string strCPictureName;
		_REAL rCPercent;

		/* Activate progress bar for slide show pictures only if current state
		   can be queried and if data service is active
		   (check box is checked) */
		if ((TransThread.DRMTransmitter.GetAudSrcEnc()->
			GetTransStat(strCPictureName, rCPercent) ==	TRUE) &&
			(CheckBoxEnableData->isChecked()))
		{
			/* Enable controls */
			ProgressBarCurPict->setEnabled(TRUE);
			TextLabelCurPict->setEnabled(TRUE);

			/* We want to file name, not the complete path -> "QFileInfo" */
			QFileInfo FileInfo(strCPictureName.c_str());

			/* Show current file name and percentage */
			TextLabelCurPict->setText(FileInfo.fileName());
			ProgressBarCurPict->setProgress((int) (rCPercent * 100)); /* % */
		}
		else
		{
			/* Disable controls */
			ProgressBarCurPict->setEnabled(FALSE);
			TextLabelCurPict->setEnabled(FALSE);
		}
	}
}

void TransmDialog::OnButtonStartStop()
{
	int i;

	if (bIsStarted == TRUE)
	{
		/* Stop transmitter */
		TransThread.Stop();

		ButtonStartStop->setText(tr("&Start"));

		EnableAllControlsForSet();

		bIsStarted = FALSE;
	}
	else
	{
		/* Start transmitter */
		/* Set text message */
		TransThread.DRMTransmitter.GetAudSrcEnc()->ClearTextMessage();

		for (i = 1; i < vecstrTextMessage.Size(); i++)
			TransThread.DRMTransmitter.GetAudSrcEnc()->
				SetTextMessage(vecstrTextMessage[i]);

		/* Set file names for data application */
		TransThread.DRMTransmitter.GetAudSrcEnc()->ClearPicFileNames();

		/* Iteration through list view items. Code based on QT sample code for
		   list view items */
		QListViewItemIterator it(ListViewFileNames);

		for (; it.current(); it++)
		{
			/* Complete file path is in third column */
			const QString strFileName = it.current()->text(2);

			/* Extract format string */
			QFileInfo FileInfo(strFileName);
			const QString strFormat = FileInfo.extension(FALSE);

			TransThread.DRMTransmitter.GetAudSrcEnc()->
				SetPicFileName(strFileName.latin1(), strFormat.latin1());
		}

		TransThread.start();

		ButtonStartStop->setText(tr("&Stop"));

		DisableAllControlsForSet();

		bIsStarted = TRUE;
	}
}

void TransmDialog::OnToggleCheckBoxEnableTextMessage(bool bState)
{
	EnableTextMessage(bState);
}

void TransmDialog::EnableTextMessage(const _BOOLEAN bFlag)
{
	if (bFlag == TRUE)
	{
		/* Enable text message controls */
		ComboBoxTextMessage->setEnabled(TRUE);
		MultiLineEditTextMessage->setEnabled(TRUE);
		PushButtonAddText->setEnabled(TRUE);
		PushButtonClearAllText->setEnabled(TRUE);

		/* Set text message flag in global struct */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].AudioParam.bTextflag = TRUE;
	}
	else
	{
		/* Disable text message controls */
		ComboBoxTextMessage->setEnabled(FALSE);
		MultiLineEditTextMessage->setEnabled(FALSE);
		PushButtonAddText->setEnabled(FALSE);
		PushButtonClearAllText->setEnabled(FALSE);

		/* Set text message flag in global struct */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].AudioParam.bTextflag = FALSE;
	}
}

void TransmDialog::OnToggleCheckBoxEnableAudio(bool bState)
{
	EnableAudio(bState);

	if (bState)
	{
		/* Set audio enable check box */
		CheckBoxEnableData->setChecked(FALSE);
		EnableData(FALSE);
	}
	else
	{
		/* Set audio enable check box */
		CheckBoxEnableData->setChecked(TRUE);
		EnableData(TRUE);
	}
}

void TransmDialog::EnableAudio(const _BOOLEAN bFlag)
{
	if (bFlag == TRUE)
	{
		/* Enable audio controls */
		GroupBoxTextMessage->setEnabled(TRUE);
		ComboBoxProgramType->setEnabled(TRUE);

		/* Only one audio service */
		TransThread.DRMTransmitter.GetParameters()->
			iNumAudioService = 1;
		TransThread.DRMTransmitter.GetParameters()->
			iNumDataService = 0;

		/* Audio flag of this service */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].eAudDataFlag = CParameter::SF_AUDIO;

		/* Always use stream number 0 right now, TODO */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].AudioParam.iStreamID = 0;

		/* Programme Type code, get it from combo box */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].iServiceDescr = ComboBoxProgramType->currentItem();
	}
	else
	{
		/* Disable audio controls */
		GroupBoxTextMessage->setEnabled(FALSE);
		ComboBoxProgramType->setEnabled(FALSE);
	}
}

void TransmDialog::OnToggleCheckBoxEnableData(bool bState)
{
	EnableData(bState);

	if (bState)
	{
		/* Set audio enable check box */
		CheckBoxEnableAudio->setChecked(FALSE);
		EnableAudio(FALSE);
	}
	else
	{
		/* Set audio enable check box */
		CheckBoxEnableAudio->setChecked(TRUE);
		EnableAudio(TRUE);
	}
}

void TransmDialog::EnableData(const _BOOLEAN bFlag)
{
	if (bFlag == TRUE)
	{
		/* Enable data controls */
		ListViewFileNames->setEnabled(TRUE);
		PushButtonClearAllFileNames->setEnabled(TRUE);
		PushButtonAddFile->setEnabled(TRUE);

		/* Only one data service */
		TransThread.DRMTransmitter.GetParameters()->
			iNumAudioService = 0;
		TransThread.DRMTransmitter.GetParameters()->
			iNumDataService = 1;

		/* Data flag for this service */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].eAudDataFlag = CParameter::SF_DATA;

		/* Always use stream number 0, TODO */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].DataParam.iStreamID = 0;

		/* Init SlideShow application */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].DataParam.iPacketLen = 45; /* TEST */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].DataParam.eDataUnitInd = CParameter::DU_DATA_UNITS;
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].DataParam.eAppDomain = CParameter::AD_DAB_SPEC_APP;

		/* The value 0 indicates that the application details are provided
		   solely by SDC data entity type 5 */
		TransThread.DRMTransmitter.GetParameters()->
			Service[0].iServiceDescr = 0;
	}
	else
	{
		/* Disable data controls */
		ListViewFileNames->setEnabled(FALSE);
		PushButtonClearAllFileNames->setEnabled(FALSE);
		PushButtonAddFile->setEnabled(FALSE);
	}
}

_BOOLEAN TransmDialog::GetMessageText(const int iID)
{
	_BOOLEAN bTextIsNotEmpty = TRUE;

	/* Check if text control is not empty */
	if (MultiLineEditTextMessage->edited())
	{
		/* Check size of container. If not enough space, enlarge */
		if (iID == vecstrTextMessage.Size())
			vecstrTextMessage.Enlarge(1);

		/* First line */
		vecstrTextMessage[iID] =
			MultiLineEditTextMessage->textLine(0).utf8();

		/* Other lines */
		const int iNumLines = MultiLineEditTextMessage->numLines();

		for (int i = 1; i < iNumLines; i++)
		{
			/* Insert line break */
			vecstrTextMessage[iID].append("\x0A");

			/* Insert text of next line */
			vecstrTextMessage[iID].
				append(MultiLineEditTextMessage->textLine(i).utf8());
		}
	}
	else
		bTextIsNotEmpty = FALSE;

	return bTextIsNotEmpty;
}

void TransmDialog::OnPushButtonAddText()
{
	if (iIDCurrentText == 0)
	{
		/* Add new message */
		if (GetMessageText(vecstrTextMessage.Size()) == TRUE)
		{
			/* If text was not empty, add new text in combo box */
			const int iNewID = vecstrTextMessage.Size() - 1;
			ComboBoxTextMessage->insertItem(QString().setNum(iNewID), iNewID);

			/* Clear added text */
			MultiLineEditTextMessage->clear();
			MultiLineEditTextMessage->setEdited(FALSE);
		}
	}
	else
	{
		/* Text was modified */
		GetMessageText(iIDCurrentText);
	}
}

void TransmDialog::OnButtonClearAllText()
{
	/* Clear container */
	vecstrTextMessage.Init(1);
	iIDCurrentText = 0;

	/* Clear combo box */
	ComboBoxTextMessage->clear();
	ComboBoxTextMessage->insertItem(tr("new"), 0);

	/* Clear multi line edit */
	MultiLineEditTextMessage->clear();
	MultiLineEditTextMessage->setEdited(FALSE);
}

void TransmDialog::OnPushButtonAddFileName()
{
	/* Show "open file" dialog. Let the user select more than one file */
	QStringList list = QFileDialog::getOpenFileNames(
		tr("Image Files (*.png *.jpg *.jpeg *.jfif)"), NULL, this);

	/* Check if user not hit the cancel button */
	if (!list.isEmpty())
	{
		/* Insert all selected file names */
		for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		{
			QFileInfo FileInfo((*it));

			/* Insert list view item. The object which is created here will be
			   automatically destroyed by QT when the parent
			   ("ListViewFileNames") is destroyed */
			ListViewFileNames->insertItem(
				new QListViewItem(ListViewFileNames, FileInfo.fileName(),
				QString().setNum((float) FileInfo.size() / 1000.0, 'f', 2),
				FileInfo.filePath()));
		}
	}
}

void TransmDialog::OnButtonClearAllFileNames()
{
	/* Clear list box for file names */
	ListViewFileNames->clear();
}

void TransmDialog::OnComboBoxTextMessageHighlighted(int iID)
{
	iIDCurrentText = iID;

	/* Set text control with selected message */
	MultiLineEditTextMessage->clear();
	MultiLineEditTextMessage->setEdited(FALSE);

	if (iID != 0)
	{
		/* Write stored text in multi line edit control */
		MultiLineEditTextMessage->insertLine(vecstrTextMessage[iID].c_str());
	}
}

void TransmDialog::OnTextChangedSndCrdIF(const QString& strIF)
{
	/* Convert string to floating point value "toFloat()" */
	TransThread.DRMTransmitter.SetCarOffset(strIF.toFloat());
}

void TransmDialog::OnTextChangedServiceID(const QString& strID)
{
	/* Convert string to unsigned integer "toUInt()" */
	TransThread.DRMTransmitter.GetParameters()->Service[0].iServiceID =
		strID.toUInt();
}

void TransmDialog::OnTextChangedServiceLabel(const QString& strLabel)
{
	/* Set additional text for log file. Conversion from QString to STL
	   string is needed (done with .utf8() function of QT string) */
	TransThread.DRMTransmitter.GetParameters()->Service[0].strLabel =
		strLabel.utf8();
}

void TransmDialog::OnComboBoxMSCInterleaverHighlighted(int iID)
{
	switch (iID)
	{
	case 0:
		TransThread.DRMTransmitter.GetParameters()->eSymbolInterlMode =
			CParameter::SI_LONG;
		break;

	case 1:
		TransThread.DRMTransmitter.GetParameters()->eSymbolInterlMode =
			CParameter::SI_SHORT;
		break;
	}
}

void TransmDialog::OnComboBoxMSCConstellationHighlighted(int iID)
{
	switch (iID)
	{
	case 0:
		TransThread.DRMTransmitter.GetParameters()->eMSCCodingScheme =
			CParameter::CS_2_SM;
		break;

	case 1:
		TransThread.DRMTransmitter.GetParameters()->eMSCCodingScheme =
			CParameter::CS_3_SM;
		break;

	case 2:
		TransThread.DRMTransmitter.GetParameters()->eMSCCodingScheme =
			CParameter::CS_3_HMSYM;
		break;

	case 3:
		TransThread.DRMTransmitter.GetParameters()->eMSCCodingScheme =
			CParameter::CS_3_HMMIX;
		break;
	}

	/* Protection level must be re-adjusted when constelletion mode was
	   changed */
	UpdateMSCProtLevCombo();
}

void TransmDialog::OnComboBoxMSCProtLevHighlighted(int iID)
{
	TransThread.DRMTransmitter.GetParameters()->MSCPrLe.iPartB = iID;
}

void TransmDialog::UpdateMSCProtLevCombo()
{
	if (TransThread.DRMTransmitter.GetParameters()->eMSCCodingScheme ==
		CParameter::CS_2_SM)
	{
		/* Only two protection levels possible in 16 QAM mode */
		ComboBoxMSCProtLev->clear();
		ComboBoxMSCProtLev->insertItem("0", 0);
		ComboBoxMSCProtLev->insertItem("1", 1);
	}
	else
	{
		/* Four protection levels defined */
		ComboBoxMSCProtLev->clear();
		ComboBoxMSCProtLev->insertItem("0", 0);
		ComboBoxMSCProtLev->insertItem("1", 1);
		ComboBoxMSCProtLev->insertItem("2", 2);
		ComboBoxMSCProtLev->insertItem("3", 3);
	}

	/* Set protection level to 1 */
	ComboBoxMSCProtLev->setCurrentItem(1);
	TransThread.DRMTransmitter.GetParameters()->MSCPrLe.iPartB = 1;
}

void TransmDialog::OnComboBoxSDCConstellationHighlighted(int iID)
{
	switch (iID)
	{
	case 0:
		TransThread.DRMTransmitter.GetParameters()->eSDCCodingScheme =
			CParameter::CS_1_SM;
		break;

	case 1:
		TransThread.DRMTransmitter.GetParameters()->eSDCCodingScheme =
			CParameter::CS_2_SM;
		break;
	}
}

void TransmDialog::OnComboBoxLanguageHighlighted(int iID)
{
	TransThread.DRMTransmitter.GetParameters()->Service[0].iLanguage = iID;
}

void TransmDialog::OnComboBoxProgramTypeHighlighted(int iID)
{
	TransThread.DRMTransmitter.GetParameters()->Service[0].iServiceDescr = iID;
}

void TransmDialog::OnRadioOutput(int iID)
{
	switch (iID)
	{
	case 0:
		/* Button "Real Valued" */
		TransThread.DRMTransmitter.GetTransData()->
			SetIQOutput(CTransmitData::OF_REAL_VAL);
		break;

	case 1:
		/* Button "I / Q (pos)" */
		TransThread.DRMTransmitter.GetTransData()->
			SetIQOutput(CTransmitData::OF_IQ_POS);
		break;

	case 2:
		/* Button "I / Q (neg)" */
		TransThread.DRMTransmitter.GetTransData()->
			SetIQOutput(CTransmitData::OF_IQ_NEG);
		break;

	case 3:
		/* Button "E / P" */
		TransThread.DRMTransmitter.GetTransData()->
			SetIQOutput(CTransmitData::OF_EP);
		break;
	}
}

void TransmDialog::OnRadioRobustnessMode(int iID)
{
	/* Check, which bandwith's are possible with this robustness mode */
	switch (iID)
	{
	case 0:
		/* All bandwidth modes are possible */
		RadioButtonBandwidth45->setEnabled(TRUE);
		RadioButtonBandwidth5->setEnabled(TRUE);
		RadioButtonBandwidth9->setEnabled(TRUE);
		RadioButtonBandwidth10->setEnabled(TRUE);
		RadioButtonBandwidth18->setEnabled(TRUE);
		RadioButtonBandwidth20->setEnabled(TRUE);
		break;

	case 1:
		/* All bandwidth modes are possible */
		RadioButtonBandwidth45->setEnabled(TRUE);
		RadioButtonBandwidth5->setEnabled(TRUE);
		RadioButtonBandwidth9->setEnabled(TRUE);
		RadioButtonBandwidth10->setEnabled(TRUE);
		RadioButtonBandwidth18->setEnabled(TRUE);
		RadioButtonBandwidth20->setEnabled(TRUE);
		break;

	case 2:
		/* Only 10 and 20 kHz possible in robustness mode C */
		RadioButtonBandwidth45->setEnabled(FALSE);
		RadioButtonBandwidth5->setEnabled(FALSE);
		RadioButtonBandwidth9->setEnabled(FALSE);
		RadioButtonBandwidth10->setEnabled(TRUE);
		RadioButtonBandwidth18->setEnabled(FALSE);
		RadioButtonBandwidth20->setEnabled(TRUE);

		/* Set check on a default value to be sure we are "in range" */
		RadioButtonBandwidth10->setChecked(TRUE);
		OnRadioBandwidth(3); /* TODO better solution for that */
		break;

	case 3:
		/* Only 10 and 20 kHz possible in robustness mode D */
		RadioButtonBandwidth45->setEnabled(FALSE);
		RadioButtonBandwidth5->setEnabled(FALSE);
		RadioButtonBandwidth9->setEnabled(FALSE);
		RadioButtonBandwidth10->setEnabled(TRUE);
		RadioButtonBandwidth18->setEnabled(FALSE);
		RadioButtonBandwidth20->setEnabled(TRUE);

		/* Set check on a default value to be sure we are "in range" */
		RadioButtonBandwidth10->setChecked(TRUE);
		OnRadioBandwidth(3); /* TODO better solution for that */
		break;
	}


	/* Set new parameters */
	ERobMode eNewRobMode;

	switch (iID)
	{
	case 0:
		eNewRobMode = RM_ROBUSTNESS_MODE_A;
		break;

	case 1:
		eNewRobMode = RM_ROBUSTNESS_MODE_B;
		break;

	case 2:
		eNewRobMode = RM_ROBUSTNESS_MODE_C;
		break;

	case 3:
		eNewRobMode = RM_ROBUSTNESS_MODE_D;
		break;
	}

	/* Set new robustness mode. Spectrum occupancy is the same as before */
	TransThread.DRMTransmitter.GetParameters()->InitCellMapTable(eNewRobMode,
		TransThread.DRMTransmitter.GetParameters()->GetSpectrumOccup());
}

void TransmDialog::OnRadioBandwidth(int iID)
{
	ESpecOcc eNewSpecOcc;

	switch (iID)
	{
	case 0:
		eNewSpecOcc = SO_0;
		break;

	case 1:
		eNewSpecOcc = SO_1;
		break;

	case 2:
		eNewSpecOcc = SO_2;
		break;

	case 3:
		eNewSpecOcc = SO_3;
		break;

	case 4:
		eNewSpecOcc = SO_4;
		break;

	case 5:
		eNewSpecOcc = SO_5;
		break;
	}

	/* Set new spectrum occupancy. Robustness mode is the same as before */
	TransThread.DRMTransmitter.GetParameters()->InitCellMapTable(
		TransThread.DRMTransmitter.GetParameters()->GetWaveMode(), eNewSpecOcc);
}

void TransmDialog::DisableAllControlsForSet()
{
	GroupBoxChanParam->setEnabled(FALSE);
	TabWidgetServices->setEnabled(FALSE);
	ButtonGroupOutput->setEnabled(FALSE);

	GroupInput->setEnabled(TRUE); /* For run-mode */
}

void TransmDialog::EnableAllControlsForSet()
{
	GroupBoxChanParam->setEnabled(TRUE);
	TabWidgetServices->setEnabled(TRUE);
	ButtonGroupOutput->setEnabled(TRUE);

	GroupInput->setEnabled(FALSE); /* For run-mode */

	/* Reset status bars */
	ProgrInputLevel->setValue(RET_VAL_LOG_0);
	ProgressBarCurPict->setProgress(0);
	TextLabelCurPict->setText("");
}

void TransmDialog::AddWhatsThisHelp()
{
	/* Dream Logo */
	QWhatsThis::add(PixmapLabelDreamLogo,
		tr("<b>Dream Logo:</b> This is the official logo of "
		"the Dream software."));

	/* Input Level */
	QWhatsThis::add(ProgrInputLevel,
		tr("<b>Input Level:</b> The input level meter shows "
		"the relative input signal peak level in dB. If the level is too high, "
		"the meter turns from green to red."));

	/* DRM Robustness Mode */
	const QString strRobustnessMode =
		tr("<b>DRM Robustness Mode:</b> In a DRM system, "
		"four possible robustness modes are defined to adapt the system to "
		"different channel conditions. According to the DRM standard:"
		"<ul><li><i>Mode A:</i> Gaussian channels, with "
		"minor fading</li><li><i>Mode B:</i> Time "
		"and frequency selective channels, with longer delay spread"
		"</li><li><i>Mode C:</i> As robustness mode B, "
		"but with higher Doppler spread</li><li><i>Mode D:"
		"</i> As robustness mode B, but with severe delay and "
		"Doppler spread</li></ul>");

	QWhatsThis::add(RadioButtonRMA, strRobustnessMode);
	QWhatsThis::add(RadioButtonRMB, strRobustnessMode);
	QWhatsThis::add(RadioButtonRMC, strRobustnessMode);
	QWhatsThis::add(RadioButtonRMD, strRobustnessMode);

	/* Bandwidth */
	const QString strBandwidth =
		tr("<b>DRM Bandwidth:</b> The bandwith is the gross "
		"bandwidth of the generated DRM signal. Not all DRM robustness mode / "
		"bandwidth constellations are possible, e.g., DRM robustness mode D "
		"and C are only defined for the bandwidths 10 kHz and 20 kHz.");

	QWhatsThis::add(RadioButtonBandwidth45, strBandwidth);
	QWhatsThis::add(RadioButtonBandwidth5, strBandwidth);
	QWhatsThis::add(RadioButtonBandwidth9, strBandwidth);
	QWhatsThis::add(RadioButtonBandwidth10, strBandwidth);
	QWhatsThis::add(RadioButtonBandwidth18, strBandwidth);
	QWhatsThis::add(RadioButtonBandwidth20, strBandwidth);

	/* Output intermediate frequency of DRM signal */
	const QString strOutputIF =
		tr("<b>Output intermediate frequency of DRM signal:</b> "
		"Set the output intermediate frequency (IF) of generated DRM signal "
		"in the 'sound-card pass-band'. In some DRM modes, the IF is located "
		"at the edge of the DRM signal, in other modes it is centered. The IF "
		"should be chosen that the DRM signal lies entirely inside the "
		"sound-card bandwidth.");

	QWhatsThis::add(TextLabelIF, strOutputIF);
	QWhatsThis::add(LineEditSndCrdIF, strOutputIF);
	QWhatsThis::add(TextLabelIFUnit, strOutputIF);

	/* Output format */
	const QString strOutputFormat =
		tr("<b>Output format:</b> Since the sound-card "
		"outputs signals in stereo format, it is possible to output the DRM "
		"signal in three formats:<ul><li><b>real valued"
		"</b> output on both, left and right, sound-card "
		"channels</li><li><b>I / Q</b> output "
		"which is the in-phase and quadrature component of the complex "
		"base-band signal at the desired IF. In-phase is output on the "
		"left channel and quadrature on the right channel."
		"</li><li><b>E / P</b> output which is the "
		"envelope and phase on separate channels. This output type cannot "
		"be used if the Dream transmitter is regularly compiled with a "
		"sound-card sample rate of 48 kHz since the spectrum of these "
		"components exceed the bandwidth of 20 kHz.<br>The envelope signal "
		"is output on the left channel and the phase is output on the right "
		"channel.</li></ul>");

	QWhatsThis::add(RadioButtonOutReal, strOutputFormat);
	QWhatsThis::add(RadioButtonOutIQPos, strOutputFormat);
	QWhatsThis::add(RadioButtonOutIQNeg, strOutputFormat);
	QWhatsThis::add(RadioButtonOutEP, strOutputFormat);

	/* MSC interleaver mode */
	const QString strInterleaver =
		tr("<b>MSC interleaver mode:</b> The symbol "
		"interleaver depth can be either short (approx. 400 ms) or long "
		"(approx. 2 s). The longer the interleaver the better the channel "
		"decoder can correct errors from slow fading signals. But the longer "
		"the interleaver length the longer the delay until (after a "
		"re-synchronization) audio can be heard.");

	QWhatsThis::add(TextLabelInterleaver, strInterleaver);
	QWhatsThis::add(ComboBoxMSCInterleaver, strInterleaver);
}
