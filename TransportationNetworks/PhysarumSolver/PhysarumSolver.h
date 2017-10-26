#ifndef PHYSARUMSOLVER_H
#define PHYSARUMSOLVER_H

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

/** \addtogroup  metric */
/*@{*/
/** \brief An implementation of transportation network construction using a
 *   Physarum solver.
 *
 * This plugin is an implementation of Tero et al. (2007) procedure (see ref below). It uses
 * the method described by Kelner et al. (2013) to solve linear systems in nearly-linear time.
 * Note however this last method is an approximation (set to 0.05 here).
 *
 * To use this plugin, you need a graph that combine edges of two different types along with three parameters:
 * 1) edges (with parameter is road = False) that represents the flow between the nodes
 * 2) edges (with parameter is road = True) that represents the edges with can qualify has being part of the resulting network.
 *
 * Two properties are therefore mandatory:
 * - the boolean property "is road" that indicates if a given edge has type 1 ou 2.
 * - the metric property "flow" that indicates the amount of flow going between two nodes
 *
 *
 * Authors: A Tero, R. Kobayashi and T. Nakagaki
 * Title:  A mathematical model for adaptive transport network in path finding by true slime mold.
 * Pulbished in: Journal of Theoretical Biology 244
 * Year: 2007
 * Pages: 533-564
 *
 * WARNING! Using a small precision (epsilon parameter) or a small decay rate considerably slow the convergence of the algorithm.
 *
 *  \note 2017 Version 1.0: Initial release by François Queyroi, Géographie-Cités
 *
 */
/*@}*/
class PhysarumSolver : public tlp::DoubleAlgorithm {
    PLUGININFORMATION("Physarum Solver",
                      "Francois Queyroi",
                      "05/09/2017",
                      "Transportation network construction mimicking natural process<br/>"
                      "First designated by Tero et al. (2007), it assigns a value [0,1] to each edge.This value correspond to the diameter/speed of the route in the computed network.",
                      "1.0",
                      "Graph")
    public:
        PhysarumSolver(const tlp::PluginContext*);
        bool run();
        bool check(std::string &);
};
#endif //PHYSARUMSOLVER_H
