/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Audio source encoder/decoder
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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 1111
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "AudioSourceDecoder.h"


/* Implementation *************************************************************/
/******************************************************************************\
* Encoder                                                                      *
\******************************************************************************/
void CAudioSourceEncoder::ProcessDataInternal(CParameter& TransmParam)
{
	int i, j;

	/* Reset data to zero. This is important since usually not all data is used
	   and this data has to be set to zero as defined in the DRM standard */
	for (i = 0; i < iOutputBlockSize; i++)
		(*pvecOutputData)[i] = 0;


#ifdef USE_FAAC_LIBRARY
	if (bIsDataService == FALSE)
	{
		/* AAC encoder ------------------------------------------------------ */
		/* Resample data to encoder bit-rate */
		/* Change type of data (short -> real), take left channel! */
		for (i = 0; i < iInputBlockSize / 2; i++)
			vecTempResBufIn[i] = (*pvecInputData)[i * 2];

		/* Resample data */
		ResampleObj.Resample(vecTempResBufIn, vecTempResBufOut);

		/* Split data in individual audio blocks */
		for (j = 0; j < iNumAACFrames; j++)
		{
			/* Convert _REAL type to _SAMPLE type, copy in smaller buffer */
			for (i = 0; i < lNumSampEncIn; i++)
			{
				vecsEncInData[i] =
					Real2Sample(vecTempResBufOut[j * lNumSampEncIn + i]);
			}

			/* Actual AAC encoding */
			CVector<unsigned char> vecsTmpData(lMaxBytesEncOut);
			int bytesEncoded = faacEncEncode(hEncoder,
				(int32_t*) &vecsEncInData[0], lNumSampEncIn, &vecsTmpData[0],
				lMaxBytesEncOut);

			if (bytesEncoded > 0)
			{
				/* Extract CRC */
				aac_crc_bits[j] = vecsTmpData[0];

				/* Extract actual data */
				for (i = 0; i < bytesEncoded - 1 /* "-1" for CRC */; i++)
					audio_frame[j][i] = vecsTmpData[i + 1];

				/* Store block lengths for boarders in AAC super-frame-header */
				veciFrameLength[j] = bytesEncoded - 1;
			}
			else
			{
				/* Encoder is in initialization phase, reset CRC and length */
				aac_crc_bits[j] = 0;
				veciFrameLength[j] = 0;
			}
		}

		/* Write data to output vector */
		/* First init buffer with zeros */
		for (i = 0; i < iOutputBlockSize; i++)
			(*pvecOutputData)[i] = 0;

		/* Reset bit extraction access */
		(*pvecOutputData).ResetBitAccess();

		/* AAC super-frame-header */
		int iAccFrameLength = 0;
		for (j = 0; j < iNumAACFrames - 1; j++)
		{
			iAccFrameLength += veciFrameLength[j];

			/* Frame border in bytes (12 bits) */
			(*pvecOutputData).Enqueue(iAccFrameLength, 12);
		}

		/* Byte-alignment (4 bits) in case of 10 audio frames */
		if (iNumAACFrames == 10)
			(*pvecOutputData).Enqueue(0, 4);

		/* Higher protected part */
		int iCurNumBytes = 0;
		for (j = 0; j < iNumAACFrames; j++)
		{
			/* Data */
			for (i = 0; i < iNumHigherProtectedBytes; i++)
			{
				/* Check if enough data is available, set data to 0 if not */
				if (i < veciFrameLength[j])
					(*pvecOutputData).Enqueue(audio_frame[j][i], 8);
				else
					(*pvecOutputData).Enqueue(0, 8);

				iCurNumBytes++;
			}

			/* CRCs */
			(*pvecOutputData).Enqueue(aac_crc_bits[j], 8);
		}

		/* Lower protected part */
		for (j = 0; j < iNumAACFrames; j++)
		{
			for (i = iNumHigherProtectedBytes; i < veciFrameLength[j]; i++)
			{
				/* If encoder produced too many bits, we have to drop them */
				if (iCurNumBytes < iAudioPayloadLen)
					(*pvecOutputData).Enqueue(audio_frame[j][i], 8);

				iCurNumBytes++;
			}
		}

#ifdef _DEBUG_
/* Save number of bits actually used by audio encoder */
static FILE* pFile = fopen("test/audbits.dat", "w");
fprintf(pFile, "%d %d\n", iAudioPayloadLen, iCurNumBytes);
fflush(pFile);
#endif
	}
#endif


	/* Data service and text message application ---------------------------- */
	if (bIsDataService == TRUE)
	{
// TODO: make a separate modul for data encoding
		/* Write data packets in stream */
		CVector<_BINARY> vecbiData;
		const int iNumPack = iOutputBlockSize / iTotPacketSize;
		int iPos = 0;

		for (int j = 0; j < iNumPack; j++)
		{
			/* Get new packet */
			DataEncoder.GeneratePacket(vecbiData);

			/* Put it on stream */
			for (i = 0; i < iTotPacketSize; i++)
			{
				(*pvecOutputData)[iPos] = vecbiData[i];
				iPos++;
			}
		}
	}
	else
	{
		/* Text message application. Last four bytes in stream are written */
		if (bUsingTextMessage == TRUE)
		{
			/* Always four bytes for text message "piece" */
			CVector<_BINARY> vecbiTextMessBuf(
				SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR);

			/* Get a "piece" */
			TextMessage.Encode(vecbiTextMessBuf);

			/* Calculate start point for text message */
			const int iByteStartTextMess = iTotNumBitsForUsage - SIZEOF__BYTE *
				NUM_BYTES_TEXT_MESS_IN_AUD_STR;

			/* Add text message bytes to output stream */
			for (i = iByteStartTextMess; i < iTotNumBitsForUsage; i++)
				(*pvecOutputData)[i] = vecbiTextMessBuf[i -	iByteStartTextMess];
		}
	}
}

