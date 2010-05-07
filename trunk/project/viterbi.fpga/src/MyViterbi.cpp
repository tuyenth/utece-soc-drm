#include "MyViterbi.h"
//#include "arm.h"

//the number of bytes copied must be a multiple of 4
#if 0
_FREAL* MyMemcpy(_FREAL* dest, _FREAL* src, int n)
{
	int iterations = n >> 2;
	int i = 0;
	_FREAL* dptr;
	_FREAL* sptr;

	dptr = dest;
	sptr = src;
	//MEMCPY:
	for(i = 0; i < iterations; i++)
	{
		*dptr = *sptr;
		dptr += 1;
		sptr += 1;
	}
	return dest;
}
#endif

//len: number of bytes
#pragma hls_design top
//Use ac_int<8, false> for vecNewDistance
//Use ac_int<1, false> for vecOutputBits, concatenate single bits into bytes
//Use ac_int<1, false> for matdecDecisions
//Use ac_int<4, false> for veciTablePuncPat(or ac_int<3, 0> if block RAM is used
short PureDecode(short iNumOutBits, short	iNumOutBitsWithMemory, 
		PUNCTYPE veciTablePuncPat[INUMOUTBITSWITHMEMORY],
		short nDeInput, DISTTYPE vecNewDistance[INPUTSIZE*2],
		short* nDeOutput, _DECISION vecOutputBits[INUMOUTBITS])
{
	//static _DECISIONTYPE  matdecDecisions[INUMOUTBITSWITHMEMORY][MC_NUM_STATES];
	static MATTYPE  matdecDecisions[INUMOUTBITSWITHMEMORY][MC_NUM_STATES];
	static int			vecTrelMetric1[MC_NUM_STATES];
	static int			vecTrelMetric2[MC_NUM_STATES];
	static int			vecrMetricSet[MC_NUM_OUTPUT_COMBINATIONS];

	static int		i;
	static int		iDistCnt;
	static int		iCurDecState;
	static int*		pCurTrelMetric;	// Used to be float*
	static int*		pOldTrelMetric; // Used to be float*


	/* Init pointers for old and new trellis state */
	pCurTrelMetric = vecTrelMetric1;
	pOldTrelMetric = vecTrelMetric2;

	/* Reset all metrics in the trellis. We initialize all states exept of
	   the zero state with a high metric, because we KNOW that the state "0"
	   is the transmitted state */
	pOldTrelMetric[0] = (_VITMETRTYPE) 0;
	//RESET_TRELLIS:
	for (i = 1; i < MC_NUM_STATES; i++)	// MC_NUM_STATES = 64
		pOldTrelMetric[i] = MC_METRIC_INIT_VALUE;

	/* Reset counter for puncturing and distance (from metric) */
	iDistCnt = 0;

	/* Main loop over all bits ---------------------------------------------- */
	//MAIN_DEC_LOOP:
	for (i = 0; i < iNumOutBitsWithMemory; i++)
	{
		/* Calculate all possible metrics ----------------------------------- */
		/* There are only a small set of possible puncturing patterns used for
		   DRM: 0001, 0101, 0011, 0111, 1111. These need different numbers of
		   input bits (increment of "iDistCnt" is dependent on pattern!). To
		   optimize the calculation of the metrics, a "subset" of bits are first
		   calculated which are used to get the final result. In this case,
		   redundancy can be avoided.
		   Note, that not all possible bit-combinations are used in the coder,
		   only a subset of numbers: 0, 2, 4, 6, 9, 11, 13, 15 (compare numbers
		   in the BUTTERFLY() calls) */

		/* Get first position in input vector (is needed for all cases) */
		const int iPos0 = iDistCnt;
		iDistCnt++;

		//if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0001)
		if (veciTablePuncPat[i] == PP_TYPE_0001)
		{
			/* Pattern 0001 */
			vecrMetricSet[ 0] = vecNewDistance[2*iPos0];
			vecrMetricSet[ 2] = vecNewDistance[2*iPos0];
			vecrMetricSet[ 4] = vecNewDistance[2*iPos0];
			vecrMetricSet[ 6] = vecNewDistance[2*iPos0];
			vecrMetricSet[ 9] = vecNewDistance[2*iPos0+1];
			vecrMetricSet[11] = vecNewDistance[2*iPos0+1];
			vecrMetricSet[13] = vecNewDistance[2*iPos0+1];
			vecrMetricSet[15] = vecNewDistance[2*iPos0+1];
		}
		else
		{
			/* The following patterns need one more bit */
			const int iPos1 = iDistCnt;
			iDistCnt++;

			/* Calculate "subsets" of bit-combinations. "rIRxx00" means that
			   the fist two bits are used, others are x-ed. "IR" stands for
			   "intermediate result" */
			const int rIRxx00 =
				vecNewDistance[2*iPos1] + vecNewDistance[2*iPos0];
			const int rIRxx10 =
				vecNewDistance[2*iPos1+1] + vecNewDistance[2*iPos0];
			const int rIRxx01 =
				vecNewDistance[2*iPos1] + vecNewDistance[2*iPos0+1];
			const int rIRxx11 =
				vecNewDistance[2*iPos1+1] + vecNewDistance[2*iPos0+1];

			//if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0101)
			if (veciTablePuncPat[i] == PP_TYPE_0101)
			{
				/* Pattern 0101 */
				vecrMetricSet[ 0] = rIRxx00;
				vecrMetricSet[ 2] = rIRxx00;
				vecrMetricSet[ 4] = rIRxx10;
				vecrMetricSet[ 6] = rIRxx10;
				vecrMetricSet[ 9] = rIRxx01;
				vecrMetricSet[11] = rIRxx01;
				vecrMetricSet[13] = rIRxx11;
				vecrMetricSet[15] = rIRxx11;
			}
			//else if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0011)
			else if (veciTablePuncPat[i] == PP_TYPE_0011)
			{
				/* Pattern 0011 */
				vecrMetricSet[ 0] = rIRxx00;
				vecrMetricSet[ 2] = rIRxx10;
				vecrMetricSet[ 4] = rIRxx00;
				vecrMetricSet[ 6] = rIRxx10;
				vecrMetricSet[ 9] = rIRxx01;
				vecrMetricSet[11] = rIRxx11;
				vecrMetricSet[13] = rIRxx01;
				vecrMetricSet[15] = rIRxx11;
			}
			else
			{
				/* The following patterns need one more bit */
				const int iPos2 = iDistCnt;
				iDistCnt++;

				//if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0111)
				if (veciTablePuncPat[i] == PP_TYPE_0111)
				{
					/* Pattern 0111 */
					vecrMetricSet[ 0] = vecNewDistance[2*iPos2] + rIRxx00;
					vecrMetricSet[ 2] = vecNewDistance[2*iPos2] + rIRxx10;
					vecrMetricSet[ 4] = vecNewDistance[2*iPos2+1] + rIRxx00;
					vecrMetricSet[ 6] = vecNewDistance[2*iPos2+1] + rIRxx10;
					vecrMetricSet[ 9] = vecNewDistance[2*iPos2] + rIRxx01;
					vecrMetricSet[11] = vecNewDistance[2*iPos2] + rIRxx11;
					vecrMetricSet[13] = vecNewDistance[2*iPos2+1] + rIRxx01;
					vecrMetricSet[15] = vecNewDistance[2*iPos2+1] + rIRxx11;
				}
				else
				{
					/* Pattern 1111 */
					/* This pattern needs all four bits */
					const int iPos3 = iDistCnt;
					iDistCnt++;

					/* Calculate "subsets" of bit-combinations. "rIRxx00" means
					   that the last two bits are used, others are x-ed.
					   "IR" stands for "intermediate result" */
					const int rIR00xx = vecNewDistance[2*iPos3] +
						vecNewDistance[2*iPos2];
					const int rIR10xx = vecNewDistance[2*iPos3+1] +
						vecNewDistance[2*iPos2];
					const int rIR01xx = vecNewDistance[2*iPos3] +
						vecNewDistance[2*iPos2+1];
					const int rIR11xx = vecNewDistance[2*iPos3+1] +
						vecNewDistance[2*iPos2+1];

					vecrMetricSet[ 0] = rIR00xx + rIRxx00; /* 0 */
					vecrMetricSet[ 2] = rIR00xx + rIRxx10; /* 2 */
					vecrMetricSet[ 4] = rIR01xx + rIRxx00; /* 4 */
					vecrMetricSet[ 6] = rIR01xx + rIRxx10; /* 6 */
					vecrMetricSet[ 9] = rIR10xx + rIRxx01; /* 9 */
					vecrMetricSet[11] = rIR10xx + rIRxx11; /* 11 */
					vecrMetricSet[13] = rIR11xx + rIRxx01; /* 13 */
					vecrMetricSet[15] = rIR11xx + rIRxx11; /* 15 */
				}
			}
		}


		/* Update trellis --------------------------------------------------- */
#define BUTTERFLY(cur, next, prev0, prev1, met0, met1) \
		{ \
			/* First state in this set ------------------------------------ */ \
			/* Calculate metrics from the two previous states, use the old
			   metric from the previous states plus the "transition-metric" */ \
			const _VITMETRTYPE rFiStAccMetricPrev0 = \
				pOldTrelMetric[prev0] + vecrMetricSet[met0]; \
			const _VITMETRTYPE  rFiStAccMetricPrev1 = \
				pOldTrelMetric[prev1] + vecrMetricSet[met1]; \
			\
			/* Take path with smallest metric */ \
			if (rFiStAccMetricPrev0 < rFiStAccMetricPrev1) \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[cur] = rFiStAccMetricPrev0; \
				matdecDecisions[i][cur] = 0; \
			} \
			else \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[cur] = rFiStAccMetricPrev1; \
				matdecDecisions[i][cur] = 1; \
			} \
			\
			/* Second state in this set ----------------------------------- */ \
			/* The only difference is that we swapped the matric sets */ \
			const _VITMETRTYPE rSecStAccMetricPrev0 = \
				pOldTrelMetric[prev0] + vecrMetricSet[met1]; \
			const _VITMETRTYPE  rSecStAccMetricPrev1 = \
				pOldTrelMetric[prev1] + vecrMetricSet[met0]; \
			\
			/* Take path with smallest metric */ \
			if (rSecStAccMetricPrev0 < rSecStAccMetricPrev1) \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[next] = rSecStAccMetricPrev0; \
				matdecDecisions[i][next] = 0; \
			} \
			else \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[next] = rSecStAccMetricPrev1; \
				matdecDecisions[i][next] = 1; \
			} \
		}


		/* Unroll butterflys to avoid loop overhead. For c++ version, the
		   actual calculation of the trellis update is done here, for MMX
		   version, only the reordering of the new metrics is done here */
		BUTTERFLY( 0,  1,  0, 32,  0, 15)
		BUTTERFLY( 2,  3,  1, 33,  6,  9)
		BUTTERFLY( 4,  5,  2, 34, 11,  4)
		BUTTERFLY( 6,  7,  3, 35, 13,  2)
		BUTTERFLY( 8,  9,  4, 36, 11,  4)
		BUTTERFLY(10, 11,  5, 37, 13,  2)
		BUTTERFLY(12, 13,  6, 38,  0, 15)
		BUTTERFLY(14, 15,  7, 39,  6,  9)
		BUTTERFLY(16, 17,  8, 40,  4, 11)
		BUTTERFLY(18, 19,  9, 41,  2, 13)
		BUTTERFLY(20, 21, 10, 42, 15,  0)
		BUTTERFLY(22, 23, 11, 43,  9,  6)
		BUTTERFLY(24, 25, 12, 44, 15,  0)
		BUTTERFLY(26, 27, 13, 45,  9,  6)
		BUTTERFLY(28, 29, 14, 46,  4, 11)
		BUTTERFLY(30, 31, 15, 47,  2, 13)
		BUTTERFLY(32, 33, 16, 48,  9,  6)
		BUTTERFLY(34, 35, 17, 49, 15,  0)
		BUTTERFLY(36, 37, 18, 50,  2, 13)
		BUTTERFLY(38, 39, 19, 51,  4, 11)
		BUTTERFLY(40, 41, 20, 52,  2, 13)
		BUTTERFLY(42, 43, 21, 53,  4, 11)
		BUTTERFLY(44, 45, 22, 54,  9,  6)
		BUTTERFLY(46, 47, 23, 55, 15,  0)
		BUTTERFLY(48, 49, 24, 56, 13,  2)
		BUTTERFLY(50, 51, 25, 57, 11,  4)
		BUTTERFLY(52, 53, 26, 58,  6,  9)
		BUTTERFLY(54, 55, 27, 59,  0, 15)
		BUTTERFLY(56, 57, 28, 60,  6,  9)
		BUTTERFLY(58, 59, 29, 61,  0, 15)
		BUTTERFLY(60, 61, 30, 62, 13,  2)
		BUTTERFLY(62, 63, 31, 63, 11,  4)

