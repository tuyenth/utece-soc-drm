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

#include <qtextbrowser.h>
#include <qmime.h>
#include <qimage.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qtextstream.h>

#ifdef _WIN32
# include "../../Windows/moc/MultimediaDlgbase.h"
#else
# include "moc/MultimediaDlgbase.h"
#endif

#include "../GlobalDefinitions.h"
#include "../DrmReceiver.h"
#include "MultColorLED.h"
#include "../datadecoding/DABMOT.h"

#ifdef HAVE_LIBFREEIMAGE
# include <FreeImage.h>
#endif


/* Definitions ****************************************************************/
/* Maximum number of levels. A maximum of 20 hierarchy levels is set
   (including the Main Menu and the final Message Object) */
#define MAX_NUM_LEV_JOURNALINE			20


/* Classes ********************************************************************/
class CNewIDHistory
{
public:
	CNewIDHistory() : veciNewsID(MAX_NUM_LEV_JOURNALINE), iNumHist(0) {}
	virtual ~CNewIDHistory() {}

	void Add(const int iNewID)
	{
		veciNewsID[iNumHist] = iNewID;
		iNumHist++;
	}

	int Back()
	{
		if (iNumHist > 0)
		{
			iNumHist--;
			return veciNewsID[iNumHist];
		}
		else
			return 0; /* Root ID */
	}

	void Reset() {iNumHist = 0;}

protected:
	CVector<int>	veciNewsID;
	int				iNumHist;
};

class MultimediaDlg : public MultimediaDlgBase
{
	Q_OBJECT

public:
	MultimediaDlg(CDRMReceiver* pNDRMR, QWidget* parent = 0,
		const char* name = 0, bool modal = FALSE, WFlags f = 0);

	virtual ~MultimediaDlg();

	void SetStatus(int MessID, int iMessPara);

protected:
	CDRMReceiver*			pDRMRec;

	QTimer					Timer;
	QMenuBar*				pMenu;
	QPopupMenu*				pFileMenu;
	virtual void			showEvent(QShowEvent* pEvent);
	virtual void			hideEvent(QHideEvent* pEvent);
	CVector<CMOTObject>		vecRawImages;
	int						iCurImagePos;
	QString					strFhGIISText;
	QString					strJournalineHeadText;
	int						iCurJourObjID;
	CDataDecoder::EAppType	eAppType;
	CNewIDHistory			NewIDHistory;

	void SetSlideShowPicture();
	void SetJournalineText();
	void UpdateAccButtonsSlideShow();
	int GetIDLastPicture() {return vecRawImages.Size() - 1;}
	void SavePicture(const int iPicID, const QString& strFileName);
	void ClearAllSlideShow();

	void InitApplication(CDataDecoder::EAppType eNewAppType);

	void InitNotSupported();
	void InitMOTSlideShow();
	void InitJournaline();

	void JpgToPng(CMOTObject& NewPic);

	void ExtractJournalineBody(const int iCurJourID, const _BOOLEAN bHTMLExport,
		QString &strTitle, QString &strItems);

public slots:
	void OnTimer();
	void OnButtonStepBack();
	void OnButtonStepForw();
	void OnButtonJumpBegin();
	void OnButtonJumpEnd();
	void OnSave();
	void OnSaveAll();
	void OnClearAll() {ClearAllSlideShow();}
	void OnTextChanged();
};
