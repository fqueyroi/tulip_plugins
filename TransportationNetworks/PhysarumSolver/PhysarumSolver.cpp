#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "PhysarumSolver.h"
#include "IterativeCycleSolver.h"


//======================================================================================================
using namespace tlp;
using namespace std;
//======================================================================================================
// Methods and classes to sort nodes according to their degree
//======================================================================================================
struct NodeDeg {
    NodeDeg ( const double d = DBL_MAX, const tlp::node n= tlp::node()) : d(d),n(n) {}
    bool operator==(const NodeDeg &b) const {return n == b.n;}
    bool operator!=(const NodeDeg &b) const {return n != b.n;}
    double d;
    tlp::node n;
};
//================================================================================
struct HighNodeDeg {
    bool operator()(const NodeDeg&  a,const NodeDeg&  b ) const {
        if (fabs(a.d - b.d) > 1.E-9)
            return (a.d > b.d);
        else
            return (a.n.id < b.n.id);
    }
};
//================================================================================
//Sort the nodes of g in decreasing order of g.deg
void sortNodesByDeg(Graph* g,vector<node>& sorted){
    set<NodeDeg,HighNodeDeg> sorted_nd;
    node u;
    forEach(u,g->getNodes()){
        sorted_nd.insert(NodeDeg(g->deg(u),u));
    }
    sorted.clear();
    set<NodeDeg,HighNodeDeg>::const_iterator itn;
    for(itn=sorted_nd.begin();itn!=sorted_nd.end();++itn){
        sorted.push_back((*itn).n);
    }
}
//================================================================================
PLUGIN(PhysarumSolver)
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
//================================================================================================
PhysarumSolver::PhysarumSolver(const PluginContext *context) : DoubleAlgorithm(context){
    addInParameter<BooleanProperty>("is road",paramHelp[0],"",true);
    addInParameter<DoubleProperty>("flow value",paramHelp[1],"",true);
    addInParameter<DoubleProperty>("length",paramHelp[2],"",false);
    addInParameter<double>("mu",paramHelp[3],"1.8",false);
    addInParameter<double>("alpha",paramHelp[4],"20",false);
    addInParameter<double>("decay",paramHelp[5],"0.2",false);
    addInParameter<double>("epsilon",paramHelp[6],"0.0001",false);
}
//================================================================================================
bool PhysarumSolver::run(){

    // Initialize parameters
    DoubleProperty* input_length;
    DoubleProperty* input_flow;
    BooleanProperty* is_road;
    double mu=1.8;
    double alpha=20.;
    double decay=0.2;
    double epsilon=0.0001;

    // Get parameters
    if(dataSet!=0){
        dataSet->get("is road",is_road);
        dataSet->get("flow value",input_flow);
        dataSet->get("length",input_length);
        dataSet->get("mu",mu);
        dataSet->get("alpha",alpha);
        dataSet->get("decay",decay);
        dataSet->get("epsilon",epsilon);
    }

    // Make flow and transport subgraph as subgraphs of graph
    Graph* flow = graph->addCloneSubGraph("flow");
    Graph* roads = graph->addCloneSubGraph("roads");
    edge e;
    forEach(e,graph->getEdges()){
        if(is_road->getEdgeValue(e)){
            flow->delEdge(e);
        }else{
            roads->delEdge(e);
        }
    }

    // Remove node that exchange no flow with others in flow
    node u;
    forEach(u,flow->getNodes()){
        if(flow->deg(u)==0)
            flow->delNode(u);
    }

    //Copy the length of edges
    DoubleProperty length(graph);
    if(input_length!=NULL){
        edge e;
        forEach(e,graph->getEdges()){
            length.setEdgeValue(e,input_length->getEdgeValue(e));
        }
    }else
        length.setAllEdgeValue(1.);

    // Copy the flow property and compute the total flow on the network
    DoubleProperty flow_value(flow);
    double sum_flow=0.;
    forEach(e,flow->getEdges()){
        flow_value.setEdgeValue(e,input_flow->getEdgeValue(e));
        sum_flow+=input_flow->getEdgeValue(e);
    }

    // Find a small vertex cover of the flow graph
    // and change edge orientations
    // so that for each edge (a,b), if deg(b) > deg(a) then a-> b
    vector<node> sorted_nodes;
    BooleanProperty visited(flow);
    visited.setAllNodeValue(false);
    sortNodesByDeg(flow,sorted_nodes);

    for(unsigned int i=0;i<sorted_nodes.size();++i){
        node u = sorted_nodes[i];
        visited.setNodeValue(u,true);
        edge e;
        forEach(e,flow->getOutEdges(u)){
            if(!visited.getNodeValue(flow->opposite(e,u))){
                flow->reverse(e);
            }
        }
    }

    // Flow on the transportation network at each iteration
    DoubleProperty* part_crossings=new DoubleProperty(graph);
    part_crossings->setAllEdgeValue(0.);

    //Init edge diameter to 1
    result->setAllEdgeValue(1.);

    // Set the precision of solver
    double solver_epsilon=0.05;

    // Variable to check  wether diameter on roads edges changed a lot
    double max_diff_part=DBL_MAX;
    unsigned int i_step=0;

    // Main loop
    while(max_diff_part>epsilon){
        ++i_step;
        part_crossings->setAllEdgeValue(0.);

        //Compute connected components of roads and apply the flow computation of each components
        vector<vector<node> > components;
        tlp::ConnectedTest::computeConnectedComponents(roads,components);
        for(unsigned int id_comp=0;id_comp<components.size();++id_comp){
            //get induced subgraph of roads
            Graph* sub_roads=roads->inducedSubGraph(components[id_comp]);
            //get induced subgraph of flow;
            Graph* sub_flow = flow->addCloneSubGraph("sub_flow");
            node u;
            forEach(u,flow->getNodes()){
                if(!sub_roads->isElement(u)){
                    sub_flow->delNode(u);
                }
            }
            //if no flow between nodes in sub_roads then remove this part of roads and continue
            if(sub_flow->numberOfEdges()==0){
                node u;
                forEach(u,sub_roads->getNodes()){
                    roads->delNode(u);
                }
            }else{
                // Init Cycle solver
                string errMsg="";
                IterativeCycleSolver cyclesolver(sub_roads,&length,result,solver_epsilon);
                bool init_res = cyclesolver.compute(errMsg);
                if(!init_res){
                    pluginProgress->setError(errMsg);
                    return false;
                }

                // Compute flow on road network for each flow destination in the flow graph
                for(unsigned int i_tgt=0;i_tgt<sorted_nodes.size();++i_tgt){
                    node tgt=sorted_nodes[i_tgt];
                    if(sub_flow->isElement(tgt) && sub_flow->indeg(tgt)>0){
                        bool comp_res = cyclesolver.solve(sub_flow,&flow_value,tgt,part_crossings,errMsg);
                        if(!comp_res){
                            pluginProgress->setError(errMsg);
                            return false;
                        }
                    }
                }
            }

            //remove sub graphs of flow and roads
            roads->delAllSubGraphs(sub_roads);
            flow->delAllSubGraphs(sub_flow);
        }

        //  Update edge diameter according to current flow part_crossings
        max_diff_part=0.;
        edge er;
        forEach(er,roads->getEdges()){
            double q=part_crossings->getEdgeValue(er);

            // Apply Type II formula (sigmoid-like function)
            double fq = (alpha+1.)*pow(q/sum_flow,mu)/(1.+alpha*pow(q/sum_flow,mu));

            // Update edge diameter according to D[n+1] = decay * fq +(1 - decay)* D[n]
            double new_diam = decay*fq+(1.-decay)*result->getEdgeValue(er);

            // Update max_diff_part
            max_diff_part=max(max_diff_part,fabs(new_diam-result->getEdgeValue(er)));

            // If new diameter is too small (close to zero) remove the edge (and its endpoints if needed)
            if(new_diam<1E-7){
                result->setEdgeValue(er,0.);
                node u=roads->source(er);
                node v=roads->target(er);
                roads->delEdge(er);
                if(roads->deg(u)==0){
                    roads->delNode(u);
                }
                if(roads->deg(v)==0){
                    roads->delNode(v);
                }
            }else
                result->setEdgeValue(er,new_diam);
        }
        cout << "Step " << i_step  << "  Max diff = " << max_diff_part << " / " << epsilon << endl;
        if(pluginProgress->progress(i_step,roads->numberOfEdges())!=TLP_CONTINUE) break;
    }

    //set diameter to zero to edges with flow or diameter less than epsilon
    edge er;
    forEach(er,is_road->getEdgesEqualTo(true)){
        if(part_crossings->getEdgeValue(er)<epsilon || result->getEdgeValue(er)<epsilon)
            result->setEdgeValue(er,0.);
    }
    forEach(er,is_road->getEdgesEqualTo(false)){
        result->setEdgeValue(er,0.);
    }

    dataSet->set<unsigned int>("final nb steps",i_step+1);
    delete(part_crossings);
    graph->delSubGraph(roads);
    graph->delSubGraph(flow);
    return pluginProgress->state()!=TLP_CANCEL;
}
//================================================================================================
bool PhysarumSolver::check(string &errMsg){}
//================================================================================================


