#ifndef AVERAGETRAVELTIME_H
#define AVERAGETRAVELTIME_H

#include "TimeDependentDijkstra.h"

#include <vector>
#include <queue>
#include <map>
#include <string>
#include <iostream>
#include <numeric>

#include <tulip/StaticProperty.h>
#include <tulip/DoubleProperty.h>
#include <tulip/BooleanProperty.h>

using namespace tlp;
using namespace std;

//================================================================================
namespace{
    const char * paramHelp[] = {
        // departures
        "Departure times of each edge.",
        // arrival
        "Arrival times of each edge.",
        // min time
        "Minimum time value of fatest-paths.",
        // max time
        "Maximum time value of fatest-paths.",
        // delta
        "Timestep value for average computation.",
        // Cost stopover
        "The cost in time of stoping at a non-target vertex."
    };
}
/** \addtogroup metric */

/** This plugin computes time-dependent shortest-paths durations
 * using the times of arrival of departure on each (directed) edges.
 * It produces the average over each node of the average
 * time needed to go from on node to the others when
 * the starting time varies from [t_0,T] (given as parameters).
 *
 * The plugin also allow to keep track of
 * - The average total of time spent waiting for next departure.
 * - The average number of reachable nodes.
 * - The average number of stopovers (internal nodes in the fastest path).
 *
 * The parameter "Cost stopover" is the time added for each internal nodes used.
 * In the algorithm, if we arrive at time t on a node with a cost of c, the next best departures
 * will be found in the time interval [t + c,T].
 *
 *  <b>HISTORY</b>
 *
 *  - 18/06/18 Version 1.0: Initial release
 */

