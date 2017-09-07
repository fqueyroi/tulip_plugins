#ifndef LABELPROPAGATION_H
#define LABELPROPAGATION_H

#include <ctime>
#include <map>
#include <tulip/TulipPluginHeaders.h>

using namespace std;
using namespace tlp;

class LabelPropagation : public tlp::DoubleAlgorithm {
public:
    PLUGININFORMATION("Label Propagation","François Queyroi","13/02/15","Nodes partitioning measure used for community detection.","1.0","Clustering")

    LabelPropagation(const tlp::PluginContext*);
    ~LabelPropagation();
    bool run();

private:

    vector<double> bestNeighbors(node toMove);
    bool onePass();
    DoubleProperty* weights;

};


#endif
