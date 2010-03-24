/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * 6/8/2005 Andrea Russo
 *	- save Journaline pages as HTML
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

#include "MultimediaDlg.h"


MultimediaDlg::MultimediaDlg(CDRMReceiver* pNDRMR, QWidget* parent,
	const char* name, bool modal, WFlags f) : pDRMRec(pNDRMR),
	MultimediaDlgBase(parent, name, modal, f)
{
#ifdef _WIN32 /* This works only reliable under Windows :-( */
	/* Get window geometry data from DRMReceiver module and apply it */
	const QRect WinGeom(pDRMRec->GeomMultimediaDlg.iXPos,
		pDRMRec->GeomMultimediaDlg.iYPos,
		pDRMRec->GeomMultimediaDlg.iWSize,
		pDRMRec->GeomMultimediaDlg.iHSize);

	if (WinGeom.isValid() && !WinGeom.isEmpty() && !WinGeom.isNull())
		setGeometry(WinGeom);
#else /* Under Linux only restore the size */
	resize(pDRMRec->GeomMultimediaDlg.iWSize,
		pDRMRec->GeomMultimediaDlg.iHSize);
#endif

	/* Picture controls should be invisable. These controls are only used for
	   storing the resources */
	PixmapFhGIIS->hide();
	PixmapLogoJournaline->hide();

	/* Set pictures in source factory */
	QMimeSourceFactory::defaultFactory()->setImage("PixmapFhGIIS",
		PixmapFhGIIS->pixmap()->convertToImage());
	QMimeSourceFactory::defaultFactory()->setImage("PixmapLogoJournaline",
		PixmapLogoJournaline->pixmap()->convertToImage());

	/* Set FhG IIS text */
	strFhGIISText = "<table><tr><td><img source=\"PixmapFhGIIS\"></td>"
		"<td><font face=\"Courier\" size=\"-1\">Features NewsService "
		"Journaline(R) decoder technology by Fraunhofer IIS, Erlangen, "
		"Germany. For more information visit http://www.iis.fhg.de/dab"
		"</font></td></tr></table>";

	/* Set Journaline headline text */
	strJournalineHeadText =
		"<table><tr><td><img source=\"PixmapLogoJournaline\"></td>"
		"<td><h2>NewsService Journaline" + QString(QChar(174)) /* (R) */ +
		"</h2></td></tr></table>";

	/* Set Menu ***************************************************************/
	/* File menu ------------------------------------------------------------ */
	pFileMenu = new QPopupMenu(this);
	CHECK_PTR(pFileMenu);
	pFileMenu->insertItem(tr("C&lear all"), this, SLOT(OnClearAll()),
		CTRL+Key_X, 0);
	pFileMenu->insertSeparator();
	pFileMenu->insertItem(tr("&Save..."), this, SLOT(OnSave()), CTRL+Key_S, 1);
	pFileMenu->insertItem(tr("Save &all..."), this, SLOT(OnSaveAll()),
		CTRL+Key_A, 2);
	pFileMenu->insertSeparator();
	pFileMenu->insertItem(tr("&Close"), this, SLOT(close()), 0, 3);


	/* Main menu bar -------------------------------------------------------- */
	pMenu = new QMenuBar(this);
	CHECK_PTR(pMenu);
	pMenu->insertItem(tr("&File"), pFileMenu);

	/* Now tell the layout about the menu */
	MultimediaDlgBaseLayout->setMenuBar(pMenu);

	
	/* Update time for color LED */
	LEDStatus->SetUpdateTime(1000);

	/* Init slide-show (needed for setting up vectors and indices) */
	ClearAllSlideShow();

	/* Init container and GUI */
	InitApplication(pDRMRec->GetDataDecoder()->GetAppType());


	/* Connect controls */
	connect(PushButtonStepBack, SIGNAL(clicked()),
		this, SLOT(OnButtonStepBack()));
	connect(PushButtonStepForw, SIGNAL(clicked()),
		this, SLOT(OnButtonStepForw()));
	connect(PushButtonJumpBegin, SIGNAL(clicked()),
		this, SLOT(OnButtonJumpBegin()));
	connect(PushButtonJumpEnd, SIGNAL(clicked()),
		this, SLOT(OnButtonJumpEnd()));
	connect(TextBrowser, SIGNAL(textChanged()),
		this, SLOT(OnTextChanged()));

	connect(&Timer, SIGNAL(timeout()),
		this, SLOT(OnTimer()));
}

