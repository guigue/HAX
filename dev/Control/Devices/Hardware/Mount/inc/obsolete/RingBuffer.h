#ifndef RBUFFER
#define RBUFFER

#define NREC 100

#define BackingFile "Hats_Ring_Buffer"
#define SemaphoreName "semaphore"
#define AccessPerms 0644

typedef struct {
  float azi,ele;
  unsigned long long time;
  int   adcu[5];
} HatsData ;

typedef struct {
  HatsData block[NREC];
} HatsRBuffer ;

#endif