#undef BUTTERFLY

		/* Swap trellis data pointers (old -> new, new -> old) */
		_VITMETRTYPE* pTMPTrelMetric = pCurTrelMetric;
		pCurTrelMetric = pOldTrelMetric;
		pOldTrelMetric = pTMPTrelMetric;
	}


#ifndef USE_MAX_LOG_MAP
	/* Chainback the decoded bits from trellis (only for MLSE) -------------- */
	/* The end-state is defined by the DRM standard as all-zeros (shift register
	   in the encoder is padded with zeros at the end */
	iCurDecState = 0;

	for (i = 0; i < iNumOutBits; i++)
	{
		/* Read out decisions "backwards". Mask only first bit, because in MMX
		   implementation, all 8 bits of a "char" are set to the decision */
		const _DECISIONTYPE decCurBit =
			(matdecDecisions)[iNumOutBitsWithMemory - i - 1][iCurDecState] & 1;

		/* Calculate next state from previous decoded bit -> shift old data
		   and add new bit */
		iCurDecState = (iCurDecState >> 1) | (decCurBit << 5);

		/* Set decisions "backwards" in actual result vector */
		vecOutputBits[iNumOutBits - i - 1] = (_BINARY) decCurBit;
	}
#endif

	*nDeOutput = iNumOutBits;
	//cout << "iNumOutBits = " << iNumOutBits[mSerial] << endl;
	/* Return normalized accumulated minimum metric */
	// return pOldTrelMetric[0] / iDistCnt;

