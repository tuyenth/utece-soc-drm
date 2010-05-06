/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	MSC audio/data demultiplexer
 *
 *
 * - (6.2.3.1) Multiplex frames (DRM standard):
 * The multiplex frames are built by placing the logical frames from each
 * non-hierarchical stream together. The logical frames consist, in general, of
 * two parts each with a separate protection level. The multiplex frame is
 * constructed by taking the data from the higher protected part of the logical
 * frame from the lowest numbered stream (stream 0 when hierarchical modulation
 * is not used, or stream 1 when hierarchical modulation is used) and placing
 * it at the start of the multiplex frame. Next the data from the higher
 * protected part of the logical frame from the next lowest numbered stream is
 * appended and so on until all streams have been transferred. The data from
 * the lower protected part of the logical frame from the lowest numbered
 * stream (stream 0 when hierarchical modulation is not used, or stream 1 when
 * hierarchical modulation is used) is then appended, followed by the data from
 * the lower protected part of the logical frame from the next lowest numbered
 * stream, and so on until all streams have been transferred. The higher
 * protected part is designated part A and the lower protected part is
 * designated part B in the multiplex description. The multiplex frame will be
 * larger than or equal to the sum of the logical frames from which it is
 * formed. The remainder, if any, of the multiplex frame shall be filled with
 * 0s. These bits shall be ignored by the receiver.
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

#include "MSCMultiplexer.h"


/* Implementation *************************************************************/
void CMSCDemultiplexer::ProcessDataInternal(CParameter& ReceiverParam)
{
	/* Audio ---------------------------------------------------------------- */
	/* Extract audio data from input-stream */
	ExtractData(*pvecInputData, *pvecOutputData, AudStreamPos);


	/* Data ----------------------------------------------------------------- */
	/* Extract data from input-stream */
	ExtractData(*pvecInputData, *pvecOutputData2, DataStreamPos);


#ifdef USE_QT_GUI
	/* MDI ------------------------------------------------------------------ */
	/* MDI (check that the pointer to the MDI object is not NULL. It can be NULL
	   in case of simulation because in this case there is no MDI) */
	if (pMDI != NULL)
	{
		/* Only put data in MDI object if MDI is enabled */
		if (pMDI->GetMDIOutEnabled() == TRUE)
		{
			/* Put all streams to MDI object */
			for (int j = 0; j < veciMDIActStre.Size(); j++)
			{
				/* Prepare temporary vector */
				CVectorEx<_BINARY> vecbiStrData;
				vecbiStrData.Init(vecMDIStrPos[j].iLenLow +
					vecMDIStrPos[j].iLenHigh);

				/* Extract data */
				ExtractData(*pvecInputData, vecbiStrData, vecMDIStrPos[j]);

				/* Now put the data to the MDI object */
				pMDI->SetStreamData(veciMDIActStre[j], vecbiStrData);
			}
		}

		if (pMDI->GetMDIInEnabled() == TRUE)
		{
			/* Get stream data from received MDI packets */
			/* OutputData1 is audio */
			pMDI->GetStreamData(*pvecOutputData, iOutputBlockSize,
				iAudioStreamID);

			/* OutputData2 is data */
			pMDI->GetStreamData(*pvecOutputData2, iOutputBlockSize2,
				iDataStreamID);
		}
	}
#endif
}

void CMSCDemultiplexer::InitInternal(CParameter& ReceiverParam)
{
	/* Audio ---------------------------------------------------------------- */
	/* Get audio stream ID of current selected audio service (might be an
	   invalid stream) */
	iAudioStreamID = ReceiverParam.
		Service[ReceiverParam.GetCurSelAudioService()].AudioParam.iStreamID;

	/* Check if current selected service is an audio service and get stream
	   position */
	if (ReceiverParam.Service[ReceiverParam.GetCurSelAudioService()].
		eAudDataFlag == CParameter::SF_AUDIO)
	{
		AudStreamPos = GetStreamPos(ReceiverParam, iAudioStreamID);
	}
	else
	{
		/* This is not an audio stream, zero the lengths */
		AudStreamPos.iLenHigh = 0;
		AudStreamPos.iLenLow = 0;
	}

	/* Set audio output block size */
	iOutputBlockSize = AudStreamPos.iLenHigh + AudStreamPos.iLenLow;

	/* Set number of output bits for audio decoder in global struct */
	ReceiverParam.SetNumAudioDecoderBits(iOutputBlockSize);


	/* Data ----------------------------------------------------------------- */
	/* If multimedia is not used, set stream ID to "not used" which leads to
	   an output size of "0" -> no output data generated */
	if (ReceiverParam.bUsingMultimedia)
	{
		iDataStreamID = ReceiverParam.
			Service[ReceiverParam.GetCurSelDataService()].DataParam.iStreamID;
	}
	else
		iDataStreamID = STREAM_ID_NOT_USED;

	/* Get stream position of current selected data service */
	DataStreamPos = GetStreamPos(ReceiverParam, iDataStreamID);

	/* Set data output block size */
	iOutputBlockSize2 = DataStreamPos.iLenHigh + DataStreamPos.iLenLow;

	/* Set number of output bits for data decoder in global struct */
	ReceiverParam.SetNumDataDecoderBits(iOutputBlockSize2);


#ifdef USE_QT_GUI
	/* MDI ------------------------------------------------------------------ */
	/* Get all active streams and stream positions */
	ReceiverParam.GetActiveStreams(veciMDIActStre);

	for (int i = 0; i < veciMDIActStre.Size(); i++)
		vecMDIStrPos[i] = GetStreamPos(ReceiverParam, veciMDIActStre[i]);
#endif


	/* Set input block size */
	iInputBlockSize = ReceiverParam.iNumDecodedBitsMSC;
}

