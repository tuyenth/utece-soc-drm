#ifndef _MY_VITERBI_H_
#define _MY_VITERBI_H_

#include <ac_fixed.h>

/* Puncturing --------------------------------------------------------------- */
/* Only these types of patterns are used in DRM */
#define PP_TYPE_0000					0 /* not used, dummy */
#define PP_TYPE_1111					1
#define PP_TYPE_0111					2
#define PP_TYPE_0011					3
#define PP_TYPE_0001					4
#define PP_TYPE_0101					5
#define LENGTH_TAIL_BIT_PAT				6

/* Data type for Viterbi metric */
#define _VITMETRTYPE				int 	// Used to be float
#define _DECISIONTYPE				_BINARY
#define MC_NUM_OUTPUT_BITS_PER_STEP		4	/* MC: Multi-level Coder */
#define MC_CONSTRAINT_LENGTH			7
#define MC_NUM_STATES					(1 << (MC_CONSTRAINT_LENGTH - 1))
#define MC_NUM_OUTPUT_COMBINATIONS		(1 << MC_NUM_OUTPUT_BITS_PER_STEP)
#define MC_METRIC_INIT_VALUE		((_VITMETRTYPE) 10000)
#define MAP_SIZE			4096UL

//Input/Output Size
#define INUMOUTBITS				72
#define INUMOUTBITSWITHMEMORY	78
#define INPUTSIZE				130

typedef bool						_BINARY;
typedef ac_int<1, false>			MATTYPE;
typedef ac_int<8, false>			DISTTYPE;
typedef ac_int<4, false>			PUNCTYPE;
typedef _BINARY						_DECISION;
#if 0 //keep
class CFDistance
{
public:
	/* Distance towards 0 or towards 1 */
	_FREAL rTow0;
	_FREAL rTow1;
};
#endif

enum ECodScheme {CS_1_SM, CS_2_SM, CS_3_SM, CS_3_HMSYM, CS_3_HMMIX};
enum EChanType {CT_MSC, CT_SDC, CT_FAC};
enum ECommand {READ_COMMAND, WRITE_COMMAND};

short PureDecode(short iNumOutBits, short iNumOutBitsWithMemory, 
		PUNCTYPE veciTablePuncPat[INUMOUTBITSWITHMEMORY],
		short nDeInput, DISTTYPE vecNewDistance[INPUTSIZE*2],
		short* nDeOutput, _DECISION vecOutputBits[INUMOUTBITS]);

#endif

