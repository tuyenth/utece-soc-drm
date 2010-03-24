/******************************************************************************\
* Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
* Copyright (c) 2001
*
* Author(s):
*	Alexander Kurpiers
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

#include "sound.h"

#include "../../common/GlobalDefinitions.h"
#include "../../common/util/Buffer.h"
#include "../../common/util/Vector.h"

#ifdef WITH_SOUND
#include <qthread.h>
#include <string.h>


#define RECORD 0
#define PLAY   1


#define SOUNDBUFLEN 102400

#define FRAGSIZE 8192
//#define FRAGSIZE 1024



/*****************************************************************************/

#ifdef USE_DEVDSP

#include <linux/soundcard.h>
#include <errno.h>

static int fdSound = 0;

void CSound::Init_HW( int mode )
{
	int arg;      /* argument for ioctl calls */
	int status;   /* return status of system calls */
	
	if (fdSound >0) 
	{
//		qDebug("already open");
		return;	// already open
	}

	/* Open sound device (Use O_RDWR only when writing a program which is
	   going to both record and play back digital audio) */
	fdSound = open("/dev/dsp", O_RDWR );
	if (fdSound < 0) 
		throw CGenErr("open of /dev/dsp failed");
	
	/* Get ready for us.
	   ioctl(audio_fd, SNDCTL_DSP_SYNC, 0) can be used when application wants 
	   to wait until last byte written to the device has been played (it doesn't
	   wait in recording mode). After that the call resets (stops) the device
	   and returns back to the calling program. Note that this call may take
	   several seconds to execute depending on the amount of data in the 
	   buffers. close() calls SNDCTL_DSP_SYNC automaticly */
	ioctl(fdSound, SNDCTL_DSP_SYNC, 0);

	/* Set sampling parameters always so that number of channels (mono/stereo) 
	   is set before selecting sampling rate! */
	/* Set number of channels (0=mono, 1=stereo) */
	arg = NUM_IN_OUT_CHANNELS - 1;
	status = ioctl(fdSound, SNDCTL_DSP_STEREO, &arg);
	if (status == -1) 
		throw CGenErr("SNDCTL_DSP_CHANNELS ioctl failed");		

	if (arg != (NUM_IN_OUT_CHANNELS - 1))
		throw CGenErr("unable to set number of channels");		
	

	/* Sampling rate */
	arg = SOUNDCRD_SAMPLE_RATE;
	status = ioctl(fdSound, SNDCTL_DSP_SPEED, &arg);
	if (status == -1)
		throw CGenErr("SNDCTL_DSP_SPEED ioctl failed");
	if (arg != SOUNDCRD_SAMPLE_RATE)
		throw CGenErr("unable to set sample rate");
	

	/* Sample size */
	arg = (BITS_PER_SAMPLE == 16) ? AFMT_S16_LE : AFMT_U8;      
	status = ioctl(fdSound, SNDCTL_DSP_SAMPLESIZE, &arg);
	if (status == -1)
		throw CGenErr("SNDCTL_DSP_SAMPLESIZE ioctl failed");
	if (arg != ((BITS_PER_SAMPLE == 16) ? AFMT_S16_LE : AFMT_U8))
		throw CGenErr("unable to set sample size");


	/* Check capabilities of the sound card */
	status = ioctl(fdSound, SNDCTL_DSP_GETCAPS, &arg);
	if (status ==  -1)
		throw CGenErr("SNDCTL_DSP_GETCAPS ioctl failed");
	if ((arg & DSP_CAP_DUPLEX) == 0)
		throw CGenErr("Soundcard not full duplex capable!");
}


int CSound::read_HW( void * recbuf, int size) {
	
	int ret = read(fdSound, recbuf, size * NUM_IN_OUT_CHANNELS * BYTES_PER_SAMPLE );

	if (ret < 0) {
		if ( (errno != EINTR) && (errno != EAGAIN))
			throw CGenErr("CSound:Read");
		else
			return 0;
	} else
		return ret / (NUM_IN_OUT_CHANNELS * BYTES_PER_SAMPLE);
}

int CSound::write_HW( _SAMPLE *playbuf, int size ){

	int start = 0;
	int ret;

	size *= BYTES_PER_SAMPLE * NUM_IN_OUT_CHANNELS;

	while (size) {

		ret = write(fdSound, &playbuf[start], size);
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN) 
			{
				continue;
			}
			throw CGenErr("CSound:Write");
		}
		size -= ret;
		start += ret / BYTES_PER_SAMPLE;
	}
	return 0;
}

void CSound::close_HW( void ) {

	if (fdSound >0)
		close(fdSound);
	fdSound = 0;

}
#endif

