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

#include "systemevalDlg.h"


/* Implementation *************************************************************/
systemevalDlg::systemevalDlg(CDRMReceiver* pNDRMR, QWidget* parent,
	const char* name, bool modal, WFlags f) : vecpDRMPlots(0),
	systemevalDlgBase(parent, name, modal, f), pDRMRec(pNDRMR)
{
	/* Set help text for the controls */
	AddWhatsThisHelp();

#ifdef _WIN32 /* This works only reliable under Windows :-( */
	/* Get window geometry data from DRMReceiver module and apply it */
	const QRect WinGeom(pDRMRec->GeomSystemEvalDlg.iXPos,
		pDRMRec->GeomSystemEvalDlg.iYPos,
		pDRMRec->GeomSystemEvalDlg.iWSize,
		pDRMRec->GeomSystemEvalDlg.iHSize);

	if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
		setGeometry(WinGeom);
#else /* Under Linux only restore the size */
	resize(pDRMRec->GeomSystemEvalDlg.iWSize,
		pDRMRec->GeomSystemEvalDlg.iHSize);
#endif


	/* Init controls -------------------------------------------------------- */
	/* Init main plot */
	MainPlot->SetRecObj(pDRMRec);
	MainPlot->SetPlotStyle(pDRMRec->iMainPlotColorStyle);
	MainPlot->setMargin(1);

	/* Init slider control */
	SliderNoOfIterations->setRange(0, 4);
	SliderNoOfIterations->
		setValue(pDRMRec->GetMSCMLC()->GetInitNumIterations());
	TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
		QString().setNum(pDRMRec->GetMSCMLC()->GetInitNumIterations()));

	/* Update times for color LEDs */
	LEDFAC->SetUpdateTime(1500);
	LEDSDC->SetUpdateTime(1500);
	LEDMSC->SetUpdateTime(600);
	LEDFrameSync->SetUpdateTime(600);
	LEDTimeSync->SetUpdateTime(600);
	LEDIOInterface->SetUpdateTime(2000); /* extra long -> red light stays long */

	/* Init parameter for frequency edit for log file */
	iCurFrequency = 0;

	/* Update controls */
	UpdateControls();


	/* Init chart selector list view ---------------------------------------- */
	ListViewCharSel->clear();

	/* No sorting of items */
	ListViewCharSel->setSorting(-1);

	/* Insert parent list view items. Parent list view items should not be
	   selectable */
	CCharSelItem* pHistoryLiViIt =
		new CCharSelItem(ListViewCharSel, tr("History"),
		CDRMPlot::NONE_OLD, FALSE);

	CCharSelItem* pConstellationLiViIt =
		new CCharSelItem(ListViewCharSel, tr("Constellation"),
		CDRMPlot::NONE_OLD, FALSE);

	CCharSelItem* pChannelLiViIt =
		new CCharSelItem(ListViewCharSel, tr("Channel"),
		CDRMPlot::NONE_OLD, FALSE);

	CCharSelItem* pSpectrumLiViIt =
		new CCharSelItem(ListViewCharSel, tr("Spectrum"),
		CDRMPlot::NONE_OLD, FALSE);
 
	/* Inser actual items. The list is not sorted -> items which are inserted
	   first show up at the end of the list */
	/* Spectrum */
	CCharSelItem* pListItSNRSpec = new CCharSelItem(pSpectrumLiViIt,
		tr("SNR Spectrum"), CDRMPlot::SNR_SPECTRUM);
	CCharSelItem* pListItAudSpec = new CCharSelItem(pSpectrumLiViIt,
		tr("Audio Spectrum"), CDRMPlot::AUDIO_SPECTRUM);
	CCharSelItem* pListItPowSpecDens = new CCharSelItem(pSpectrumLiViIt,
		tr("Shifted PSD"), CDRMPlot::POWER_SPEC_DENSITY);
	CCharSelItem* pListItInpSpecWater = new CCharSelItem(pSpectrumLiViIt,
		tr("Waterfall Input Spectrum"), CDRMPlot::INP_SPEC_WATERF);
	CCharSelItem* pListItInpSpectrNoAv = new CCharSelItem(pSpectrumLiViIt,
		tr("Input Spectrum"), CDRMPlot::INPUTSPECTRUM_NO_AV);
	CCharSelItem* pListItInpPSD = new CCharSelItem(pSpectrumLiViIt,
		tr("Input PSD"), CDRMPlot::INPUT_SIG_PSD);

	/* Constellation */
	CCharSelItem* pListItConstMSC = new CCharSelItem(pConstellationLiViIt,
		tr("MSC"), CDRMPlot::MSC_CONSTELLATION);
	CCharSelItem* pListItConstSDC = new CCharSelItem(pConstellationLiViIt,
		tr("SDC"), CDRMPlot::SDC_CONSTELLATION);
	CCharSelItem* pListItConstFAC = new CCharSelItem(pConstellationLiViIt,
		tr("FAC"), CDRMPlot::FAC_CONSTELLATION);
	CCharSelItem* pListItConstAll = new CCharSelItem(pConstellationLiViIt,
		tr("FAC / SDC / MSC"), CDRMPlot::ALL_CONSTELLATION);

	/* History */
	CCharSelItem* pListItHistFrSa = new CCharSelItem(pHistoryLiViIt,
		tr("Frequency / Sample Rate"), CDRMPlot::FREQ_SAM_OFFS_HIST);
	CCharSelItem* pListItHistDeDo = new CCharSelItem(pHistoryLiViIt,
		tr("Delay / Doppler"), CDRMPlot::DOPPLER_DELAY_HIST);
	CCharSelItem* pListItHistSNRAu = new CCharSelItem(pHistoryLiViIt,
		tr("SNR / Audio"), CDRMPlot::SNR_AUDIO_HIST);

	/* Channel */
	CCharSelItem* pListItChanTF = new CCharSelItem(pChannelLiViIt,
		tr("Transfer Function"), CDRMPlot::TRANSFERFUNCTION);
	CCharSelItem* pListItChanIR = new CCharSelItem(pChannelLiViIt,
		tr("Impulse Response"), CDRMPlot::AVERAGED_IR);

/* _WIN32 fix because in Visual c++ the GUI files are always compiled even
   if USE_QT_GUI is set or not (problem with MDI in DRMReceiver) */
