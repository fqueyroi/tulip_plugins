
#ifndef AVERAGETRAVELTIME_H
#define AVERAGETRAVELTIME_H

#include <tulip/Algorithm.h>

class AverageTravelTime : public tlp::Algorithm {

public:
  PLUGININFORMATION(
  "Average Travel Time",
  "Francois Queyroi",
  "18/06/2018","","1.0","Measure")

  AverageTravelTime(tlp::PluginContext *context);

  bool run() override;    
};

#endif //AVERAGETRAVELTIME_H
