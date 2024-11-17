#include <stdio.h>
#include <stdlib.h>

int main(void){

  FILE *fp;
  size_t len = 0 ;
  ssize_t Nread;
  char * buff = NULL, str[20];
  float Dt;
  long N,i=0;
  int adc;
  double t;
  typedef struct {
    double time;
    long adc;
  } data ;

  data * val ;

  fp = fopen("test01.txt","r");
  Nread = getline(&buff, &len, fp);
  sscanf(buff,"%5c%f",str,&Dt);
  
  Nread = getline(&buff, &len, fp);  
  sscanf(buff,"%4c%ld",str,&N);

  printf("\n\n N = %ld  Dt = %f\n\n",N,Dt);

  Nread = getline(&buff, &len, fp);
  val = malloc(N*sizeof(data));
  
  while (( Nread = getline(&buff, &len, fp)) != -1){
    sscanf(buff,"%5d%lf",&adc,&t);  
    val[i].time = t;
    val[i].adc = adc;
    printf("time = %10.5lf  adc = %d\n",val[i].time,val[i].adc);    
    i++;
  }

  fclose(fp);

}

  