MultimediaDlg::~MultimediaDlg()
{
	/* Set window geometry data in DRMReceiver module */
	QRect WinGeom = geometry();

	pDRMRec->GeomMultimediaDlg.iXPos = WinGeom.x();
	pDRMRec->GeomMultimediaDlg.iYPos = WinGeom.y();
	pDRMRec->GeomMultimediaDlg.iHSize = WinGeom.height();
	pDRMRec->GeomMultimediaDlg.iWSize = WinGeom.width();
}

void MultimediaDlg::InitApplication(CDataDecoder::EAppType eNewAppType)
{
	/* Set internal parameter */
	eAppType = eNewAppType;

	/* Actual inits */
	switch (eAppType)
	{
	case CDataDecoder::AT_MOTSLISHOW:
		InitMOTSlideShow();
		break;

	case CDataDecoder::AT_JOURNALINE:
		InitJournaline();
		break;

	default:
		InitNotSupported();
		break;
	}
}

void MultimediaDlg::OnTextChanged()
{
	/* Check, if the current text is a link ID or regular text */
	if (TextBrowser->text().compare(TextBrowser->text().left(1), "<") != 0)
	{
		/* Save old ID */
		NewIDHistory.Add(iCurJourObjID);

		/* Set text to news ID text which was selected by the user */
		iCurJourObjID = TextBrowser->text().toInt();
		SetJournalineText();

		/* Enable back button */
		PushButtonStepBack->setEnabled(TRUE);
	}
}

void MultimediaDlg::OnTimer()
{
	CMOTObject	NewPic;
	QPixmap		NewImage;
	FILE*		pFiBody;
	int			iCurNumPict;

	/* Check out which application is transmitted right now */
	CDataDecoder::EAppType eNewAppType =
		pDRMRec->GetDataDecoder()->GetAppType();

	if (eNewAppType != eAppType)
		InitApplication(eNewAppType);

	switch (eAppType)
	{
	case CDataDecoder::AT_MOTSLISHOW:
		/* Poll the data decoder module for new picture */
		if (pDRMRec->GetDataDecoder()->GetSlideShowPicture(NewPic) == TRUE)
		{
			/* Store received picture */
			iCurNumPict = vecRawImages.Size();
			vecRawImages.Add(NewPic);

			/* If the last received picture was selected, automatically show
			   new picture */
			if (iCurImagePos == iCurNumPict - 1)
			{
				iCurImagePos = iCurNumPict;
				SetSlideShowPicture();
			}
			else
				UpdateAccButtonsSlideShow();
		}
		break;

	case CDataDecoder::AT_JOURNALINE:
		SetJournalineText();
		break;
	}
}

void MultimediaDlg::ExtractJournalineBody(const int iCurJourID,
										  const _BOOLEAN bHTMLExport,
										  QString& strTitle, QString& strItems)
{
	/* Get news from actual Journaline decoder */
	CNews News;
	pDRMRec->GetDataDecoder()->GetNews(iCurJourID, News);

	/* Decode UTF-8 coding for title */
	strTitle = QString().fromUtf8(QCString(News.sTitle.c_str()));

	strItems = "";
	for (int i = 0; i < News.vecItem.Size(); i++)
	{
		QString strCurItem;
		if (bHTMLExport == FALSE)
		{
			/* Decode UTF-8 coding of this item text */
			strCurItem = QString().fromUtf8(
				QCString(News.vecItem[i].sText.c_str()));
		}
		else
		{
			/* In case of HTML export, do not decode UTF-8 coding */
			strCurItem = News.vecItem[i].sText.c_str();
		}

		/* Replace \n by html command <br> */
		strCurItem = strCurItem.replace(QRegExp("\n"), "<br>");

		if (News.vecItem[i].iLink == JOURNALINE_IS_NO_LINK)
		{
			/* Only text, no link */
			strItems += strCurItem + QString("<br>");
		}
		else if (News.vecItem[i].iLink == JOURNALINE_LINK_NOT_ACTIVE)
		{
			/* Un-ordered list item without link */
			strItems += QString("<li>") + strCurItem + QString("</li>");
		}
		else
		{
			if (bHTMLExport == FALSE) 
			{
				QString strLinkStr = QString().setNum(News.vecItem[i].iLink);

				/* Un-ordered list item with link */
				strItems += QString("<li><a href=\"") + strLinkStr +
					QString("\">") + strCurItem +
					QString("</a></li>");

				/* Store link location in factory (stores ID) */
				QMimeSourceFactory::defaultFactory()->
					setText(strLinkStr, strLinkStr);
			}
			else
				strItems += QString("<li>") + strCurItem + QString("</li>");
		}
	}
}

