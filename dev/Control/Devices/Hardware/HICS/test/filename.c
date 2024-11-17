#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>

int main(){
  time_t time_now;
  struct tm * time_now_cal;
  char year[4],month[2],day[2],hour[2],name[26];

  time_now = time(NULL);
  time_now_cal = gmtime(&time_now);

  sprintf(name, "hats-%.4d-%.2d-%.2dT%.2d00.rbd",
	  time_now_cal->tm_year+1900,
	  time_now_cal->tm_mon+1,
	  time_now_cal->tm_mday,
	  time_now_cal->tm_hour);

  printf("\n\n Today filename is %s\n\n",name);

  printf("\n\n Year = %d , Month = %d , Day = %d , Hour = %d\n\n",
	  time_now_cal->tm_year+1900,
	  time_now_cal->tm_mon+1,
	  time_now_cal->tm_mday,
	  time_now_cal->tm_hour);

  printf("%s\n\n",asctime(time_now_cal));
  
}

  
  
  
