
#ifndef FASTESTPATHSELECTION_H
#define FASTESTPATHSELECTION_H

#include <tulip/BooleanProperty.h>
#include <tulip/StringProperty.h>

#include <vector>

#include "TimeDependentDijkstra.h"

using namespace tlp;
using namespace std;
//================================================================================
namespace{
    const char * paramHelp[] = {
        // id node src
        "Id of source node",
        // id node tgt
        "Id of target node",
        // departures
        "Departure times of each edge.",
        // arrival
        "Arrival times of each edge.",
        // starting time
        "Starting time of the journey."
        // Cost stopover
        "The cost in time of stoping on a non-target node."
    };
}
//================================================================================
/** \addtogroup selection */

/** Select one fastest-path from node id_src to node id_tgt
 * using the times of arrival of departure on each (directed) edges
 * with "Start" as starting time for the travel.
 *
 * The parameter "Cost stopover" is the time added for each internal nodes used.
 * In the algorithm, if we arrive at time t on a node with a cost of c, the next best departures
 * will be found in the time interval [t + c,T].
 *
 */
//================================================================================
class FastestPathSelection : public tlp::BooleanAlgorithm {
//================================================================================
public:
  PLUGININFORMATION(
  "Fastest Path Selection",
  "Francois Queyroi",
  "18/05/2019","","1.0","Measure")
  //================================================================================
  FastestPathSelection(tlp::PluginContext *context) : BooleanAlgorithm(context){
    addInParameter<unsigned int>("id src",paramHelp[0],"0",true);
    addInParameter<unsigned int>("id tgt",paramHelp[1],"1",true);
    addInParameter<DoubleVectorProperty>("Departures",paramHelp[2],"",true);
    addInParameter<DoubleVectorProperty>("Arrivals",paramHelp[3],"",true);
    addInParameter<double>("Cost Stopover",paramHelp[4],"0",true);
    addInParameter<double>("Start",paramHelp[4],"0",true);
    addOutParameter<double>("Duration","Duration of the trip","-1");
    addOutParameter<double>("Wait","Total wait during the trip","-1");
    addOutParameter<int>("Nb Stops","Number of stopover during the trip","-1");
  }
  //================================================================================
  bool run() override{
    // Get Parameters
    unsigned int id_src = 0;
    unsigned int id_tgt = 1;
    DoubleVectorProperty* departures;
    DoubleVectorProperty* arrivals;
    double start = 14335.;
    double cost_stop = 0.;

    if(dataSet!=0){
      dataSet->get("id src",id_src);
      dataSet->get("id tgt",id_tgt);
      dataSet->get("Departures",departures);
      dataSet->get("Arrivals",arrivals);
      dataSet->get("Start",start);
      dataSet->get("Cost Stopover",cost_stop);
    }

    //Get src and tgt nodes
    node src = node(id_src);
    node tgt = node(id_tgt);
    if(!graph->isElement(src)){
      pluginProgress->setError("Graph id "+graph->getName()+" does not contain node id "+to_string(id_src));
      return false;
    }
    if(!graph->isElement(tgt)){
      pluginProgress->setError("Graph id "+graph->getName()+" does not contain node id "+to_string(id_tgt));
      return false;
    }

    // Check if departures and arrivals size are equals
    for(auto e : graph->getEdges()){
      vector<double> vect_dep = departures->getEdgeValue(e);
      vector<double> vect_arr = arrivals->getEdgeValue(e);
      if(vect_dep.size() != vect_arr.size()){
        pluginProgress->setError("Error: edge "+to_string(e.id)+"'s departure and arrival are not of the same size.");
        return false;
      }
    }

    NodeStaticProperty<double> nodeDuration(graph);
    NodeStaticProperty<double> waitingTime(graph);
    NodeStaticProperty<unsigned int> nbOfSteps(graph);
    TimeDependentDijkstra tvg_dikj(graph,src,departures,arrivals,nodeDuration,waitingTime,nbOfSteps,cost_stop);
    tvg_dikj.compute(start);

    // Compute fastest path
//    DoubleProperty* indexes = graph->getLocalProperty<DoubleProperty>("indexes");
//    bool isPath = tvg_dikj.searchPath(tgt,result,indexes);
//    if(!isPath){
//      cout << " No Path Found !" << endl;
//      return false;
//    }

    vector<int> indexes;
    vector<edge> path;
    bool isPath = tvg_dikj.searchPath(tgt,path,&indexes);
    if(!isPath)
      return false;

    // Print path and save results
    cout << "From Node " << src.id << " t= " << start << endl;
    for(unsigned int i = 0; i < path.size(); ++i){
      edge e = path[i];
      unsigned int ind_e = indexes[i];
      cout << "  To Node " << graph->target(e).id
           << " dep: " << int(departures->getEdgeValue(e)[ind_e])
           << " arr: " << int(arrivals->getEdgeValue(e)[ind_e])
           << " D: "   << nodeDuration[graph->target(e)]
           << " W: "   << waitingTime[graph->target(e)]
           << endl;
      result->setEdgeValue(e,true);
      result->setNodeValue(graph->source(e),true);
      result->setNodeValue(graph->target(e),true);
    }

    dataSet->set("Duration",nodeDuration[tgt]);
    dataSet->set("Wait",waitingTime[tgt]);
    dataSet->set("Nb Stops",int(path.size() - 1));
    return true;
  }
};
//================================================================================
PLUGIN(FastestPathSelection)
//================================================================================
#endif //FASTESTPATHSELECTION_H