void MultimediaDlg::SetJournalineText()
{
	/* Get title and body with html links */
	QString strTitle("");
	QString strItems("");
	ExtractJournalineBody(iCurJourObjID, FALSE, strTitle, strItems);

	/* Set html text. Standard design. The first character must be a "<". This
	   is used to identify whether normal text is displayed or an ID was set */
	QString strAllText =
		"<table>"
		"<tr><th>" + strJournalineHeadText + "</th></tr>"
		"<tr><td><hr></td></tr>" /* horizontial line */
		"<tr><th>" + strTitle + "</th></tr>"
		"<tr><td><ul type=\"square\">" + strItems + "</ul></td></tr>"
		"<tr><td><hr></td></tr>" /* horizontial line */
		"<tr><td>" + strFhGIISText + "</td></tr>"
		"</table>";

	/* Only update text browser if text has changed */
	if (TextBrowser->text().compare(strAllText) != 0)
		TextBrowser->setText(strAllText);

	/* Enable / disable "save" menu item if title is present or not */
	if (strTitle == "")
		pFileMenu->setItemEnabled(1, FALSE);
	else
		pFileMenu->setItemEnabled(1, TRUE);
}

void MultimediaDlg::showEvent(QShowEvent* pEvent)
{
	/* Activte real-time timer when window is shown */
	Timer.start(GUI_CONTROL_UPDATE_TIME);

	/* Update window */
	OnTimer();
}

void MultimediaDlg::hideEvent(QHideEvent* pEvent)
{
	/* Deactivate real-time timer so that it does not get new pictures */
	Timer.stop();
}

void MultimediaDlg::SetStatus(int MessID, int iMessPara)
{
	switch(MessID)
	{
	case MS_MOT_OBJ_STAT:
		LEDStatus->SetLight(iMessPara);
		break;
	}
}

void MultimediaDlg::OnButtonStepBack()
{
	switch (eAppType)
	{
	case CDataDecoder::AT_MOTSLISHOW:
		iCurImagePos--;
		SetSlideShowPicture();
		break;

	case CDataDecoder::AT_JOURNALINE:
		/* Step one level back, get ID from history */
		iCurJourObjID = NewIDHistory.Back();

		/* If root ID is reached, disable back button */
		if (iCurJourObjID == 0)
			PushButtonStepBack->setEnabled(FALSE);

		SetJournalineText();
		break;
	}
}

void MultimediaDlg::OnButtonStepForw()
{
	iCurImagePos++;
	SetSlideShowPicture();
}

void MultimediaDlg::OnButtonJumpBegin()
{
	/* Reset current picture number to zero (begin) */
	iCurImagePos = 0;
	SetSlideShowPicture();
}

void MultimediaDlg::OnButtonJumpEnd()
{
	/* Go to last received picture */
	iCurImagePos = GetIDLastPicture();
	SetSlideShowPicture();
}

