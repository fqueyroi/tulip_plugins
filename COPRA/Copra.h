#ifndef COPRA_H
#define COPRA_H

#include <ctime>
#include <map>
#include <tulip/TulipPluginHeaders.h>
#include <tulip/MutableContainer.h>

using namespace std;
using namespace tlp;
//========================================================================================
struct CommPair {
	CommPair(const node n = node(),const double b = 0.) : n(n),belong(b){}
	bool operator==(const CommPair &b) const {return n.id == b.n.id;}
    bool operator!=(const CommPair &b) const {return n.id != b.n.id;}
	node n;
	double belong;
}
//========================================================================================
struct LessCommPair {
    bool operator()(const LessCommPair&  a,const LessCommPair&  b )  const{
        if (fabs(a.belong - b.belong) > 1.E-9)
            return (a.belong < b.belong);
        else
            return (a.n.id < b.n.id);
    }
};
//========================================================================================
class Copra : public tlp::IntegerVectorAlgorithm {
public:
    PLUGININFORMATION("COPRA Clustering","François Queyroi","04/03/19","Nodes partitioning into multiple communities.","1.0","Clustering")

    Copra(const tlp::PluginContext*);
    ~Copra();
    bool run();

private:
    vector<double> propagate(node);


    DoubleProperty* weights;
	MutableContainer<set<CommPair, LessCommPair> > old_labels;
	MutableContainer<set<CommPair, LessCommPair> > new_labels;
};

#endif
