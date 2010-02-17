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

#include <qtimer.h>
#include <qstring.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qwt_thermo.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <qtooltip.h>
#include <qfiledialog.h>
#include <qwhatsthis.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qpopupmenu.h>

#ifdef _WIN32
# include "../../Windows/moc/systemevalDlgbase.h"
#else
# include "moc/systemevalDlgbase.h"
#endif
#include "DRMPlot.h"
#include "MultColorLED.h"
#include "../GlobalDefinitions.h"
#include "../util/Vector.h"
#include "../DrmReceiver.h"


/* Definitions ****************************************************************/
/* Define this macro if you prefer the QT-type of displaying date and time */
#define GUI_QT_DATE_TIME_TYPE


/* Classes ********************************************************************/
class systemevalDlg : public systemevalDlgBase
{
	Q_OBJECT

public:
	systemevalDlg(CDRMReceiver* pNDRMR, QWidget* parent = 0,
		const char* name = 0, bool modal = FALSE, WFlags f = 0);

	virtual ~systemevalDlg();

	void SetStatus(int MessID, int iMessPara);

protected:
	class CCharSelItem : public QListViewItem
	{
	public:
		CCharSelItem(QListView* parent, QString str1,
			CDRMPlot::ECharType eNewCharTy, _BOOLEAN bSelble = TRUE) :
			QListViewItem(parent, str1), eCharTy(eNewCharTy)
			{setSelectable(bSelble);}
		CCharSelItem(QListViewItem* parent, QString str1,
			CDRMPlot::ECharType eNewCharTy, _BOOLEAN bSelble = TRUE) :
			QListViewItem(parent, str1), eCharTy(eNewCharTy)
			{setSelectable(bSelble);}

		CDRMPlot::ECharType GetCharType() {return eCharTy;}

	protected:
		CDRMPlot::ECharType eCharTy;
	};

	CDRMReceiver*		pDRMRec;

	QTimer				Timer;
	QTimer				TimerLogFileLong;
	QTimer				TimerLogFileShort;
	QTimer				TimerLogFileStart;
	int					iCurFrequency;
    virtual void		showEvent(QShowEvent* pEvent);
	virtual void		hideEvent(QHideEvent* pEvent);
	void				UpdateControls();
	void				AddWhatsThisHelp();
	CDRMPlot*			OpenChartWin(const CDRMPlot::ECharType eNewType);

	QString				GetRobModeStr();
	QString				GetSpecOccStr();

	QPopupMenu*			pListViewContextMenu;
	CVector<CDRMPlot*>	vecpDRMPlots;

public slots:
	void OnTimer();
	void OnTimerLogFileLong();
	void OnTimerLogFileShort();
	void OnTimerLogFileStart();
	void OnRadioTimeLinear();
	void OnRadioTimeWiener();
	void OnRadioFrequencyLinear();
	void OnRadioFrequencyDft();
	void OnRadioFrequencyWiener();
	void OnRadioTiSyncFirstPeak();
	void OnRadioTiSyncEnergy();
	void OnSliderIterChange(int value);
	void OnCheckFlipSpectrum();
	void OnCheckBoxMuteAudio();
	void OnCheckBoxReverb();
	void OnCheckWriteLog();
	void OnCheckSaveAudioWAV();
	void OnCheckRecFilter();
	void OnCheckModiMetric();
	void OnListSelChanged(QListViewItem* NewSelIt);
	void OnListViContMenu();
	void OnListRightButClicked(QListViewItem* NewSelIt, const QPoint& iPnt,
		int iCol);
};
