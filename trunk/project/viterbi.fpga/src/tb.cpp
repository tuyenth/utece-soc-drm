#include <stdio.h>
#include <stdlib.h>
#define BUFSIZE	256
#ifdef CCS_SCVERIFY
    #include "mc_testbench.h"
#else
	#include "MyViterbi.h"
#endif

//Puncture table file, each line has a number.
//The first line is iNumOutBits
//The second line is iNumOutBitsWithMemory
//The other lines are the content of the puncture table 
//(iNumOutBitsWithMemory lines), puncture table size equals to iNumOutBitsWithMemory
//All values are integers
#define PUNC_TABLE_FILE		"table.txt.1"
//The first line is the number of elements of CFDistance (integer)
//Then every two lines compose a CFDistance element.
//The first one is rTow0 and the second rTow1. They are doubles.
#define INPUT_FILE			"input.txt.1"
//The first line is the return value (double)
//The second line is the number of output bits. (integer)
//Then follows all the output bits (integer)
#define GOLDEN_FILE			"golden.txt.1"

int* readPuncTable(char* file, short* iNumOutBits, 
	short * iNumOutBitsWithMemory);
_FREAL* readInput(char* file, short* iInputNum);
_DECISION* readGolden(char* file, FXP* retValue, short* iOutputNum);

#ifdef CCS_SCVERIFY
    #define CCS_RETURN(x) return
    #define CATFUNC exec_fir
    void testbench::main()
#else
    #define CCS_RETURN(x) void(x)
    #define CATFUNC fir
    int main()
#endif
{
	short numOutBits, numOutBitsWithMemory;
	int* pPuncTable;
	short inputNum, gOutputNum, rOutputNum;
	int i, same;
	FXP gRetValue, rRetValue;
	_FREAL* pInput;
	_DECISION* pGolden;
	_DECISION pOutput[INUMOUTBITS];

	pPuncTable = readPuncTable(PUNC_TABLE_FILE, &numOutBits, 
		&numOutBitsWithMemory);
	pInput = readInput(INPUT_FILE, &inputNum);
	pGolden = readGolden(GOLDEN_FILE, &gRetValue, &gOutputNum);

#ifndef CCS_SCVERIFY
	rRetValue = PureDecode(numOutBits, numOutBitsWithMemory,
        pPuncTable,
		inputNum, pInput,
		&rOutputNum, pOutput);
	printf("\n\n[PC Decode Result]\n");
	printf("rRetValue = %f\n", (double)rRetValue);
	printf("rOutputNum = %d\n", rOutputNum);
	same = 1;
	for(i = 0; i < rOutputNum; i++)
	{
			printf("pOutput,pgolden =%d,%d", pOutput[i],pGolden[i]);
			if(pOutput[i] != pGolden[i])
			same = 0;
	}
	printf("same = %d\n", same);
#else
	rRetValue = testbench::exec_PureDecode(numOutBits, numOutBitsWithMemory,
        pPuncTable, inputNum, pInput,
		&rOutputNum, pOutput);
	printf("\n\n[FPGA Decode Result]\n");
	printf("rRetValue = %f\n", (double)rRetValue);
	printf("rOutputNum = %d\n", rOutputNum);
	same = 1;
	for(i = 0; i < rOutputNum; i++)
	{
		printf("pOutput,pgolden =%d,%d", pOutput[i],pGolden[i]);
		if(pOutput[i] != pGolden[i])
			same = 0;
	}
	printf("same = %d\n", same);

#endif
	


#ifdef CCS_SCVERIFY
	return;
#else
	return 0;
#endif
}

