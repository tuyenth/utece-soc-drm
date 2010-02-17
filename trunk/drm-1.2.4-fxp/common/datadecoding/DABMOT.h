/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	See DABMOT.cpp
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

#if !defined(DABMOT_H__3B0UBVE98732KJVEW363E7A0D31912__INCLUDED_)
#define DABMOT_H__3B0UBVE98732KJVEW363E7A0D31912__INCLUDED_

#include "../GlobalDefinitions.h"
#include "../util/Vector.h"
#include "../util/CRC.h"


/* Classes ********************************************************************/
class CMOTObjectRaw
{
public:
	class CDataUnit
	{
	public:
		CDataUnit() {Reset();}

		void Reset();
		void Add(CVector<_BINARY>& vecbiNewData,
			const int iSegmentSize, const int iSegNum);

		CVector<_BINARY>	vecbiData;
		_BOOLEAN			bOK, bReady;
		int					iDataSegNum;
	};

	int			iTransportID;
	CDataUnit	Header;
	CDataUnit	Body;
};

class CMOTObject
{
public:
	CMOTObject() {Reset();}
	CMOTObject(const CMOTObject& NewObj) : vecbRawData(NewObj.vecbRawData),
		strFormat(NewObj.strFormat), strName(NewObj.strName) {}

	inline CMOTObject& operator=(const CMOTObject& NewObj)
	{
		strName = NewObj.strName;
		strFormat = NewObj.strFormat;
		vecbRawData.Init(NewObj.vecbRawData.Size());
		vecbRawData = NewObj.vecbRawData;

		return *this;
	}

	void Reset()
	{
		vecbRawData.Init(0);
		strFormat = "";
		strName = "";
	}

	CVector<_BYTE>	vecbRawData;
	string			strName;
	string			strFormat;
};


/* Encoder ------------------------------------------------------------------ */
class CMOTDABEnc
{
public:
	CMOTDABEnc() {}
	virtual ~CMOTDABEnc() {}

	void Reset();
	_BOOLEAN GetDataGroup(CVector<_BINARY>& vecbiNewData);
	void SetMOTObject(CMOTObject& NewMOTObject);
	_REAL GetProgPerc() const;

protected:
	class CMOTObjSegm
	{
	public:
		CVector<CVector<_BINARY> > vvbiHeader;
		CVector<CVector<_BINARY> > vvbiBody;
	};

	void GenMOTSegments(CMOTObjSegm& MOTObjSegm);
	void PartitionUnits(CVector<_BINARY>& vecbiSource,
						CVector<CVector<_BINARY> >& vecbiDest,
						const int iPartiSize);

	void GenMOTObj(CVector<_BINARY>& vecbiData, CVector<_BINARY>& vecbiSeg,
				   const _BOOLEAN bHeader, const int iSegNum,
				   const int iTranspID, const _BOOLEAN bLastSeg);

	CMOTObject		MOTObject;
	CMOTObjSegm		MOTObjSegments;

	int				iSegmCntHeader;
	int				iSegmCntBody;
	_BOOLEAN		bCurSegHeader;

	int				iContIndexHeader;
	int				iContIndexBody;

	int				iTransportID;
};


/* Decoder ------------------------------------------------------------------ */
class CMOTDABDec
{
public:
	CMOTDABDec() {}
	virtual ~CMOTDABDec() {}

	_BOOLEAN	AddDataGroup(CVector<_BINARY>& vecbiNewData);
	void		GetMOTObject(CMOTObject& NewMOTObject)
					{NewMOTObject = MOTObject; /* Simply copy object */}

protected:
	void DecodeObject(CMOTObjectRaw& MOTObjectRaw);

	CMOTObject		MOTObject;
	CMOTObjectRaw	MOTObjectRaw;
};


#endif // !defined(DABMOT_H__3B0UBVE98732KJVEW363E7A0D31912__INCLUDED_)