void MultimediaDlg::SetSlideShowPicture()
{
	QPixmap		NewImage;
	int			iPicSize;

	/* Copy current image from image storage vector */
	CMOTObject vecbyCurPict(vecRawImages[iCurImagePos]);

	/* The standard version of QT does not have jpeg support, if FreeImage
	   library is installed, the following routine converts the picture to
	   png which can be displayed */
	JpgToPng(vecbyCurPict);

	/* Get picture size */
	iPicSize = vecbyCurPict.vecbRawData.Size();

	/* Load picture in QT format */
	if (NewImage.loadFromData(&vecbyCurPict.vecbRawData[0], iPicSize))
	{
		/* Set new picture in source factory and set it in text control */
		QMimeSourceFactory::defaultFactory()->setImage("MOTSlideShowimage",
			NewImage.convertToImage());

		TextBrowser->setText("<center><img source=\"MOTSlideShowimage\">"
			"</center>");
	}
	else
	{
		/* Show text that tells the user of load failure */
		TextBrowser->setText("<br><br><center><b>" + tr("Image could not be "
			"loaded, ") +
			 QString(vecbyCurPict.strFormat.c_str()) +
			 tr("-format not supported by this version of QT!") +
			"</b><br><br><br>" + tr("If you want to view the image, "
			"save it to file and use an external viewer") + "</center>");
	}

	/* Add tool tip showing the name of the picture */
	if (vecRawImages[iCurImagePos].strName.length() != 0)
	{
		QToolTip::add(TextBrowser,
			QString(vecRawImages[iCurImagePos].strName.c_str()));
	}

	UpdateAccButtonsSlideShow();
}

void MultimediaDlg::UpdateAccButtonsSlideShow()
{
	/* Set enable menu entry for saving a picture */
	if (iCurImagePos < 0)
	{
		pFileMenu->setItemEnabled(0, FALSE);
		pFileMenu->setItemEnabled(1, FALSE);
		pFileMenu->setItemEnabled(2, FALSE);
	}
	else
	{
		pFileMenu->setItemEnabled(0, TRUE);
		pFileMenu->setItemEnabled(1, TRUE);
		pFileMenu->setItemEnabled(2, TRUE);
	}

	if (iCurImagePos <= 0)
	{
		/* We are already at the beginning */
		PushButtonStepBack->setEnabled(FALSE);
		PushButtonJumpBegin->setEnabled(FALSE);
	}
	else
	{
		PushButtonStepBack->setEnabled(TRUE);
		PushButtonJumpBegin->setEnabled(TRUE);
	}

	if (iCurImagePos == GetIDLastPicture())
	{
		/* We are already at the end */
		PushButtonStepForw->setEnabled(FALSE);
		PushButtonJumpEnd->setEnabled(FALSE);
	}
	else
	{
		PushButtonStepForw->setEnabled(TRUE);
		PushButtonJumpEnd->setEnabled(TRUE);
	}

	LabelCurPicNum->setText(QString().setNum(iCurImagePos + 1) + "/" +
		QString().setNum(GetIDLastPicture() + 1));

	/* If no picture was received, show the following text */
	if (iCurImagePos < 0)
	{
		/* Init text browser window */
		TextBrowser->setText("<center><h2>" +
			tr("MOT Slideshow Viewer") + "</h2></center>");
	}
}

void MultimediaDlg::OnSave()
{
	QString strFileName;
	QString strDefFileName;

	switch (eAppType)
	{
	case CDataDecoder::AT_MOTSLISHOW:
		/* Show "save file" dialog */
		/* Set file name */
		strDefFileName = vecRawImages[iCurImagePos].strName.c_str();

		/* Use default file name if no file name was transmitted */
		if (strDefFileName.length() == 0)
			strDefFileName = "RecPic";

		strFileName =
			QFileDialog::getSaveFileName(strDefFileName + "." +
			QString(vecRawImages[iCurImagePos].strFormat.c_str()),
			"*." + QString(vecRawImages[iCurImagePos].strFormat.c_str()), this);

		/* Check if user not hit the cancel button */
		if (!strFileName.isNull())
			SavePicture(iCurImagePos, strFileName);
		break;

	case CDataDecoder::AT_JOURNALINE:
		/* Save to file current journaline page */
		QString strTitle("");
		QString strItems("");

		/* TRUE = without html links */
		ExtractJournalineBody(iCurJourObjID, TRUE, strTitle, strItems);

		/* Prepare HTML page for storing the content (header, body tags, etc) */
		QString strJornalineText = "<html>\n<head>\n"
			"<meta http-equiv=\"content-Type\" "
			"content=\"text/html; charset=utf-8\">\n<title>" + strTitle +
			"</title>\n</head>\n\n<body>\n<table>\n"
			"<tr><th>" + strTitle + "</th></tr>\n"
			"<tr><td><ul type=\"square\">" + strItems + "</ul></td></tr>\n"
			"</table>\n</body>\n</html>";

		strFileName = QFileDialog::getSaveFileName(strTitle + ".html",
			"*.html", this);

		if (!strFileName.isNull())
		{
			/* Save Journaline page as a text stream */
			QFile FileObj(strFileName);

			if (FileObj.open(IO_WriteOnly))
			{
				QTextStream TextStream(&FileObj);
				TextStream << strJornalineText; /* Actual writing */
				FileObj.close();
			}
		}
		break;
	}
}