/*****************************************************************************/

#ifdef USE_ALSA

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API

#include <alsa/asoundlib.h>

static snd_pcm_t *rhandle = NULL;
static snd_pcm_t *phandle = NULL;


void CSound::Init_HW( int mode ){
 	
	int err, dir;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_sw_params_t *swparams;
	unsigned int rrate;
	snd_pcm_uframes_t period_size = FRAGSIZE * NUM_IN_OUT_CHANNELS/2;
	snd_pcm_uframes_t buffer_size;
	int periods = 2;
	snd_pcm_t *  handle;
	
	
	/* playback/record device */
	static const char *recdevice = "hw:0,0";	
	static const char *playdevice = "hw:0,0";	
	
	if (mode == RECORD) {
		if (rhandle != NULL)
			return;
			
		err = snd_pcm_open( &rhandle, recdevice, SND_PCM_STREAM_CAPTURE, 0 );
		if ( err != 0) 
		{
			qDebug("open error: %s", snd_strerror(err));
			throw CGenErr("alsa CSound::Init_HW record");	
		}
		handle = rhandle;
	} else {
		if (phandle != NULL)
			return;

		err = snd_pcm_open( &phandle, playdevice, SND_PCM_STREAM_PLAYBACK, 0 );
		if ( err != 0) 
		{
			qDebug("open error: %s", snd_strerror(err));
			throw CGenErr("alsa CSound::Init_HW playback");	
		}
		handle = phandle;
	}
	
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);
	
	/* Choose all parameters */
	err = snd_pcm_hw_params_any(handle, hwparams);
	if (err < 0) {
		qDebug("Broken configuration : no configurations available: %s", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");	
	}
	/* Set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);	

	if (err < 0) {
		qDebug("Access type not available : %s", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");	
		
	}
	/* Set the sample format */
	err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16);
	if (err < 0) {
		qDebug("Sample format not available : %s", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");	
	}
	/* Set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, hwparams, NUM_IN_OUT_CHANNELS);
	if (err < 0) {
		qDebug("Channels count (%i) not available s: %s", NUM_IN_OUT_CHANNELS, snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
	}
	/* Set the stream rate */
	rrate = SOUNDCRD_SAMPLE_RATE;
	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rrate, &dir);
	if (err < 0) {
		qDebug("Rate %iHz not available : %s dir %d", rrate, snd_strerror(err), dir);
		throw CGenErr("alsa CSound::Init_HW ");
		
	}
	if (rrate != SOUNDCRD_SAMPLE_RATE) {
		qDebug("Rate doesn't match (requested %iHz, get %iHz)", rrate, err);
		throw CGenErr("alsa CSound::Init_HW ");
	}
	
	dir=0;
	unsigned int buffer_time = 500000;              /* ring buffer length in us */
        /* set the buffer time */
        err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, &dir);
        if (err < 0) {
                qDebug("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
        }
        err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
        if (err < 0) {
                qDebug("Unable to get buffer size for playback: %s\n", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
        }
	// qDebug("buffer size %d", buffer_size);
        /* set the period time */
	unsigned int period_time = 100000;              /* period time in us */
        err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, &dir);
        if (err < 0) {
                qDebug("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
        }
        err = snd_pcm_hw_params_get_period_size(hwparams, &period_size, &dir);
        if (err < 0) {
                qDebug("Unable to get period size for playback: %s\n", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
        }
	// qDebug("period size %d", period_size);

	/* Write the parameters to device */
	err = snd_pcm_hw_params(handle, hwparams);
	if (err < 0) {
		qDebug("Unable to set hw params : %s", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
	}
	/* Get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		qDebug("Unable to determine current swparams : %s", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
	}
	if (mode == RECORD) {
		/* Start the transfer when the buffer immediately */
		err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 0);
		if (err < 0) {
			qDebug("Unable to set start threshold mode : %s", snd_strerror(err));
			throw CGenErr("alsa CSound::Init_HW ");
		}
		/* Allow the transfer when at least period_size samples can be processed */
		err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
		if (err < 0) {
			qDebug("Unable to set avail min : %s", snd_strerror(err));
			throw CGenErr("alsa CSound::Init_HW ");
		}
		/* Align all transfers to 1 sample */
		err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
		if (err < 0) {
			qDebug("Unable to set transfer align : %s", snd_strerror(err));
			throw CGenErr("alsa CSound::Init_HW ");
		}
	}
	/* Write the parameters to the record/playback device */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		qDebug("Unable to set sw params : %s", snd_strerror(err));
		throw CGenErr("alsa CSound::Init_HW ");
	}
	snd_pcm_start(handle);
	qDebug("alsa init done");

}

