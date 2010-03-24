/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
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

#if !defined(JOURNALINE_H__3B0UBVE987346456363LIHGEW982__INCLUDED_)
#define JOURNALINE_H__3B0UBVE987346456363LIHGEW982__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../util/Vector.h"

#ifdef HAVE_JOURNALINE
# ifdef _WIN32
#  include <NML.h>
#  include <newssvcdec.h>
#  include <dabdatagroupdecoder.h>
# else
#  include <journaline/NML.h>
#  include <journaline/newssvcdec.h>
#  include <journaline/dabdatagroupdecoder.h>
# endif 
#endif


/* Definitions ****************************************************************/
/* Definitions for links which objects are not yet received or items which
   do not have links (no menu) */
#define JOURNALINE_IS_NO_LINK			-2
#define JOURNALINE_LINK_NOT_ACTIVE		-1


/* Classes ********************************************************************/
struct CNewsItem
{
	string	sText;
	int		iLink;
};

class CNews
{
public:
	CNews() : sTitle(""), vecItem(0) {}

	string				sTitle;
	CVector<CNewsItem>	vecItem;
};


#ifdef HAVE_JOURNALINE
class CJournaline
{
public:
	CJournaline();
	virtual ~CJournaline();

	void GetNews(const int iObjID, CNews& News);
	void AddDataUnit(CVector<_BINARY>& vecbiNewData);
	void Reset() {ResetOpenJournalineDecoder();}

protected:
	DAB_DATAGROUP_DECODER_t	dgdec;
	NEWS_SVC_DEC_decoder_t	newsdec;

	void ResetOpenJournalineDecoder();

	/* Callback functions for journaline decoder internal tasks */
	static void obj_avail_cb(unsigned long, NEWS_SVC_DEC_obj_availability_t*,
		void* data) {}
	static void dg_cb(const DAB_DATAGROUP_DECODER_msc_datagroup_header_t*,
		const unsigned long len, const unsigned char* buf, void* data)
		{NEWS_SVC_DEC_putData(((CJournaline*) data)->newsdec, len, buf);}
};
#else
/* No Journaline implementation if library is not installed */
class CJournaline
{
public:
	CJournaline() {}
	virtual ~CJournaline() {}

	void GetNews(const int iObjID, CNews& News)
	{
		/* Show in GUI that Journaline decoder is not available */
		News.sTitle = "Dream Decoder Message";
		News.vecItem.Init(1);
		News.vecItem[0].iLink = JOURNALINE_IS_NO_LINK;
		News.vecItem[0].sText = "No Journaline decoder available.";
	}
	void AddDataUnit(CVector<_BINARY>& vecbiNewData) {}
	void Reset() {}
};
#endif

#endif // !defined(JOURNALINE_H__3B0UBVE987346456363LIHGEW982__INCLUDED_)