int* readPuncTable(char* file, short* iNumOutBits, short* iNumOutBitsWithMemory)
{
	FILE* fp;
	char buf[BUFSIZE];
	int pindex;
	int value;
	int* table;
	short numOutBits, numOutBitsWithMemory;

	fp = fopen(file, "rt");

	if(!fp) 
	{
		fprintf(stderr, "Can't open Puncture Table %s\n", file);
		exit(1);
	}

	if(fgets(buf, BUFSIZE, fp) == NULL)
	{
		fprintf(stderr, "Error reading numOutBits in %s\n", file);
		exit(1);
	}
	numOutBits = (short)strtol(buf, NULL, 10);
	printf("numOutBits = %d\n", numOutBits);

	if(fgets(buf, BUFSIZE, fp) == NULL)
	{
		fprintf(stderr, "Error reading numOutBitsWithMemory in %s\n", file);
		exit(1);
	}
	numOutBitsWithMemory = (short)strtol(buf, NULL, 10);
	printf("numOutBitsWithMemory = %d\n", numOutBitsWithMemory);

	table = (int*)malloc(numOutBitsWithMemory*sizeof(int));

	for(pindex = 0; pindex < numOutBitsWithMemory; pindex++)
	{
		if(fgets(buf, BUFSIZE, fp) == NULL)
		{
			fprintf(stderr, "Error reading Puncture Table in %s, pindex = %d\n", file, pindex);
			exit(1);
		}
	
		value = strtol(buf, NULL, 10);
		table[pindex] = value;
	}
	
	fclose(fp);

	*iNumOutBits = numOutBits;
	*iNumOutBitsWithMemory = numOutBitsWithMemory;
	
	return table;
}

_FREAL* readInput(char* file, short* iInputNum)
{
	FILE* fp;
	char buf[BUFSIZE];
	int pindex;
	double value;
	_FREAL* table;
	short inputNum;

	fp = fopen(file, "rt");

	if(!fp) 
	{
		fprintf(stderr, "Can't open Input File %s\n", file);
		exit(1);
	}

	if(fgets(buf, BUFSIZE, fp) == NULL)
	{
		fprintf(stderr, "Error reading number of elements in %s\n", file);
		exit(1);
	}
	inputNum = strtol(buf, NULL, 10);
	printf("inputNum = %d\n", inputNum);

	table = (_FREAL*)malloc(inputNum*2*sizeof(_FREAL));

	for(pindex = 0; pindex < inputNum*2; pindex++)
	{
		if(fgets(buf, BUFSIZE, fp) == NULL)
		{
			fprintf(stderr, "Error reading Input in %s, pindex = %d\n", file, pindex);
			exit(1);
		}
	
		value = strtod(buf, NULL);
		table[pindex] = value;
	}
	
	fclose(fp);

	*iInputNum = inputNum;
	
	return table;

}

_DECISION* readGolden(char* file, FXP* retValue, short* iOutputNum)
{
	FILE* fp;
	char buf[BUFSIZE];
	int pindex;
	double tRetValue;
	_DECISION value;
	_DECISION* table;
	int outputNum;

	fp = fopen(file, "rt");

	if(!fp) 
	{
		fprintf(stderr, "Can't open Input File %s\n", file);
		exit(1);
	}

	if(fgets(buf, BUFSIZE, fp) == NULL)
	{
		fprintf(stderr, "Error reading ret value in %s\n", file);
		exit(1);
	}
	tRetValue = strtod(buf, NULL);
	printf("tRetValue = %f\n", tRetValue);

	if(fgets(buf, BUFSIZE, fp) == NULL)
	{
		fprintf(stderr, "Error reading # of output bits in %s\n", file);
		exit(1);
	}
	outputNum = strtol(buf, NULL, 10);
	printf("outputNum = %d\n", outputNum);

	table = (_DECISION*)malloc(outputNum*sizeof(_DECISION));

	for(pindex = 0; pindex < outputNum; pindex++)
	{
		if(fgets(buf, BUFSIZE, fp) == NULL)
		{
			fprintf(stderr, "Error reading Input in %s, pindex = %d\n", file, pindex);
			exit(1);
		}
	
		value = strtol(buf, NULL, 10);
		table[pindex] = value;
	}
	
	fclose(fp);

	*retValue = tRetValue;
	
	return table;
}