int CSound::read_HW( void * recbuf, int size) {

	int ret = snd_pcm_readi(rhandle, recbuf, size);


	if (ret < 0) 
	{
		if (ret == -EPIPE) 
		{    
			qDebug("rpipe");
			/* Under-run */
			qDebug("rprepare");
			ret = snd_pcm_prepare(rhandle);

			if (ret < 0)
				qDebug("Can't recover from underrun, prepare failed: %s", snd_strerror(ret));

			ret = snd_pcm_start(rhandle);

			if (ret < 0)
				qDebug("Can't recover from underrun, start failed: %s", snd_strerror(ret));
			return 0;

		} 
		else if (ret == -ESTRPIPE) 
		{
			qDebug("strpipe");

			/* Wait until the suspend flag is released */
			while ((ret = snd_pcm_resume(rhandle)) == -EAGAIN)
				sleep(1);       

			if (ret < 0) 
			{
				ret = snd_pcm_prepare(rhandle);

				if (ret < 0)
					qDebug("Can't recover from suspend, prepare failed: %s", snd_strerror(ret));
					throw CGenErr("CSound:Read");
			}
			return 0;
		} 
		else 
		{
			qDebug("CSound::Read: %s", snd_strerror(ret));
			throw CGenErr("CSound:Read");
		}
	} else 
		return ret;
			
}

int CSound::write_HW( _SAMPLE *playbuf, int size ){

	int start = 0;
	int ret;

	while (size) {

		ret = snd_pcm_writei(phandle, &playbuf[start], size );
		if (ret < 0) {
			if (ret ==  -EAGAIN) {
				if ((ret = snd_pcm_wait (phandle, 100)) < 0) {
			        	qDebug ("poll failed (%s)", snd_strerror (ret));
			        	break;
				}	           
				continue;
			} else 
			if (ret == -EPIPE) {    /* under-run */
qDebug("underrun");
        			ret = snd_pcm_prepare(phandle);
        			if (ret < 0)
                			qDebug("Can't recover from underrun, prepare failed: %s", snd_strerror(ret));
        			continue;
			} else if (ret == -ESTRPIPE) {
qDebug("strpipe");
        			while ((ret = snd_pcm_resume(phandle)) == -EAGAIN)
                			sleep(1);       /* wait until the suspend flag is released */
        			if (ret < 0) {
                			ret = snd_pcm_prepare(phandle);
                			if (ret < 0)
                        			qDebug("Can't recover from suspend, prepare failed: %s", snd_strerror(ret));
        			}
        			continue;
			} else {
                                qDebug("Write error: %s", snd_strerror(ret));
								throw CGenErr("Write error");
                        }
                        break;  /* skip one period */
		}
		size -= ret;
		start += ret;
	}
	return 0;
}
void CSound::close_HW( void ) {

	if (rhandle != NULL)
		snd_pcm_close( rhandle );

	rhandle = NULL;
}

#endif




/* ************************************************************************* */

class CSoundBuf : public CCyclicBuffer<_SAMPLE> {

public:
	CSoundBuf() : keep_running(TRUE) {}
	void lock (void){ data_accessed.lock(); }
	void unlock (void){ data_accessed.unlock(); }
	
	bool keep_running;
protected:
	QMutex	data_accessed;
} SoundBufP, SoundBufR;


class RecThread : public QThread {
public:
	virtual void run() {
	
	
		while (SoundBufR.keep_running) {

			int fill;

			SoundBufR.lock();
			fill = SoundBufR.GetFillLevel();
			SoundBufR.unlock();
				
			if (  (SOUNDBUFLEN - fill) > (FRAGSIZE * NUM_IN_OUT_CHANNELS) ) {
				// enough space in the buffer
				
				int size = CSound::read_HW( tmprecbuf, FRAGSIZE);

				// common code
				if (size > 0) {
					CVectorEx<_SAMPLE>*	ptarget;

					/* Copy data from temporary buffer in output buffer */
					SoundBufR.lock();

					ptarget = SoundBufR.QueryWriteBuffer();

					for (int i = 0; i < size * NUM_IN_OUT_CHANNELS; i++)
						(*ptarget)[i] = tmprecbuf[i];

					SoundBufR.Put( size * NUM_IN_OUT_CHANNELS );
					SoundBufR.unlock();
				}
			} else {
				msleep( 1 );
			}
		}
		qDebug("Rec Thread stopped");
	}

protected:
	_SAMPLE	tmprecbuf[NUM_IN_OUT_CHANNELS * FRAGSIZE];
} RecThread1;



