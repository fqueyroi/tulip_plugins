#ifndef SPROUTING_H
#define SPROUTING_H

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
/*@{*/
/** \addtogroup  metric */

/** \brief Shortest-Path Routing
 *
 * This plugin computes a transportation network by iteratively routing flows
 * accordings to shortest-path and then updating each edge diameter/length.
 * It requires the plugin "FlowBetweenness"
 *
 * To use this plugin, you need a graph that combine edges of two different types along with three parameters:
 * 1) edges (with parameter is road = False) that represents the flow between the nodes
 * 2) edges (with parameter is road = True) that represents the edges with can qualify has being part of the resulting network.
 *
 * Two properties are therefore mandatory:
 * - the boolean property "is road" that indicates if a given edge has type 1 ou 2.
 * - the metric property "flow" that indicates the amount of flow going between two nodes
 *
 *  \note 2017 Version 1.0: Initial release by François Queyroi, Géographie-Cités
 *
 */
/*@}*/
class SpRouting : public tlp::DoubleAlgorithm{
    PLUGININFORMATION("Shortest-Path Routing", "Francois Queyroi","03/04/2015","Alpha","1.0", "Graph")

    public:
    SpRouting(const tlp::PluginContext* context);
    bool run();
    bool check(std::string &);
};


#endif // SPROUTING_H
