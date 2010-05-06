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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qstring.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qwt_thermo.h>
#include <qevent.h>
#include <qcstring.h>
#include <qlayout.h>
#include <qwhatsthis.h>


#ifdef _WIN32
# include "../../Windows/moc/fdrmdialogbase.h"
#else
# include "moc/fdrmdialogbase.h"
#endif
#include "DialogUtil.h"
#include "systemevalDlg.h"
#include "MultimediaDlg.h"
#include "StationsDlg.h"
#include "AnalogDemDlg.h"
#include "MultColorLED.h"
#include "../DrmReceiver.h"
#include "../util/Vector.h"


/* Classes ********************************************************************/
class FDRMDialog : public FDRMDialogBase
{
	Q_OBJECT

public:
	FDRMDialog(CDRMReceiver* pNDRMR, QWidget* parent = 0, const char* name = 0,
		bool modal = FALSE,	WFlags f = 0);

	virtual ~FDRMDialog();

protected:
	CDRMReceiver*	pDRMRec;

	systemevalDlg*	pSysEvalDlg;
	MultimediaDlg*	pMultiMediaDlg;
	StationsDlg*	pStationsDlg;
	AnalogDemDlg*	pAnalogDemDlg;
	QMenuBar*		pMenu;
	QPopupMenu*		pReceiverModeMenu;
	QPopupMenu*		pSettingsMenu;
	int				iCurSelServiceGUI;
	int				iOldNoServicesGUI;
	QTimer			Timer;
	CAboutDlg		AboutDlg;

	_BOOLEAN		bSysEvalDlgWasVis;
	_BOOLEAN		bMultMedDlgWasVis;

	QString			SetServParamStr(int iServiceID);
	QString			SetBitrIDStr(int iServiceID);
	virtual void	customEvent(QCustomEvent* Event);
	void			SetService(int iNewServiceID);
	void			AddWhatsThisHelp();
	void			SetReceiverMode(const CDRMReceiver::ERecMode eNewReMo);

public slots:
	void OnTimer();
	void OnButtonService1();
	void OnButtonService2();
	void OnButtonService3();
	void OnButtonService4();
	void OnViewEvalDlg();
	void OnViewMultiMediaDlg();
	void OnViewStationsDlg();
	void OnSwitchToDRM() {SetReceiverMode(CDRMReceiver::RM_DRM);}
	void OnSwitchToAM() {SetReceiverMode(CDRMReceiver::RM_AM);}
};