/* Wave in ********************************************************************/

void CSound::InitRecording(int iNewBufferSize, _BOOLEAN bNewBlocking)
{
	qDebug("initrec %d", iNewBufferSize);

	/* Save < */
	SoundBufR.lock();
	iInBufferSize = iNewBufferSize;
	bBlockingRec = bNewBlocking;
	SoundBufR.unlock();
	
	Init_HW( RECORD );

	if ( RecThread1.running() == FALSE ) {
		SoundBufR.Init( SOUNDBUFLEN );
		SoundBufR.unlock();
		RecThread1.start();
	}

}


_BOOLEAN CSound::Read(CVector< _SAMPLE >& psData)
{
	CVectorEx<_SAMPLE>*	p;

	SoundBufR.lock();	// we need exclusive access
	
	
	while ( SoundBufR.GetFillLevel() < iInBufferSize ) {
	
		
		// not enough data, sleep a little
		SoundBufR.unlock();
		usleep(1000); //1ms
		SoundBufR.lock();
	}
	
	// copy data
	
	p = SoundBufR.Get( iInBufferSize );
	for (int i=0; i<iInBufferSize; i++)
		psData[i] = (*p)[i];
	
	SoundBufR.unlock();

	return FALSE;
}


/* Wave out *******************************************************************/


class PlayThread : public QThread {
public:
	virtual void run() {
	
		while ( SoundBufP.keep_running ) {
			int fill;

			SoundBufP.lock();
			fill = SoundBufP.GetFillLevel();
			SoundBufP.unlock();
				
			if ( fill > (FRAGSIZE * NUM_IN_OUT_CHANNELS) ) {

				// enough data in the buffer

				CVectorEx<_SAMPLE>*	p;
				
				SoundBufP.lock();
				p = SoundBufP.Get( FRAGSIZE * NUM_IN_OUT_CHANNELS );

				for (int i=0; i < FRAGSIZE * NUM_IN_OUT_CHANNELS; i++)
					tmpplaybuf[i] = (*p)[i];

				SoundBufP.unlock();
				
				CSound::write_HW( tmpplaybuf, FRAGSIZE );

			} else {
			
				do {			
					msleep( 1 );
					
					SoundBufP.lock();
					fill = SoundBufP.GetFillLevel();
					SoundBufP.unlock();

				} while ((SoundBufP.keep_running) && ( fill < SOUNDBUFLEN/2 ));	// wait until buffer is at least half full
			}
		}
		qDebug("Play Thread stopped");
	}

protected:
	_SAMPLE	tmpplaybuf[NUM_IN_OUT_CHANNELS * FRAGSIZE];
} PlayThread1;


void CSound::InitPlayback(int iNewBufferSize, _BOOLEAN bNewBlocking)
{
	qDebug("initplay %d", iNewBufferSize);
	
	/* Save buffer size */
	SoundBufP.lock();
	iBufferSize = iNewBufferSize;
	bBlockingPlay = bNewBlocking;
	SoundBufP.unlock();

	Init_HW( PLAY );

	if ( PlayThread1.running() == FALSE ) {
		SoundBufP.Init( SOUNDBUFLEN );
		SoundBufP.unlock();
		PlayThread1.start();
	}
}


_BOOLEAN CSound::Write(CVector< _SAMPLE >& psData)
{

	if ( bBlockingPlay ) {
		// blocking write
		while( SoundBufP.keep_running ) {
			SoundBufP.lock();
			int fill = SOUNDBUFLEN - SoundBufP.GetFillLevel();
			SoundBufP.unlock();
			if ( fill > iBufferSize) break;
		}
	}
	
	SoundBufP.lock();	// we need exclusive access

	if ( ( SOUNDBUFLEN - SoundBufP.GetFillLevel() ) > iBufferSize) {
		 
		CVectorEx<_SAMPLE>*	ptarget;
		 
		 // data fits, so copy
		 ptarget = SoundBufP.QueryWriteBuffer();

		 for (int i=0; i < iBufferSize; i++)
		 	(*ptarget)[i] = psData[i];

		 SoundBufP.Put( iBufferSize );
	}
	
	SoundBufP.unlock();

	return FALSE;
}
void CSound::Close()
{
	qDebug("stoprec/play");
	
	// stop the recording and playback threads
	
	if (RecThread1.running() ) {
		SoundBufR.keep_running = FALSE;
		// wait 1sec max. for the threads to terminate
		RecThread1.wait(1000);
	}
	
	if (PlayThread1.running() ) {
		SoundBufP.keep_running = FALSE;
		PlayThread1.wait(1000);
	}
	
	close_HW();	
}


#endif