#ifdef RET_ACC
	short retValue = pOldTrelMetric[0] / iDistCnt;
	return retValue;
#else
	return 0;
#endif
	
	//printf("DumpOut: number = %d\n", iNumOutBits[mSerial]);
	/*
	for(int i = 0; i < iNumOutBits[mSerial]; i++)
	{
		if(i%8 == 0)
			printf("\n DumpOut %04d:", i);
		printf("%x ", vecOutputBits[i]);
	}
	printf("\n");
	printf("DumpOut: ret = %f\n", (double)retValue);
	*/
}

#if USE_CLASS
/*
FXP InvokeViterbi(int cmd, InitData* init, int nInput, _FREAL* input, int* nOutput, _DECISION* output)
{
	static MyViterbi viterbi;
	FXP ret = 0;

	//viterbi.mSerial = serial;
	viterbi.MyInit(init._eNewCodingScheme,
		init._eNewChannelType, init._iN1,
		init._iN2, init._iNewNumOutBitsPartA,
		init._iNewNumOutBitsPartB, init._iPunctPatPartA,
		init._iPunctPatPartB, init._iLevel);

	MyMemcpy(viterbi.vecNewDistance, input, nInput);

	viterbi.MyDecode();
	*nOutput = nDeOutput;

	return ret;
}
*/

