#include "stdio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define READ_CMD  (0x0 << 31)
#define WRITE_CMD (0x1 << 31)

#define COMMAND_MASK 0x80000000


int det_int = 0;

void sighandler(int signo)
{
  if(signo==SIGIO)
    {
      det_int++;
      printf("\nInterrupt detected\n");
    }
  return;
}


int main(int argc, char * argv[]) 
{
  int val1, val2, result;
  struct sigaction action;
  int fd;

  //Ensure proper usage
  if(argc != 3)
  {
    printf("Usage: %s value1 value2\n",argv[0]);
    return -1;
  }

  //assign val1 and val2
  val1 = atoi(argv[1]);
  val2 = atoi(argv[2]);

  // initialize device
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, SIGIO);

  action.sa_handler = sighandler;
  action.sa_flags=0;

  sigaction(SIGIO, & action, NULL);

  fd=open("/dev/fpga", O_RDWR);
  fcntl(fd, F_SETOWN, getpid());
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_ASYNC);
  if(!fd)
  {

      printf("Unable to open /dev/fpga.  Ensure it exists (major=246, minor=1)\n");
      return -1;
  }

  //Write to addr1
  ioctl(fd, WRITE_CMD + 1, &val1);

  //Write to addr2
  ioctl(fd, WRITE_CMD + 2, &val2);

  //Trigger addition by writing to addr0
  ioctl(fd, WRITE_CMD + 0, &val2);

  //Wait for interrupt
  while(!det_int) continue;

  //Read from addr0. The result should be the sum of val1 and val2
  ioctl(fd, READ_CMD, &result);

  printf("The sum of %d and %d is %d\n", val1, val2, result);

  //In the end, unmap the base address and close the memory file
  close(fd);

  return 0;
}
