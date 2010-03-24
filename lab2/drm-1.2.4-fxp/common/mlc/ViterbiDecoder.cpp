/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer, Alexander Kurpiers
 *
 * Description:
 *
 * 02/04/08
 * Modified by John Jones
 * Stripped to stub by Lev Shuhatovich
 * Converted all floating point operations to integer.	
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

#include "ViterbiDecoder.h"

#include "stdio.h"
#include <iostream.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define VTB_CMD_STOP   0
#define VTB_CMD_INIT   1
#define VTB_CMD_DECODE 2
/* Implementation *************************************************************/

int CViterbiDecoder::mTotalInit = 0;

FXP CViterbiDecoder::Decode(CVector<CFDistance>& vecNewDistance,
							  CVector<_DECISION>& vecOutputBits)
{
	/*
`	CFDistance dist_array[10000];
	for(int i = 0; i<vecNewDistance.Size();i++){
		array[i] = vecNewDistance[i];
	}*/
	volatile unsigned int *base, *address;
	unsigned long offset, value;
	int val1, val2, result;
	//Predefined addresses.
 
	unsigned long addrCmd = strtoul("0xd3000040", 0, 0);
	unsigned long addrStat= strtoul("0xd3000004", 0, 0);
	unsigned long addrWNum= strtoul("0xd3000008", 0, 0);
	unsigned long addrRNum= strtoul("0xd300000c", 0, 0);

	unsigned long addrIni0= strtoul("0xd3000010", 0, 0);
	unsigned long addrIni1= strtoul("0xd3000014", 0, 0);
	unsigned long addrIni2= strtoul("0xd3000018", 0, 0);
	unsigned long addrIni3= strtoul("0xd300001c", 0, 0);
	unsigned long addrIni4= strtoul("0xd3000020", 0, 0);
	unsigned long addrIni5= strtoul("0xd3000024", 0, 0);
	unsigned long addrIni6= strtoul("0xd3000028", 0, 0);
	unsigned long addrIni7= strtoul("0xd300002c", 0, 0);
	unsigned long addrIni8= strtoul("0xd3000030", 0, 0);
	unsigned long addrRetV= strtoul("0xd3000034", 0, 0);
  
	unsigned long addrWDat= strtoul("0xd3000200", 0, 0);
	unsigned long addrRDat= strtoul("0xd3000300", 0, 0);

	unsigned long addr0 = strtoul("0xd3000000", 0, 0);;
	
	addr0 += mInitSerial * MAP_SIZE;
	//Open memory as a file
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(!fd)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		//return -1;
	}	

	//Map the base address to base
	base = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, 
		MAP_SHARED, fd, addr0 & ~MAP_MASK);	
	if((base == MAP_FAILED))
	{
		printf("mapping failed\n");
		fflush(stdout);
	}
	printf("Debug>> Decode, mInitSerial = %d, addr0 = 0x%x, iNumOutBits = %d\n", this->mInitSerial, addr0, iNumOutBits);
	//printf("Decoding 1\n");
	//Write data out to FPGA
	address = base + ((addrWNum & MAP_MASK)>>2);
	*address = vecNewDistance.Size()<<3;
	//printf("Wnum written\n");
	unsigned long i = 0, eIndex = 0;
	CFDistance inputArray[32];
	unsigned long bytesRemaining = vecNewDistance.Size()*8;
	unsigned long bytesToWrite, elements;
	
	printf("DumpIn number = %d\n", vecNewDistance.Size());
	for(int i = 0; i < vecNewDistance.Size(); i++)
	{
		if(i%2 == 0)
			printf("\n DumpIn %04d:", i);
		printf("%f %f ", (double)vecNewDistance[i].rTow0, 
			(double)vecNewDistance[i].rTow1);
	}
	printf("\n");
	
	eIndex = 0;
	while(bytesRemaining > 0)
	{
		bytesToWrite = (bytesRemaining > 0x100) ? 0x100 : bytesRemaining;
		elements = bytesToWrite>>3;
		for(i = 0; i < elements; i++, eIndex++)
			inputArray[i] = vecNewDistance[eIndex];
		address = base + ((addrWDat & MAP_MASK)>>2);
		memcpy(address, (void*)inputArray, bytesToWrite);
		bytesRemaining -= bytesToWrite;
	}