void CAudioSourceEncoder::InitInternal(CParameter& TransmParam)
{
	int iCurStreamID;

int iCurSelServ = 0; // TEST

	/* Calculate number of input samples in mono. Audio block are always
	   400 ms long */
	const int iNumInSamplesMono = (int) ((_REAL) SOUNDCRD_SAMPLE_RATE *
		(_REAL) 0.4 /* 400 ms */);

	/* Set the total available number of bits, byte aligned */
	iTotNumBitsForUsage =
		(TransmParam.iNumDecodedBitsMSC / SIZEOF__BYTE) * SIZEOF__BYTE;

	/* Total number of bytes which can be used for data and audio */
	const int iTotNumBytesForUsage = iTotNumBitsForUsage / SIZEOF__BYTE;

	if (TransmParam.iNumDataService == 1)
	{
		/* Data service ----------------------------------------------------- */
		bIsDataService = TRUE;
		iTotPacketSize = DataEncoder.Init(TransmParam);

		/* Get stream ID for data service */
		iCurStreamID = TransmParam.Service[iCurSelServ].DataParam.iStreamID;
	}
	else
	{
		/* Audio service ---------------------------------------------------- */
		bIsDataService = FALSE;

		/* Get stream ID for audio service */
		iCurStreamID = TransmParam.Service[iCurSelServ].AudioParam.iStreamID;

#ifdef USE_FAAC_LIBRARY
		/* Total frame size is input block size minus the bytes for the text
		   message (if text message is used) */
		int iTotAudFraSizeBits = iTotNumBitsForUsage;
		if (bUsingTextMessage == TRUE)
			iTotAudFraSizeBits -= SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR;

		/* Set encoder sample rate. This parameter decides other parameters */
// TEST make threshold decision TODO: improvement
if (iTotAudFraSizeBits > 7000) /* in bits! */
	lEncSamprate = 24000;
else
	lEncSamprate = 12000;

		int iTimeEachAudBloMS;
		int	iNumHeaderBytes;

		switch (lEncSamprate)
		{
		case 12000:
			iTimeEachAudBloMS = 80; /* ms */
			iNumAACFrames = 5;
			iNumHeaderBytes = 6;
			TransmParam.Service[iCurSelServ].AudioParam.eAudioSamplRate =
				CParameter::AS_12KHZ; /* Set parameter in global struct */
			break;

		case 24000:
			iTimeEachAudBloMS = 40; /* ms */
			iNumAACFrames = 10;
			iNumHeaderBytes = 14;
			TransmParam.Service[iCurSelServ].AudioParam.eAudioSamplRate =
				CParameter::AS_24KHZ; /* Set parameter in global struct */
			break;
		}

		/* The audio_payload_length is derived from the length of the audio
		   super frame (data_length_of_part_A + data_length_of_part_B)
		   subtracting the audio super frame overhead (bytes used for the audio
		   super frame header() and for the aac_crc_bits) (5.3.1.1, Table 5) */
		iAudioPayloadLen = iTotAudFraSizeBits / SIZEOF__BYTE -
			iNumHeaderBytes - iNumAACFrames /* for CRCs */;

		const int iActEncOutBytes = (int) (iAudioPayloadLen / iNumAACFrames);

		/* Set to mono */
		TransmParam.Service[iCurSelServ].AudioParam.eAudioMode =
			CParameter::AM_MONO;

		/* Open encoder instance */
		if (hEncoder != NULL)
			faacEncClose(hEncoder);

		hEncoder = faacEncOpen(lEncSamprate, 1 /* mono */,
			&lNumSampEncIn, &lMaxBytesEncOut);

// TEST needed since 960 transform length is not yet implemented in faac!
int iBitRate;
if (lNumSampEncIn == 1024)
	iBitRate = (int) (((_REAL) iActEncOutBytes * SIZEOF__BYTE * 960.0 / 1024.0) /
		iTimeEachAudBloMS * 1000);
else
	iBitRate = (int) (((_REAL) iActEncOutBytes * SIZEOF__BYTE) /
		iTimeEachAudBloMS * 1000);

		/* Set encoder configuration */
		CurEncFormat = faacEncGetCurrentConfiguration(hEncoder);
		CurEncFormat->inputFormat = FAAC_INPUT_16BIT;
		CurEncFormat->useTns = 1;
		CurEncFormat->aacObjectType = LOW;
		CurEncFormat->mpegVersion = MPEG4;
		CurEncFormat->outputFormat = 0; /* (0 = Raw; 1 = ADTS -> Raw) */
		CurEncFormat->bitRate = iBitRate;
		CurEncFormat->bandWidth = 0; /* Let the encoder choose the bandwidth */
		faacEncSetConfiguration(hEncoder, CurEncFormat);

		/* Init storage for actual data, CRCs and frame lengths */
		audio_frame.Init(iNumAACFrames, lMaxBytesEncOut);
		vecsEncInData.Init(lNumSampEncIn);
		aac_crc_bits.Init(iNumAACFrames);
		veciFrameLength.Init(iNumAACFrames);

		/* Additional buffers needed for resampling since we need conversation
		   between _SAMPLE and _REAL */
		vecTempResBufIn.Init(iNumInSamplesMono);
		vecTempResBufOut.Init(lNumSampEncIn * iNumAACFrames, (_REAL) 0.0);

		/* Init resample objects */
// TEST needed since 960 transform length is not yet implemented in faac!
if (lNumSampEncIn == 1024)
	ResampleObj.Init(iNumInSamplesMono,
		(_REAL) lEncSamprate / SOUNDCRD_SAMPLE_RATE * 1024.0 / 960.0);
else
	ResampleObj.Init(iNumInSamplesMono,
		(_REAL) lEncSamprate / SOUNDCRD_SAMPLE_RATE);

		/* Calculate number of bytes for higher protected blocks */
		iNumHigherProtectedBytes =
			(TransmParam.Stream[iCurStreamID].iLenPartA
			- iNumHeaderBytes - iNumAACFrames /* CRC bytes */) / iNumAACFrames;

		if (iNumHigherProtectedBytes < 0)
			iNumHigherProtectedBytes = 0;
#endif
	}

	/* Adjust part B length for SDC stream. Notice, that the
	   "TransmParam.iNumDecodedBitsMSC" paramter depends on these settings.
	   Thus, lenght part A and B have to be set before, preferably in the
	   DRMTransmitter initialization */
	if ((TransmParam.Stream[iCurStreamID].iLenPartA == 0) ||
		(iTotNumBytesForUsage < TransmParam.Stream[iCurStreamID].iLenPartA))
	{
		/* Equal error protection was chosen or protection part A was chosen too
		   high, set to equal error protection! */
		TransmParam.Stream[iCurStreamID].iLenPartA = 0;
		TransmParam.Stream[iCurStreamID].iLenPartB = iTotNumBytesForUsage;
	}
	else
		TransmParam.Stream[iCurStreamID].iLenPartB = iTotNumBytesForUsage -
			TransmParam.Stream[iCurStreamID].iLenPartA;

	/* Define input and output block size */
	iOutputBlockSize = TransmParam.iNumDecodedBitsMSC;
	iInputBlockSize = iNumInSamplesMono * 2 /* stereo */;
}