#ifdef USE_QT_GUI
	/* If MDI in is enabled, disable some of the controls and use different
	   initialization for the chart and chart selector */
	if (pDRMRec->GetMDI()->GetMDIInEnabled() == TRUE)
	{
		ListViewCharSel->setEnabled(FALSE);
		SliderNoOfIterations->setEnabled(FALSE);

		ButtonGroupChanEstFreqInt->setEnabled(FALSE);
		ButtonGroupChanEstTimeInt->setEnabled(FALSE);
		ButtonGroupTimeSyncTrack->setEnabled(FALSE);
		CheckBoxFlipSpec->setEnabled(FALSE);
		EdtFrequency->setText("0");
		EdtFrequency->setEnabled(FALSE);
		GroupBoxInterfRej->setEnabled(FALSE);

		/* Only audio spectrum makes sence for MDI in */
		ListViewCharSel->setSelected(pListItAudSpec, TRUE);
		ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
		MainPlot->SetupChart(CDRMPlot::AUDIO_SPECTRUM);
	}
	else
	{
		/* Set chart type */
		switch (pDRMRec->iSysEvalDlgPlotType)
		{
		case (int) CDRMPlot::POWER_SPEC_DENSITY:
			ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItPowSpecDens, TRUE);
			MainPlot->SetupChart(CDRMPlot::POWER_SPEC_DENSITY);
			break;

		case (int) CDRMPlot::INPUTSPECTRUM_NO_AV:
			ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItInpSpectrNoAv, TRUE);
			MainPlot->SetupChart(CDRMPlot::INPUTSPECTRUM_NO_AV);
			break;

		case (int) CDRMPlot::AUDIO_SPECTRUM:
			ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItAudSpec, TRUE);
			MainPlot->SetupChart(CDRMPlot::AUDIO_SPECTRUM);
			break;

		case (int) CDRMPlot::SNR_SPECTRUM:
			ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItSNRSpec, TRUE);
			MainPlot->SetupChart(CDRMPlot::SNR_SPECTRUM);
			break;

		case (int) CDRMPlot::INP_SPEC_WATERF:
			ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItInpSpecWater, TRUE);
			MainPlot->SetupChart(CDRMPlot::INP_SPEC_WATERF);
			break;

		case (int) CDRMPlot::TRANSFERFUNCTION:
			ListViewCharSel->setOpen(pChannelLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItChanTF, TRUE);
			MainPlot->SetupChart(CDRMPlot::TRANSFERFUNCTION);
			break;

		case (int) CDRMPlot::AVERAGED_IR:
			ListViewCharSel->setOpen(pChannelLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItChanIR, TRUE);
			MainPlot->SetupChart(CDRMPlot::AVERAGED_IR);
			break;

		case (int) CDRMPlot::FAC_CONSTELLATION:
			ListViewCharSel->setOpen(pConstellationLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItConstFAC, TRUE);
			MainPlot->SetupChart(CDRMPlot::FAC_CONSTELLATION);
			break;

		case (int) CDRMPlot::SDC_CONSTELLATION:
			ListViewCharSel->setOpen(pConstellationLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItConstSDC, TRUE);
			MainPlot->SetupChart(CDRMPlot::SDC_CONSTELLATION);
			break;

		case (int) CDRMPlot::MSC_CONSTELLATION:
			ListViewCharSel->setOpen(pConstellationLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItConstMSC, TRUE);
			MainPlot->SetupChart(CDRMPlot::MSC_CONSTELLATION);
			break;

		case (int) CDRMPlot::ALL_CONSTELLATION:
			ListViewCharSel->setOpen(pConstellationLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItConstAll, TRUE);
			MainPlot->SetupChart(CDRMPlot::ALL_CONSTELLATION);
			break;

		case (int) CDRMPlot::FREQ_SAM_OFFS_HIST:
			ListViewCharSel->setOpen(pHistoryLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItHistFrSa, TRUE);
			MainPlot->SetupChart(CDRMPlot::FREQ_SAM_OFFS_HIST);
			break;

		case (int) CDRMPlot::DOPPLER_DELAY_HIST:
			ListViewCharSel->setOpen(pHistoryLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItHistDeDo, TRUE);
			MainPlot->SetupChart(CDRMPlot::DOPPLER_DELAY_HIST);
			break;

		case (int) CDRMPlot::SNR_AUDIO_HIST:
			ListViewCharSel->setOpen(pHistoryLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItHistSNRAu, TRUE);
			MainPlot->SetupChart(CDRMPlot::SNR_AUDIO_HIST);
			break;

		default: /* INPUT_SIG_PSD, includes INPUT_SIG_PSD_ANALOG and NONE_OLD */
			ListViewCharSel->setOpen(pSpectrumLiViIt, TRUE);
			ListViewCharSel->setSelected(pListItInpPSD, TRUE);
			MainPlot->SetupChart(CDRMPlot::INPUT_SIG_PSD);
			break;
		}
	}
#endif

	/* Init context menu for list view */
	pListViewContextMenu = new QPopupMenu(this, tr("ListView context menu"));
	pListViewContextMenu->insertItem(tr("&Open in separate window"), this,
		SLOT(OnListViContMenu()));


	/* Connect controls ----------------------------------------------------- */
	connect(SliderNoOfIterations, SIGNAL(valueChanged(int)),
		this, SLOT(OnSliderIterChange(int)));

	/* Radio buttons */
	connect(RadioButtonTiLinear, SIGNAL(clicked()),
		this, SLOT(OnRadioTimeLinear()));
	connect(RadioButtonTiWiener, SIGNAL(clicked()),
		this, SLOT(OnRadioTimeWiener()));
	connect(RadioButtonFreqLinear, SIGNAL(clicked()),
		this, SLOT(OnRadioFrequencyLinear()));
	connect(RadioButtonFreqDFT, SIGNAL(clicked()),
		this, SLOT(OnRadioFrequencyDft()));
	connect(RadioButtonFreqWiener, SIGNAL(clicked()),
		this, SLOT(OnRadioFrequencyWiener()));
	connect(RadioButtonTiSyncEnergy, SIGNAL(clicked()),
		this, SLOT(OnRadioTiSyncEnergy()));
	connect(RadioButtonTiSyncFirstPeak, SIGNAL(clicked()),
		this, SLOT(OnRadioTiSyncFirstPeak()));

	/* Char selector list view */
	connect(ListViewCharSel, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(OnListSelChanged(QListViewItem*)));
	connect(ListViewCharSel,
		SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
		this, SLOT(OnListRightButClicked(QListViewItem*, const QPoint&, int)));

	/* Buttons */
	connect(buttonOk, SIGNAL(clicked()),
		this, SLOT(accept()));

	/* Check boxes */
	connect(CheckBoxFlipSpec, SIGNAL(clicked()),
		this, SLOT(OnCheckFlipSpectrum()));
	connect(CheckBoxMuteAudio, SIGNAL(clicked()),
		this, SLOT(OnCheckBoxMuteAudio()));
	connect(CheckBoxWriteLog, SIGNAL(clicked()),
		this, SLOT(OnCheckWriteLog()));
	connect(CheckBoxSaveAudioWave, SIGNAL(clicked()),
		this, SLOT(OnCheckSaveAudioWAV()));
	connect(CheckBoxRecFilter, SIGNAL(clicked()),
		this, SLOT(OnCheckRecFilter()));
	connect(CheckBoxModiMetric, SIGNAL(clicked()),
		this, SLOT(OnCheckModiMetric()));
	connect(CheckBoxReverb, SIGNAL(clicked()),
		this, SLOT(OnCheckBoxReverb()));

	/* Timers */
	connect(&Timer, SIGNAL(timeout()),
		this, SLOT(OnTimer()));
	connect(&TimerLogFileLong, SIGNAL(timeout()),
		this, SLOT(OnTimerLogFileLong()));
	connect(&TimerLogFileShort, SIGNAL(timeout()),
		this, SLOT(OnTimerLogFileShort()));
	connect(&TimerLogFileStart, SIGNAL(timeout()),
		this, SLOT(OnTimerLogFileStart()));

	/* Activte real-time timer */
	Timer.start(GUI_CONTROL_UPDATE_TIME);

	/* Activate delayed log file start if necessary (timer is set to shot
	   only once) */
	if (pDRMRec->GetParameters()->ReceptLog.IsDelLogStart() == TRUE)
	{
		/* One shot timer */
		TimerLogFileStart.start(pDRMRec->GetParameters()->
			ReceptLog.GetDelLogStart() * 1000 /* ms */, TRUE);
	}
}

systemevalDlg::~systemevalDlg()
{
	/* Set window geometry data in DRMReceiver module */
	QRect WinGeom = geometry();

	pDRMRec->GeomSystemEvalDlg.iXPos = WinGeom.x();
	pDRMRec->GeomSystemEvalDlg.iYPos = WinGeom.y();
	pDRMRec->GeomSystemEvalDlg.iHSize = WinGeom.height();
	pDRMRec->GeomSystemEvalDlg.iWSize = WinGeom.width();

	/* Store current plot type. Convert plot type into an integer type.
       TODO: better solution */
	pDRMRec->iSysEvalDlgPlotType = (int) MainPlot->GetChartType();

	/* Call the hide event handler routine to make sure the chart window sizes
	   and positions are stored */
	hideEvent(NULL);
}

