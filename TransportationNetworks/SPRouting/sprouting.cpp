#include "sprouting.h"

//======================================================================================================
using namespace tlp;
using namespace std;
//================================================================================
PLUGIN(SpRouting)
//================================================================================
namespace{
    const char * paramHelp[] = {
        // are road
        "Indicates which edges can be part of the transportation network.",
        // flow value
        "Flow[e] for edge e=(a<->b) with is road = False\n indicates the amount of flow between node a and b",
        // length
        "The length of edges with is road = True",
        // Reinforcement power
        "Chooose the reinforcement power mu >= 1 of the update function ",
        // Amplitude alpha
        "Choose the amplitude alpha>=0 of the update function",
        // Decay rate
        "Choose the devay rate gamma in ]0,1]",
        // Approximation error
        "Choose the precision of the algorithm (need to be small)"
    };
}
//================================================================================
SpRouting::SpRouting(const PluginContext *context): DoubleAlgorithm(context){
    addInParameter<BooleanProperty>("is road",paramHelp[0],"",true);
    addInParameter<DoubleProperty>("flow value",paramHelp[1],"",true);
    addInParameter<DoubleProperty>("length",paramHelp[2],"",false);
    addInParameter<double>("mu",paramHelp[3],"1.8",false);
    addInParameter<double>("alpha",paramHelp[4],"20",false);
    addInParameter<double>("decay",paramHelp[5],"0.2",false);
    addInParameter<double>("epsilon",paramHelp[6],"0.0001",false);
    addDependency("Flow Betweenness","1.0");
}
//================================================================================
bool SpRouting::run(){

    // Initialize parameters
    DoubleProperty* input_length;
    DoubleProperty* flow_value;
    BooleanProperty* is_road;
    double mu=1.8;
    double alpha=20.;
    double decay=0.2;
    double epsilon=0.0001;

    // get In parameters
    if(dataSet!=0){
        dataSet->get("length",input_length);
        dataSet->get("is road",is_road);
        dataSet->get("flow value",flow_value);
        dataSet->get("mu",mu);
        dataSet->get("alpha",alpha);
        dataSet->get("decay",decay);
        dataSet->get("epsilon",epsilon);
    }

    //Copy the length of edges
    DoubleProperty _length(graph);
    if(input_length!=NULL){
        edge e;
        forEach(e,graph->getEdges()){
            _length.setEdgeValue(e,input_length->getEdgeValue(e));
        }
    }else
        _length.setAllEdgeValue(1.);

    // Compute the total flow
    edge e;
    double sum_flow=0.;
    forEach(e,graph->getEdges()){
        if(!is_road->getEdgeValue(e)){
            sum_flow+=flow_value->getEdgeValue(e);
        }
    }

    // Flow on the transportation network at each iteration
    DoubleProperty* part_crossings=new DoubleProperty(graph);
    part_crossings->setAllEdgeValue(0.);

    // Variable to check  wether diameter on roads edges changed a lot
    double max_diff_part=DBL_MAX;
    unsigned int i_step=0;

    //init result
    result->setAllEdgeValue(1.);

    // Main loop
    while(max_diff_part>epsilon){
        ++i_step;
        // Compute flow distribution using Flow betweeness plugin
        string errMsg="";
        DataSet ds;
        ds.set("is road",is_road);
        ds.set("length",&_length);
        ds.set("flow value",flow_value);
        bool computation_res=graph->applyPropertyAlgorithm("Flow Betweenness",part_crossings,errMsg,pluginProgress,&ds);

        if(!computation_res){
            pluginProgress->setError(errMsg);
            return false;
        }

        //  Update edge diameter according to current flow part_crossings
        max_diff_part=0.;
        edge er;
        forEach(er,is_road->getEdgesEqualTo(true)){

            double q=part_crossings->getEdgeValue(er);

            // Apply Type II formula (sigmoid-like function)
            double fq = (alpha+1.)*pow(q/sum_flow,mu)/(1.+alpha*pow(q/sum_flow,mu));

            // Update edge diameter according to D[n+1] = decay * fq +(1 - decay)* D[n]
            double new_diam = decay*fq+(1.-decay)*result->getEdgeValue(er);

           // Update max_diff_part
            max_diff_part=max(max_diff_part,fabs(new_diam-result->getEdgeValue(er)));

            result->setEdgeValue(er,new_diam);            
            //update length using new diameter
            if(input_length!=NULL){
                _length.setEdgeValue(er,input_length->getEdgeValue(er)/result->getEdgeValue(er));
            }else{
                _length.setEdgeValue(er,1./result->getEdgeValue(er));
            }
        }
        cout << i_step  << "  Max diff = " << max_diff_part << " / " << epsilon << endl;
        if(pluginProgress->progress(i_step,graph->numberOfEdges())!=TLP_CONTINUE) break;
    }
    dataSet->set<unsigned int>("final nb steps",i_step+1);

    //set diameter to zero to edges with flow or diameter less than epsilon
    edge er;
    forEach(er,is_road->getEdgesEqualTo(true)){
        if(part_crossings->getEdgeValue(er)<epsilon || result->getEdgeValue(er)<epsilon)
            result->setEdgeValue(er,0.);
    }
    forEach(er,is_road->getEdgesEqualTo(false)){
        result->setEdgeValue(er,0.);
    }
    delete(part_crossings);
    return pluginProgress->state()!=TLP_CANCEL;;
}
//================================================================================
bool SpRouting::check(string &errMsg){
    errMsg="";
    return true;
}
//================================================================================
