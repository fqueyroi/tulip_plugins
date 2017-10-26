#ifndef FLOWBETWEENNESS_H
#define FLOWBETWEENNESS_H

/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux
 *
 * Tulip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Tulip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */

#include <tulip/TulipPluginHeaders.h>

#include <stack>

/** \addtogroup  metric */

/** Flow Betweenness
 *
 *  Warning : ASSUME ONLY ONE SHORTEST-PATH BETWEEN EACH PAIR OF NODES
 *  \note 2015 Version 1.0: Initial release by François Queyroi, Géographie-Cités
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
                      tlp::MutableContainer<std::list<tlp::node> >& ancestors,
//                      tlp::MutableContainer<tlp::node >& ancestor,
                      tlp::DoubleProperty& nb_paths,
                      std::stack<tlp::node> &S);
};


#endif // FLOWBETWEENNESS_H