void initiatorTransport(UINT32 addr, UINT32* dataPtr, UINT32 len, ECommand command)
{
    //Uns64 addr     = trans.get_address();
    //Uns32 *dataPtr = (Uns32*)trans.get_data_ptr();
	//unsigned int len = trans.get_data_length();
	static MyViterbi viterbi;
    UINT32  data = *dataPtr;
	//char *what;
	int i;
	//int offset = mSerial*MAP_SIZE;
	UINT32 base_addr;
	
	//WHICH_INSTANCE:
	for(i = 0; i < TOTAL_INSTANCES; i++)
	{
		if(addr >= 0xd3000000 + i*MAP_SIZE && addr < 0xd3000000 + (i + 1)*MAP_SIZE) {
			viterbi.mSerial = i;
			break;
		}
	}
	
	base_addr = addr - viterbi.mSerial*MAP_SIZE;
		
	switch( command )
	{
        case READ_COMMAND :
			//what = "read";
			if (base_addr == 0xd3000004) // Return status
			{
				//intr.write(0); // This would disable the interrupt line again
				//return current status, busy or available
				data = viterbi.status;
				MyMemcpy(dataPtr, &data, len);
			}
			else if (base_addr == 0xd300000C) //# of bytes to ARM
			{
				//put the number of bytes to the memory
				data = viterbi.nDeOutput;
				MyMemcpy(dataPtr, &data, len);
				//prepare for outputing decoded data
				viterbi.pOutput = (unsigned int*) viterbi.vecOutputBits;
			}
			else if (base_addr == 0xd3000034) //return value by MyDecode
			{
				data = viterbi.retDecode;
				MyMemcpy(dataPtr, &data, len);
			}
			else if (base_addr >= 0xd3000300 && 
				base_addr <= 0xd30003FC)
			{
				data = *(viterbi.pOutput);
				MyMemcpy(dataPtr, &data, len);
				/*
				if(viterbi.pOutput - (unsigned int*)vecOutputBits >
					viterbi.nDeOutput>>2)
					cout << "Output Pointer Out of Boundary" << endl;
					*/
				viterbi.pOutput++;
			}
			break;
        case WRITE_COMMAND:
			//what = "write" ;
			if (base_addr == 0xd3000040) // Command
			{
				if(data == 0) { //shutdown
				
				}
				else if(data == 1) //init
				{
					viterbi.status = -1;

					viterbi.MyInit(viterbi.eNewCodingScheme[viterbi.mSerial],
						viterbi.eNewChannelType[viterbi.mSerial], viterbi.iN1[viterbi.mSerial],
						viterbi.iN2[viterbi.mSerial], viterbi.iNewNumOutBitsPartA[viterbi.mSerial],
						viterbi.iNewNumOutBitsPartB[viterbi.mSerial], viterbi.iPunctPatPartA[viterbi.mSerial],
						viterbi.iPunctPatPartB[viterbi.mSerial], viterbi.iLevel[viterbi.mSerial]);
					viterbi.status = 0;
				}
				else if(data == 2) //decode
				{
					/*
					cout << "FPGA: command DECODE received" << endl;
					for(i = 0; i < nDeInput>>3; i++)
					{
						cout << i << ")" << (double)vecNewDistance[i].rTow0 << " " << (double)vecNewDistance[i].rTow1 << " " << (int)vecNewDistance[i].rTow0 << " "<< (int)vecNewDistance[i].rTow1 << endl;
					}*/
					
					viterbi.status = -1;
					viterbi.retDecode = viterbi.MyDecode();
					viterbi.status = 0;
					//cout << "FPGA: command DECODE done" << endl;
				}
				
				/*
				m_result = this->do_add(m_value1, m_value2);   
				intr.write(1);    // Signal an interrupt to the CPU
				*/
			}
			else if(base_addr == 0xd3000008) // # of bytes from ARM
			{
				//Save the number of bytes to be written by ARM
				viterbi.nDeInput = data;
				viterbi.bytesReceived = 0;
				//Initialize the memory for writing
				//viterbi.pInput = (unsigned int*)viterbi.vecNewDistance;
				viterbi.pInput = viterbi.vecNewDistance;
			}
			else if(base_addr == 0xd3000010) // The following 9 addresses are for init input
				viterbi.eNewCodingScheme[viterbi.mSerial] = (ECodScheme)data;
			else if(base_addr == 0xd3000014)	
				viterbi.eNewChannelType[viterbi.mSerial] = (EChanType)data;
			else if(base_addr == 0xd3000018)
				viterbi.iN1[viterbi.mSerial] = data;
			else if(base_addr == 0xd300001C)
				viterbi.iN2[viterbi.mSerial] = data;
			else if(base_addr == 0xd3000020)
				viterbi.iNewNumOutBitsPartA[viterbi.mSerial] = data;
			else if(base_addr == 0xd3000024)
				viterbi.iNewNumOutBitsPartB[viterbi.mSerial] = data;
			else if(base_addr == 0xd3000028)
				viterbi.iPunctPatPartA[viterbi.mSerial] = data;
			else if(base_addr == 0xd300002C)
				viterbi.iPunctPatPartB[viterbi.mSerial] = data;
			else if(base_addr == 0xd3000030) {	
				viterbi.iLevel[viterbi.mSerial] = data;
			}
			else if(base_addr >= 0xd3000200 && 
				base_addr <= 0xd30002FC)
			{
				//*(viterbi.pInput) = data;
				viterbi.pInput->directCopy(data);
				viterbi.pInput++;
				viterbi.bytesReceived += 4;
				/*
				if(viterbi.bytesReceived > viterbi.nDeInput)
					cout << "Should Receive " << nDeInput << "Bytes. But got " 
						<< bytesReceived << "Bytes Already!!" << endl;
						*/
				//nDeInput -= 4;
				//if(nDeInput < 0)
				//	cout << "Input Pointer Out of Boundary" << endl;
			}
				
			break;
        default:  	
			//what = "not interested";
			break;
    }
	/*
    if (1) {
        cout << hex << "Peripheral " << name() << " "<< what << " addr(base): " << addr << "(" << base_addr << ")" << " data: " << data << endl;
    }*/

    //trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

#endif
/*
int* MyViterbi::RetPuncTabPat()
{
	return veciTablePuncPat;
}*/
/*
int* MyViterbi::RetPuncTabPat()
{
    int* ret;
    switch(this->mSerial)
    {
        case 0:
            ret = veciTablePuncPat0;
            break;
        case 1:
            ret = veciTablePuncPat1;
            break;
        case 2:
            ret = veciTablePuncPat2;
            break;
        case 3:
            ret = veciTablePuncPat3;
            break;
        case 4:
            ret = veciTablePuncPat4;
            break;
        case 5:
            ret = veciTablePuncPat5;
            break;
    }
    return ret;
}
*/


#if 0
MyViterbi::MyViterbi()
{
	//SC_HAS_PROCESS(MyViterbi);
	//SC_THREAD(MyViterbi_thread);
	//sensitive << inputInit;
	this->status = 0;
	//bus.register_b_transport(this, &MyViterbi::initiatorTransport);
}

FXP MyViterbi::MyDecode()
{
	int		i;
	int		iDistCnt;
	int		iCurDecState;
	int*		pCurTrelMetric;	// Used to be float*
	int*		pOldTrelMetric; // Used to be float*

	/*
	for(int i = 0; i < (this->nDeInput>>3); i++)
	{
		if(i%2 == 0)
			printf("\n DumpIn %04d:", i);
		printf("%f %f ", (double)vecNewDistance[i].rTow0, 
			(double)vecNewDistance[i].rTow1);
	}
	printf("\n");
	*/

	/* Init pointers for old and new trellis state */
	pCurTrelMetric = this->vecTrelMetric1;
	pOldTrelMetric = this->vecTrelMetric2;

	/* Reset all metrics in the trellis. We initialize all states exept of
	   the zero state with a high metric, because we KNOW that the state "0"
	   is the transmitted state */
	pOldTrelMetric[0] = (_VITMETRTYPE) 0;
	//RESET_TRELLIS:
	for (i = 1; i < MC_NUM_STATES; i++)	// MC_NUM_STATES = 64
		pOldTrelMetric[i] = MC_METRIC_INIT_VALUE;

	/* Reset counter for puncturing and distance (from metric) */
	iDistCnt = 0;

	/* Main loop over all bits ---------------------------------------------- */
	//MAIN_DEC_LOOP:
	for (i = 0; i < this->iNumOutBitsWithMemory; i++)
	{
		/* Calculate all possible metrics ----------------------------------- */
		/* There are only a small set of possible puncturing patterns used for
		   DRM: 0001, 0101, 0011, 0111, 1111. These need different numbers of
		   input bits (increment of "iDistCnt" is dependent on pattern!). To
		   optimize the calculation of the metrics, a "subset" of bits are first
		   calculated which are used to get the final result. In this case,
		   redundancy can be avoided.
		   Note, that not all possible bit-combinations are used in the coder,
		   only a subset of numbers: 0, 2, 4, 6, 9, 11, 13, 15 (compare numbers
		   in the BUTTERFLY() calls) */

		/* Get first position in input vector (is needed for all cases) */
		const int iPos0 = iDistCnt;
		iDistCnt++;

		//if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0001)
		if (RetPuncTabPat()[i] == PP_TYPE_0001)
		{
			/* Pattern 0001 */
			this->vecrMetricSet[ 0] = this->vecNewDistance[2*iPos0];
			this->vecrMetricSet[ 2] = this->vecNewDistance[2*iPos0];
			this->vecrMetricSet[ 4] = this->vecNewDistance[2*iPos0];
			this->vecrMetricSet[ 6] = this->vecNewDistance[2*iPos0];
			this->vecrMetricSet[ 9] = this->vecNewDistance[2*iPos0+1];
			this->vecrMetricSet[11] = this->vecNewDistance[2*iPos0+1];
			this->vecrMetricSet[13] = this->vecNewDistance[2*iPos0+1];
			this->vecrMetricSet[15] = this->vecNewDistance[2*iPos0+1];
		}
		else
		{
			/* The following patterns need one more bit */
			const int iPos1 = iDistCnt;
			iDistCnt++;

			/* Calculate "subsets" of bit-combinations. "rIRxx00" means that
			   the fist two bits are used, others are x-ed. "IR" stands for
			   "intermediate result" */
			const int rIRxx00 =
				this->vecNewDistance[2*iPos1] + this->vecNewDistance[2*iPos0];
			const int rIRxx10 =
				this->vecNewDistance[2*iPos1+1] + this->vecNewDistance[2*iPos0];
			const int rIRxx01 =
				this->vecNewDistance[2*iPos1] + this->vecNewDistance[2*iPos0+1];
			const int rIRxx11 =
				this->vecNewDistance[2*iPos1+1] + this->vecNewDistance[2*iPos0+1];

			//if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0101)
			if (RetPuncTabPat()[i] == PP_TYPE_0101)
			{
				/* Pattern 0101 */
				this->vecrMetricSet[ 0] = rIRxx00;
				this->vecrMetricSet[ 2] = rIRxx00;
				this->vecrMetricSet[ 4] = rIRxx10;
				this->vecrMetricSet[ 6] = rIRxx10;
				this->vecrMetricSet[ 9] = rIRxx01;
				this->vecrMetricSet[11] = rIRxx01;
				this->vecrMetricSet[13] = rIRxx11;
				this->vecrMetricSet[15] = rIRxx11;
			}
			//else if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0011)
			else if (RetPuncTabPat()[i] == PP_TYPE_0011)
			{
				/* Pattern 0011 */
				this->vecrMetricSet[ 0] = rIRxx00;
				this->vecrMetricSet[ 2] = rIRxx10;
				this->vecrMetricSet[ 4] = rIRxx00;
				this->vecrMetricSet[ 6] = rIRxx10;
				this->vecrMetricSet[ 9] = rIRxx01;
				this->vecrMetricSet[11] = rIRxx11;
				this->vecrMetricSet[13] = rIRxx01;
				this->vecrMetricSet[15] = rIRxx11;
			}
			else
			{
				/* The following patterns need one more bit */
				const int iPos2 = iDistCnt;
				iDistCnt++;

				//if (this->veciTablePuncPat[this->mSerial][i] == PP_TYPE_0111)
				if (RetPuncTabPat()[i] == PP_TYPE_0111)
				{
					/* Pattern 0111 */
					this->vecrMetricSet[ 0] = this->vecNewDistance[2*iPos2] + rIRxx00;
					this->vecrMetricSet[ 2] = this->vecNewDistance[2*iPos2] + rIRxx10;
					this->vecrMetricSet[ 4] = this->vecNewDistance[2*iPos2+1] + rIRxx00;
					this->vecrMetricSet[ 6] = this->vecNewDistance[2*iPos2+1] + rIRxx10;
					this->vecrMetricSet[ 9] = this->vecNewDistance[2*iPos2] + rIRxx01;
					this->vecrMetricSet[11] = this->vecNewDistance[2*iPos2] + rIRxx11;
					this->vecrMetricSet[13] = this->vecNewDistance[2*iPos2+1] + rIRxx01;
					this->vecrMetricSet[15] = this->vecNewDistance[2*iPos2+1] + rIRxx11;
				}
				else
				{
					/* Pattern 1111 */
					/* This pattern needs all four bits */
					const int iPos3 = iDistCnt;
					iDistCnt++;

					/* Calculate "subsets" of bit-combinations. "rIRxx00" means
					   that the last two bits are used, others are x-ed.
					   "IR" stands for "intermediate result" */
					const int rIR00xx = this->vecNewDistance[2*iPos3] +
						this->vecNewDistance[2*iPos2];
					const int rIR10xx = this->vecNewDistance[2*iPos3+1] +
						this->vecNewDistance[2*iPos2];
					const int rIR01xx = this->vecNewDistance[2*iPos3] +
						this->vecNewDistance[2*iPos2+1];
					const int rIR11xx = this->vecNewDistance[2*iPos3+1] +
						this->vecNewDistance[2*iPos2+1];

					this->vecrMetricSet[ 0] = rIR00xx + rIRxx00; /* 0 */
					this->vecrMetricSet[ 2] = rIR00xx + rIRxx10; /* 2 */
					this->vecrMetricSet[ 4] = rIR01xx + rIRxx00; /* 4 */
					this->vecrMetricSet[ 6] = rIR01xx + rIRxx10; /* 6 */
					this->vecrMetricSet[ 9] = rIR10xx + rIRxx01; /* 9 */
					this->vecrMetricSet[11] = rIR10xx + rIRxx11; /* 11 */
					this->vecrMetricSet[13] = rIR11xx + rIRxx01; /* 13 */
					this->vecrMetricSet[15] = rIR11xx + rIRxx11; /* 15 */
				}
			}
		}


		/* Update trellis --------------------------------------------------- */
#define BUTTERFLY(cur, next, prev0, prev1, met0, met1) \
		{ \
			/* First state in this set ------------------------------------ */ \
			/* Calculate metrics from the two previous states, use the old
			   metric from the previous states plus the "transition-metric" */ \
			const _VITMETRTYPE rFiStAccMetricPrev0 = \
				pOldTrelMetric[prev0] + vecrMetricSet[met0]; \
			const _VITMETRTYPE  rFiStAccMetricPrev1 = \
				pOldTrelMetric[prev1] + vecrMetricSet[met1]; \
			\
			/* Take path with smallest metric */ \
			if (rFiStAccMetricPrev0 < rFiStAccMetricPrev1) \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[cur] = rFiStAccMetricPrev0; \
				matdecDecisions[i][cur] = 0; \
			} \
			else \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[cur] = rFiStAccMetricPrev1; \
				matdecDecisions[i][cur] = 1; \
			} \
			\
			/* Second state in this set ----------------------------------- */ \
			/* The only difference is that we swapped the matric sets */ \
			const _VITMETRTYPE rSecStAccMetricPrev0 = \
				pOldTrelMetric[prev0] + vecrMetricSet[met1]; \
			const _VITMETRTYPE  rSecStAccMetricPrev1 = \
				pOldTrelMetric[prev1] + vecrMetricSet[met0]; \
			\
			/* Take path with smallest metric */ \
			if (rSecStAccMetricPrev0 < rSecStAccMetricPrev1) \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[next] = rSecStAccMetricPrev0; \
				matdecDecisions[i][next] = 0; \
			} \
			else \
			{ \
				/* Save minimum metric for this state and store decision */ \
				pCurTrelMetric[next] = rSecStAccMetricPrev1; \
				matdecDecisions[i][next] = 1; \
			} \
		}


		/* Unroll butterflys to avoid loop overhead. For c++ version, the
		   actual calculation of the trellis update is done here, for MMX
		   version, only the reordering of the new metrics is done here */
		BUTTERFLY( 0,  1,  0, 32,  0, 15)
		BUTTERFLY( 2,  3,  1, 33,  6,  9)
		BUTTERFLY( 4,  5,  2, 34, 11,  4)
		BUTTERFLY( 6,  7,  3, 35, 13,  2)
		BUTTERFLY( 8,  9,  4, 36, 11,  4)
		BUTTERFLY(10, 11,  5, 37, 13,  2)
		BUTTERFLY(12, 13,  6, 38,  0, 15)
		BUTTERFLY(14, 15,  7, 39,  6,  9)
		BUTTERFLY(16, 17,  8, 40,  4, 11)
		BUTTERFLY(18, 19,  9, 41,  2, 13)
		BUTTERFLY(20, 21, 10, 42, 15,  0)
		BUTTERFLY(22, 23, 11, 43,  9,  6)
		BUTTERFLY(24, 25, 12, 44, 15,  0)
		BUTTERFLY(26, 27, 13, 45,  9,  6)
		BUTTERFLY(28, 29, 14, 46,  4, 11)
		BUTTERFLY(30, 31, 15, 47,  2, 13)
		BUTTERFLY(32, 33, 16, 48,  9,  6)
		BUTTERFLY(34, 35, 17, 49, 15,  0)
		BUTTERFLY(36, 37, 18, 50,  2, 13)
		BUTTERFLY(38, 39, 19, 51,  4, 11)
		BUTTERFLY(40, 41, 20, 52,  2, 13)
		BUTTERFLY(42, 43, 21, 53,  4, 11)
		BUTTERFLY(44, 45, 22, 54,  9,  6)
		BUTTERFLY(46, 47, 23, 55, 15,  0)
		BUTTERFLY(48, 49, 24, 56, 13,  2)
		BUTTERFLY(50, 51, 25, 57, 11,  4)
		BUTTERFLY(52, 53, 26, 58,  6,  9)
		BUTTERFLY(54, 55, 27, 59,  0, 15)
		BUTTERFLY(56, 57, 28, 60,  6,  9)
		BUTTERFLY(58, 59, 29, 61,  0, 15)
		BUTTERFLY(60, 61, 30, 62, 13,  2)
		BUTTERFLY(62, 63, 31, 63, 11,  4)

