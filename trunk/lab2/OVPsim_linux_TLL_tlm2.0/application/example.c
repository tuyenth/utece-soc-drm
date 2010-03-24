#include "stdio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define TANK_SPACE 0x100	

int main(int argc, char * argv[]) {
  volatile unsigned int *base, *address;
  volatile unsigned int *ptrTank;

  unsigned long addr1, addr2, addr0, addrBack, addrNum, addrTank, addrComp, offset, value;
  int val1, val2, result;
  int i;
#define ELEMENTS 4096	
  int Tank[ELEMENTS];
  int Back[ELEMENTS];
  int *ptr;
  int number, remaining;

  for(i = 0; i < ELEMENTS; i++) {
	Tank[i] = i;
	Back[i] = -1;
  }

  //Predefined addresses.
  //Write the values to be added to addr1 and addr2
  //Read the result from addr0
  addr0 = strtoul("0xd3000000", 0, 0);
  addr1 = strtoul("0xd3000004", 0, 0);
  addr2 = strtoul("0xd3000008", 0, 0);
  addrComp = strtoul("0xd300000C", 0, 0);
  addrNum = strtoul("0xd3000010", 0, 0);
  addrTank = strtoul("0xd3000020", 0, 0);
  addrBack = strtoul("0xd3000200", 0, 0);

  //Ensure proper usage
  if(argc != 3)
  {
    printf("Usage: %s value1 value2\n",argv[0]);
    return -1;
  }
  //assign val1 and val2
  val1 = atoi(argv[1]);
  val2 = atoi(argv[2]);


  //Open memory as a file
  int fd = open("/dev/mem", O_RDWR|O_SYNC);
  if(!fd)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}	

  //Map the base address to base
  base = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr0 & ~MAP_MASK);	
  if((base == MAP_FAILED))
  {
    printf("mapping failed\n");
    fflush(stdout);
  }

  //Write to addr1
  address = base + ((addr1 & MAP_MASK)>>2);
  *address = val1;

  //Write to addr2
  address = base + ((addr2 & MAP_MASK)>>2);
  *address = val2;
 
  //write how many dwords will be sent
  address = base + ((addrNum & MAP_MASK)>>2);
  *address = ELEMENTS;

  ptrTank = base + ((addrTank & MAP_MASK)>>2);
  ptr = Tank;
  remaining = ELEMENTS*4;
  while(remaining != 0) 
  {
	number = (remaining > TANK_SPACE) ? TANK_SPACE : remaining;
	memcpy(ptrTank, ptr, number);
	ptr += (number >> 2);
	remaining -= number;
  }

  //Trigger the verification of data
  address = base + ((addrComp & MAP_MASK)>>2);
  *address = 1;

  //Trigger reading back data
  //address = base + ((addrComp & MAP_MASK)>>2);
  //*address = 2;
  printf("====Before Reading Back====\n");
  for(i = 0;i < ELEMENTS; i++) {
	printf("%d ", Back[i]);
	if(i%8 == 0)
		printf("\n");
  }

  ptrTank = base + ((addrBack & MAP_MASK)>>2);
  ptr = Back;
  remaining = ELEMENTS*4;
  while(remaining != 0) 
  {
	number = (remaining > TANK_SPACE) ? TANK_SPACE : remaining;
	memcpy(ptr, ptrTank, number);
	ptr += (number >> 2);
	remaining -= number;
  }
  printf("====After Reading Back====\n");

  for(i = 0;i < ELEMENTS; i++) {
	printf("%d ", Back[i]);
	if(i%8 == 0)
		printf("\n");
  }

  //Trigger addition by writing to addr0
  address = base + ((addr0 & MAP_MASK)>>2);
  *address = -1;

  //Poll hardware until it is ready and
  //read from addr0. The result should be the sum of val1 and val2
  do {
    address = base + ((addr0 & MAP_MASK)>>2);
    result = *address;
  } while (result < 0);

  printf("The sum of %d and %d is %d\n", val1, val2, result);

  //In the end, unmap the base address and close the memory file
  munmap((void*)base, MAP_SIZE);
  close(fd);

  return 0;
}