#if 0
	i = 0;
	do{
		/*
		address = base + ((addrWDat & MAP_MASK)>>2) + (dwIndex % (0x100>>2));
		*address = vecNewDistance[i].rTow0;
		dwIndex++;
		address = base + ((addrWDat & MAP_MASK)>>2) + (dwIndex % (0x100>>2));
		*address = vecNewDistance[i].rTow1;
		dwIndex++; */
		printf("%d) %f %f 0x%x 0x%x\n", i, (double)vecNewDistance[i].rTow0, 
			(double)vecNewDistance[i].rTow1, 
			(int)vecNewDistance[i].rTow0,
			(int)vecNewDistance[i].rTow1);
	}while(++i < vecNewDistance.Size());
#endif	
	//printf("Input Block written\n");
	address = base + ((addrCmd & MAP_MASK)>>2);
	*address = VTB_CMD_DECODE;
  
	printf("Command sent \n");
	//Poll hardware until it is ready and
	//read from addr0. The result should be the sum of val1 and val2
	do {
		address = base + ((addrStat & MAP_MASK)>>2);
		result = *address;
		printf("status = %d\n", result);
	} while (result < 0);
  
	printf("Status read\n");
	address = base + ((addrRNum & MAP_MASK)>>2);
	unsigned long bytesToRead = *address;
	printf("RNum read: %d bytes to read\n", bytesToRead);  
	address = base + ((addrRetV & MAP_MASK)>>2);
	FXP rval = *address;
	printf("RetVal read\n");

	_DECISION decodeOutput[0x100];
	unsigned long bytes, reqBytes;
	vecOutputBits.clear();
	do { 
		bytes = (bytesToRead > 0x100) ? 0x100 : bytesToRead;
		address = base + ((addrRDat & MAP_MASK)>>2);
		reqBytes = ((bytes+3) >> 2) << 2;
		printf("bytes = %d, reqBytes = %d\n", bytes, reqBytes);
		memcpy(decodeOutput, address, reqBytes);
		bytesToRead -= bytes;
		for(i = 0; i < bytes; i++) {
			//if(i%8 == 0)
			//	printf("\n %d: ", i);
			vecOutputBits.push_back(decodeOutput[i]);
			//printf("0x%x ", decodeOutput[i]);
		}
		//if(i == 210)
		//	vecOutputBits[209] = 1;
		//printf("\n");
		//dataRead = *address;
	} while(bytesToRead > 0); 
	printf("Debug Decode, input# = %d(%x), output#=%d(%x)\n", 
		vecNewDistance.Size(), vecNewDistance.Size(),
		vecOutputBits.Size(), vecOutputBits.Size());
	//In the end, unmap the base address and close the memory file
	printf("DumpOut: number = %d\n", vecOutputBits.Size());
    for(int i = 0; i < vecOutputBits.Size(); i++)
	{
		if(i%8 == 0)
			printf("\n DumpOut %04d:", i);
		printf("%x ", vecOutputBits[i]);
	}
	printf("\n");
	printf("DumpOut: ret = %f\n", (double)rval);
	munmap((void*)base, MAP_SIZE);
	close(fd);
	return rval;
}


