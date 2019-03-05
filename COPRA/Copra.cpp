#include <vector>
#include <ctime>
#include <cstdlib>

#include "Copra.h"

using namespace std;
using namespace tlp;

//========================================================================================
PLUGIN(Copra)
//========================================================================================
int alea(int n){
	assert (0 < n && n <= RAND_MAX);
	int partSize = n == RAND_MAX ? 1 : 1 + (RAND_MAX-n)/(n+1);
        int maxUsefull = partSize * n + (partSize-1);
        int draw;
        do {
          draw = rand();
        } while (draw > maxUsefull);
        return draw/partSize;
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
	HTML_HELP_CLOSE(),
	// max number of communities per node
	HTML_HELP_OPEN()              \
	HTML_HELP_DEF( "type", "unsigned int" )       \
	HTML_HELP_DEF( "value", "3" )                 \
	HTML_HELP_BODY()              \
	"The maximum number of communities per node"\
	HTML_HELP_CLOSE(),
};
}
//========================================================================================
Copra::Copra(const PluginContext* context): IntegerVectorAlgorithm(context){
    addInParameter<NumericProperty*>("metric",paramHelp[0],"",false);
    addInParameter<unsigned int>("max_comm",paramHelp[1],"3",true);
}
//========================================================================================
LabelPropagation::~LabelPropagation(){}
//========================================================================================
bool LabelPropagation::run(){
	unsigned int max_comm = 3;
	if(dataSet!=0){
		dataSet->get("metric",weights);
		dataSet->get("max_comm"max_comm);
	}

	if(weights==NULL){
		weights = new DoubleProperty(graph);
		weights->setAllEdgeValue(1.0);
	}

	// initialize a random sequence according the given seed
    tlp::initRandomSequence();

	// initialize label values
	forEach(n,graph->getNodes()){
		<set<CommPair, LessCommPair> > lab;	
		lab.insert(CommPair(n,1.));
		old_labels.set(n.id,lab);
	}