void systemevalDlg::UpdateControls()
{
	/* Slider for MLC number of iterations */
	const int iNumIt = pDRMRec->GetMSCMLC()->GetInitNumIterations();
	if (SliderNoOfIterations->value() != iNumIt)
	{
		/* Update slider and label */
		SliderNoOfIterations->setValue(iNumIt);
		TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
			QString().setNum(iNumIt));
	}

	/* Update for channel estimation and time sync switches */
	switch (pDRMRec->GetChanEst()->GetTimeInt())
	{
	case CChannelEstimation::TLINEAR:
		if (!RadioButtonTiLinear->isChecked())
			RadioButtonTiLinear->setChecked(TRUE);
		break;

	case CChannelEstimation::TWIENER:
		if (!RadioButtonTiWiener->isChecked())
			RadioButtonTiWiener->setChecked(TRUE);
		break;
	}

	switch (pDRMRec->GetChanEst()->GetFreqInt())
	{
	case CChannelEstimation::FLINEAR:
		if (!RadioButtonFreqLinear->isChecked())
			RadioButtonFreqLinear->setChecked(TRUE);
		break;

	case CChannelEstimation::FDFTFILTER:
		if (!RadioButtonFreqDFT->isChecked())
			RadioButtonFreqDFT->setChecked(TRUE);
		break;

	case CChannelEstimation::FWIENER:
		if (!RadioButtonFreqWiener->isChecked())
			RadioButtonFreqWiener->setChecked(TRUE);
		break;
	}

	switch (pDRMRec->GetChanEst()->GetTimeSyncTrack()->GetTiSyncTracType())
	{
	case CTimeSyncTrack::TSFIRSTPEAK:
		if (!RadioButtonTiSyncFirstPeak->isChecked())
			RadioButtonTiSyncFirstPeak->setChecked(TRUE);
		break;

	case CTimeSyncTrack::TSENERGY:
		if (!RadioButtonTiSyncEnergy->isChecked())
			RadioButtonTiSyncEnergy->setChecked(TRUE);
		break;
	}

	/* Update settings checkbuttons */
	CheckBoxReverb->setChecked(pDRMRec->GetAudSorceDec()->GetReverbEffect());
	CheckBoxRecFilter->setChecked(pDRMRec->GetFreqSyncAcq()->GetRecFilter());
	CheckBoxModiMetric->setChecked(pDRMRec->GetChanEst()->GetIntCons());
	CheckBoxMuteAudio->setChecked(pDRMRec->GetWriteData()->GetMuteAudio());
	CheckBoxFlipSpec->
		setChecked(pDRMRec->GetReceiver()->GetFlippedSpectrum());

	CheckBoxSaveAudioWave->
		setChecked(pDRMRec->GetWriteData()->GetIsWriteWaveFile());


	/* Update frequency edit control (frequency could be changed by
	   schedule dialog */
	QString strFreq = EdtFrequency->text();
	const int iCurLogFreq =
		pDRMRec->GetParameters()->ReceptLog.GetFrequency();

	if (iCurLogFreq != iCurFrequency)
	{
		EdtFrequency->setText(QString().setNum(iCurLogFreq));
		iCurFrequency = iCurLogFreq;
	}
}

void systemevalDlg::showEvent(QShowEvent* pEvent)
{
	/* Restore chart windows */
	const int iNumChartWin = pDRMRec->GeomChartWindows.Size();

	for (int i = 0; i < iNumChartWin; i++)
	{
		/* Convert int to enum type. TODO: better solution for storing enum
		   types in init file! */
		const CDRMPlot::ECharType eNewType =
			(CDRMPlot::ECharType) pDRMRec->GeomChartWindows[i].iType;

		/* Open new chart window */
		CDRMPlot* pNewChartWin = OpenChartWin(eNewType);

		/* Add window pointer in vector (needed for closing the windows) */
		vecpDRMPlots.Add(pNewChartWin);

#ifdef _WIN32 /* This works only reliable under Windows :-( */
		/* Chart windows: get window geometry data from DRMReceiver module
		   and apply it */
		const QRect WinGeom(pDRMRec->GeomChartWindows[i].iXPos,
			pDRMRec->GeomChartWindows[i].iYPos,
			pDRMRec->GeomChartWindows[i].iWSize,
			pDRMRec->GeomChartWindows[i].iHSize);

		if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
			pNewChartWin->setGeometry(WinGeom);
#else /* Under Linux only restore the size */
		resize(pDRMRec->GeomChartWindows[i].iWSize,
			pDRMRec->GeomChartWindows[i].iHSize);
#endif
	}

	/* Update controls */
	UpdateControls();
}

void systemevalDlg::hideEvent(QHideEvent* pEvent)
{
	/* Store size and position of all additional chart windows */
	pDRMRec->GeomChartWindows.Init(0);

	for (int i = 0; i < vecpDRMPlots.Size(); i++)
	{
		/* Check, if window wasn't closed by the user */
		if (vecpDRMPlots[i]->isVisible())
		{
			const QRect CWGeom = vecpDRMPlots[i]->geometry();

			/* Enlarge vector for storing parameters of new window */
			const int iOldSizeCWV = pDRMRec->GeomChartWindows.Size();
			pDRMRec->GeomChartWindows.Enlarge(1);

			/* Set parameters */
			pDRMRec->GeomChartWindows[iOldSizeCWV].iXPos = CWGeom.x();
			pDRMRec->GeomChartWindows[iOldSizeCWV].iYPos = CWGeom.y();
			pDRMRec->GeomChartWindows[iOldSizeCWV].iHSize = CWGeom.height();
			pDRMRec->GeomChartWindows[iOldSizeCWV].iWSize = CWGeom.width();

			/* Convert plot type into an integer type. TODO: better solution */
			pDRMRec->GeomChartWindows[iOldSizeCWV].iType =
				(int) vecpDRMPlots[i]->GetChartType();
		}

		/* Close window afterwards */
		vecpDRMPlots[i]->close();
	}

	/* We do not need the pointers anymore, reset vector */
	vecpDRMPlots.Init(0);
}

CDRMPlot* systemevalDlg::OpenChartWin(const CDRMPlot::ECharType eNewType)
{
	/* Create new chart window */
	CDRMPlot* pNewChartWin = new CDRMPlot(NULL);
	pNewChartWin->setCaption(tr("Chart Window"));

	/* Set color scheme */
	pNewChartWin->SetPlotStyle(pDRMRec->iMainPlotColorStyle);

	/* Set correct icon (use the same as this dialog) */
	pNewChartWin->setIcon(*this->icon());

	/* Set receiver object and correct chart type */
	pNewChartWin->SetRecObj(pDRMRec);
	pNewChartWin->SetupChart(eNewType);

	/* Show new window */
	pNewChartWin->show();

	return pNewChartWin;
}

void systemevalDlg::SetStatus(int MessID, int iMessPara)
{
	switch(MessID)
	{
	case MS_FAC_CRC:
		LEDFAC->SetLight(iMessPara);
		break;

	case MS_SDC_CRC:
		LEDSDC->SetLight(iMessPara);
		break;

	case MS_MSC_CRC:
		LEDMSC->SetLight(iMessPara);
		break;

	case MS_FRAME_SYNC:
		LEDFrameSync->SetLight(iMessPara);
		break;

	case MS_TIME_SYNC:
		LEDTimeSync->SetLight(iMessPara);
		break;

	case MS_IOINTERFACE:
		LEDIOInterface->SetLight(iMessPara);
		break;

	case MS_RESET_ALL:
		LEDFAC->Reset();
		LEDSDC->Reset();
		LEDMSC->Reset();
		LEDFrameSync->Reset();
		LEDTimeSync->Reset();
		LEDIOInterface->Reset();
		break;
	}
}