#undef BUTTERFLY

		/* Swap trellis data pointers (old -> new, new -> old) */
		_VITMETRTYPE* pTMPTrelMetric = pCurTrelMetric;
		pCurTrelMetric = pOldTrelMetric;
		pOldTrelMetric = pTMPTrelMetric;
	}


#ifndef USE_MAX_LOG_MAP
	/* Chainback the decoded bits from trellis (only for MLSE) -------------- */
	/* The end-state is defined by the DRM standard as all-zeros (shift register
	   in the encoder is padded with zeros at the end */
	iCurDecState = 0;

	for (i = 0; i < this->iNumOutBits; i++)
	{
		/* Read out decisions "backwards". Mask only first bit, because in MMX
		   implementation, all 8 bits of a "char" are set to the decision */
		const _DECISIONTYPE decCurBit =
			(this->matdecDecisions)[this->iNumOutBitsWithMemory - i - 1][iCurDecState] & 1;

		/* Calculate next state from previous decoded bit -> shift old data
		   and add new bit */
		iCurDecState = (iCurDecState >> 1) | (decCurBit << 5);

		/* Set decisions "backwards" in actual result vector */
		this->vecOutputBits[this->iNumOutBits - i - 1] = (_BINARY) decCurBit;
	}
#endif

	this->nDeOutput = iNumOutBits;
	//cout << "iNumOutBits = " << iNumOutBits[mSerial] << endl;
	/* Return normalized accumulated minimum metric */
	// return pOldTrelMetric[0] / iDistCnt;
	FXP retValue = pOldTrelMetric[0] / iDistCnt;
	//printf("DumpOut: number = %d\n", iNumOutBits[mSerial]);
	/*
	for(int i = 0; i < iNumOutBits[mSerial]; i++)
	{
		if(i%8 == 0)
			printf("\n DumpOut %04d:", i);
		printf("%x ", vecOutputBits[i]);
	}
	printf("\n");
	printf("DumpOut: ret = %f\n", (double)retValue);
	*/
	return retValue;
}

