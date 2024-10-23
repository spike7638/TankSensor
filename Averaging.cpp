// how many readings to record
#include "Averaging.h"


static int readings[N_AVERAGE];
static int which_reading = 0; // index into which to place next reading
static int sum = 0; 

void averageInit(){
    for (int i = 0; i < N_AVERAGE; i++){
      readings[i] = 0; 
    }
    which_reading = 0; 
}

int updateAverage(int reading)
{
  sum = sum - readings[which_reading] + reading;
  readings[which_reading] = reading; 
  which_reading++;
  if (which_reading == N_AVERAGE) which_reading = 0;
  return round(double(sum))/N_AVERAGE;
}