void systemevalDlg::OnTimer()
{
	_REAL rSNREstimate;
	_REAL rSigmaEst;

	/* Show SNR if receiver is in tracking mode */
	if (pDRMRec->GetReceiverState() == CDRMReceiver::AS_WITH_SIGNAL)
	{
		/* Get SNR value and use it if available and valid */
		if (pDRMRec->GetChanEst()->GetSNREstdB(rSNREstimate))
		{
			/* SNR */
			ValueSNR->setText("<b>" +
				QString().setNum(rSNREstimate, 'f', 1) + " dB</b>");

			/* MSC WMER / MER */
			ValueMERWMER->setText(QString().
				setNum(pDRMRec->GetChanEst()->GetMSCWMEREstdB(), 'f', 1) +
				" dB / " + QString().
				setNum(pDRMRec->GetChanEst()->GetMSCMEREstdB(), 'f', 1) +
				" dB");

			/* Set SNR for log file */
			pDRMRec->GetParameters()->ReceptLog.SetSNR(rSNREstimate);
		}
		else
		{
			ValueSNR->setText("<b>---</b>");
			ValueMERWMER->setText("<b>---</b>");
		}

		/* Doppler estimation (assuming Gaussian doppler spectrum) */
		if (pDRMRec->GetChanEst()->GetSigma(rSigmaEst))
		{
			/* Plot delay and Doppler values */
			ValueWiener->setText(
				QString().setNum(rSigmaEst, 'f', 2) + " Hz / " +
				QString().setNum(
				pDRMRec->GetChanEst()->GetMinDelay(), 'f', 2) + " ms");
		}
		else
		{
			/* Plot only delay, Doppler not available */
			ValueWiener->setText("--- / " + QString().setNum(
				pDRMRec->GetChanEst()->GetMinDelay(), 'f', 2) + " ms");
		}

		/* Sample frequency offset estimation */
		const _REAL rCurSamROffs = pDRMRec->GetParameters()->GetSampFreqEst();

		/* Display value in [Hz] and [ppm] (parts per million) */
		ValueSampFreqOffset->setText(
			QString().setNum(rCurSamROffs, 'f', 2) + " Hz (" +
			QString().setNum((int) (rCurSamROffs / SOUNDCRD_SAMPLE_RATE * 1e6))
			+ " ppm)");
	}
	else
	{
		ValueSNR->setText("<b>---</b>");
		ValueMERWMER->setText("<b>---</b>");
		ValueWiener->setText("--- / ---");
		ValueSampFreqOffset->setText("---");
	}

#ifdef _DEBUG_
	TextFreqOffset->setText("DC: " +
		QString().setNum(pDRMRec->GetParameters()->
		GetDCFrequency(), 'f', 3) + " Hz ");

	/* Metric values */
	ValueFreqOffset->setText(tr("Metrics [dB]: MSC: ") +
		QString().setNum(
		pDRMRec->GetMSCMLC()->GetAccMetric(), 'f', 2) +	"\nSDC: " +
		QString().setNum(
		pDRMRec->GetSDCMLC()->GetAccMetric(), 'f', 2) +	" / FAC: " +
		QString().setNum(
		pDRMRec->GetFACMLC()->GetAccMetric(), 'f', 2));
#else
	/* DC frequency */
	ValueFreqOffset->setText(QString().setNum(
		pDRMRec->GetParameters()->GetDCFrequency(), 'f', 2) + " Hz");
#endif

/* _WIN32 fix because in Visual c++ the GUI files are always compiled even
   if USE_QT_GUI is set or not (problem with MDI in DRMReceiver) */
#ifdef USE_QT_GUI
	/* If MDI in is enabled, do not show any synchronization parameter */
	if (pDRMRec->GetMDI()->GetMDIInEnabled() == TRUE)
	{
		ValueSNR->setText("<b>---</b>");
		ValueMERWMER->setText("<b>---</b>");
		ValueWiener->setText("--- / ---");
		ValueSampFreqOffset->setText("---");
		ValueFreqOffset->setText("---");
	}
#endif


	/* FAC info static ------------------------------------------------------ */
	QString strFACInfo;

	/* Robustness mode #################### */
	strFACInfo = GetRobModeStr() + " / " + GetSpecOccStr();

	FACDRMModeBWL->setText("DRM Mode / Bandwidth:"); /* Label */
	FACDRMModeBWV->setText(strFACInfo); /* Value */


	/* Interleaver Depth #################### */
	switch (pDRMRec->GetParameters()->eSymbolInterlMode)
	{
	case CParameter::SI_LONG:
		strFACInfo = tr("2 s (Long Interleaving)");
		break;

	case CParameter::SI_SHORT:
		strFACInfo = tr("400 ms (Short Interleaving)");
		break;
	}

	FACInterleaverDepthL->setText(tr("Interleaver Depth:")); /* Label */
	FACInterleaverDepthV->setText(strFACInfo); /* Value */


	/* SDC, MSC mode #################### */
	/* SDC */
	switch (pDRMRec->GetParameters()->eSDCCodingScheme)
	{
	case CParameter::CS_1_SM:
		strFACInfo = "4-QAM / ";
		break;

	case CParameter::CS_2_SM:
		strFACInfo = "16-QAM / ";
		break;
	}

	/* MSC */
	switch (pDRMRec->GetParameters()->eMSCCodingScheme)
	{
	case CParameter::CS_2_SM:
		strFACInfo += "SM 16-QAM";
		break;

	case CParameter::CS_3_SM:
		strFACInfo += "SM 64-QAM";
		break;

	case CParameter::CS_3_HMSYM:
		strFACInfo += "HMsym 64-QAM";
		break;

	case CParameter::CS_3_HMMIX:
		strFACInfo += "HMmix 64-QAM";
		break;
	}

	FACSDCMSCModeL->setText(tr("SDC / MSC Mode:")); /* Label */
	FACSDCMSCModeV->setText(strFACInfo); /* Value */


	/* Code rates #################### */
	strFACInfo = QString().setNum(pDRMRec->GetParameters()->MSCPrLe.iPartB);
	strFACInfo += " / ";
	strFACInfo += QString().setNum(pDRMRec->GetParameters()->MSCPrLe.iPartA);

	FACCodeRateL->setText(tr("Prot. Level (B / A):")); /* Label */
	FACCodeRateV->setText(strFACInfo); /* Value */


	/* Number of services #################### */
	strFACInfo = tr("Audio: ");
	strFACInfo += QString().setNum(pDRMRec->GetParameters()->iNumAudioService);
	strFACInfo += tr(" / Data: ");
	strFACInfo +=QString().setNum(pDRMRec->GetParameters()->iNumDataService);

	FACNumServicesL->setText(tr("Number of Services:")); /* Label */
	FACNumServicesV->setText(strFACInfo); /* Value */


	/* Time, date #################### */
	if ((pDRMRec->GetParameters()->iUTCHour == 0) &&
		(pDRMRec->GetParameters()->iUTCMin == 0) &&
		(pDRMRec->GetParameters()->iDay == 0) &&
		(pDRMRec->GetParameters()->iMonth == 0) &&
		(pDRMRec->GetParameters()->iYear == 0))
	{
		/* No time service available */
		strFACInfo = tr("Service not available");
	}
	else
	{
#ifdef GUI_QT_DATE_TIME_TYPE
		/* QT type of displaying date and time */
		QDateTime DateTime;
		DateTime.setDate(QDate(pDRMRec->GetParameters()->iYear,
			pDRMRec->GetParameters()->iMonth,
			pDRMRec->GetParameters()->iDay));
		DateTime.setTime(QTime(pDRMRec->GetParameters()->iUTCHour,
			pDRMRec->GetParameters()->iUTCMin));

		strFACInfo = DateTime.toString();
#else
		/* Set time and date */
		QString strMin;
		const int iMin = pDRMRec->GetParameters()->iUTCMin;

		/* Add leading zero to number smaller than 10 */
		if (iMin < 10)
			strMin = "0";
		else
			strMin = "";

		strMin += QString().setNum(iMin);

		strFACInfo =
			/* Time */
			QString().setNum(pDRMRec->GetParameters()->iUTCHour) + ":" +
			strMin + "  -  " +
			/* Date */
			QString().setNum(pDRMRec->GetParameters()->iMonth) + "/" +
			QString().setNum(pDRMRec->GetParameters()->iDay) + "/" +
			QString().setNum(pDRMRec->GetParameters()->iYear);
#endif
	}

	FACTimeDateL->setText(tr("Received time - date:")); /* Label */
	FACTimeDateV->setText(strFACInfo); /* Value */


	/* Update controls */
	UpdateControls();
}

