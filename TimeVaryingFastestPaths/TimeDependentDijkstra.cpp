#include "TimeDependentDijkstra.h"

#include <set>
#include <queue>

#include <tulip/BooleanProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/GraphTools.h>

using namespace tlp;
using namespace std;

//============================================================
int index_next_best_dep(const vector<double>& departures,const vector<double>& arrivals,double t){
  auto it_first_dep            = lower_bound(departures.begin(), departures.end(), t);
  if(it_first_dep == departures.end())
    return -1;
  unsigned int index_first_dep = distance(departures.begin(),it_first_dep);
  auto it_best_arr             = min_element(arrivals.begin() + index_first_dep, arrivals.end());
  return distance(arrivals.begin(),it_best_arr);
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

    // Get the time at which we visit node u.n
    double t = start + u.duration;
    for (auto e : graph->getOutEdges(u.n)) {
      node v = graph->opposite(e, u.n);
      TimeDikjstraElement* dEle = mapDik[v];      

      // Compute the duration from src to v using e = (u.n -> v)
      vector<double> arr_e = arrivals->getEdgeValue(e);
      vector<double> dep_e = departures->getEdgeValue(e);
      assert(arr_e.size() == dep_e.size());

      int index_next_arr_e = index_next_best_dep(dep_e, arr_e, t);
      if(index_next_arr_e < 0)
        continue;
      double dur_e  = arr_e[index_next_arr_e] - start;
      double wait_e = dep_e[index_next_arr_e] - t;

      if (fabs(dur_e - dEle->duration) < 1E-9) {
        // path of the same length
        dEle->usedEdge.push_back(e);
        dEle->index_selectedTime.push_back(index_next_arr_e);
        dEle->wait = min(dEle->wait, u.wait + wait_e);
      } else if (dur_e < dEle->duration) {
        // node v is closer with that path
        dEle->usedEdge.clear();
        dEle->index_selectedTime.clear();
        dikjstraTable.erase(dEle);
        dEle->duration = dur_e;
        dEle->usedEdge.push_back(e);
        dEle->index_selectedTime.push_back(index_next_arr_e);
        dEle->wait = u.wait + wait_e;
        dikjstraTable.insert(dEle);
      }
    }
  }

//  BooleanProperty* selec = graph->getProperty<BooleanProperty>("viewSelection2");
//  selec->setAllEdgeValue(false);
  indexSelectedTime.setAll(-1);
  i = 0;
  for (auto n : graph->nodes()) {
    TimeDikjstraElement *dEle = mapDik[i++];
    nodeDurations[n] = dEle->duration;
    waitingTime[n]   = dEle->wait;
    for (unsigned int j = 0; j < dEle->usedEdge.size(); ++j){
//      selec->setEdgeValue(dEle->usedEdge[j],true);
      indexSelectedTime.set(dEle->usedEdge[j].id,dEle->index_selectedTime[j]);
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
      if(selection.getEdgeValue(e) || indexSelectedTime.get(e.id) < 0)
        continue;
      node s = graph->opposite(e,n);
//      if(selection.getNodeValue(s) || nodeDurations[s] > nodeDurations[n])
      if(nodeDurations[s] > nodeDurations[n])
        continue;

      selection.setEdgeValue(e,true);
      path.push_back(e);
      if(index)
        index->push_back(indexSelectedTime.get(e.id));
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
  StringProperty* lab = graph->getProperty<StringProperty>("name");

  bool ok = true;
  while (ok) {
    selection->setNodeValue(n,true);
    cout << " P " << lab->getNodeValue(n) << endl;
    if(n == src)
      break;
    ok = false;
    for(auto e : graph->getInEdges(n)){
      if(selection->getEdgeValue(e) || indexSelectedTime.get(e.id) < 0)
        continue;
      node s = graph->opposite(e,n);
//      if(selection->getNodeValue(s) || nodeDurations[s] > nodeDurations[n])
      if(nodeDurations[s] > nodeDurations[n])
        continue;
      cout << "   from edge " << e.id << " index : " << indexSelectedTime.get(e.id) << " P : " << lab->getNodeValue(s) << endl;
      selection->setEdgeValue(e,true);
      indexes->setEdgeValue(e,indexSelectedTime.get(e.id));
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
                                             const EdgeStaticProperty<vector<double> >* dep,
                                             const EdgeStaticProperty<vector<double> >* arr,
                                             NodeStaticProperty<double> &nD,
                                             NodeStaticProperty<double> &wT)
                                            : graph(graph), src(src), departures(dep), arrivals(arr),
                                            nodeDurations(nD), waitingTime(wT) {

  assert(src.isValid());
  indexSelectedTime.setAll(-1);
  nodeDurations.setAll(DBL_MAX / 2. + 10.);
  waitingTime.setAll(DBL_MAX / 2. + 10.);
}
//============================================================

