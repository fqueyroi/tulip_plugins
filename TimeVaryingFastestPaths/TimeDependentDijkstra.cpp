#include "TimeDependentDijkstra.h"

#include <set>
#include <queue>
#include <utility>
#include <algorithm>
#include <numeric>


#include <tulip/BooleanProperty.h>
#include <tulip/GraphTools.h>

using namespace tlp;
using namespace std;
//================================================================================
//template <typename T>
//vector<size_t> sort_indexes(const vector<T> &v) {

//  // initialize original index locations
//  vector<size_t> idx(v.size());
//  iota(idx.begin(), idx.end(), 0);

//  // sort indexes based on comparing values in v
//  sort(idx.begin(), idx.end(),
//       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

//  return idx;
//}
//============================================================
int index_next_best_dep(const vector<double>& departures,const vector<double>& arrivals,double t){
//  auto it_first_dep            = lower_bound(departures.begin(), departures.end(), t);
//  if(it_first_dep == departures.end())
//    return -1;
//  unsigned int index_first_dep = distance(departures.begin(),it_first_dep);
//  auto it_best_arr             = min_element(arrivals.begin() + index_first_dep, arrivals.end());
//  return distance(arrivals.begin(),it_best_arr);
  int index_earliest_pos_arr = -1;
  for(unsigned int i = 0; i < departures.size(); ++i){
    if(departures[i] < t)
      continue;
    if(index_earliest_pos_arr < 0 || arrivals[index_earliest_pos_arr] > arrivals[i])
      index_earliest_pos_arr = i;
  }
  return index_earliest_pos_arr;
}

