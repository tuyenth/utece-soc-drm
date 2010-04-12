#ifndef _MY_VITERBI_H_
#define _MY_VITERBI_H_

#include "FXP.h"
//#include "Vector.h"
#include "GlobalDefinitions.h"
#include "Puncture.h"
//#include "tlm.h"
//#include "tlm_utils/simple_target_socket.h"

#include <ostream>

#define TOTAL_INSTANCES 6

// using namespace sc_core;
// using namespace tlm;
// using namespace std;

typedef unsigned int 				UINT32;
typedef FXP							_FREAL;
typedef unsigned char/*bool*/		_BINARY;

enum ECodScheme {CS_1_SM, CS_2_SM, CS_3_SM, CS_3_HMSYM, CS_3_HMMIX};
enum EChanType {CT_MSC, CT_SDC, CT_FAC};
enum ECommand {READ_COMMAND, WRITE_COMMAND};

/* Data type for Viterbi metric */
#define _VITMETRTYPE				int 	// Used to be float
#define _DECISIONTYPE				_BINARY
#define MC_NUM_OUTPUT_BITS_PER_STEP		4	/* MC: Multi-level Coder */
#define MC_CONSTRAINT_LENGTH			7
#define MC_NUM_STATES					(1 << (MC_CONSTRAINT_LENGTH - 1))
#define MC_NUM_OUTPUT_COMBINATIONS		(1 << MC_NUM_OUTPUT_BITS_PER_STEP)
#define MC_METRIC_INIT_VALUE		((_VITMETRTYPE) 10000)
#define MAP_SIZE			4096UL

//SC_MODULE(MyViterbi)
class MyViterbi
{
public:
	//typedef tlm_utils::simple_target_socket<MyViterbi> target_socket_type;
	//typedef tlm_generic_payload transaction_type;

	//tlm_analysis_port<int> intr;
	//port declarations
	/*
	sc_port< sc_fifo_in_if<int> > inputInit;
	sc_port< sc_fifo_in_if<InitStruct> > inputInitData;
	
	sc_port< sc_fifo_in_if<CVector<CFDistance> > > inputCFDistance;
	
	sc_port< sc_fifo_out_if<CVector<_DECISION> > > outputDecision;
	
	sc_port< sc_fifo_out_if<FXP> > outputRet;*/
	//channel declarations
	//target_socket_type bus;
	//variable declarations
	//event declarations
	//process declarations
	//void MyViterbi_thread(void);
	//helper declarations
	//void initiatorTransport(transaction_type& trans,
	//	sc_core::sc_time& t);
	//module instantiations
	
	//Constructor
	MyViterbi();
//private:
	void MyGenPuncPatTable(ECodScheme eNewCodingScheme,
		EChanType eNewChannelType,
		int iN1, int iN2,
		int iNewNumOutBitsPartA,
		int iNewNumOutBitsPartB,
		int iPunctPatPartA, int iPunctPatPartB,
		int iLevel);
	
	void MyInit(ECodScheme eNewCodingScheme,
		EChanType eNewChannelType, int iN1,
		int iN2, int iNewNumOutBitsPartA,
		int iNewNumOutBitsPartB, int iPunctPatPartA,
		int iPunctPatPartB, int iLevel);

	FXP MyDecode();
		
	int			iNumOutBits[TOTAL_INSTANCES];
	int			iNumOutBitsWithMemory[TOTAL_INSTANCES];
	//CVector<int>		veciTablePuncPat[TOTAL_INSTANCES];
	int*        RetPuncTabPat();
	int         veciTablePuncPat0[78];
	int         veciTablePuncPat1[216];
	int         veciTablePuncPat2[426];
	int         veciTablePuncPat3[1560];
	int         veciTablePuncPat4[3114];
	int         veciTablePuncPat5[3734];
	//CMatrix<_DECISIONTYPE>	matdecDecisions[TOTAL_INSTANCES];
	_DECISIONTYPE   matdecDecisions[3734][MC_NUM_STATES];

	/* Two trellis data vectors are needed for current and old state */
	int			vecTrelMetric1[MC_NUM_STATES];  // Used to be float
	int			vecTrelMetric2[MC_NUM_STATES];	// Used to be float
	int			vecrMetricSet[MC_NUM_OUTPUT_COMBINATIONS];

	//Decoder input: Maximum elements=4674, each is 8 bytes	
	int bytesReceived;
	int nDeInput;
	//unsigned int* pInput;
	_FREAL* pInput;
	//CFDistance vecNewDistance[5000];
	_FREAL vecNewDistance[5000*2];
	//CVector<CFDistance> vecNewDistance;
    //Decoder output: Maximum elements=3495, each is 1 byte
	int nDeOutput;
	unsigned int* pOutput;
	_DECISION vecOutputBits[4000];
	FXP retDecode;
	//CVector<_DECISION> vecOutputBits;
	//init input
	ECodScheme eNewCodingScheme[TOTAL_INSTANCES];
	EChanType eNewChannelType[TOTAL_INSTANCES];
	int iN1[TOTAL_INSTANCES];
	int iN2[TOTAL_INSTANCES];
	int iNewNumOutBitsPartA[TOTAL_INSTANCES];
	int iNewNumOutBitsPartB[TOTAL_INSTANCES]; 
	int iPunctPatPartA[TOTAL_INSTANCES];
	int iPunctPatPartB[TOTAL_INSTANCES];
	int iLevel[TOTAL_INSTANCES];

	//ToBeChanged: should check where this variable is used
	int mSerial;
	//status
	int status;
};	

#endif