void systemevalDlg::OnRadioTimeLinear() 
{
	if (pDRMRec->GetChanEst()->GetTimeInt() != CChannelEstimation::TLINEAR)
		pDRMRec->GetChanEst()->SetTimeInt(CChannelEstimation::TLINEAR);
}

void systemevalDlg::OnRadioTimeWiener() 
{
	if (pDRMRec->GetChanEst()->GetTimeInt() != CChannelEstimation::TWIENER)
		pDRMRec->GetChanEst()->SetTimeInt(CChannelEstimation::TWIENER);
}

void systemevalDlg::OnRadioFrequencyLinear() 
{
	if (pDRMRec->GetChanEst()->GetFreqInt() != CChannelEstimation::FLINEAR)
		pDRMRec->GetChanEst()->SetFreqInt(CChannelEstimation::FLINEAR);
}

void systemevalDlg::OnRadioFrequencyDft() 
{
	if (pDRMRec->GetChanEst()->GetFreqInt() != CChannelEstimation::FDFTFILTER)
		pDRMRec->GetChanEst()->SetFreqInt(CChannelEstimation::FDFTFILTER);
}

void systemevalDlg::OnRadioFrequencyWiener() 
{
	if (pDRMRec->GetChanEst()->GetFreqInt() != CChannelEstimation::FWIENER)
		pDRMRec->GetChanEst()->SetFreqInt(CChannelEstimation::FWIENER);
}

void systemevalDlg::OnRadioTiSyncFirstPeak() 
{
	if (pDRMRec->GetChanEst()->GetTimeSyncTrack()->GetTiSyncTracType() != 
		CTimeSyncTrack::TSFIRSTPEAK)
	{
		pDRMRec->GetChanEst()->GetTimeSyncTrack()->
			SetTiSyncTracType(CTimeSyncTrack::TSFIRSTPEAK);
	}
}

void systemevalDlg::OnRadioTiSyncEnergy() 
{
	if (pDRMRec->GetChanEst()->GetTimeSyncTrack()->GetTiSyncTracType() != 
		CTimeSyncTrack::TSENERGY)
	{
		pDRMRec->GetChanEst()->GetTimeSyncTrack()->
			SetTiSyncTracType(CTimeSyncTrack::TSENERGY);
	}
}

void systemevalDlg::OnSliderIterChange(int value)
{
	/* Set new value in working thread module */
	pDRMRec->GetMSCMLC()->SetNumIterations(value);

	/* Show the new value in the label control */
	TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
		QString().setNum(value));
}

void systemevalDlg::OnListSelChanged(QListViewItem* NewSelIt)
{
	/* Get char type from selected item and setup chart */
	MainPlot->SetupChart(((CCharSelItem*) NewSelIt)->GetCharType());
}

void systemevalDlg::OnListRightButClicked(QListViewItem* NewSelIt,
										  const QPoint& iPnt, int iCol)
{
	/* Make sure that list item is valid */
	if (NewSelIt != NULL)
	{
		/* Show menu at mouse position only if selectable item was chosen */
		if (NewSelIt->isSelectable())
			pListViewContextMenu->exec(QCursor::pos());
	}
}

void systemevalDlg::OnListViContMenu()
{
	/* Get chart type from current selected list view item */
	QListViewItem* pCurSelLVItem = ListViewCharSel->selectedItem();

	if (pCurSelLVItem != NULL)
	{
		/* Open new chart window and add window pointer in vector
		   (needed for closing the windows) */
		vecpDRMPlots.Add(
			OpenChartWin(((CCharSelItem*) pCurSelLVItem)->GetCharType()));
	}
}

void systemevalDlg::OnCheckFlipSpectrum()
{
	/* Set parameter in working thread module */
	pDRMRec->GetReceiver()->
		SetFlippedSpectrum(CheckBoxFlipSpec->isChecked());
}

void systemevalDlg::OnCheckRecFilter()
{
	/* Set parameter in working thread module */
	pDRMRec->GetFreqSyncAcq()->
		SetRecFilter(CheckBoxRecFilter->isChecked());

	/* If filter status is changed, a new aquisition is necessary */
	pDRMRec->SetReceiverMode(CDRMReceiver::RM_DRM);
}

void systemevalDlg::OnCheckModiMetric()
{
	/* Set parameter in working thread module */
	pDRMRec->GetChanEst()->SetIntCons(CheckBoxModiMetric->isChecked());
}

void systemevalDlg::OnCheckBoxMuteAudio()
{
	/* Set parameter in working thread module */
	pDRMRec->GetWriteData()->MuteAudio(CheckBoxMuteAudio->isChecked());
}

void systemevalDlg::OnCheckBoxReverb()
{
	/* Set parameter in working thread module */
	pDRMRec->GetAudSorceDec()->SetReverbEffect(CheckBoxReverb->isChecked());
}

void systemevalDlg::OnCheckSaveAudioWAV()
{
/*
	This code is copied in AnalogDemDlg.cpp. If you do changes here, you should
	apply the changes in the other file, too
*/
	if (CheckBoxSaveAudioWave->isChecked() == TRUE)
	{
		/* Show "save file" dialog */
		QString strFileName =
			QFileDialog::getSaveFileName(tr("DreamOut.wav"), "*.wav", this);

		/* Check if user not hit the cancel button */
		if (!strFileName.isNull())
		{
			pDRMRec->GetWriteData()->
				StartWriteWaveFile(strFileName.latin1());
		}
		else
		{
			/* User hit the cancel button, uncheck the button */
			CheckBoxSaveAudioWave->setChecked(FALSE);
		}
	}
	else
		pDRMRec->GetWriteData()->StopWriteWaveFile();
}

void systemevalDlg::OnTimerLogFileStart()
{
	/* Start logging (if not already done) */
	if (!CheckBoxWriteLog->isChecked())
	{
		CheckBoxWriteLog->setChecked(TRUE);
		OnCheckWriteLog();
	}
}

void systemevalDlg::OnCheckWriteLog()
{
	if (CheckBoxWriteLog->isChecked())
	{
		/* Activte log file timer for long and short log file */
		TimerLogFileShort.start(60000); /* Every minute (i.e. 60000 ms) */
		TimerLogFileLong.start(1000); /* Every second */

		/* Get frequency from front-end edit control */
		QString strFreq = EdtFrequency->text();
		iCurFrequency = strFreq.toUInt();
		pDRMRec->GetParameters()->ReceptLog.SetFrequency(iCurFrequency);

		/* Set some other information obout this receiption */
		QString strAddText = "";

		/* Check if receiver does receive a DRM signal */
		if ((pDRMRec->GetReceiverState() == CDRMReceiver::AS_WITH_SIGNAL) &&
			(pDRMRec->GetReceiverMode() == CDRMReceiver::RM_DRM))
		{
			/* First get current selected audio service */
			int iCurSelServ =
				pDRMRec->GetParameters()->GetCurSelAudioService();

			/* Check whether service parameters were not transmitted yet */
			if (pDRMRec->GetParameters()->Service[iCurSelServ].IsActive())
			{
				strAddText = tr("Label            ");

				/* Service label (UTF-8 encoded string -> convert) */
				strAddText += QString().fromUtf8(QCString(
					pDRMRec->GetParameters()->Service[iCurSelServ].
					strLabel.c_str()));

				strAddText += tr("\nBitrate          ");

				strAddText += QString().setNum(pDRMRec->GetParameters()->
					GetBitRateKbps(iCurSelServ, FALSE), 'f', 2) + " kbps";

				strAddText += tr("\nMode             ") + GetRobModeStr();
				strAddText += tr("\nBandwidth        ") + GetSpecOccStr();
			}
		}

		/* Set additional text for log file. Conversion from QString to STL
		   string is needed (done with .latin1() function of QT string) */
		string strTemp = strAddText.latin1();
		pDRMRec->GetParameters()->ReceptLog.SetAdditText(strTemp);

		/* Set current transmission parameters. TDOD: better solution */
		pDRMRec->GetParameters()->ReceptLog.
			SetRobMode(pDRMRec->GetParameters()->GetWaveMode());
		pDRMRec->GetParameters()->ReceptLog.
			SetMSCScheme(pDRMRec->GetParameters()->eMSCCodingScheme);
		pDRMRec->GetParameters()->ReceptLog.
			SetProtLev(pDRMRec->GetParameters()->MSCPrLe);

		/* Activate log file */
		pDRMRec->GetParameters()->ReceptLog.SetLog(TRUE);
	}
	else
	{
		/* Deactivate log file timer */
		TimerLogFileShort.stop();
		TimerLogFileLong.stop();

		pDRMRec->GetParameters()->ReceptLog.SetLog(FALSE);
	}
}