void MultimediaDlg::OnSaveAll()
{
	/* Let the user choose a directory */
	QString strDirName =
		QFileDialog::getExistingDirectory(NULL, this);

	if (!strDirName.isNull())
	{
		/* Loop over all pictures received yet */
		for (int j = 0; j < GetIDLastPicture() + 1; j++)
		{
			QString strFileName = vecRawImages[j].strName.c_str();

			if (strFileName.length() == 0)
			{
				/* Construct file name from date and picture number (default) */
				strFileName = "Dream_" + QDate().currentDate().toString() +
					"_#" + QString().setNum(j);
			}

			/* Add directory and ending */
			strFileName = strDirName + strFileName + "." +
				QString(vecRawImages[j].strFormat.c_str());

			SavePicture(j, strFileName);
		}
	}
}

void MultimediaDlg::SavePicture(const int iPicID, const QString& strFileName)
{
	/* Get picture size */
	const int iPicSize = vecRawImages[iPicID].vecbRawData.Size();

	/* Open file */
	FILE* pFiBody = fopen(strFileName.latin1(), "wb");

	if (pFiBody != NULL)
	{
		for (int i = 0; i < iPicSize; i++)
		{
			fwrite((void*) &vecRawImages[iPicID].vecbRawData[i],
				size_t(1), size_t(1), pFiBody);
		}

		/* Close the file afterwards */
		fclose(pFiBody);
	}
}

void MultimediaDlg::ClearAllSlideShow()
{
	/* Init vector which will store the received images with zero size */
	vecRawImages.Init(0);

	/* Init current image position */
	iCurImagePos = -1;

	/* Update GUI */
	UpdateAccButtonsSlideShow();

	/* Remove tool tips */
	QToolTip::remove(TextBrowser);
}

void MultimediaDlg::InitNotSupported()
{
	/* Hide all controls, disable menu items */
	pFileMenu->setItemEnabled(0, FALSE);
	pFileMenu->setItemEnabled(1, FALSE);
	pFileMenu->setItemEnabled(2, FALSE);
	PushButtonStepForw->hide();
	PushButtonJumpBegin->hide();
	PushButtonJumpEnd->hide();
	LabelCurPicNum->hide();
	PushButtonStepBack->hide();
	QToolTip::remove(TextBrowser);

	/* Show that application is not supported */
	TextBrowser->setText("<center><h2>" + tr("No data service or data service "
		"not supported.") + "</h2></center>");
}

void MultimediaDlg::InitMOTSlideShow()
{
	/* Make all browse buttons visible */
	PushButtonStepBack->show();
	PushButtonStepForw->show();
	PushButtonJumpBegin->show();
	PushButtonJumpEnd->show();
	LabelCurPicNum->show();

	/* Set current image position to the last picture and display it (if at
	   least one picture is available) */
	iCurImagePos = GetIDLastPicture();
	if (iCurImagePos >= 0)
		SetSlideShowPicture();
	else
	{
		/* Remove tool tips */
		QToolTip::remove(TextBrowser);
	}

	/* Update buttons and menu */
	UpdateAccButtonsSlideShow();
}

