/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2005
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

#if !defined(DIALOGUTIL_H__FD6B23452398345OIJ9453_804E1606C2AC__INCLUDED_)
#define DIALOGUTIL_H__FD6B23452398345OIJ9453_804E1606C2AC__INCLUDED_

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qevent.h>
#include <qtextview.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#ifdef _WIN32
# include "../../Windows/moc/AboutDlgbase.h"
#else
# include "moc/AboutDlgbase.h"
#endif
#include "../DrmReceiver.h"
#include "../util/Vector.h"


/* Classes ********************************************************************/
/* DRM events --------------------------------------------------------------- */
class DRMEvent : public QCustomEvent
{
public:
	DRMEvent(const int iNewMeTy, const int iNewSt) : 
		QCustomEvent(QEvent::User + 11), iMessType(iNewMeTy), iStatus(iNewSt) {}

	int iMessType;
	int iStatus;
};


/* About dialog ------------------------------------------------------------- */
class CAboutDlg : public CAboutDlgBase
{
	Q_OBJECT

public:
	CAboutDlg(QWidget* parent = 0, const char* name = 0, bool modal = FALSE,
		WFlags f = 0);
};


/* Help menu ---------------------------------------------------------------- */
class CDreamHelpMenu : public QPopupMenu
{
	Q_OBJECT

public:
	CDreamHelpMenu(QWidget* parent = 0);

protected:
	CAboutDlg AboutDlg;

public slots:
	void OnHelpWhatsThis() {QWhatsThis::enterWhatsThisMode();}
	void OnHelpAbout() {AboutDlg.exec();}
};


/* Sound card selection menu ------------------------------------------------ */
class CSoundCardSelMenu : public QPopupMenu
{
	Q_OBJECT

public:
	CSoundCardSelMenu(CSound* pNS, QWidget* parent = 0);

protected:
	CSound*		pSoundIF;
	int			iNumSoundDev;
	QPopupMenu*	pSoundInMenu;
	QPopupMenu*	pSoundOutMenu;

public slots:
	void OnSoundInDevice(int id);
	void OnSoundOutDevice(int id);
};


#endif // DIALOGUTIL_H__FD6B23452398345OIJ9453_804E1606C2AC__INCLUDED_