void CViterbiDecoder::Init(CParameter::ECodScheme eNewCodingScheme,
						   CParameter::EChanType eNewChannelType, int iN1,
						   int iN2, int iNewNumOutBitsPartA,
						   int iNewNumOutBitsPartB, int iPunctPatPartA,
						   int iPunctPatPartB, int iLevel)
{
	volatile unsigned int *base, *address;
	unsigned long offset, value;
	int val1, val2, result;
	
	unsigned long addrCmd = strtoul("0xd3000040", 0, 0);
	unsigned long addrIni0= strtoul("0xd3000010", 0, 0);
	unsigned long addrIni1= strtoul("0xd3000014", 0, 0);
	unsigned long addrIni2= strtoul("0xd3000018", 0, 0);
	unsigned long addrIni3= strtoul("0xd300001c", 0, 0);
	unsigned long addrIni4= strtoul("0xd3000020", 0, 0);
	unsigned long addrIni5= strtoul("0xd3000024", 0, 0);
	unsigned long addrIni6= strtoul("0xd3000028", 0, 0);
	unsigned long addrIni7= strtoul("0xd300002c", 0, 0);
	unsigned long addrIni8= strtoul("0xd3000030", 0, 0);
	unsigned long addrRetV= strtoul("0xd3000034", 0, 0);
	unsigned long addr0 = strtoul("0xd3000000", 0, 0);;

	if(mInitSerial == -1)
	{
		this->mInitSerial = CViterbiDecoder::mTotalInit;
		CViterbiDecoder::mTotalInit += 1;
	}
	mLevel = iLevel;
	addr0 += this->mInitSerial*MAP_SIZE;
	printf("Debug Init mInitSerial = %d, iLevel = %d, addr0 = 0x%x\n", this->mInitSerial, iLevel, addr0);
	//Open memory as a file
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if(!fd)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		//return -1;
	}	

	//Map the base address to base
	base = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, 
		MAP_SHARED, fd, addr0 & ~MAP_MASK);	
	if((base == MAP_FAILED))
	{
		printf("mapping failed\n");
		fflush(stdout);
	}
	printf("CViterbiDecoder::Init: \n");
	printf("0x%x 0x%x 0x%x 0x%x 0x%x\n", eNewCodingScheme, eNewChannelType,
		iN1, iN2, iNewNumOutBitsPartA);
	printf("0x%x 0x%x 0x%x 0x%x \n", iNewNumOutBitsPartB, iPunctPatPartA,
		iPunctPatPartB, iLevel);
	address = base + ((addrIni0 & MAP_MASK)>>2);
	*address = eNewCodingScheme;
	address = base + ((addrIni1 & MAP_MASK)>>2);
	*address = eNewChannelType; 

	address = base + ((addrIni2 & MAP_MASK)>>2);
	*address = iN1;

	address = base + ((addrIni3 & MAP_MASK)>>2);
	*address = iN2;

	address = base + ((addrIni4 & MAP_MASK)>>2);
	*address = iNewNumOutBitsPartA;

	address = base + ((addrIni5 & MAP_MASK)>>2);
	*address = iNewNumOutBitsPartB;

	address = base + ((addrIni6 & MAP_MASK)>>2);
	*address = iPunctPatPartA;

	address = base + ((addrIni7 & MAP_MASK)>>2);
	*address = iPunctPatPartB;

	address = base + ((addrIni8 & MAP_MASK)>>2);
	*address = iLevel;

	address = base + ((addrCmd & MAP_MASK)>>2);
	*address = VTB_CMD_INIT;

	munmap((void*)base, MAP_SIZE);
	close(fd);
#if 0
	/* Number of bits out is the sum of all protection levels */
	iNumOutBits = iNewNumOutBitsPartA + iNewNumOutBitsPartB;

	/* Number of out bits including the state memory */
	iNumOutBitsWithMemory = iNumOutBits + MC_CONSTRAINT_LENGTH - 1;

	/* Init vector, storing table for puncturing pattern and generate pattern */
	veciTablePuncPat.Init(iNumOutBitsWithMemory);

	veciTablePuncPat = GenPuncPatTable(eNewCodingScheme, eNewChannelType, iN1,
		iN2, iNewNumOutBitsPartA, iNewNumOutBitsPartB, iPunctPatPartA,
		iPunctPatPartB, iLevel);

	/* Init vector for storing the decided bits */
	matdecDecisions.Init(iNumOutBitsWithMemory, MC_NUM_STATES);
#endif 
}