class AverageTravelTime : public tlp::DoubleAlgorithm {
  public:
  PLUGININFORMATION(
    "Average Travel Time", "Francois Queyroi", "18/06/2018",
                  "Computes the average travel time of a node to every other node"
                  " using departures and arrival times for each (directed) edge.",
                  "1.1","Measure")
  //================================================================================
  AverageTravelTime(PluginContext *c) : DoubleAlgorithm(c){
      addInParameter<DoubleVectorProperty>("Departures",paramHelp[0],"",true);
      addInParameter<DoubleVectorProperty>("Arrivals",paramHelp[1],"",true);
      addInParameter<double>("Min time computation",paramHelp[2],"0",true);
      addInParameter<double>("Max time computation",paramHelp[3],"100",true);
      addInParameter<double>("Delta time computation",paramHelp[4],"1",true);
      addInParameter<double>("Cost Stopover",paramHelp[5],"0",true);
      addOutParameter<DoubleProperty>("Reachable nodes","Average number of reachable nodes.","",false);
      addOutParameter<DoubleProperty>("Waiting time","Average total time spent waiting for the next departure","",false);
      addOutParameter<DoubleProperty>("Stopovers","Average number stopovers (internal nodes in the fastest path).","",false);

  }
  //================================================================================
  bool run() override {

      // Get Parameters
      DoubleVectorProperty* departures;
      DoubleVectorProperty* arrivals;
      double T0 = 0;
      double T  = 100;
      double delta = 1.;
      double cost_stop = 0.;
      DoubleProperty* avg_wait_time  = nullptr;
      DoubleProperty* avg_nb_reachable = nullptr;
      DoubleProperty* avg_nb_steps = nullptr;

      if(dataSet!=0){
        dataSet->get("Departures",departures);
        dataSet->get("Arrivals",arrivals);
        dataSet->get("Min time computation",T0);
        dataSet->get("Max time computation",T);
        dataSet->get("Delta time computation",delta);
        dataSet->get("Cost Stopover",cost_stop);
        dataSet->get("Reachable nodes",avg_nb_reachable);
        dataSet->get("Waiting time",avg_wait_time);
        dataSet->get("Stopovers",avg_nb_steps);
      }

      // Check if size departures vectors equals size of arrivals vectors
      for(auto e : graph->getEdges()){
        vector<double> vect_dep = departures->getEdgeValue(e);
        vector<double> vect_arr = arrivals->getEdgeValue(e);
        if(vect_dep.size() != vect_arr.size()){
          pluginProgress->setError("Error: edge "+to_string(e.id)+"'s departure and arrival are not of the same size.");
          return false;
        }
      }

      // Init result property
      result->setAllNodeValue(0.);
      if(avg_wait_time) avg_wait_time->setAllNodeValue(0);
      if(avg_nb_reachable) avg_nb_reachable->setAllNodeValue(0);
      if(avg_nb_steps) avg_nb_steps->setAllNodeValue(0);

      // main loop
      unsigned int loop_node = 0;
      for(auto s : graph->getNodes()){
        pluginProgress->progress(loop_node++,graph->numberOfNodes());
        // Initialization for node s
        vector<double> time_s_change;
        for(auto e : graph->getOutEdges(s)){
          vector<double> dep_e = departures->getEdgeValue(e);
          for(double t : dep_e){
            if(time_s_change.size() == 0 || time_s_change.back() != t){
              time_s_change.push_back(t);
            }
          }
        }
        if(time_s_change.size() == 0)
          continue;

        std::sort(time_s_change.begin(),time_s_change.end());

        // Main loop
        double nb_timeteps = 0.;
        double last_t_comp = T0;
        NodeStaticProperty<double> nodeDuration(graph);
        NodeStaticProperty<double> waitingTime(graph);
        NodeStaticProperty<unsigned int> nbOfSteps(graph);
        TimeDependentDijkstra tvg_dikj(graph,s,departures,arrivals,nodeDuration,waitingTime,nbOfSteps,cost_stop);
        for(double t = T0; t <= T; t += delta){
          nb_timeteps++;
          auto ind_t = lower_bound(time_s_change.begin(),time_s_change.end(),t);
          if(ind_t == time_s_change.end()){
            nodeDuration.setAll(T*graph->numberOfNodes() + 10.);
            nodeDuration.setNodeValue(s,0.);
          }else {
            if(*ind_t > last_t_comp || *ind_t == T0){
              pluginProgress->setComment("Node "+to_string(s.id)+" t= "+to_string((int) last_t_comp));
              last_t_comp = *ind_t;
              tvg_dikj.compute(last_t_comp);
            }
          }

          double nb_reachable_s  = 0.;
          double avg_durations_s = 0.;
          double avg_wait_s      = 0.;
          double avg_nbsteps_s   = 0.;
          for(auto n : graph->getNodes()){
            if(nodeDuration.getNodeValue(n) < T*graph->numberOfNodes()){
              if(n.id != s.id){
                avg_durations_s = avg_durations_s
                                + ((last_t_comp - t) + nodeDuration.getNodeValue(n));
                avg_wait_s += (last_t_comp - t) + waitingTime.getNodeValue(n);
              }
              avg_nbsteps_s += nbOfSteps.getNodeValue(n);
              nb_reachable_s++;
            }
          }
          result->setNodeValue(s,result->getNodeValue(s) + avg_durations_s / nb_reachable_s);
          if(avg_wait_time) avg_wait_time->setNodeValue(s,avg_wait_time->getNodeValue(s) + avg_wait_s / nb_reachable_s);
          if(avg_nb_reachable) avg_nb_reachable->setNodeValue(s,avg_nb_reachable->getNodeValue(s) + nb_reachable_s);
          if(avg_nb_steps) avg_nb_steps->setNodeValue(s,avg_nb_steps->getNodeValue(s) + avg_nbsteps_s / nb_reachable_s);
        }
        result ->setNodeValue(s,result ->getNodeValue(s)   / nb_timeteps);
        if(avg_wait_time) avg_wait_time ->setNodeValue(s,avg_wait_time ->getNodeValue(s)   / nb_timeteps);
        if(avg_nb_reachable) avg_nb_reachable->setNodeValue(s,avg_nb_reachable->getNodeValue(s) / nb_timeteps);
        if(avg_nb_steps) avg_nb_steps->setNodeValue(s,avg_nb_steps->getNodeValue(s) / nb_timeteps);
      }
      return true;
  }
//================================================================================
};

PLUGIN(AverageTravelTime)

#endif //AVERAGETRAVELTIME_H