void CAudioSourceEncoder::SetTextMessage(const string& strText)
{
	/* Set text message in text message object */
	TextMessage.SetMessage(strText);

	/* Set text message flag */
	bUsingTextMessage = TRUE;
}

void CAudioSourceEncoder::ClearTextMessage()
{
	/* Clear all text segments */
	TextMessage.ClearAllText();

	/* Clear text message flag */
	bUsingTextMessage = FALSE;
}

CAudioSourceEncoder::~CAudioSourceEncoder()
{
#ifdef USE_FAAC_LIBRARY
	/* Close encoder instance afterwards */
	if (hEncoder != NULL)
		faacEncClose(hEncoder);
#endif
}


/******************************************************************************\
* Decoder                                                                      *
\******************************************************************************/
void CAudioSourceDecoder::ProcessDataInternal(CParameter& ReceiverParam)
{
	int i;

	/* Check if something went wrong in the initialization routine */
	if (DoNotProcessData == TRUE)
		return;


	/* Text Message ***********************************************************/
	/* Total frame size depends on whether text message is used or not */
	if (bTextMessageUsed == TRUE)
	{
		/* Decode last for bytes of input block for text message */
		for (i = 0; i < SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR; i++)
			vecbiTextMessBuf[i] = (*pvecInputData)[iTotalFrameSize + i];

		TextMessage.Decode(vecbiTextMessBuf);
	}


#ifdef USE_FAAD2_LIBRARY
	faacDecFrameInfo	DecFrameInfo;
	_BOOLEAN			bGoodValues;
	short*				psDecOutSampleBuf;
	int					j;

	/* Check if AAC should not be decoded */
	if (DoNotProcessAAC == TRUE)
		return;


	/* Extract audio data from stream *****************************************/
	/* Reset bit extraction access */
	(*pvecInputData).ResetBitAccess();


	/* AAC super-frame-header ----------------------------------------------- */
	int iPrevBorder = 0;
	for (i = 0; i < iNumBorders; i++)
	{
		/* Frame border in bytes (12 bits) */
		const int iFrameBorder = (*pvecInputData).Separate(12);

		/* The lenght is difference between borders */
		veciFrameLength[i] = iFrameBorder - iPrevBorder;
		iPrevBorder = iFrameBorder;
	}

	/* Byte-alignment (4 bits) in case of 10 audio frames */
	if (iNumBorders == 9)
		(*pvecInputData).Separate(4); 

	/* Frame length of last frame */
	veciFrameLength[iNumBorders] = iAudioPayloadLen - iPrevBorder;

	/* Check if frame length entries represent possible values */
	bGoodValues = TRUE;
	for (i = 0; i < iNumAACFrames; i++)
	{
		if ((veciFrameLength[i] < 0) ||
			(veciFrameLength[i] > iMaxLenOneAudFrame))
		{
			bGoodValues = FALSE;
		}
	}

	if (bGoodValues == TRUE)
	{
		/* Higher-protected part -------------------------------------------- */
		for (i = 0; i < iNumAACFrames; i++)
		{
			/* Extract higher protected part bytes (8 bits per byte) */
			for (j = 0; j < iNumHigherProtectedBytes; j++)
				audio_frame[i][j] = (*pvecInputData).Separate(8);

			/* Extract CRC bits (8 bits) */
			aac_crc_bits[i] = (*pvecInputData).Separate(8);
		}


		/* Lower-protected part --------------------------------------------- */
		for (i = 0; i < iNumAACFrames; i++)
		{
			/* First calculate frame length, derived from higher protected part
			   frame length and total size */
			const int iNumLowerProtectedBytes =
				veciFrameLength[i] - iNumHigherProtectedBytes;

			/* Extract lower protected part bytes (8 bits per byte) */
			for (j = 0; j < iNumLowerProtectedBytes; j++)
			{
				audio_frame[i][iNumHigherProtectedBytes + j] =
					(*pvecInputData).Separate(8);
			}
		}
	}


	/* AAC decoder ************************************************************/
	/* Init output block size to zero, this variable is also used for
	   determining the position for writing the output vector */
	iOutputBlockSize = 0;

	for (j = 0; j < iNumAACFrames; j++)
	{
		if (bGoodValues == TRUE)
		{
			/* Prepare data vector with CRC at the beginning (the definition
			   with faad2 DRM interface) */
			vecbyPrepAudioFrame[0] = aac_crc_bits[j];

			for (i = 0; i < veciFrameLength[j]; i++)
				vecbyPrepAudioFrame[i + 1] = audio_frame[j][i];

#if 0
// Store AAC-data in file
string strAACTestFileName = "test/aac_";
if (ReceiverParam.Service[ReceiverParam.GetCurSelAudioService()].
	AudioParam.eAudioSamplRate == CParameter::AS_12KHZ)
{
	strAACTestFileName += "12kHz_";
}
else
	strAACTestFileName += "24kHz_";

switch (ReceiverParam.Service[ReceiverParam.GetCurSelAudioService()].
	AudioParam.eAudioMode)
{
case CParameter::AM_MONO:
	strAACTestFileName += "mono";
	break;

case CParameter::AM_P_STEREO:
	strAACTestFileName += "pstereo";
	break;

case CParameter::AM_STEREO:
	strAACTestFileName += "stereo";
	break;
}

if (ReceiverParam.Service[ReceiverParam.GetCurSelAudioService()].AudioParam.
	eSBRFlag == CParameter::SB_USED)
{
	strAACTestFileName += "_sbr";
}
strAACTestFileName += ".dat";

static FILE* pFile2 = fopen(strAACTestFileName.c_str(), "wb");

int iNewFrL = veciFrameLength[j] + 1;

// Frame length
fwrite((void*) &iNewFrL, size_t(4), size_t(1), pFile2);

size_t count = veciFrameLength[j] + 1; /* Number of bytes to write */
fwrite((void*) &vecbyPrepAudioFrame[0], size_t(1), count, pFile2);

fflush(pFile2);
#endif

			/* Call decoder routine */
			psDecOutSampleBuf = (short*) NeAACDecDecode(HandleAACDecoder,
				&DecFrameInfo, &vecbyPrepAudioFrame[0], veciFrameLength[j] + 1);
		}
		else
		{
			/* DRM AAC header was wrong, set decoder error code */
			DecFrameInfo.error = 1;
		}

		if (DecFrameInfo.error != 0)
		{
			/* Set AAC CRC result in log file */
			ReceiverParam.ReceptLog.SetMSC(FALSE);

			if (bAudioWasOK == TRUE)
			{
				/* Post message to show that CRC was wrong (yellow light) */
				PostWinMessage(MS_MSC_CRC, 1);

				/* Fade-out old block to avoid "clicks" in audio. We use linear
				   fading which gives a log-fading impression */
				for (i = 0; i < iResOutBlockSize; i++)
				{
					/* Linear attenuation with time of OLD buffer */
					const _REAL rAtt =
						(_REAL) 1.0 - (_REAL) i / iResOutBlockSize;

					vecTempResBufOutOldLeft[i] *= rAtt;
					vecTempResBufOutOldRight[i] *= rAtt;

					if (bUseReverbEffect == TRUE)
					{
						/* Fade in input signal for reverberation to avoid
						   clicks */
						const _REAL rAttRev = (_REAL) i / iResOutBlockSize;

						/* Cross-fade reverberation effect */
						const _REAL rRevSam = (1.0 - rAtt) * AudioRev.
							ProcessSample(vecTempResBufOutOldLeft[i] * rAttRev,
							vecTempResBufOutOldRight[i] * rAttRev);

						/* Mono reverbration signal */
						vecTempResBufOutOldLeft[i] += rRevSam;
						vecTempResBufOutOldRight[i] += rRevSam;
					}
				}

				/* Set flag to show that audio block was bad */
				bAudioWasOK = FALSE;
			}
			else
			{
				/* Post message to show that CRC was wrong (red light) */
				PostWinMessage(MS_MSC_CRC, 2);

				if (bUseReverbEffect == TRUE)
				{
					/* Add Reverberation effect */
					for (i = 0; i < iResOutBlockSize; i++)
					{
						/* Mono reverberation signal */
						vecTempResBufOutOldLeft[i] =
							vecTempResBufOutOldRight[i] = AudioRev.
							ProcessSample(0, 0);
					}
				}
			}

			/* Write zeros in current output buffer */
			for (i = 0; i < iResOutBlockSize; i++)
			{
				vecTempResBufOutCurLeft[i] = (_REAL) 0.0;
				vecTempResBufOutCurRight[i] = (_REAL) 0.0;
			}
		}
		else
		{
			/* Set AAC CRC result in log file */
			ReceiverParam.ReceptLog.SetMSC(TRUE);

			/* Increment correctly decoded audio blocks counter */
			iNumCorDecAudio++;

			/* Post message to show that CRC was OK */
			PostWinMessage(MS_MSC_CRC, 0);

			/* Conversion from _SAMPLE vector to _REAL vector for resampling.
			   ATTENTION: We use a vector which was allocated inside
			   the AAC decoder! */
			if (DecFrameInfo.channels == 1)
			{
				/* Change type of data (short -> real) */
				for (i = 0; i < iLenDecOutPerChan; i++)
					vecTempResBufInLeft[i] = psDecOutSampleBuf[i];

				/* Resample data */
				ResampleObjL.Resample(vecTempResBufInLeft,
					vecTempResBufOutCurLeft);

				/* Mono (write the same audio material in both channels) */
				for (i = 0; i < iResOutBlockSize; i++)
					vecTempResBufOutCurRight[i] = vecTempResBufOutCurLeft[i];
			}
			else
			{
				/* Stereo */
				for (i = 0; i < iLenDecOutPerChan; i++)
				{
					vecTempResBufInLeft[i] = psDecOutSampleBuf[i * 2];
					vecTempResBufInRight[i] = psDecOutSampleBuf[i * 2 + 1];
				}

				/* Resample data */
				ResampleObjL.Resample(vecTempResBufInLeft,
					vecTempResBufOutCurLeft);
				ResampleObjR.Resample(vecTempResBufInRight,
					vecTempResBufOutCurRight);
			}

			if (bAudioWasOK == FALSE)
			{
				if (bUseReverbEffect == TRUE)
				{
					/* Add "last" reverbration only to old block */
					for (i = 0; i < iResOutBlockSize; i++)
					{
						/* Mono reverberation signal */
						vecTempResBufOutOldLeft[i] =
							vecTempResBufOutOldRight[i] = AudioRev.
							ProcessSample(vecTempResBufOutOldLeft[i],
							vecTempResBufOutOldRight[i]);
					}
				}

				/* Fade-in new block to avoid "clicks" in audio. We use linear
				   fading which gives a log-fading impression */
				for (i = 0; i < iResOutBlockSize; i++)
				{
					/* Linear attenuation with time */
					const _REAL rAtt = (_REAL) i / iResOutBlockSize;

					vecTempResBufOutCurLeft[i] *= rAtt;
					vecTempResBufOutCurRight[i] *= rAtt;

					if (bUseReverbEffect == TRUE)
					{
						/* Cross-fade reverberation effect */
						const _REAL rRevSam = (1.0 - rAtt) * AudioRev.
							ProcessSample(0, 0);

						/* Mono reverberation signal */
						vecTempResBufOutCurLeft[i] += rRevSam;
						vecTempResBufOutCurRight[i] += rRevSam;
					}
				}

				/* Reset flag */
				bAudioWasOK = TRUE;
			}
		}

		/* Conversion from _REAL to _SAMPLE with special function */
		for (i = 0; i < iResOutBlockSize; i++)
		{
			(*pvecOutputData)[iOutputBlockSize + i * 2] = 
				Real2Sample(vecTempResBufOutOldLeft[i]); /* Left channel */
			(*pvecOutputData)[iOutputBlockSize + i * 2 + 1] =
				Real2Sample(vecTempResBufOutOldRight[i]); /* Right channel */
		}

		/* Add new block to output block size ("* 2" for stereo output block) */
		iOutputBlockSize += iResOutBlockSize * 2;

		/* Store current audio block */
		for (i = 0; i < iResOutBlockSize; i++)
		{
			vecTempResBufOutOldLeft[i] = vecTempResBufOutCurLeft[i];
			vecTempResBufOutOldRight[i] = vecTempResBufOutCurRight[i];
		}
	}
#endif
}

