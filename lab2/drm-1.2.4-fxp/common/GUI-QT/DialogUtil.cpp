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

#include "DialogUtil.h"


/* Implementation *************************************************************/
/* About dialog ------------------------------------------------------------- */
CAboutDlg::CAboutDlg(QWidget* parent, const char* name, bool modal, WFlags f)
	: CAboutDlgBase(parent, name, modal, f)
{
	/* Set the text for the about dialog html text control */
	TextViewCredits->setText(
		"<p>" /* General description of Dream software */
		"<big><b>Dream</b> " + tr("is a software implementation of a Digital "
		"Radio Mondiale (DRM) receiver. All what is needed to receive DRM "
		"transmissions is a PC with a sound card and a modified analog "
		"short-wave (MW, LW) receiver.") + "</big>"
		"</p><br>"
		"<p><font face=\"courier\">" /* GPL header text */
		"This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.<br>This program is distributed in "
		"the hope that it will be useful, but WITHOUT ANY WARRANTY; without "
		"even the implied warranty of MERCHANTABILITY or FITNESS FOR A "
		"PARTICULAR PURPOSE. See the GNU General Public License for more "
		"details.<br>You should have received a copy of the GNU General Public "
		"License along with his program; if not, write to the Free Software "
		"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 "
		"USA"
		"</font></p><br>" /* Our warning text */
		"<p><font color=\"#ff0000\" face=\"courier\">" +
		tr("Although this software is going to be "
		"distributed as free software under the terms of the GPL this does not "
		"mean that its use is free of rights of others. The use may infringe "
		"third party IP and thus may not be legal in some countries.") +
		"</font></p><br>"
		"<p>" /* Libraries used by this compilation of Dream */
		"<b>" + tr("This compilation of Dream uses the following libraries:") +
		"</b></p>"
		"<ul>"
		"<li><b>FFTW</b> <i>http://www.fftw.org</i></li>"
#ifdef USE_FAAD2_LIBRARY
		"<li><b>FAAD2</b> <i>AAC/HE-AAC/HE-AACv2/DRM decoder "
		"(c) Ahead Software, www.nero.com (http://faac.sf.net)</i></li>"
#endif
#ifdef USE_FAAC_LIBRARY
		"<li><b>FAAC</b> <i>http://faac.sourceforge.net</i></li>"
#endif
#ifdef USE_QT_GUI /* QWT */
		"<li><b>QWT</b> <i>Dream is based in part on the work of the Qwt "
		"project (http://qwt.sf.net).</i></li>"
#endif
#ifdef HAVE_LIBHAMLIB
		"<li><b>Hamlib</b> <i>http://hamlib.sourceforge.net</i></li>"
#endif
#ifdef HAVE_JOURNALINE
		"<li><b>FhG IIS Journaline Decoder</b> <i>Features NewsService "
		"Journaline(R) decoder technology by Fraunhofer IIS, Erlangen, "
		"Germany. For more information visit http://www.iis.fhg.de/dab</i></li>"
#endif
#ifdef HAVE_LIBFREEIMAGE
		"<li><b>FreeImage</b> <i>This software uses the FreeImage open source "
		"image library. See http://freeimage.sourceforge.net for details. "
		"FreeImage is used under the GNU GPL.</i></li>"
#endif
		"</ul><br><br><hr/><br><br>"
		"<center><b>CREDITS</b></center><br>"
		"We want to thank all the contributors to the Dream software (in "
		"alphabetical order):<br><br>"
		"<b>Developers</b>"
		"<center>"
		"<p>Bakker, Menno</p>"
		"<p>Cesco</p>"
		"<p>Fillod, Stephane</p>"
		"<p>Fine, Mark J.</p>"
		"<p>Manninen, Tomi</p>"
		"<p>Pascutto, Gian C.</p>"
		"<p>Richard, Doyle</p>"
		"<p>Russo, Andrea</p>"
		"</center>"
		"<br><b>Parts of Dream are based on code by</b>"
		"<center>"
		"<p>Karn, Phil (www.ka9q.net)</p>"
		"<p>Ptolemy Project (http://ptolemy.eecs.berkeley.edu)</p>"
		"<p>Tavernini, Lucio (http://tavernini.com/home.html)</p>"
		"<p>The Math Forum (http://mathforum.org)</p>"
		"<p>The Synthesis ToolKit in C++ (STK) "
		"(http://ccrma.stanford.edu/software/stk)</p>"
		"</center>"
		"<br><b>Supporters</b>"
		"<center>"
		"<p>Amorim, Roberto José de</p>"
		"<p>Kainka, Burkhard</p>"
		"<p>Keil, Jens</p>"
		"<p>Kilian, Gerd</p>"
		"<p>Knuetter, Carsten</p>"
		"<p>Ramisch, Roland</p>"
		"<p>Schall, Norbert</p>"
		"<p>Schill, Dietmar</p>"
		"<p>Schneider, Klaus</p>"
		"<p>St&ouml;ppler, Simone</p>"
		"<p>Varlamov, Oleg</p>"
		"<p>Wade, Graham</p>"
		"</center><br>");

	/* Set version number in about dialog */
	QString strVersionText;
	strVersionText = "<center><b>" + tr("Dream, Version ");
	strVersionText += VERSION;
	strVersionText += "</b><br> " + tr("Open-Source Software Implementation of "
		"a DRM-Receiver") + "<br>";
	strVersionText += tr("Under the GNU General Public License (GPL)") +
		"</center>";
	TextLabelVersion->setText(strVersionText);
}


