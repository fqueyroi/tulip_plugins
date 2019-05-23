#ifndef TIMEDEPENDENTDIKJSTRA_TOOL_H
#define TIMEDEPENDENTDIKJSTRA_TOOL_H

#include <vector>
#include <stack>
#include <list>
#include <climits>
#include <functional>

#include <tulip/Graph.h>
#include <tulip/DoubleProperty.h>
#include <tulip/BooleanProperty.h>
#include <tulip/StaticProperty.h>
#include <tulip/GraphTools.h>


namespace tlp {

//====================================================================
class TimeDependentDijkstra {
public:
  //====================================================================
  TimeDependentDijkstra(Graph * graph,
                        node src,
                        const EdgeStaticProperty<std::vector<double> >* dep,
                        const EdgeStaticProperty<std::vector<double> >* arr,
                        NodeStaticProperty<double> &nD,
                        NodeStaticProperty<double> &wT);
  //====================================================================
  bool compute(const double start = 0.);
  //====================================================================
  bool searchPath(node , std::vector<edge>&, std::vector<int>*);
  bool searchPath(node , BooleanProperty*, DoubleProperty*);
  //====================================================================
private:
  //====================================================================
  struct TimeDikjstraElement {
    TimeDikjstraElement(const double duration = DBL_MAX,
                        const node n = node(),
                        const double w = 0)
        : duration(duration), n(n), wait(w) {}
    bool operator==(const TimeDikjstraElement &b) const {
      return n == b.n;
    }
    bool operator!=(const TimeDikjstraElement &b) const {
      return n != b.n;
    }
    double duration;
    double wait;
    node n;
    std::vector<edge> usedEdge;
    std::vector<unsigned int> index_selectedTime;
  };
  //====================================================================
  struct LessTimeDikjstraElement {
    bool operator()(const TimeDikjstraElement *const a,
                    const TimeDikjstraElement *const b) const {
      if (fabs(a->duration - b->duration) > 1.E-9) {
        return (a->duration < b->duration);
      } else {
        return (a->n.id < b->n.id);
      }
    }
  };
  //====================================================================
  Graph *graph;
  node src;
  const EdgeStaticProperty<std::vector<double> >* departures;
  const EdgeStaticProperty<std::vector<double> >* arrivals;
  NodeStaticProperty<double> &nodeDurations;
  NodeStaticProperty<double> &waitingTime;
  MutableContainer<int> indexSelectedTime;
  //====================================================================
};

}

#endif // TIMEDEPENDENTDIKJSTRA_TOOL_H