CViterbiDecoder::CViterbiDecoder() 
{
	mInitSerial = -1;
	/*mSerial = CViterbiDecoder::mTotalNum;
	CViterbiDecoder::mTotalNum += 1;
	printf("Constructs Viterbi #%d, this=0x%0x8\n", 
		CViterbiDecoder::mTotalNum, (unsigned int) this);*/
#if 0
	/* Create trellis *********************************************************/
	/* Activate this code to generate the table needed for the butterfly calls
	   in the processing routine */

	/* We need to analyze 2^(MC_CONSTRAINT_LENGTH - 1) states in the trellis */
	int	i;
	int	iPrev0IndexForw[MC_NUM_STATES];
	int	iPrev1IndexForw[MC_NUM_STATES];
	int	iMetricPrev0Forw[MC_NUM_STATES];
	int	iMetricPrev1Forw[MC_NUM_STATES];
	int	iPrev0IndexBackw[MC_NUM_STATES];
	int	iPrev1IndexBackw[MC_NUM_STATES];
	int	iMetricPrev0Backw[MC_NUM_STATES];
	int	iMetricPrev1Backw[MC_NUM_STATES];

	for (i = 0; i < MC_NUM_STATES; i++)
	{
		/* Define previous states ------------------------------------------- */
		/* We define in this trellis that we shift the bits from right to
		   the left. We use the transition-bits which "fall" out of the
		   shift-register (forward case) */
		iPrev0IndexForw[i] = (i >> 1);			/* Old state, Leading "0"
												   (automatically inserted by
												   shifting */
		iPrev1IndexForw[i] = (i >> 1)			 /* Old state */
			| (1 << (MC_CONSTRAINT_LENGTH - 2)); /* New "1", must be on position
													MC_CONSTRAINT_LENGTH - 1 */

		/* We define in this trellis that we shift the bits from left to
		   the right. We use the transition-bits which get in the
		   shift-register (backward case) */
		/* Mask operator for length of state register of this code
		   [... 0, 0, 1, 1, 1, 1, 1, 1] */
		const int iStRegMask = /* "MC_CONSTRAINT_LENGTH - 1" number of ones */
			1 | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);

		iPrev0IndexBackw[i] = (i << 1) & iStRegMask; /* Old state, Beginning "0"
												        (automatically inserted
														by shifting. Mask result
														to have length of state
														register */
		iPrev1IndexBackw[i] = ((i << 1) & iStRegMask) | 1;	/* New "1", must on
															   first position */


		/* Assign metrics to the transitions from both paths ---------------- */
		/* We define with the metrics the order: [b_3, b_2, b_1, b_0] */
		iMetricPrev0Forw[i] = 0;
		iMetricPrev1Forw[i] = 0;
		iMetricPrev0Backw[i] = 0;
		iMetricPrev1Backw[i] = 0;
		for (int j = 0; j < MC_NUM_OUTPUT_BITS_PER_STEP; j++)
		{
			/* Calculate respective metrics from convolution of state and
			   transition bit */
			/* forwards */
			/* "0" */
			iMetricPrev0Forw[i] |= Convolution(
				/* Set all states in the shift-register for encoder. Use
				   current state with a leading "0" (which is automatically
				   there) */
				i
				/* Use generator-polynomial j */
				, j) 
				/* Shift generated bit to the correct position */
				<< j;

			/* "1" */
			iMetricPrev1Forw[i] |= Convolution(
				/* Set all states in the shift-register for encoder. Use
				   current state with a leading "1". The position of this
				   bit is "MC_CONSTRAINT_LENGTH" (shifting from position 1:
				   "MC_CONSTRAINT_LENGTH - 1") */
				i | (1 << (MC_CONSTRAINT_LENGTH - 1))
				/* Use generator-polynomial j */
				, j)
				/* Shift generated bit to the correct position */
				<< j;

			/* backwards */
			/* "0" */
			iMetricPrev0Backw[i] |= Convolution(
				/* Set all states in the shift-register for encoder. Use
				   current state with a "0" at beginning (which is automatically
				   there) */
				(i << 1),
				/* Use generator-polynomial j */
				j) 
				/* Shift generated bit to the correct position */
				<< j;

			/* "1" */
			iMetricPrev1Backw[i] |= Convolution(
				/* Set all states in the shift-register for encoder. Use
				   current state with a "1" at beginning */
				(i << 1) | 1,
				/* Use generator-polynomial j */
				j)
				/* Shift generated bit to the correct position */
				<< j;
		}
	}

	/* Save trellis in file (for substituting in the code) */
	static FILE* pFile = fopen("test/trellis.dat", "w");

	/* forwards */
	fprintf(pFile, "/* forwards */\n");
	for (i = 0; i < MC_NUM_STATES; i += 2)
		fprintf(pFile, "BUTTERFLY(%2d, %2d, %2d, %2d, %2d, %2d)\n", i, i + 1,
			iPrev0IndexForw[i], iPrev1IndexForw[i],
			iMetricPrev0Forw[i], iMetricPrev1Forw[i]);

	/* backwards */
	fprintf(pFile, "\n\n\n/* backwards */\n");
	for (i = 0; i < MC_NUM_STATES; i++)
		fprintf(pFile, "BUTTERFLY(%2d, %2d, %2d, %2d, %2d)\n", i,
			iPrev0IndexBackw[i], iPrev1IndexBackw[i],
			iMetricPrev0Backw[i], iMetricPrev1Backw[i]);

	fprintf(pFile, "\n");
	fflush(pFile);
	exit(1);
#endif
}