/* Help menu ---------------------------------------------------------------- */
CDreamHelpMenu::CDreamHelpMenu(QWidget* parent) : QPopupMenu(parent)
{
	/* Standard help menu consists of about and what's this help */
	insertItem(tr("What's &This"), this ,
		SLOT(OnHelpWhatsThis()), SHIFT+Key_F1);
	insertSeparator();
	insertItem(tr("&About..."), this, SLOT(OnHelpAbout()));
}


/* Sound card selection menu ------------------------------------------------ */
CSoundCardSelMenu::CSoundCardSelMenu(CSound* pNS, QWidget* parent) :
	pSoundIF(pNS), QPopupMenu(parent)
{
	pSoundInMenu = new QPopupMenu(parent);
	CHECK_PTR(pSoundInMenu);
	pSoundOutMenu = new QPopupMenu(parent);
	CHECK_PTR(pSoundOutMenu);

	/* Get sound device names */
	iNumSoundDev = pSoundIF->GetNumDev();
	for (int i = 0; i < iNumSoundDev; i++)
	{
		const string strName = pSoundIF->GetDeviceName(i);
		pSoundInMenu->insertItem(QString(strName.c_str()), this,
			SLOT(OnSoundInDevice(int)), 0, i);
		pSoundOutMenu->insertItem(QString(strName.c_str()), this,
			SLOT(OnSoundOutDevice(int)), 0, i);
	}

	/* Insert "Wave mapper". "iNumSoundDev" is no valid ID for a device, use
	   this for wave-mapper */
	pSoundInMenu->insertSeparator();
	pSoundInMenu->insertItem(tr("Primary Audio &Device (Recording)"), this,
		SLOT(OnSoundInDevice(int)), 0, iNumSoundDev);
	pSoundOutMenu->insertSeparator();
	pSoundOutMenu->insertItem(tr("Primary Audio &Device (Playback)"), this,
		SLOT(OnSoundOutDevice(int)), 0, iNumSoundDev);

	/* Set default device. If no valid device was selected, select
	   "Wave mapper" */
	int iDefaultInDev = pSoundIF->GetInDev();
	if ((iDefaultInDev > iNumSoundDev) || (iDefaultInDev < 0))
		iDefaultInDev = iNumSoundDev;

	int iDefaultOutDev = pSoundIF->GetOutDev();
	if ((iDefaultOutDev > iNumSoundDev) || (iDefaultOutDev < 0))
		iDefaultOutDev = iNumSoundDev;

	pSoundInMenu->setItemChecked(iDefaultInDev, TRUE);
	pSoundOutMenu->setItemChecked(iDefaultOutDev, TRUE);

	insertItem(tr("Sound &In"), pSoundInMenu);
	insertItem(tr("Sound &Out"), pSoundOutMenu);
}

void CSoundCardSelMenu::OnSoundInDevice(int id)
{
	pSoundIF->SetInDev(id);

	/* Taking care of checks in the menu. "+ 1" because of wave mapper entry */
	for (int i = 0; i < iNumSoundDev + 1; i++)
		pSoundInMenu->setItemChecked(i, i == id);
}

void CSoundCardSelMenu::OnSoundOutDevice(int id)
{
	pSoundIF->SetOutDev(id);

	/* Taking care of checks in the menu. "+ 1" because of wave mapper entry */
	for (int i = 0; i < iNumSoundDev + 1; i++)
		pSoundOutMenu->setItemChecked(i, i == id);
}
