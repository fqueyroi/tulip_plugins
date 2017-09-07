
#include <vector>
#include <ctime>
#include <cstdlib>

#include "LabelPropagation.h"


using namespace std;
using namespace tlp;


//========================================================================================
PLUGIN(LabelPropagation)
//========================================================================================

int alea(int n){
        assert (0 < n && n <= RAND_MAX);
         int partSize =
                  n == RAND_MAX ? 1 : 1 + (RAND_MAX-n)/(n+1);
         int maxUsefull = partSize * n + (partSize-1);
         int draw;
         do {
           draw = rand();
         } while (draw > maxUsefull);
         return draw/partSize;
}

//========================================================================================
vector<double> LabelPropagation::bestNeighbors(node toMove){
    map<double,double> neigh;
    //Insert node's own clusters
    double comm = result->getNodeValue(toMove);
    neigh.insert(make_pair(comm,0.0));


    double max=0.0;
    edge e;
    forEach(e,graph->getInOutEdges(toMove)){       
        double cInd =result->getNodeValue(graph->opposite(e,toMove));
        if(neigh.find(cInd)==neigh.end())
            neigh.insert(make_pair(cInd,0.0));
        neigh[cInd]+=weights->getEdgeValue(e);
        if(neigh[cInd]>max)
            max=neigh[cInd];
    }
    vector<double> best;
    map<double,double>::const_iterator it;
    for(it=neigh.begin();it!=neigh.end();++it){
        if(it->second==max)
            best.push_back(it->first);
    }
    return best;
}
//========================================================================================
bool LabelPropagation::onePass(){
    vector<node> random_nodes;
    random_nodes.resize(graph->numberOfNodes());
    unsigned int i=0;
    node n;
    forEach(n,graph->getNodes()){
        random_nodes[i]=n;
        i++;
    }
    random_shuffle(random_nodes.begin(),random_nodes.end());

    bool change=false;
    for(unsigned int i=0;i<random_nodes.size();++i){
        n  = random_nodes[i];
        vector<double> bneigh = bestNeighbors(n);
        double best_comm;
        if(bneigh.size()>1){
            unsigned int choose = alea(bneigh.size()-1);
            best_comm = bneigh[choose];
        }else
            best_comm = (*bneigh.begin());
        if(result->getNodeValue(n)!=best_comm)
            change=true;
        result->setNodeValue(n,best_comm);
    }
    return change;
}
//========================================================================================
namespace {
const char * paramHelp[] = {
  // metric
  HTML_HELP_OPEN()              \
  HTML_HELP_DEF( "type", "NumericProperty" )       \
  HTML_HELP_DEF( "value", "An existing edge metric" )                 \
  HTML_HELP_BODY()              \
  "An existing edge metric property"\
  HTML_HELP_CLOSE()
};
}
//========================================================================================
LabelPropagation::LabelPropagation(const PluginContext* context): DoubleAlgorithm(context){
    addInParameter<NumericProperty*>("metric",paramHelp[0],"",false);
}
//========================================================================================
LabelPropagation::~LabelPropagation(){}
//========================================================================================
bool LabelPropagation::run(){

    if(dataSet!=0){
        dataSet->get("metric",weights);
    }

    if(weights==NULL){
        weights = new DoubleProperty(graph);
        weights->setAllEdgeValue(1.0);

    }
    // initialize a random sequence according the given seed
    tlp::initRandomSequence();

    //init result
    node n;
    forEach(n,graph->getNodes())
        result->setNodeValue(n,n.id);

    //propagation
    unsigned int max_iteration=100;
    bool change = true;
    unsigned int i = 0;

    while(change && i<max_iteration){
        change=onePass();
        ++i;
    }
    return true;

}
//========================================================================================
