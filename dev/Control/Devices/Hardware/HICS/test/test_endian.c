#include <stdio.h>

unsigned char test_endian( void )
{
    int test_var = 1;
    unsigned char *test_endian = (unsigned char*)&test_var;

    return (test_endian[0] == 0);
}

int main(){

  FILE *fp;
  unsigned int test_var = 1, r_var;

  printf("\n\n sizeof(int) = %ld, test_var = %d\n",sizeof(test_var),test_var);
  fp = fopen("test_endian.bin","wb");
  fwrite(&test_var,sizeof(test_var), 1, fp);
  fclose(fp);
  fp = fopen("test_endian.bin","rb");
  fread(&r_var,sizeof(r_var),1,fp);
  printf("\n\n sizeof(int) = %ld, r_var = %d\n",sizeof(r_var),r_var);


  if (test_endian){
    printf("\n\n This Machine is LITTLE endian\n\n");
  } else {
    printf("\n\n This Machine is BIG endian\n\n");
  }

  return 0;
    
}