void CAudioSourceDecoder::InitInternal(CParameter& ReceiverParam)
{
/*
	Since we use the exception mechanism in this init routine, the sequence of
	the individual initializations is very important!
	Requirement for text message is "stream is used" and "audio service".
	Requirement for AAC decoding are the requirements above plus "audio coding
	is AAC"
*/
	int iCurAudioStreamID;
	int iMaxLenResamplerOutput;
	int iCurSelServ;
	int iDRMchanMode;
	int iAudioSampleRate;
	int iAACSampleRate;
	int	iLenAudHigh;
	int	iNumHeaderBytes;

	//try
	{
		/* Init error flags and output block size parameter. The output block
		   size is set in the processing routine. We must set it here in case
		   of an error in the initialization, this part in the processing
		   routine is not being called */
		DoNotProcessAAC = FALSE;
		DoNotProcessData = FALSE;
		iOutputBlockSize = 0;

		/* Init counter for correctly decoded audio blocks */
		iNumCorDecAudio = 0;

		/* Get number of total input bits for this module */
		iInputBlockSize = ReceiverParam.iNumAudioDecoderBits;

		/* Get current selected audio service */
		iCurSelServ = ReceiverParam.GetCurSelAudioService();

		/* Current audio stream ID */
		iCurAudioStreamID =
			ReceiverParam.Service[iCurSelServ].AudioParam.iStreamID;

		/* The requirement for this module is that the stream is used and the
		   service is an audio service. Check it here */
		if ((ReceiverParam.Service[iCurSelServ].
			eAudDataFlag != CParameter::SF_AUDIO) ||
			(iCurAudioStreamID == STREAM_ID_NOT_USED))
		{
			//throw CInitErr(ET_ALL);
			printf("CInitErr(ET_ALL)\n");
			DoNotProcessData = TRUE;
			iOutputBlockSize = 0;
			return;
		}


		/* Init text message application ------------------------------------ */
		switch (ReceiverParam.Service[iCurSelServ].AudioParam.bTextflag)
		{
		case TRUE:
			bTextMessageUsed = TRUE;

			/* Get a pointer to the string */
			TextMessage.Init(&ReceiverParam.Service[iCurSelServ].AudioParam.
				strTextMessage);

			/* Total frame size is input block size minus the bytes for the text
			   message */
			iTotalFrameSize = iInputBlockSize -
				SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR;

			/* Init vector for text message bytes */
			vecbiTextMessBuf.Init(SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR);
			break;

		case FALSE:
			bTextMessageUsed = FALSE;

			/* All bytes are used for AAC data, no text message present */
			iTotalFrameSize = iInputBlockSize;
			break;
		}


#ifdef USE_FAAD2_LIBRARY
		/* Init for AAC decoding -------------------------------------------- */
		/* Check, if AAC is used */
		if (ReceiverParam.Service[iCurSelServ].AudioParam.
			eAudioCoding != CParameter::AC_AAC)
		{
			//throw CInitErr(ET_AAC);
			printf("CInitErr(ET_AAC)\n");
			DoNotProcessAAC = TRUE;
			iOutputBlockSize = 0;
			return;
		}

		/* Init "audio was ok" flag */
		bAudioWasOK = TRUE;

		/* Length of higher protected part of audio stream */
		iLenAudHigh = ReceiverParam.Stream[iCurAudioStreamID].iLenPartA;

		/* Set number of AAC frames in a AAC super-frame */
		switch (ReceiverParam.Service[iCurSelServ].AudioParam.eAudioSamplRate)
		{ /* only 12 kHz and 24 kHz is allowed */
		case CParameter::AS_12KHZ:
			iNumAACFrames = 5;
			iNumHeaderBytes = 6;
			iAACSampleRate = 12000;
			break;

		case CParameter::AS_24KHZ:
			iNumAACFrames = 10;
			iNumHeaderBytes = 14;
			iAACSampleRate = 24000;
			break;

		default:
			/* Some error occurred, throw error */
			//throw CInitErr(ET_AAC);
			printf("CInitErr(ET_AAC)\n");
			DoNotProcessAAC = TRUE;
			iOutputBlockSize = 0;
			return;
			break;
		}

		/* Number of borders */
		iNumBorders = iNumAACFrames - 1;

		/* Set number of AAC frames for log file */
		ReceiverParam.ReceptLog.SetNumAAC(iNumAACFrames);

		/* Number of channels for AAC: Mono, PStereo, Stereo */
		switch (ReceiverParam.Service[iCurSelServ].AudioParam.eAudioMode)
		{
		case CParameter::AM_MONO:
			if (ReceiverParam.Service[iCurSelServ].AudioParam.
				eSBRFlag == CParameter::SB_USED)
			{
				iDRMchanMode = DRMCH_SBR_MONO;
			}
			else
				iDRMchanMode = DRMCH_MONO;
			break;

		case CParameter::AM_P_STEREO:
			/* Low-complexity only defined in SBR mode */
			iDRMchanMode = DRMCH_SBR_PS_STEREO;
			break;

		case CParameter::AM_STEREO:
			if (ReceiverParam.Service[iCurSelServ].AudioParam.
				eSBRFlag == CParameter::SB_USED)
			{
				iDRMchanMode = DRMCH_SBR_STEREO;
			}
			else
			{
				iDRMchanMode = DRMCH_STEREO;
			}
			break;
		}

		/* In case of SBR, AAC sample rate is half the total sample rate. Length
		   of output is doubled if SBR is used */
		if (ReceiverParam.Service[iCurSelServ].AudioParam.
			eSBRFlag == CParameter::SB_USED)
		{
			iAudioSampleRate = iAACSampleRate * 2;
			iLenDecOutPerChan = AUD_DEC_TRANSFROM_LENGTH * 2;
		}
		else
		{
			iAudioSampleRate = iAACSampleRate;
			iLenDecOutPerChan = AUD_DEC_TRANSFROM_LENGTH;
		}

		/* The audio_payload_length is derived from the length of the audio
		   super frame (data_length_of_part_A + data_length_of_part_B)
		   subtracting the audio super frame overhead (bytes used for the audio
		   super frame header() and for the aac_crc_bits) (5.3.1.1, Table 5) */
		iAudioPayloadLen =
			iTotalFrameSize / SIZEOF__BYTE - iNumHeaderBytes - iNumAACFrames;

		/* Check iAudioPayloadLen value, only positive values make sense */
		if (iAudioPayloadLen < 0)
		{
			//throw CInitErr(ET_AAC);
			printf("CInitErr(ET_AAC)\n");
			DoNotProcessAAC = TRUE;
			iOutputBlockSize = 0;
			return;
		}

		/* Calculate number of bytes for higher protected blocks */
		iNumHigherProtectedBytes =
			(iLenAudHigh - iNumHeaderBytes - iNumAACFrames /* CRC bytes */) /
			iNumAACFrames;

		if (iNumHigherProtectedBytes < 0)
			iNumHigherProtectedBytes = 0;

		/* Since we do not correct for sample rate offsets here (yet), we do not
		   have to consider larger buffers. An audio frame always corresponds
		   to 400 ms */
		iMaxLenResamplerOutput = (int) ((_REAL) SOUNDCRD_SAMPLE_RATE *
			(_REAL) 0.4 /* 400ms */ * 2 /* for stereo */);

		iResOutBlockSize = (int) ((_REAL) iLenDecOutPerChan *
			SOUNDCRD_SAMPLE_RATE / iAudioSampleRate);

		/* Additional buffers needed for resampling since we need conversation
		   between _REAL and _SAMPLE. We have to init the buffers with
		   zeros since it can happen, that we have bad CRC right at the
		   start of audio blocks */
		vecTempResBufInLeft.Init(iLenDecOutPerChan);
		vecTempResBufInRight.Init(iLenDecOutPerChan);
		vecTempResBufOutCurLeft.Init(iResOutBlockSize, (_REAL) 0.0);
		vecTempResBufOutCurRight.Init(iResOutBlockSize, (_REAL) 0.0);
		vecTempResBufOutOldLeft.Init(iResOutBlockSize, (_REAL) 0.0);
		vecTempResBufOutOldRight.Init(iResOutBlockSize, (_REAL) 0.0);

		/* Init resample objects */
		ResampleObjL.Init(iLenDecOutPerChan,
			(_REAL) SOUNDCRD_SAMPLE_RATE / iAudioSampleRate);
		ResampleObjR.Init(iLenDecOutPerChan,
			(_REAL) SOUNDCRD_SAMPLE_RATE / iAudioSampleRate);

		/* Clear reverberation object */
		AudioRev.Clear();


		/* AAC decoder ------------------------------------------------------ */
		/* The maximum length for one audio frame is "iAudioPayloadLen". The
		   regular size will be much shorter since all audio frames share the
		   total size, but we do not know at this time how the data is 
		   split in the transmitter source coder */
		iMaxLenOneAudFrame = iAudioPayloadLen;
		audio_frame.Init(iNumAACFrames, iMaxLenOneAudFrame);

		/* Init vector which stores the data with the CRC at the beginning
		   ("+ 1" for CRC) */
		vecbyPrepAudioFrame.Init(iMaxLenOneAudFrame + 1);

		/* Init storage for CRCs and frame lengths */
		aac_crc_bits.Init(iNumAACFrames);
		veciFrameLength.Init(iNumAACFrames);

		/* Init AAC-decoder */
		NeAACDecInitDRM(&HandleAACDecoder, iAACSampleRate, iDRMchanMode);

		/* With this parameter we define the maximum lenght of the output
		   buffer. The cyclic buffer is only needed if we do a sample rate
		   correction due to a difference compared to the transmitter. But for
		   now we do not correct and we could stay with a single buffer
		   Maybe TODO: sample rate correction to avoid audio dropouts */
		iMaxOutputBlockSize = iMaxLenResamplerOutput;
#else
		/* No audio output if AAC library is not used */
		iOutputBlockSize = 0;
#endif
	}

	//catch (CInitErr CurErr)
	//{
		//switch (CurErr.eErrType)
		//{
		//case ET_ALL:
			///* An init error occurred, do not process data in this module */
			//DoNotProcessData = TRUE;
			//break;
//
		//case ET_AAC:
			///* AAC part should not be decdoded, set flag */
			//DoNotProcessAAC = TRUE;
			//break;
//
		//default:
			//DoNotProcessData = TRUE;
		//}
//
		///* In all cases set output size to zero */
		//iOutputBlockSize = 0;
	//}
}

int CAudioSourceDecoder::GetNumCorDecAudio()
{
	/* Return number of correctly decoded audio blocks. Reset counter
	   afterwards */
	const int iRet = iNumCorDecAudio;

	iNumCorDecAudio = 0;

	return iRet;
}

CAudioSourceDecoder::CAudioSourceDecoder()
#ifdef USE_FAAD2_LIBRARY
	: AudioRev((CReal) 1.0 /* seconds delay */), bUseReverbEffect(TRUE)
#endif
{
#ifdef USE_FAAD2_LIBRARY
	/* Open AACEncoder instance */
	HandleAACDecoder = NeAACDecOpen();

	/* Decoder MUST be initialized at least once, therefore do it here in the
	   constructor with arbitrary values to be sure that this is satisfied */
	NeAACDecInitDRM(&HandleAACDecoder, 24000, DRMCH_MONO);
#endif
}

CAudioSourceDecoder::~CAudioSourceDecoder()
{
#ifdef USE_FAAD2_LIBRARY
	/* Close decoder handle */
	NeAACDecClose(HandleAACDecoder);
#endif
}