void MyViterbi::MyInit(ECodScheme eNewCodingScheme,
						   EChanType eNewChannelType, int iN1,
						   int iN2, int iNewNumOutBitsPartA,
						   int iNewNumOutBitsPartB, int iPunctPatPartA,
						   int iPunctPatPartB, int iLevel)
{

	/*
	cout << "CallMyInit: mSerial = " << mSerial << endl;

	cout << (int)eNewCodingScheme << " " << (int) eNewChannelType <<
		" " << iN1 << " " << iN2 << " " << iNewNumOutBitsPartA << endl;
	cout << iNewNumOutBitsPartB << " " << iPunctPatPartA <<
		" " << iPunctPatPartB << " " << iLevel << endl;
	*/
	/* Number of bits out is the sum of all protection levels */
	this->iNumOutBits = iNewNumOutBitsPartA + iNewNumOutBitsPartB;

	/* Number of out bits including the state memory */
	this->iNumOutBitsWithMemory = this->iNumOutBits + MC_CONSTRAINT_LENGTH - 1;

	/* Init vector, storing table for puncturing pattern and generate pattern */
	//this->veciTablePuncPat[this->mSerial].Init(this->iNumOutBitsWithMemory[this->mSerial]);

	this->MyGenPuncPatTable(eNewCodingScheme, eNewChannelType, iN1,
		iN2, iNewNumOutBitsPartA, iNewNumOutBitsPartB, iPunctPatPartA,
		iPunctPatPartB, iLevel);

	/* Init vector for storing the decided bits */
	//this->matdecDecisions[this->mSerial].Init(this->iNumOutBitsWithMemory[this->mSerial], MC_NUM_STATES);
}

