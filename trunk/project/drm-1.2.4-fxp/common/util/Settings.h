/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2004
 *
 * Author(s):
 *	Volker Fischer, Robert Kesterson
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

#if !defined(SETTINGS_H__3B0BA660_DGEG56GE64B2B_23DSG9876D31912__INCLUDED_)
#define SETTINGS_H__3B0BA660_DGEG56GE64B2B_23DSG9876D31912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../DrmReceiver.h"
#include "../DrmTransmitter.h"
#ifdef USE_QT_GUI
# include "../GUI-QT/DRMPlot.h"
#endif
#include <map>
#include <string>
#include <fstream>


/* Definitions ****************************************************************/
#define DREAM_INIT_FILE_NAME		"Dream.ini"

/* Maximum number of sound devices */
#define MAX_NUM_SND_DEV				10

/* Maximum number of iterations in MLC decoder */
#define MAX_NUM_MLC_IT				4

/* Maximum value of input/output channel selection */
#define MAX_VAL_IN_CHAN_SEL			6
#define MAX_VAL_OUT_CHAN_SEL		4

/* Minimum and maximum of initial sample rate offset parameter */
#define MIN_SAM_OFFS_INI			-200
#define MAX_SAM_OFFS_INI			200

/* Maximum for frequency acqisition search window size and center frequency */
#define MAX_FREQ_AQC_SE_WIN_SI		(SOUNDCRD_SAMPLE_RATE / 2)
#define MAX_FREQ_AQC_SE_WIN_CEN		(SOUNDCRD_SAMPLE_RATE / 2)

#ifdef USE_QT_GUI
/* Maximum minutes for delayed log file start */
# define MAX_SEC_LOG_FI_START		3600 /* seconds */

/* Maximum carrier frequency for log file */
# define MAX_FREQ_LOG_FILE			30000 /* kHz */

/* Maximum value for window position and size */
# define MAX_WIN_GEOM_VAL			10000 /* Pixel */

/* Maximum value for color schemes for main plot */
# define MAX_COLOR_SCHEMES_VAL		(NUM_AVL_COLOR_SCHEMES_PLOT - 1)

# define MAX_MDI_PORT_IN_NUM		32768
#endif

#ifdef HAVE_LIBHAMLIB
/* Maximum ID number for hamlib library */
# define MAX_ID_HAMLIB				32768
#endif

/* change this if you expect to have huge lines in your INI files. Note that
   this is the max size of a single line, NOT the max number of lines */
#define MAX_INI_LINE				500

/* Maximum number of chart windows and plot types */
#define MAX_NUM_CHART_WIN_EV_DLG	50
#define MAX_IND_CHART_TYPES			1000

/* Maximum for preview */
#define MAX_NUM_SEC_PREVIEW			3600


/* Classes ********************************************************************/
class CSettings
{
public:
	CSettings(CDRMReceiver* pNDRMR) : pDRMRec(pNDRMR) {}

	_BOOLEAN Load(int argc, char** argv);
	void Save();

protected:
	void ReadIniFile();
	void WriteIniFile();

	_BOOLEAN ParseArguments(int argc, char** argv);
	string UsageArguments(char** argv);
	_BOOLEAN GetFlagArgument(int argc, char** argv, int& i, string strShortOpt,
							 string strLongOpt);
	_BOOLEAN GetNumericArgument(int argc, char** argv, int& i,
								string strShortOpt, string strLongOpt,
								_REAL rRangeStart, _REAL rRangeStop,
								_REAL& rValue);
	_BOOLEAN GetStringArgument(int argc, char** argv, int& i,
							   string strShortOpt, string strLongOpt,
							   string& strArg);

	/* Function declarations for stlini code written by Robert Kesterson */
	struct StlIniCompareStringNoCase 
	{
		bool operator()(const string& x, const string& y) const;
	};

	/* These typedefs just make the code a bit more readable */
#if defined(__ARMCC__) || defined(__CC_ARM)
	typedef map<string, string, StlIniCompareStringNoCase, allocator<string>, allocator<string>, allocator<StlIniCompareStringNoCase> > INISection;
	typedef map<string, INISection , StlIniCompareStringNoCase, allocator<string>, allocator<string>, allocator<StlIniCompareStringNoCase> > INIFile;
#else
	typedef std::map<string, string, StlIniCompareStringNoCase > INISection;
	typedef std::map<string, INISection , StlIniCompareStringNoCase > INIFile;
#endif

	string GetIniSetting(INIFile& theINI, const char* pszSection,
						 const char* pszKey, const char* pszDefaultVal = "");
	void PutIniSetting(INIFile &theINI, const char *pszSection,
					   const char* pszKey = NULL, const char* pszValue = "");
	void SaveIni(INIFile& theINI, const char* pszFilename);
	INIFile LoadIni(const char* pszFilename);


	void SetNumericIniSet(INIFile& theINI, string strSection, string strKey,
						  int iValue);
	_BOOLEAN GetNumericIniSet(INIFile& theINI, string strSection, string strKey,
							  int iRangeStart, int iRangeStop, int& iValue);
	void SetFlagIniSet(INIFile& theINI, string strSection, string strKey,
					   _BOOLEAN bValue);
	_BOOLEAN GetFlagIniSet(INIFile& theINI, string strSection, string strKey,
						   _BOOLEAN& bValue);

	/* Pointer to the DRM receiver object needed for the various settings */
	CDRMReceiver* pDRMRec;
};

#endif // !defined(SETTINGS_H__3B0BA660_DGEG56GE64B2B_23DSG9876D31912__INCLUDED_)
