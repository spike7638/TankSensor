#pragma once

#ifndef TANK_h
#define TANK_h
#include "Persistence.h";
#include "TouchAndSense.h";
#include "Averaging.h";
/*
** Report the tank fullness, as a percentage of the total range, averaged over the chosen averaging period. 
*/

int getTankLevelPercent();
int getCriticalLevelPercent();

#endif
/////////////////////////////////////////////////////////////////