void MyViterbi::MyGenPuncPatTable(ECodScheme eNewCodingScheme,
		EChanType eNewChannelType,
		int iN1, int iN2,
		int iNewNumOutBitsPartA,
		int iNewNumOutBitsPartB,
		int iPunctPatPartA, int iPunctPatPartB,
		int iLevel)
{
	int				i;
	int				iNumOutBits;
	int				iNumOutBitsWithMemory;
	int				iTailbitPattern;
	int				iTailbitParamL0;
	int				iTailbitParamL1;
	int				iPartAPatLen;
	int				iPartBPatLen;
	int				iPunctCounter;
	/*
	CVector<int>	veciPuncPatPartA;
	CVector<int>	veciPuncPatPartB;
	CVector<int>	veciTailBitPat;
	CVector<int>	veciReturn; */
	int             veciPuncPatPartA[3];
	int             veciPuncPatPartB[4];
	int             veciTailBitPat[LENGTH_TAIL_BIT_PAT];
	int*            veciReturn = RetPuncTabPat();

	/* {a, b, c ...}: a = Number of groups, b = Number of "1"s, c = Patterns */
	const UINT32 iPuncturingPatterns[13][10] = {
	/*
	B0: 1
	B1: 1
	B2: 1
	B3: 1
	*/
		{1, 4,
		 PP_TYPE_1111,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1
	B1: 1 1 1
	B2: 1 1 1
	B3: 1 0 0
	*/
		{3, 10,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1
	B1: 1
	B2: 1
	B3: 0
	*/
		{1, 3,
		 PP_TYPE_0111,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1 1
	B1: 1 1 1 1
	B2: 1 1 1 0
	B3: 0 0 0 0
	*/
		{4, 11,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1
	B1: 1
	B2: 0
	B3: 0
	*/
		{1, 2,
		 PP_TYPE_0011,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1 1
	B1: 1 0 1 0
	B2: 0 1 0 0
	B3: 0 0 0 0
	*/
		{4, 7,
		 PP_TYPE_0011,
		 PP_TYPE_0101,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1
	B1: 1 0 1
	B2: 0 0 0
	B3: 0 0 0
	*/
		{3, 5,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0011,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1
	B1: 1 0
	B2: 0 0
	B3: 0 0
	*/
		{2, 3,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1 1 1 1 1 1
	B1: 1 0 0 1 0 0 1 0
	B2: 0 0 0 0 0 0 0 0
	B3: 0 0 0 0 0 0 0 0
	*/
		{8, 11,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0011,
		 PP_TYPE_0001},

	/*
	B0: 1 1 1
	B1: 1 0 0
	B2: 0 0 0
	B3: 0 0 0
	*/
		{3, 4,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1 1
	B1: 1 0 0 0
	B2: 0 0 0 0
	B3: 0 0 0 0
	*/
		{4, 5,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1 1 1 1 1
	B1: 1 0 0 0 0 0 0
	B2: 0 0 0 0 0 0 0
	B3: 0 0 0 0 0 0 0
	*/
		{7, 8,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0000},

	/*
	B0: 1 1 1 1 1 1 1 1
	B1: 1 0 0 0 0 0 0 0
	B2: 0 0 0 0 0 0 0 0
	B3: 0 0 0 0 0 0 0 0
	*/
		{8, 9,
		 PP_TYPE_0011,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001,
		 PP_TYPE_0001}
	};

	/* Puncturing patterns for tailbits */
	const UINT32 iPunctPatTailbits[12][LENGTH_TAIL_BIT_PAT] = {
	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 0 0 0 0 0 0
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 0 0 0 0 0
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0011},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 0 0 1 0 0
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0011,
		 PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0011},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 0 1 0 0
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0011},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 0 1 1 0
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0011,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0011},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 0
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0011},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 1
	B3: 0 0 0 0 0 0
	*/
		{PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 1
	B3: 1 0 0 0 0 0
	*/
		{PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_0111},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 1
	B3: 1 0 0 1 0 0
	*/
		{PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_0111,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_0111},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 1
	B3: 1 1 0 1 0 0
	*/
		{PP_TYPE_1111,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_0111},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 1
	B3: 1 1 0 1 0 1
	*/
		{PP_TYPE_1111,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_1111},

	/*
	B0: 1 1 1 1 1 1
	B1: 1 1 1 1 1 1
	B2: 1 1 1 1 1 1
	B3: 1 1 1 1 0 1
	*/
		{PP_TYPE_1111,
		 PP_TYPE_1111,
		 PP_TYPE_1111,
		 PP_TYPE_1111,
		 PP_TYPE_0111,
		 PP_TYPE_1111},
	};

	/* Number of bits out is the sum of all protection levels */
	iNumOutBits = iNewNumOutBitsPartA + iNewNumOutBitsPartB;

	/* Number of out bits including the state memory */
	iNumOutBitsWithMemory = iNumOutBits + MC_CONSTRAINT_LENGTH - 1;

	/* Init vector, storing table for puncturing pattern */
	//veciReturn.Init(iNumOutBitsWithMemory);


	/* Set tail-bit pattern ------------------------------------------------- */
	/* We have to consider two cases because in HSYM "N1 + N2" is used
	   instead of only "N2" to calculate the tailbit pattern */
	switch (eNewCodingScheme)
	{
	case CS_3_HMMIX:
		iTailbitParamL0 = iN1 + iN2;
		iTailbitParamL1 = iN2;
		break;

	case CS_3_HMSYM:
		iTailbitParamL0 = 2 * (iN1 + iN2);
		iTailbitParamL1 = 2 * iN2;
		break;

	default:
		iTailbitParamL0 = 2 * iN2;
		iTailbitParamL1 = 2 * iN2;
	}

	/* Tailbit pattern calculated according DRM-standard. We have to consider
	   two cases because in HSYM "N1 + N2" is used instead of only "N2" */
	if (iLevel == 0)
		iTailbitPattern =
			iTailbitParamL0 - 12 - iPuncturingPatterns[iPunctPatPartB][1] *
			(int) ((iTailbitParamL0 - 12) /
			iPuncturingPatterns[iPunctPatPartB][1]);
	else
		iTailbitPattern =
			iTailbitParamL1 - 12 - iPuncturingPatterns[iPunctPatPartB][1] *
			(int) ((iTailbitParamL1 - 12) /
			iPuncturingPatterns[iPunctPatPartB][1]);


	/* Set puncturing bit patterns and lengths ------------------------------ */
	/* Lengths */
	iPartAPatLen = iPuncturingPatterns[iPunctPatPartA][0];
	iPartBPatLen = iPuncturingPatterns[iPunctPatPartB][0];

	/* Vector, storing patterns for part A. Patterns begin at [][2 + x] */
	//veciPuncPatPartA.Init(iPartAPatLen);
	//PUNCPAT_A:
	for (i = 0; i < iPartAPatLen; i++)
		veciPuncPatPartA[i] = iPuncturingPatterns[iPunctPatPartA][2 + i];

	/* Vector, storing patterns for part B. Patterns begin at [][2 + x] */
	//veciPuncPatPartB.Init(iPartBPatLen);
	//PUNCPAT_B:
	for (i = 0; i < iPartBPatLen; i++)
		veciPuncPatPartB[i] = iPuncturingPatterns[iPunctPatPartB][2 + i];

	/* Vector, storing patterns for tailbit pattern */
	//veciTailBitPat.Init(LENGTH_TAIL_BIT_PAT);
	//PUNCPAT_TAIL:
	for (i = 0; i < LENGTH_TAIL_BIT_PAT; i++)
		veciTailBitPat[i] = iPunctPatTailbits[iTailbitPattern][i];


	/* Generate actual table for puncturing pattern ------------------------- */
	/* Reset counter for puncturing */
	iPunctCounter = 0;

	//GEN_TABLE:
	for (i = 0; i < iNumOutBitsWithMemory; i++)
	{
		if (i < iNewNumOutBitsPartA)
		{
			/* Puncturing patterns part A */
			/* Get current pattern */
			veciReturn[i] = veciPuncPatPartA[iPunctCounter];

			/* Increment index and take care of wrap around */
			iPunctCounter++;
			if (iPunctCounter == iPartAPatLen)
				iPunctCounter = 0;
		}
		else
		{
			/* In case of FAC do not use special tailbit-pattern! */
			if ((i < iNumOutBits) || (eNewChannelType == CT_FAC))
			{
				/* Puncturing patterns part B */
				/* Reset counter when beginning of part B is reached */
				if (i == iNewNumOutBitsPartA)
					iPunctCounter = 0;

				/* Get current pattern */
				veciReturn[i] = veciPuncPatPartB[iPunctCounter];

				/* Increment index and take care of wrap around */
				iPunctCounter++;
				if (iPunctCounter == iPartBPatLen)
					iPunctCounter = 0;
			}
			else
			{
				/* Tailbits */
				/* Check when tailbit pattern starts */
				if (i == iNumOutBits)
					iPunctCounter = 0;

				/* Set tailbit pattern */
				veciReturn[i] = veciTailBitPat[iPunctCounter];

				/* No test for wrap around needed, since there ist only one
				   cycle of this pattern */
				iPunctCounter++;
			}
		}
	}
	
	//wait(nDeInput*407, SC_NS); //Each element takes 3.26 microseconds
	//return veciReturn;
}
#endif //USE_CLASS

