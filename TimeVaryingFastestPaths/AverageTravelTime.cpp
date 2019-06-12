#include "AverageTravelTime.h"

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
        "Minimum T value for average computation.",
        // max time
        "Maximum T value for average computation.",
        // delta
        "Delta T value for average computation.",
        // Cost stopover
        "The cost in time of stoping at a non-target vertex."
    };
}
//================================================================================
AverageTravelTime::AverageTravelTime(PluginContext *c) : Algorithm(c){
    addInParameter<DoubleVectorProperty>("Departures",paramHelp[0],"departures",true);
    addInParameter<DoubleVectorProperty>("Arrivals",paramHelp[1],"arrivals",true);
    addInParameter<double>("Min time computation",paramHelp[2],"14335",true);
    addInParameter<double>("Max time computation",paramHelp[3],"14455",true);
    addInParameter<double>("Delta time computation",paramHelp[4],"1",true);
    addInParameter<double>("Cost Stopover",paramHelp[5],"0",true);
}
//================================================================================
bool AverageTravelTime::run(){

    // Get Parameters
    DoubleVectorProperty* departures;
    DoubleVectorProperty* arrivals;
    double T0 = 14335;
    double T  = 14455;
    double delta = 1.;
    double cost_stop = 0.;

    if(dataSet!=0){
      dataSet->get("Departures",departures);
      dataSet->get("Arrivals",arrivals);
      dataSet->get("Min time computation",T0);
      dataSet->get("Max time computation",T);
      dataSet->get("Delta time computation",delta);
      dataSet->get("Cost Stopover",cost_stop);
    }

    // Check if size departures vector equals size of arrvials vector
    for(auto e : graph->getEdges()){
      vector<double> vect_dep = departures->getEdgeValue(e);
      vector<double> vect_arr = arrivals->getEdgeValue(e);
      if(vect_dep.size() != vect_arr.size()){
        pluginProgress->setError("Error: edge "+to_string(e.id)+"'s departure and arrival are not of the same size.");
        return false;
      }
    }

    // Init result property
    DoubleProperty* avg_travel_time  = graph->getProperty<DoubleProperty>("avg_travel_time");
    DoubleProperty* avg_wait_time  = graph->getProperty<DoubleProperty>("avg_wait_time");
    DoubleProperty* avg_nb_reachable = graph->getProperty<DoubleProperty>("avg_nb_reachable");
    DoubleProperty* avg_nb_steps = graph->getProperty<DoubleProperty>("avg_nb_steps");
    avg_travel_time->setAllNodeValue(0);
    avg_wait_time->setAllNodeValue(0);
    avg_nb_reachable->setAllNodeValue(0);
    avg_nb_steps->setAllNodeValue(0);

    unsigned int loop_node = 0;
    for(auto s : graph->getNodes()){
      pluginProgress->progress(loop_node++,graph->numberOfNodes());
      // Initialization for node s
//        cout << " Time stamps : ";
      vector<double> time_s_change;
      for(auto e : graph->getOutEdges(s)){
        vector<double> dep_e = departures->getEdgeValue(e);
        for(double t : dep_e){
          if(time_s_change.size() == 0 || time_s_change.back() != t){
            time_s_change.push_back(t);
//              cout << t << " ";
          }
        }
      }
//        cout << endl;

      if(time_s_change.size() == 0){
        cout << "   Node " << s.id << " no connections." << endl;
        continue;
      }

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
        avg_travel_time->setNodeValue(s,avg_travel_time->getNodeValue(s) + avg_durations_s / nb_reachable_s);
        avg_wait_time->setNodeValue(s,avg_wait_time->getNodeValue(s) + avg_wait_s / nb_reachable_s);
        avg_nb_reachable->setNodeValue(s,avg_nb_reachable->getNodeValue(s) + nb_reachable_s);
        avg_nb_steps->setNodeValue(s,avg_nb_steps->getNodeValue(s) + avg_nbsteps_s / nb_reachable_s);
      }
      avg_travel_time ->setNodeValue(s,avg_travel_time ->getNodeValue(s)   / nb_timeteps);
      avg_wait_time ->setNodeValue(s,avg_wait_time ->getNodeValue(s)   / nb_timeteps);
      avg_nb_reachable->setNodeValue(s,avg_nb_reachable->getNodeValue(s) / nb_timeteps);
      avg_nb_steps->setNodeValue(s,avg_nb_steps->getNodeValue(s) / nb_timeteps);
    }

    return true;
}
//================================================================================
PLUGIN(AverageTravelTime)

