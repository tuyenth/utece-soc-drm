/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2004
 *
 * Author(s):
 *	Volker Fischer, Stephane Fillod
 *
 * Description:
 *
 * 11/10/2004 Stephane Fillod
 *	- QT translation
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

#include "../GlobalDefinitions.h"
#include "../DrmReceiver.h"
#include "../DrmTransmitter.h"
#include "../DrmSimulation.h"
#include "../util/Settings.h"

#ifdef USE_QT_GUI
# include <qapplication.h>
# include <qthread.h>
# include <qmessagebox.h>
# include "fdrmdialog.h"
# include "TransmDlg.h"
#endif


/* Implementation *************************************************************/
#ifdef USE_QT_GUI
/******************************************************************************\
* Using GUI with QT                                                            *
\******************************************************************************/
/* Application object must be initialized before the DRMReceiver object because
   of the QT functions used in the MDI module. TODO: better solution */
int argc = 0;
QApplication app(argc, NULL);

/* The receiver is a global object */
CDRMReceiver	DRMReceiver;

/* This pointer is only used for the post-event routine */
QApplication*	pApp = NULL;


/* Thread class for the receiver */
class CReceiverThread : public QThread
{
public:
	void Stop()
	{
		/* Stop working thread and wait until it is ready for terminating. We
		   set a time-out of 5 seconds */
		DRMReceiver.Stop();

		if (wait(5000) == FALSE)
			ErrorMessage("Termination of sound interface thread failed.");
	}

	virtual void run()
	{
		/* Set thread priority (The working thread should have a higher priority
		   than the GUI) */
#ifdef _WIN32
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif

		try
		{
			/* Call receiver main routine */
			DRMReceiver.Start();
		}

		catch (CGenErr GenErr)
		{
			ErrorMessage(GenErr.strError);
		}
	}
};

int main(int argc, char** argv)
{
try
{
	CDRMSimulation DRMSimulation;

	/* Call simulation script. If simulation is activated, application is
	   automatically exit in that routine. If in the script no simulation is
	   activated, this function will immediately return */
	DRMSimulation.SimScript();

	/* Parse arguments and load settings from init-file */
	CSettings Settings(&DRMReceiver);
	const _BOOLEAN bIsReceiver = Settings.Load(argc, argv);

	/* Load and install multi-language support (if available) */
	QTranslator translator(0);
	if (translator.load("dreamtr"))
		app.installTranslator(&translator);

#ifdef _WIN32
	/* Set priority class for this application */
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	/* Low priority for GUI thread */
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif

	if (bIsReceiver == FALSE)
	{
		TransmDialog MainDlg(0, 0, FALSE, Qt::WStyle_MinMax);

		/* Set main window */
		app.setMainWidget(&MainDlg);
		pApp = &app; /* Needed for post-event routine */

		/* Show dialog */
		MainDlg.show();
		app.exec();
	}
	else
	{
		/* First, initialize the working thread. This should be done in an extra
		   routine since we cannot 100% assume that the working thread is ealier
		   ready than the GUI thread */
		DRMReceiver.Init();

		CReceiverThread	RecThread; /* Working thread object */
		FDRMDialog		MainDlg(&DRMReceiver, 0, 0, FALSE, Qt::WStyle_MinMax);

		/* Start thread */
		RecThread.start();

		/* Set main window */
		app.setMainWidget(&MainDlg);
		pApp = &app; /* Needed for post-event routine */

		/* Working thread must be initialized before starting the GUI! */
		app.exec();

		RecThread.Stop();
	}

	/* Save settings to init-file */
	Settings.Save();
}

catch (CGenErr GenErr)
{
	ErrorMessage(GenErr.strError);
}

	return 0;
}


/* Implementation of global functions *****************************************/
void PostWinMessage(const _MESSAGE_IDENT MessID, const int iMessageParam)
{
	/* In case of simulation no events should be generated */
	if (pApp != NULL)
	{
		DRMEvent* DRMEv = new DRMEvent(MessID, iMessageParam);

		/* Qt will delete the event object when done */
		QThread::postEvent(pApp->mainWidget(), DRMEv);
	}
}

void ErrorMessage(string strErrorString)
{
	/* Workaround for the QT problem */
	string strError = "The following error occured:\n";
	strError += strErrorString.c_str();
	strError += "\n\nThe application will exit now.";

#ifdef _WIN32
	MessageBox(NULL, strError.c_str(), "Dream",
		MB_SYSTEMMODAL | MB_OK | MB_ICONEXCLAMATION);
#else
	perror(strError.c_str());
#endif

/*
// Does not work correctly. If it is called by a different thread, the
// application hangs! FIXME
	QMessageBox::critical(0, "Dream",
		QString("The following error occured:<br><b>") + 
		QString(strErrorString.c_str()) +
		"</b><br><br>The application will exit now.");
*/
	exit(1);
}
#else /* USE_QT_GUI */
/******************************************************************************\
* No GUI                                                                       *
\******************************************************************************/
CDRMReceiver DRMReceiver; /* Must be a global object */

int main(int argc, char** argv)
{
	CDRMTransmitter	DRMTransmitter;
	CDRMSimulation	DRMSimulation;

//try

{
	int vargc=5;
	char* vargv[] = {"./drm", "-f", "RTL_ModeB_10kHz.wav", "-w", "dummy.wav"};
	printf("argc = %d, argv = %s %s %s %s %s\n",vargc, vargv[0], vargv[1], vargv[2], vargv[3], vargv[4]); 
	CSettings Settings(&DRMReceiver);
	const _BOOLEAN bIsReceiver = Settings.Load(vargc, vargv);
	DRMSimulation.SimScript();

	if (bIsReceiver == TRUE)
	{
		printf("bIsReceiver = TRUE\n");
		DRMReceiver.Start();
	}
	else
	{
		printf("bIsReceiver = TRUE\n");
		DRMTransmitter.Start();
	}
}

//catch (CGenErr GenErr)
//{
	//ErrorMessage(GenErr.strError);
//}

	return 0;
}

void ErrorMessage(string strErrorString) {perror(strErrorString.c_str());}
void PostWinMessage(const _MESSAGE_IDENT MessID, const int iMessageParam) {}
#endif /* USE_QT_GUI */


void DebugError(const char* pchErDescr, const char* pchPar1Descr,
				const double dPar1, const char* pchPar2Descr,
				const double dPar2)
{
	FILE* pFile = fopen("test/DebugError.dat", "a");
	fprintf(pFile, pchErDescr); fprintf(pFile, " ### ");
	fprintf(pFile, pchPar1Descr); fprintf(pFile, ": ");
	fprintf(pFile, "%e ### ", dPar1);
	fprintf(pFile, pchPar2Descr); fprintf(pFile, ": ");
	fprintf(pFile, "%e\n", dPar2);
	fclose(pFile);
	printf("\nDebug error! For more information see test/DebugError.dat\n");
	exit(1);
}
