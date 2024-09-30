// how many readings to record
#include "Averaging.h"

 // how many readings to average to get the current value-to-display
#define N 100

static int readings[N];
static int which_reading = 0; // index into which to place next reading
static int sum = 0; 

void averageInit(){
    for (int i = 0; i < N; i++){
      readings[i] = 0; 
    }
    which_reading = 0; 
}

int updateAverage(int reading)
{
  sum = sum - readings[which_reading] + reading;
  readings[which_reading] = reading; 
  which_reading++;
  if (which_reading == N) which_reading = 0;
  return round(double(sum))/N;
}