void MultimediaDlg::InitJournaline()
{
	/* Disable "clear all" menu item */
	pFileMenu->setItemEnabled(0, FALSE);

	/* Disable "save" menu items */
	pFileMenu->setItemEnabled(1, FALSE);
	pFileMenu->setItemEnabled(2, FALSE);

	/* Only one back button is visible and enabled */
	PushButtonStepForw->hide();
	PushButtonJumpBegin->hide();
	PushButtonJumpEnd->hide();
	LabelCurPicNum->hide();

	/* Show back button and disable it because we always start at the root
	   object */
	PushButtonStepBack->show();
	PushButtonStepBack->setEnabled(FALSE);

	/* Init text browser window */
	iCurJourObjID = 0;
	SetJournalineText();

	/* Remove tool tips */
	QToolTip::remove(TextBrowser);

	NewIDHistory.Reset();
}

void MultimediaDlg::JpgToPng(CMOTObject& NewPic)
{
#ifdef HAVE_LIBFREEIMAGE
	/* This class is needed for FreeImage load and save from memory. This code
	   is based on an example code shipped with FreeImage library */
	class MemIO : public FreeImageIO
	{
	public :
		/* Assign function pointers in constructor */
		MemIO(CVector<_BYTE> vecNewData) : vecbyData(vecNewData), iPos(0)
			{read_proc  = _ReadProc; write_proc = _WriteProc;
			tell_proc = _TellProc; seek_proc = _SeekProc;}
		CVector<_BYTE>& GetData() {return vecbyData;}
		void Reset() {iPos = 0;}

		static long DLL_CALLCONV _TellProc(fi_handle handle)
			{return ((MemIO*) handle)->iPos;} /* Return current position */

		static unsigned DLL_CALLCONV _ReadProc(void* buffer, unsigned size,
			unsigned count, fi_handle handle)
		{
			MemIO* memIO = (MemIO*) handle;
			_BYTE* tmpBuf = (_BYTE*) buffer;

			/* Copy new data in internal storage vector. Write at current iPos
			   and increment position. Check for out-of-range, too */
			for (unsigned int c = 0; c < count; c++)
			{
				for (unsigned int i = 0; i < size; i++)
				{
					if (memIO->iPos < memIO->vecbyData.Size())
						*tmpBuf++ = memIO->vecbyData[memIO->iPos++];
				}
			}

			return count;
		}

		static unsigned DLL_CALLCONV _WriteProc(void* buffer, unsigned size,
			unsigned count, fi_handle handle)
		{
			MemIO* memIO = (MemIO*) handle;
			_BYTE* tmpBuf = (_BYTE*) buffer;

			/* Make sure, enough space is available */
			const long int iSpaceLeft =
				memIO->vecbyData.Size() - (memIO->iPos + size * count);

			if (iSpaceLeft < 0)
				memIO->vecbyData.Enlarge(-iSpaceLeft);

			/* Copy data */
			for (unsigned int c = 0; c < count; c++)
			{
				for (unsigned int i = 0; i < size; i++)
					memIO->vecbyData[memIO->iPos++] = *tmpBuf++;
			}

			return count;
		}

		static int DLL_CALLCONV _SeekProc(fi_handle handle, long offset,
			int origin)
		{
			if (origin == SEEK_SET)
				((MemIO*) handle)->iPos = offset; /* From beginning */
			else
				((MemIO*) handle)->iPos += offset; /* From current position */

			return 0;
		}

	private:
		CVector<_BYTE>	vecbyData;
		long int		iPos;
	};

	/* Only jpeg images are converted here */
	if (NewPic.strFormat.compare("jpeg") != 0)
		return;

	/* If we use freeimage as a static library, we need to initialize it first */
	FreeImage_Initialise();

	/* Put input data in a new IO object */
	MemIO memIO(NewPic.vecbRawData);

	/* Load data from memory */
	FIBITMAP* fbmp =
		FreeImage_LoadFromHandle(FIF_JPEG, &memIO, (fi_handle) &memIO);

	/* After the reading functions, the IO must be reset for the writing */
	memIO.Reset();

	/* Actual conversion */
	if (FreeImage_SaveToHandle(FIF_PNG, fbmp, &memIO, (fi_handle) &memIO))
	{
		/* Get converted data and set new format string */
		NewPic.vecbRawData.Init(memIO.GetData().Size()); /* Size has certainly
															changed */
		NewPic.vecbRawData = memIO.GetData(); /* Actual copying */
		NewPic.strFormat = "png"; /* New format string */
	}
#endif
}
