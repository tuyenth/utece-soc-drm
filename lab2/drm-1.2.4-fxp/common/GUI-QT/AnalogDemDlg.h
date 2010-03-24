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
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qslider.h>
#include <qwt_dial.h>
#include <qwt_dial_needle.h>
#include <qlayout.h>

#ifdef _WIN32
# include "../../Windows/moc/AnalogDemDlgbase.h"
#else
# include "moc/AnalogDemDlgbase.h"
#endif
#include "DialogUtil.h"
#include "DRMPlot.h"
#include "../GlobalDefinitions.h"
#include "../util/Vector.h"
#include "../DrmReceiver.h"


/* Definitions ****************************************************************/
/* Update time of PLL phase dial control */
#define PLL_PHASE_DIAL_UPDATE_TIME				100


/* Classes ********************************************************************/
class AnalogDemDlg : public AnalogDemDlgBase
{
	Q_OBJECT

public:
	AnalogDemDlg(CDRMReceiver* pNDRMR, QWidget* parent = 0,
		const char* name = 0, bool modal = FALSE, WFlags f = 0);

	virtual ~AnalogDemDlg() {}

protected:
	CDRMReceiver*	pDRMRec;

	QTimer			Timer;
	QTimer			TimerPLLPhaseDial;
	void			UpdateControls();
	void			AddWhatsThisHelp();
    virtual void	showEvent(QShowEvent* pEvent) {UpdateControls();}
	virtual void	closeEvent(QCloseEvent* pEvent);

	int iBwAM;
	int iBwLSB;
	int iBwUSB;
	int iBwCW;
	int iBwFM;

public slots:
	void OnTimer();
	void OnTimerPLLPhaseDial();
	void OnRadioDemodulation(int iID);
	void OnRadioAGC(int iID);
	void OnCheckBoxMuteAudio();
	void OnCheckSaveAudioWAV();
	void OnCheckAutoFreqAcq();
	void OnCheckPLL();
	void OnChartxAxisValSet(double dVal);
	void OnSliderBWChange(int value);
	void OnRadioNoiRed(int iID);
	void OnNewAMAcquisition() {pDRMRec->SetReceiverMode(CDRMReceiver::RM_AM);}
	void OnButtonWaterfall();

signals:
	void SwitchToDRM();
	void ViewStationsDlg();
	void Closed();
};
