// C program to find the size of file
#include <stdio.h>
#include "data_transfer.h"
  
long int findSize(char file_name[])
{
    // opening the file in read mode
    FILE* fp = fopen(file_name, "rb");
  
    // checking if the file exist or not
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }
  
    fseek(fp, 0L, SEEK_END);
  
    // calculating the size of the file
    long int res = ftell(fp);
  
    // closing the file
    fclose(fp);
  
    return res;
}
  
// Driver code
int main()
{
    long int res = findSize(FILE_SEND_DATA);
    if (res != -1)
        printf("Size of the file is %ld bytes \n", res);
    return 0;
}