void systemevalDlg::OnTimerLogFileShort()
{
	/* Write new parameters in log file (short version) */
	pDRMRec->GetParameters()->ReceptLog.WriteParameters(FALSE);
}

void systemevalDlg::OnTimerLogFileLong()
{
	/* Write new parameters in log file (long version) */
	pDRMRec->GetParameters()->ReceptLog.WriteParameters(TRUE);
}

QString	systemevalDlg::GetRobModeStr()
{
	switch (pDRMRec->GetParameters()->GetWaveMode())
	{
	case RM_ROBUSTNESS_MODE_A:
		return "A";
		break;

	case RM_ROBUSTNESS_MODE_B:
		return "B";
		break;

	case RM_ROBUSTNESS_MODE_C:
		return "C";
		break;

	case RM_ROBUSTNESS_MODE_D:
		return "D";
		break;

	default:
		return "A";
	}
}

QString	systemevalDlg::GetSpecOccStr()
{
	switch (pDRMRec->GetParameters()->GetSpectrumOccup())
	{
	case SO_0:
		return "4,5 kHz";
		break;

	case SO_1:
		return "5 kHz";
		break;

	case SO_2:
		return "9 kHz";
		break;

	case SO_3:
		return "10 kHz";
		break;

	case SO_4:
		return "18 kHz";
		break;

	case SO_5:
		return "20 kHz";
		break;

	default:
		return "10 kHz";
	}
}