void CMSCDemultiplexer::ExtractData(CVectorEx<_BINARY>& vecIn,
									CVectorEx<_BINARY>& vecOut,
									SStreamPos& StrPos)
{
	int i;

	/* Higher protected part */
	for (i = 0; i < StrPos.iLenHigh; i++)
		vecOut[i] = vecIn[i + StrPos.iOffsetHigh];

	/* Lower protected part */
	for (i = 0; i < StrPos.iLenLow; i++)
		vecOut[i + StrPos.iLenHigh] = vecIn[i + StrPos.iOffsetLow];
}

CMSCDemultiplexer::SStreamPos CMSCDemultiplexer::GetStreamPos(CParameter& Param,
															  const int iStreamID)
{
	int								i;
	CVector<int>					veciActStreams;
	CMSCDemultiplexer::SStreamPos	StPos;

	/* Init positions with zeros (needed if an error occurs) */
	StPos.iOffsetLow = 0;
	StPos.iOffsetHigh = 0;
	StPos.iLenLow = 0;
	StPos.iLenHigh = 0;

	if (iStreamID != STREAM_ID_NOT_USED)
	{
		/* Length of higher and lower protected part of audio stream (number
		   of bits) */
		StPos.iLenHigh = Param.Stream[iStreamID].iLenPartA * SIZEOF__BYTE;
		StPos.iLenLow = Param.Stream[iStreamID].iLenPartB *	SIZEOF__BYTE;


		/* Byte-offset of higher and lower protected part of audio stream --- */
		/* Get active streams */
		Param.GetActiveStreams(veciActStreams);

		/* Get start offset for lower protected parts in stream. Since lower
		   protected part comes after the higher protected part, the offset
		   must be shifted initially by all higher protected part lengths
		   (iLenPartA of all streams are added) 6.2.3.1 */
		StPos.iOffsetLow = 0;
		for (i = 0; i < veciActStreams.Size(); i++)
		{
			StPos.iOffsetLow +=
				Param.Stream[veciActStreams[i]].iLenPartA * SIZEOF__BYTE;
		}

		/* Real start position of the streams */
		StPos.iOffsetHigh = 0;
		for (i = 0; i < veciActStreams.Size(); i++)
		{
			if (veciActStreams[i] < iStreamID)
			{
				StPos.iOffsetHigh += Param.Stream[i].iLenPartA * SIZEOF__BYTE;
				StPos.iOffsetLow += Param.Stream[i].iLenPartB * SIZEOF__BYTE;
			}
		}

		/* Special case if hierarchical modulation is used */
		if (((Param.eMSCCodingScheme == CParameter::CS_3_HMSYM) ||
			(Param.eMSCCodingScheme == CParameter::CS_3_HMMIX)))
		{
			if (iStreamID == 0)
			{
				/* Hierarchical channel is selected. Data is at the beginning
				   of incoming data block */
				StPos.iOffsetLow = 0;
			}
			else
			{
				/* Shift all offsets by the length of the hierarchical frame. We
				   cannot use the information about the length in
				   "Stream[0].iLenPartB", because the real length of the frame
				   is longer or equal to the length in "Stream[0].iLenPartB" */
				StPos.iOffsetHigh += Param.iNumBitsHierarchFrameTotal;
				StPos.iOffsetLow += Param.iNumBitsHierarchFrameTotal -
					/* We have to subtract this because we added it in the
					   for loop above which we do not need here */
					Param.Stream[0].iLenPartB * SIZEOF__BYTE;
			}
		}


		/* Possibility check ------------------------------------------------ */
		/* Test, if parameters have possible values */
		if ((StPos.iOffsetHigh + StPos.iLenHigh > Param.iNumDecodedBitsMSC) ||
			(StPos.iOffsetLow + StPos.iLenLow > Param.iNumDecodedBitsMSC))
		{
			/* Something is wrong, set everything to zero */
			StPos.iOffsetLow = 0;
			StPos.iOffsetHigh = 0;
			StPos.iLenLow = 0;
			StPos.iLenHigh = 0;
		}
	}

	return StPos;
}
