#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void main(){
  
  time_t rawtime;  
  struct tm * ti;
  char isotime[20];

  time (&rawtime);
  ti = localtime(&rawtime);
  printf("\n\n %04d-%02d-%02dT%02d:%02d:%02d\n\n",ti->tm_year+1900,
	 ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec);

}
  
  
