
#ifndef FASTESTPATHSELECTION_H
#define FASTESTPATHSELECTION_H

#include <tulip/BooleanProperty.h>
#include <tulip/StringProperty.h>

#include <vector>
#include <algorithm>
#include <utility>
#include <numeric>



#include "TimeDependentDijkstra.h"


using namespace tlp;
using namespace std;
//================================================================================
namespace{
    const char * paramHelp[] = {
        // id node src
        "Id of src node",
        // id node tgt
        "Id of tgt node",
        // departures
        "Departure times of each edge.",
        // arrival
        "Arrival times of each edge.",
        // starting time
        "Starting time of the journey."
    };
}
//================================================================================
template <typename T>
vector<size_t> sort_indexes(const vector<T> &v) {

  // initialize original index locations
  vector<size_t> idx(v.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
}
//================================================================================

/** \addtogroup selection */

/**
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
    addInParameter<unsigned int>("id src",paramHelp[0],"432",true);
    addInParameter<unsigned int>("id tgt",paramHelp[1],"173",true);
    addInParameter<DoubleVectorProperty>("Departures",paramHelp[2],"departures",true);
    addInParameter<DoubleVectorProperty>("Arrivals",paramHelp[3],"arrivals",true);
    addInParameter<double>("Start",paramHelp[4],"14335",true);
  }
  //================================================================================
  bool run() override{
    // Get Parameters
    unsigned int id_src = 432;
    unsigned int id_tgt = 173;
    DoubleVectorProperty* departures;
    DoubleVectorProperty* arrivals;
    double start = 14335.;

    if(dataSet!=0){
      dataSet->get("id src",id_src);
      dataSet->get("id tgt",id_tgt);
      dataSet->get("Departures",departures);
      dataSet->get("Arrivals",arrivals);
      dataSet->get("Start",start);
    }

    //Get src and tgt nodes
    node src = node(id_src);
    node tgt = node(id_tgt);
    if(!graph->isElement(src))
      pluginProgress->setError("Graph id "+graph->getName()+" does not contain node id "+to_string(id_src));
    if(!graph->isElement(tgt))
      pluginProgress->setError("Graph id "+graph->getName()+" does not contain node id "+to_string(id_tgt));


    // Sort arrival and departure times according to departure time
    EdgeStaticProperty<vector<double> > dep_time(graph);
    EdgeStaticProperty<vector<double> > arr_time(graph);
    for(auto e : graph->getEdges()){
      vector<double> vect_dep = departures->getEdgeValue(e);
      vector<double> vect_arr = arrivals->getEdgeValue(e);
      if(vect_dep.size() != vect_arr.size()){
        pluginProgress->setError("Error: edge "+to_string(e.id)+"'s departure and arrival are not of the same size.");
        return false;
      }
      vector<double> sorted_vect_dep;
      vector<double> sorted_vect_arr;
      for (auto i : sort_indexes(vect_dep)){
        sorted_vect_dep.push_back(vect_dep[i]);
        sorted_vect_arr.push_back(vect_arr[i]);
      }
      dep_time.setEdgeValue(e,sorted_vect_dep);
      arr_time.setEdgeValue(e,sorted_vect_arr);
    }

    NodeStaticProperty<double> nodeDuration(graph);
    NodeStaticProperty<double> waitingTime(graph);
    TimeDependentDijkstra tvg_dikj(graph,src,&dep_time,&arr_time,nodeDuration,waitingTime);
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
           << " dep: " << int(dep_time[e][ind_e])
           << " arr: " << int(arr_time[e][ind_e])
           << " D: "   << nodeDuration[graph->target(e)]
           << " W: "   << waitingTime[graph->target(e)]
           << endl;
      result->setEdgeValue(e,true);
      result->setNodeValue(graph->source(e),true);
      result->setNodeValue(graph->target(e),true);
    }

    return true;
  }
};
//================================================================================
PLUGIN(FastestPathSelection)
//================================================================================
#endif //FASTESTPATHSELECTION_H
