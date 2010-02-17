/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Alexander Kurpiers
 * 
 * Decription:
 * Linux sound interface
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

#ifndef _SOUND_H
#define _SOUND_H

#include "../../common/GlobalDefinitions.h"
#include "../../common/util/Vector.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>


/* Definitions ****************************************************************/
#define	NUM_IN_OUT_CHANNELS		2		/* Stereo recording (but we only
										   use one channel for recording) */
#define	BITS_PER_SAMPLE			16		/* Use all bits of the D/A-converter */
#define BYTES_PER_SAMPLE		2		/* Number of bytes per sample */

#define RECORDING_CHANNEL		0		/* 0: Left, 1: Right

/* Classes ********************************************************************/
class CSound
{
public:
	CSound() {}
	virtual ~CSound() {}

	/* Not implemented yet, always return one device and default string */
	int		GetNumDev() {return 1;}
	string	GetDeviceName(int iDiD) {return "Default Sound Device";}
	void	SetOutDev(int iNewDev) {}
	void	SetInDev(int iNewDev) {}

	/* Return invalid device ID which is the same as using "wave mapper" which
	   we assume here to be used */
	int		GetOutDev() {return 1;}
	int		GetInDev() {return 1;}

#if WITH_SOUND
	void InitRecording(int iNewBufferSize, _BOOLEAN bNewBlocking = TRUE);
	void InitPlayback(int iNewBufferSize, _BOOLEAN bNewBlocking = FALSE);
	_BOOLEAN Read(CVector<short>& psData);
	_BOOLEAN Write(CVector<short>& psData);

	void Close();
	
protected:
	int 	iBufferSize, iInBufferSize;
	void Init_HW( int mode );

	friend class RecThread;
	friend class PlayThread;
	static int read_HW( void * recbuf, int size);
	static int write_HW( _SAMPLE *playbuf, int size );
	void close_HW( void );
	
	short int *tmpplaybuf, *tmprecbuf;
	_BOOLEAN	bBlockingRec;
	_BOOLEAN	bBlockingPlay;
	
	
#else
	/* Dummy definitions */
	void InitRecording(int iNewBufferSize, _BOOLEAN bNewBlocking = TRUE){}
	void InitPlayback(int iNewBufferSize, _BOOLEAN bNewBlocking = FALSE){}
	_BOOLEAN Read(CVector<short>& psData){return FALSE;}
	_BOOLEAN Write(CVector<short>& psData){return FALSE;}
	void Close(){}
#endif
};


#endif