//============================================================
bool TimeDependentDijkstra::compute(const double start){
//init variables
  set<TimeDikjstraElement *, LessTimeDikjstraElement> dikjstraTable;
  NodeStaticProperty<TimeDikjstraElement *> mapDik(this->graph);
  mapDik.setAll(nullptr);

  unsigned int i = 0;
  for (auto n : graph->nodes()) {
    TimeDikjstraElement *tmp;
    if (n != src)
      // init all nodes to +inf
      tmp = new TimeDikjstraElement(DBL_MAX / 2. + 10., n, DBL_MAX / 2. + 10.);
    else
      // init starting node to 0
      tmp = new TimeDikjstraElement(0, n, 0);
    dikjstraTable.insert(tmp);
    mapDik[i++] = tmp;
  }

  // main loop
  while (!dikjstraTable.empty()) {
    // select the first element in the list the one with min value
    set<TimeDikjstraElement *, LessTimeDikjstraElement>::iterator it = dikjstraTable.begin();
    TimeDikjstraElement &u = *(*it);
    dikjstraTable.erase(it);
    // Get the time at which we can leave u.n
    double t = start + u.duration + cost_stopover;

    for (auto e : graph->getOutEdges(u.n)) {
      node v = graph->opposite(e, u.n);
      TimeDikjstraElement* dEle = mapDik[v];      

      // Compute the duration from src to v using e = (u.n -> v)
      vector<double> arr_e = arrivals[e];
      vector<double> dep_e = departures[e];
      assert(arr_e.size() == dep_e.size());

      int index_next_arr_e = index_next_best_dep(dep_e, arr_e, t);
      if(index_next_arr_e < 0)
        continue;
      double dur_e  = arr_e[index_next_arr_e] - start;
      double wait_e = dep_e[index_next_arr_e] - (start + u.duration);

      if (fabs(dur_e - dEle->duration) < 1E-9) {
        // path of the same length
        dEle->usedEdge.push_back(e);
        dEle->index_selectedTime.push_back(index_next_arr_e);
        dEle->wait = min(dEle->wait, u.wait + wait_e);
        dEle->nbSteps = min(dEle->nbSteps, u.nbSteps + 1);
      } else if (dur_e < dEle->duration) {
        // node v is closer with that path
        dEle->usedEdge.clear();
        dEle->index_selectedTime.clear();
        dikjstraTable.erase(dEle);
        dEle->duration = dur_e;
        dEle->usedEdge.push_back(e);
        dEle->index_selectedTime.push_back(index_next_arr_e);
        dEle->wait = u.wait + wait_e;
        dEle->nbSteps = u.nbSteps + 1;
        dikjstraTable.insert(dEle);
      }
    }
  }

  indexSelectedTime.setAll(-1);
  i = 0;
  for (auto n : graph->nodes()) {
    TimeDikjstraElement *dEle = mapDik[i++];
    nodeDurations[n] = dEle->duration;
    waitingTime[n]   = dEle->wait;
    numberOfSteps[n] = dEle->nbSteps;
    for (unsigned int j = 0; j < dEle->usedEdge.size(); ++j){
      indexSelectedTime.setEdgeValue(dEle->usedEdge[j],dEle->index_selectedTime[j]);
    }
    delete dEle;
  }
  return true;
}
//=============================================================================
bool TimeDependentDijkstra::searchPath(node n, vector<edge>& path, vector<int>* index = nullptr) {
  if(index)
    index->clear();
  bool ok = true;
  BooleanProperty selection(graph);
  selection.setAllNodeValue(false);
  selection.setAllEdgeValue(false);
  while (ok) {
    selection.setNodeValue(n,true);
    if(n == src)
      break;
    ok = false;
    for(auto e : graph->getInEdges(n)){
      if(selection.getEdgeValue(e) || indexSelectedTime[e] < 0)
        continue;
      node s = graph->opposite(e,n);
      if(nodeDurations[s] > nodeDurations[n])
        continue;

      selection.setEdgeValue(e,true);
      path.push_back(e);
      if(index)
        index->push_back(indexSelectedTime[e]);
      n = s;
      ok = true;
      break;
    }    
  }
  if (n != src) {
    path.clear();
    if(index)
      index->clear();
    return false;
  }
  std::reverse(path.begin(),path.end());
  if(index)
    std::reverse(index->begin(),index->end());
  return true;
}
//=============================================================================
bool TimeDependentDijkstra::searchPath(node n, BooleanProperty* selection, DoubleProperty* indexes){
  indexes->setAllNodeValue(0);
  selection->setAllNodeValue(false);
  selection->setAllEdgeValue(false);

  bool ok = true;
  while (ok) {
    selection->setNodeValue(n,true);
    if(n == src)
      break;
    ok = false;
    for(auto e : graph->getInEdges(n)){
      if(selection->getEdgeValue(e) || indexSelectedTime[e] < 0)
        continue;
      node s = graph->opposite(e,n);
      if(nodeDurations[s] > nodeDurations[n])
        continue;
      selection->setEdgeValue(e,true);
      indexes->setEdgeValue(e,indexSelectedTime[e]);
      n = s;
      ok = true;
      break;
    }
  }
  if (n != src)
    return false;
  return true;
}
//============================================================
TimeDependentDijkstra::TimeDependentDijkstra(Graph * graph,
                                             node src,
                                             const EdgeStaticProperty<vector<double> > &dep,
                                             const EdgeStaticProperty<vector<double> > &arr,
                                             NodeStaticProperty<double> &nD,
                                             NodeStaticProperty<double> &wT,
                                             NodeStaticProperty<unsigned int> &nS,
                                             double cs)
                                            : graph(graph), src(src), departures(graph), arrivals(graph),
                                            nodeDurations(nD), waitingTime(wT), numberOfSteps(nS),
                                            cost_stopover(cs), indexSelectedTime(graph)
                                            {


  departures.copyFromProperty(&dep);
  arrivals.copyFromProperty(&arr);
  assert(src.sValid());
  indexSelectedTime.setAll(-1);
  nodeDurations.setAll(DBL_MAX / 2. + 10.);
  waitingTime.setAll(DBL_MAX / 2. + 10.);
  numberOfSteps.setAll(0);
}
//============================================================
TimeDependentDijkstra::TimeDependentDijkstra(Graph *graph,
                                             node src,
                                             const DoubleVectorProperty *dep,
                                             const DoubleVectorProperty *arr,
                                             NodeStaticProperty<double> &nD,
                                             NodeStaticProperty<double> &wT,
                                             NodeStaticProperty<unsigned int> &nS,
                                             double cs)
                                             : graph(graph), src(src),
                                             departures(graph), arrivals(graph),
                                             nodeDurations(nD), waitingTime(wT), numberOfSteps(nS),
                                             cost_stopover(cs), indexSelectedTime(graph)
                                             {
  assert(src.isValid());
  indexSelectedTime.setAll(-1);
  nodeDurations.setAll(DBL_MAX / 2. + 10.);
  waitingTime.setAll(DBL_MAX / 2. + 10.);
  numberOfSteps.setAll(0);

  // Sort arrival and departure times according to departure time
  for(auto e : graph->getEdges()){
    vector<double> vect_dep = dep->getEdgeValue(e);
    vector<double> vect_arr = arr->getEdgeValue(e);
//    assert(vect_dep.size() == vect_arr.size());
//    vector<double> sorted_vect_dep;
//    vector<double> sorted_vect_arr;
//    for (auto i : sort_indexes(vect_dep)){
//      sorted_vect_dep.push_back(vect_dep[i]);
//      sorted_vect_arr.push_back(vect_arr[i]);
//    }
//    departures.setEdgeValue(e,sorted_vect_dep);
//    arrivals.setEdgeValue(e,sorted_vect_arr);
      departures.setEdgeValue(e,vect_dep);
      arrivals.setEdgeValue(e,vect_arr);
  }
}
