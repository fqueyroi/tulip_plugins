#ifndef FLOWBETWEENNESS_H
#define FLOWBETWEENNESS_H

#include <tulip/TulipPluginHeaders.h>

#include <stack>

/** \addtogroup  metric */

/** This plugin is an implementation of the Flow Betweenness measure.
 *  This generalize the betweenness centrality measure to the situation where
 *  shortest-paths between pairs of nodes are variables (given by the user).
 *
 * See:
 *  F. Queyroi, \n
 *  "Biological and Shortest-Path Routing Procedures for Transportation Network Design", \n
 *  "2018", \n
 *  arXiv:1803.03528
 *
 *
 *  <b>HISTORY</b>
 *  - 22/04/19 Version 1.2: Possible to compute for directed networks
 *  - 01/01/17 Version 1.0: Initial release by François Queyroi, Géographie-Cités
 *
 */

class FlowBetweenness : public tlp::DoubleAlgorithm{
    PLUGININFORMATION("Flow Betweenness", "Francois Queyroi","22/04/2015","Alpha","1.0", "Graph")

    public:
        FlowBetweenness(const tlp::PluginContext* context);
    bool run();
    bool check(std::string &);

private:

    void computePaths(tlp::Graph* g,
                      tlp::DoubleProperty* _length,
                      tlp::node src,
                      bool directed,
                      tlp::MutableContainer<std::list<tlp::node> >& ancestors,
                      tlp::DoubleProperty& nb_paths,
                      std::stack<tlp::node> &S);
};


#endif // FLOWBETWEENNESS_H
