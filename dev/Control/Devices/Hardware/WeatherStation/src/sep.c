#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
void stringCopy(char *s1,char *s2, int first, int last)
{
  int i;
  for(i=first;i<=last;i++) {s2[i-first]=s1[i];}
}

int main()
{
  char *s1="Ta=-12.3C,P=756HPa,H=35%", ascii_minus=45,neg,sT[6], sP[4], sH[3];
  float T,P,H;  

  if (s1[3]== ascii_minus) {neg = 1;} else {neg=0;}

  stringCopy(s1,sT,3,6+neg);
  T = atof(sT);
  stringCopy(s1,sP,11+neg,13+neg);
  P = atof(sP);
  stringCopy(s1,sH,20+neg,21+neg);
  H = atof(sH);

  printf("\n\n String = %s", s1);
  printf("\n\n Temp = %f, Hum = %f, Press = %f\n\n",T,H,P);
  return 0;   
}
