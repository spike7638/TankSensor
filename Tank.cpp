#include "Tank.h"

int getTankLevelPercent()
{
  int lowerLim = getLowerLimit();
  int upperLim = getUpperLimit();
  int s = getSenseValue();
  int newReading = updateAverage(s);

  int16_t percent = round((100.0 * (newReading - lowerLim)) / float(upperLim - lowerLim));
  return percent;
}

int getCriticalLevelPercent()
{
  int lowerLim = getLowerLimit();
  int upperLim = getUpperLimit();
  int redLine = getCriticalValue();
  int16_t redLinePercent = round((100.0 * (redLine - lowerLim))/float(upperLim - lowerLim));
  return redLinePercent;
}