void systemevalDlg::AddWhatsThisHelp()
{
/*
	This text was taken from the only documentation of Dream software
*/
	/* DC Frequency Offset */
	const QString strDCFreqOffs =
		tr("<b>DC Frequency Offset:</b> This is the "
		"estimation of the DC frequency offset. This offset corresponds "
		"to the resulting sound card intermedia frequency of the front-end. "
		"This frequency is not restricted to a certain value. The only "
		"restriction is that the DRM spectrum must be completely inside the "
		"bandwidth of the sound card.");

	QWhatsThis::add(TextFreqOffset, strDCFreqOffs);
	QWhatsThis::add(ValueFreqOffset, strDCFreqOffs);

	/* Sample Frequency Offset */
	const QString strFreqOffset =
		tr("<b>Sample Frequency Offset:</b> This is the "
		"estimation of the sample rate offset between the sound card sample "
		"rate of the local computer and the sample rate of the D / A (digital "
		"to analog) converter in the transmitter. Usually the sample rate "
		"offset is very constant for a given sound card. Therefore it is "
		"useful to inform the Dream software about this value at application "
		"startup to increase the acquisition speed and reliability.");

	QWhatsThis::add(TextSampFreqOffset, strFreqOffset);
	QWhatsThis::add(ValueSampFreqOffset, strFreqOffset);

	/* Doppler / Delay */
	const QString strDopplerDelay =
		tr("<b>Doppler / Delay:</b> The Doppler frequency "
		"of the channel is estimated for the Wiener filter design of channel "
		"estimation in time direction. If linear interpolation is set for "
		"channel estimation in time direction, this estimation is not updated. "
		"The Doppler frequency is an indication of how fast the channel varies "
		"with time. The higher the frequency, the faster the channel changes "
		"are.<br>The total delay of the Power Delay Spectrum "
		"(PDS) is estimated from the impulse response estimation derived from "
		"the channel estimation. This delay corresponds to the range between "
		"the two vertical dashed black lines in the Impulse Response (IR) "
		"plot.");

	QWhatsThis::add(TextWiener, strDopplerDelay);
	QWhatsThis::add(ValueWiener, strDopplerDelay);

	/* I / O Interface LED */
	const QString strLEDIOInterface =
		tr("<b>I / O Interface LED:</b> This LED shows the "
		"current status of the sound card interface. The yellow light shows "
		"that the audio output was corrected. Since the sample rate of the "
		"transmitter and local computer are different, from time to time the "
		"audio buffers will overflow or under run and a correction is "
		"necessary. When a correction occurs, a \"click\" sound can be heard. "
		"The red light shows that a buffer was lost in the sound card input "
		"stream. This can happen if a thread with a higher priority is at "
		"100% and the Dream software cannot read the provided blocks fast "
		"enough. In this case, the Dream software will instantly loose the "
		"synchronization and has to re-synchronize. Another reason for red "
		"light is that the processor is too slow for running the Dream "
		"software.");

	QWhatsThis::add(TextLabelLEDIOInterface, strLEDIOInterface);
	QWhatsThis::add(LEDIOInterface, strLEDIOInterface);

	/* Time Sync Acq LED */
	const QString strLEDTimeSyncAcq =
		tr("<b>Time Sync Acq LED:</b> This LED shows the "
		"state of the timing acquisition (search for the beginning of an OFDM "
		"symbol). If the acquisition is done, this LED will stay green.");

	QWhatsThis::add(TextLabelLEDTimeSyncAcq, strLEDTimeSyncAcq);
	QWhatsThis::add(LEDTimeSync, strLEDTimeSyncAcq);

	/* Frame Sync LED */
	const QString strLEDFrameSync =
		tr("<b>Frame Sync LED:</b> The DRM frame "
		"synchronization status is shown with this LED. This LED is also only "
		"active during acquisition state of the Dream receiver. In tracking "
		"mode, this LED is always green.");

	QWhatsThis::add(TextLabelLEDFrameSync, strLEDFrameSync);
	QWhatsThis::add(LEDFrameSync, strLEDFrameSync);

	/* FAC CRC LED */
	const QString strLEDFACCRC =
		tr("<b>FAC CRC LED:</b> This LED shows the Cyclic "
		"Redundancy Check (CRC) of the Fast Access Channel (FAC) of DRM. FAC "
		"is one of the three logical channels and is always modulated with a "
		"4-QAM. If the FAC CRC check was successful, the receiver changes to "
		"tracking mode. The FAC LED is the indication whether the receiver "
		"is synchronized to a DRM transmission or not.<br>"
		"The bandwidth of the DRM signal, the constellation scheme of MSC and "
		"SDC channels and the interleaver depth are some of the parameters "
		"which are provided by the FAC.");

	QWhatsThis::add(TextLabelLEDFACCRC, strLEDFACCRC);
	QWhatsThis::add(LEDFAC, strLEDFACCRC);

	/* SDC CRC LED */
	const QString strLEDSDCCRC =
		tr("<b>SDC CRC LED:</b> This LED shows the CRC "
		"check result of the Service Description Channel (SDC) which is one "
		"logical channel of the DRM stream. This data is transmitted in "
		"approx. 1 second intervals and contains information about station "
		"label, audio and data format, etc. The error protection is normally "
		"lower than the protection of the FAC. Therefore this LED will turn "
		"to red earlier than the FAC LED in general.<br>If the CRC check "
		"is ok but errors in the content were detected, the LED turns "
		"yellow.");

	QWhatsThis::add(TextLabelLEDSDCCRC, strLEDSDCCRC);
	QWhatsThis::add(LEDSDC, strLEDSDCCRC);

	/* MSC CRC LED */
	const QString strLEDMSCCRC =
		tr("<b>MSC CRC LED:</b> This LED shows the status "
		"of the Main Service Channel (MSC). This channel contains the actual "
		"audio and data bits. The LED shows the CRC check of the AAC core "
		"decoder. The SBR has a separate CRC, but this status is not shown "
		"with this LED. If SBR CRC is wrong but the AAC CRC is ok one can "
		"still hear something (of course, the high frequencies are not there "
		"in this case). If this LED turns red, interruptions of the audio are "
		"heard. The yellow light shows that only one 40 ms audio frame CRC "
		"was wrong. This causes usually no hearable artifacts.");

	QWhatsThis::add(TextLabelLEDMSCCRC, strLEDMSCCRC);
	QWhatsThis::add(LEDMSC, strLEDMSCCRC);

	/* MLC, Number of Iterations */
	const QString strNumOfIterations =
		tr("<b>MLC, Number of Iterations:</b> In DRM, a "
		"multilevel channel coder is used. With this code it is possible to "
		"iterate the decoding process in the decoder to improve the decoding "
		"result. The more iterations are used the better the result will be. "
		"But switching to more iterations will increase the CPU load. "
		"Simulations showed that the first iteration (number of "
		"iterations = 1) gives the most improvement (approx. 1.5 dB at a "
		"BER of 10-4 on a Gaussian channel, Mode A, 10 kHz bandwidth). The "
		"improvement of the second iteration will be as small as 0.3 dB."
		"<br>The recommended number of iterations given in the DRM "
		"standard is one iteration (number of iterations = 1).");

	QWhatsThis::add(TextNumOfIterations, strNumOfIterations);
	QWhatsThis::add(SliderNoOfIterations, strNumOfIterations);

	/* Flip Input Spectrum */
	QWhatsThis::add(CheckBoxFlipSpec,
		tr("<b>Flip Input Spectrum:</b> Checking this box "
		"will flip or invert the input spectrum. This is necessary if the "
		"mixer in the front-end uses the lower side band."));

	/* Mute Audio */
	QWhatsThis::add(CheckBoxMuteAudio,
		tr("<b>Mute Audio:</b> The audio can be muted by "
		"checking this box. The reaction of checking or unchecking this box "
		"is delayed by approx. 1 second due to the audio buffers."));

	/* Reverberation Effect */
	QWhatsThis::add(CheckBoxReverb,
		tr("<b>Reverberation Effect:</b> If this check box is checked, a "
		"reverberation effect is applied each time an audio drop-out occurs. "
		"With this effect it is possible to mask short drop-outs."));

	/* Log File */
	QWhatsThis::add(CheckBoxWriteLog,
		tr("<b>Log File:</b> Checking this box brings the "
		"Dream software to write a log file about the current reception. "
		"Every minute the average SNR, number of correct decoded FAC and "
		"number of correct decoded MSC blocks are logged including some "
		"additional information, e.g. the station label and bit-rate. The "
		"log mechanism works only for audio services using AAC source coding. "
#ifdef _WIN32
		"During the logging no Dream windows "
		"should be moved or re-sized. This can lead to incorrect log files "
		"(problem with QT timer implementation under Windows). This problem "
		"does not exist in the Linux version of Dream."
#endif
		"<br>The log file will be "
		"written in the directory were the Dream application was started and "
		"the name of this file is always DreamLog.txt"));

	/* Freq */
	QWhatsThis::add(EdtFrequency,
		tr("<b>Freq:</b> In this edit control, the current "
		"selected frequency on the front-end can be specified. This frequency "
		"will be written into the log file."));

	/* Wiener */
	const QString strWienerChanEst =
		tr("<b>Channel Estimation Settings:</b> With these "
		"settings, the channel estimation method in time and frequency "
		"direction can be selected. The default values use the most powerful "
		"algorithms. For more detailed information about the estimation "
		"algorithms there are a lot of papers and books available.<br>"
		"<b>Wiener:</b> Wiener interpolation method "
		"uses estimation of the statistics of the channel to design an optimal "
		"filter for noise reduction.");

	QWhatsThis::add(RadioButtonFreqWiener, strWienerChanEst);
	QWhatsThis::add(RadioButtonTiWiener, strWienerChanEst);

	/* Linear */
	const QString strLinearChanEst =
		tr("<b>Channel Estimation Settings:</b> With these "
		"settings, the channel estimation method in time and frequency "
		"direction can be selected. The default values use the most powerful "
		"algorithms. For more detailed information about the estimation "
		"algorithms there are a lot of papers and books available.<br>"
		"<b>Linear:</b> Simple linear interpolation "
		"method to get the channel estimate. The real and imaginary parts "
		"of the estimated channel at the pilot positions are linearly "
		"interpolated. This algorithm causes the lowest CPU load but "
		"performs much worse than the Wiener interpolation at low SNRs.");

	QWhatsThis::add(RadioButtonFreqLinear, strLinearChanEst);
	QWhatsThis::add(RadioButtonTiLinear, strLinearChanEst);

	/* DFT Zero Pad */
	QWhatsThis::add(RadioButtonFreqDFT,
		tr("<b>Channel Estimation Settings:</b> With these "
		"settings, the channel estimation method in time and frequency "
		"direction can be selected. The default values use the most powerful "
		"algorithms. For more detailed information about the estimation "
		"algorithms there are a lot of papers and books available.<br>"
		"<b>DFT Zero Pad:</b> Channel estimation method "
		"for the frequency direction using Discrete Fourier Transformation "
		"(DFT) to transform the channel estimation at the pilot positions to "
		"the time domain. There, a zero padding is applied to get a higher "
		"resolution in the frequency domain -> estimates at the data cells. "
		"This algorithm is very speed efficient but has problems at the edges "
		"of the OFDM spectrum due to the leakage effect."));

	/* Guard Energy */
	QWhatsThis::add(RadioButtonTiSyncEnergy,
		tr("<b>Guard Energy:</b> Time synchronization "
		"tracking algorithm utilizes the estimation of the impulse response. "
		"This method tries to maximize the energy in the guard-interval to set "
		"the correct timing."));

	/* First Peak */
	QWhatsThis::add(RadioButtonTiSyncFirstPeak,
		tr("<b>First Peak:</b> This algorithms searches for "
		"the first peak in the estimated impulse response and moves this peak "
		"to the beginning of the guard-interval (timing tracking algorithm)."));

	/* SNR */
	const QString strSNREst =
		tr("<b>SNR:</b> Signal to Noise Ratio (SNR) "
		"estimation based on FAC cells. Since the FAC cells are only "
		"located approximately in the region 0-5 kHz relative to the DRM DC "
		"frequency, it may happen that the SNR value is very high "
		"although the DRM spectrum on the left side of the DRM DC frequency "
		"is heavily distorted or disturbed by an interferer so that the true "
		"overall SNR is lower as indicated by the SNR value. Similarly, "
		"the SNR value might show a very low value but audio can still be "
		"decoded if only the right side of the DRM spectrum is degraded "
		"by an interferer.");

	QWhatsThis::add(ValueSNR, strSNREst);
	QWhatsThis::add(TextSNRText, strSNREst);

	/* MSC WMER / MSC MER */
	const QString strMERWMEREst =
		tr("<b>MSC WMER / MSC MER:</b> Modulation Error Ratio (MER) and "
		"weighted MER (WMER) calculated on the MSC cells is shown. The MER is "
		"calculated as follows: For each equalized MSC cell (only MSC cells, "
		"no FAC cells, no SDC cells, no pilot cells), the error vector from "
		"the nearest ideal point of the constellation diagram is measured. The "
		"squared magnitude of this error is found, and a mean of the squared "
		"errors is calculated (over one frame). The MER is the ratio in [dB] "
		"of the mean of the squared magnitudes of the ideal points of the "
		"constellation diagram to the mean squared error. This gives an "
		"estimate of the ratio of the total signal power to total noise "
		"power at the input to the equalizer for channels with flat frequency "
		"response.<br> In case of the WMER, the calculations of the means are "
		"multiplied by the squared magnitude of the estimated channel "
		"response.<br>For more information see ETSI TS 102 349.");

	QWhatsThis::add(ValueMERWMER, strMERWMEREst);
	QWhatsThis::add(TextMERWMER, strMERWMEREst);

	/* DRM Mode / Bandwidth */
	const QString strRobustnessMode =
		tr("<b>DRM Mode / Bandwidth:</b> In a DRM system, "
		"four possible robustness modes are defined to adapt the system to "
		"different channel conditions. According to the DRM standard:<ul>"
		"<li><i>Mode A:</i> Gaussian channels, with "
		"minor fading</li><li><i>Mode B:</i> Time "
		"and frequency selective channels, with longer delay spread</li>"
		"<li><i>Mode C:</i> As robustness mode B, but "
		"with higher Doppler spread</li>"
		"<li><i>Mode D:</i> As robustness mode B, but "
		"with severe delay and Doppler spread</li></ul>The "
		"bandwith is the gross bandwidth of the current DRM signal");

	QWhatsThis::add(FACDRMModeBWL, strRobustnessMode);
	QWhatsThis::add(FACDRMModeBWV, strRobustnessMode);

	/* Interleaver Depth */
	const QString strInterleaver =
		tr("<b>Interleaver Depth:</b> The symbol "
		"interleaver depth can be either short (approx. 400 ms) or long "
		"(approx. 2 s). The longer the interleaver the better the channel "
		"decoder can correct errors from slow fading signals. But the "
		"longer the interleaver length the longer the delay until (after a "
		"re-synchronization) audio can be heard.");

	QWhatsThis::add(FACInterleaverDepthL, strInterleaver);
	QWhatsThis::add(FACInterleaverDepthV, strInterleaver);

	/* SDC / MSC Mode */
	const QString strSDCMSCMode =
		tr("<b>SDC / MSC Mode:</b> Shows the modulation "
		"type of the SDC and MSC channel. For the MSC channel, some "
		"hierarchical modes are defined which can provide a very strong "
		"protected service channel.");

	QWhatsThis::add(FACSDCMSCModeL, strSDCMSCMode);
	QWhatsThis::add(FACSDCMSCModeV, strSDCMSCMode);

	/* Prot. Level (B/A) */
	const QString strProtLevel =
		tr("<b>Prot. Level (B/A):</b> The error protection "
		"level of the channel coder. For 64-QAM, there are four protection "
		"levels defined in the DRM standard. Protection level 0 has the "
		"highest protection whereas level 3 has the lowest protection. The "
		"letters A and B are the names of the higher and lower protected parts "
		"of a DRM block when Unequal Error Protection (UEP) is used. If Equal "
		"Error Protection (EEP) is used, only the protection level of part B "
		"is valid.");

	QWhatsThis::add(FACCodeRateL, strProtLevel);
	QWhatsThis::add(FACCodeRateV, strProtLevel);

	/* Number of Services */
	const QString strNumServices =
		tr("<b>Number of Services:</b> This shows the "
		"number of audio and data services transmitted in the DRM stream. "
		"The maximum number of streams is four.");

	QWhatsThis::add(FACNumServicesL, strNumServices);
	QWhatsThis::add(FACNumServicesV, strNumServices);

	/* Received time - date */
	const QString strTimeDate =
		tr("<b>Received time - date:</b> This label shows "
		"the received time and date in UTC. This information is carried in "
		"the SDC channel.");

	QWhatsThis::add(FACTimeDateL, strTimeDate);
	QWhatsThis::add(FACTimeDateV, strTimeDate);

	/* Save audio as wave */
	QWhatsThis::add(CheckBoxSaveAudioWave,
		tr("<b>Save Audio as WAV:</b> Save the audio signal "
		"as stereo, 16-bit, 48 kHz sample rate PCM wave file. Checking this "
		"box will let the user choose a file name for the recording."));

	/* Chart Selector */
	QWhatsThis::add(ListViewCharSel,
		tr("<b>Chart Selector:</b> With the chart selector "
		"different types of graphical display of parameters and receiver "
		"states can be chosen. The different plot types are sorted in "
		"different groups. To open a group just double-click on the group or "
		"click on the plus left of the group name. After clicking on an item "
		"it is possible to choose other items by using the up / down arrow "
		"keys. With these keys it is also possible to open and close the "
		"groups by using the left / right arrow keys.<br>A separate chart "
		"window for a selected item can be opened by right click on the item "
		"and click on the context menu item."));

	/* Interferer Rejection */
	const QString strInterfRej =
		tr("<b>Interferer Rejection:</b> There are two "
		"algorithms available to reject interferers:<ul>"
		"<li><b>Bandpass Filter (BP-Filter):</b>"
		" The bandpass filter is designed to have the same bandwidth as "
		"the DRM signal. If, e.g., a strong signal is close to the border "
		"of the actual DRM signal, under some conditions this signal will "
		"produce interference in the useful bandwidth of the DRM signal "
		"although it is not on the same frequency as the DRM signal. "
		"The reason for that behaviour lies in the way the OFDM "
		"demodulation is done. Since OFDM demodulation is a block-wise "
		"operation, a windowing has to be applied (which is rectangular "
		"in case of OFDM). As a result, the spectrum of a signal is "
		"convoluted with a Sinc function in the frequency domain. If a "
		"sinusoidal signal close to the border of the DRM signal is "
		"considered, its spectrum will not be a distinct peak but a "
		"shifted Sinc function. So its spectrum is broadened caused by "
		"the windowing. Thus, it will spread in the DRM spectrum and "
		"act as an in-band interferer.<br>"
		"There is a special case if the sinusoidal signal is in a "
		"distance of a multiple of the carrier spacing of the DRM signal. "
		"Since the Sinc function has zeros at certain positions it happens "
		"that in this case the zeros are exactly at the sub-carrier "
		"frequencies of the DRM signal. In this case, no interference takes "
		"place. If the sinusoidal signal is in a distance of a multiple of "
		"the carrier spacing plus half of the carrier spacing away from the "
		"DRM signal, the interference reaches its maximum.<br>"
		"As a result, if only one DRM signal is present in the 20 kHz "
		"bandwidth, bandpass filtering has no effect. Also,  if the "
		"interferer is far away from the DRM signal, filtering will not "
		"give much improvement since the squared magnitude of the spectrum "
		"of the Sinc function is approx -15 dB down at 1 1/2 carrier "
		"spacing (approx 70 Hz with DRM mode B) and goes down to approx "
		"-30 dB at 10 times the carrier spacing plus 1 / 2 of the carrier "
		"spacing (approx 525 Hz with DRM mode B). The bandpass filter must "
		"have very sharp edges otherwise the gain in performance will be "
		"very small.</li>"
		"<li><b>Modified Metrics:</b> Based on the "
		"information from the SNR versus sub-carrier estimation, the metrics "
		"for the Viterbi decoder can be modified so that sub-carriers with "
		"high noise are attenuated and do not contribute too much to the "
		"decoding result. That can improve reception under bad conditions but "
		"may worsen the reception in situations where a lot of fading happens "
		"and no interferer are present since the SNR estimation may be "
		"not correct.</li></ul>");

	QWhatsThis::add(GroupBoxInterfRej, strInterfRej);
	QWhatsThis::add(CheckBoxRecFilter, strInterfRej);
	QWhatsThis::add(CheckBoxModiMetric, strInterfRej);
}
