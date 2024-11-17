#include <stdio.h>
#include <stdlib.h>

int main(void){

  unsigned short num =0xabcd, swapped ;

  swapped = (num<<8) ;
  printf("\n\n num = %x swapped = %x -> %x\n\n",num,swapped,swapped&0xff00);

}

