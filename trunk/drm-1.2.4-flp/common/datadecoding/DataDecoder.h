/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See DataDecoder.cpp
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

#if !defined(DATADECODER_H__3B0BA660_CA3452363E7A0D31912__INCLUDED_)
#define DATADECODER_H__3B0BA660_CA3452363E7A0D31912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../Parameter.h"
#include "../util/Modul.h"
#include "../util/CRC.h"
#include "../util/Vector.h"
#include "MOTSlideShow.h"
#include "Journaline.h"


/* Definitions ****************************************************************/
/* Maximum number of packets per stream */
#define MAX_NUM_PACK_PER_STREAM					4


/* Classes ********************************************************************/
/* Encoder ------------------------------------------------------------------ */
class CDataEncoder
{
public:
	CDataEncoder() {}
	virtual ~CDataEncoder() {}

	int Init(CParameter& Param);
	void GeneratePacket(CVector<_BINARY>& vecbiPacket);

	CMOTSlideShowEncoder* GetSliShowEnc() {return &MOTSlideShowEncoder;}

protected:
	CMOTSlideShowEncoder	MOTSlideShowEncoder;
	CVector<_BINARY>		vecbiCurDataUnit;

	int						iPacketLen;
	int						iTotalPacketSize;
	int						iCurDataPointer;
	int						iPacketID;
	int						iContinInd;
};


/* Decoder ------------------------------------------------------------------ */
class CDataDecoder : public CReceiverModul<_BINARY, _BINARY>
{
public:
	CDataDecoder() : iServPacketID(0), DoNotProcessData(TRUE),
		eAppType(AT_NOT_SUP), iOldJournalineServiceID(0) {}
	virtual ~CDataDecoder() {}
	enum EAppType {AT_NOT_SUP, AT_MOTSLISHOW, AT_JOURNALINE};

	_BOOLEAN GetSlideShowPicture(CMOTObject& NewPic);
	void GetNews(const int iObjID, CNews& News);
	EAppType GetAppType() {return eAppType;}

protected:
	class CDataUnit
	{
	public:
		CVector<_BINARY>	vecbiData;
		_BOOLEAN			bOK;
		_BOOLEAN			bReady;

		void Reset()
		{
			vecbiData.Init(0);
			bOK = FALSE;
			bReady = FALSE;
		}
	};

	int						iTotalPacketSize;
	int						iNumDataPackets;
	int						iMaxPacketDataSize;
	int						iServPacketID;
	CVector<int>			veciCRCOk;

	_BOOLEAN				DoNotProcessData;

	int						iContInd[MAX_NUM_PACK_PER_STREAM];
	CDataUnit				DataUnit[MAX_NUM_PACK_PER_STREAM];
	CMOTSlideShowDecoder	MOTSlideShow[MAX_NUM_PACK_PER_STREAM];
	CJournaline				Journaline;
	uint32_t				iOldJournalineServiceID;

	EAppType				eAppType;

	virtual void InitInternal(CParameter& ReceiverParam);
	virtual void ProcessDataInternal(CParameter& ReceiverParam);
};


#endif // !defined(DATADECODER_H__3B0BA660_CA3452363E7A0D31912__INCLUDED_)
