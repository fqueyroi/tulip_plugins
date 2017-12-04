![Tulip Software](http://tulip.labri.fr/TulipDrupal/sites/default/files/logo_web.png)

# Graph Algorithms for [Tulip Software](http://tulip.labri.fr/TulipDrupal/)

> Tulip is an information visualization framework dedicated to the analysis and visualization of relational data. Tulip aims to provide the developer with a complete library, supporting the design of interactive information visualization applications for relational data that can be tailored to the problems he or she is addressing.

Find more info [here](http://tulip.labri.fr/TulipDrupal/).

This page contains Tulip plugins using [Tulip API](http://tulip.labri.fr/Documentation/current/doxygen/html/index.html) or [Tulip Python API](http://tulip.labri.fr/Documentation/current/tulip-python/html). They are implementation of well-known (orless know) graph algorithms and network analysis methods.

## Available Plugins

### [Transportation Networks Design](https://github.com/fqueyroi/tulip_plugins/tree/master/TransportationNetworks)

Directory "TransportationNetworks" contains two transportation network construction algorithms. 
- SPRouting: use iterative shortest-path routing of flow
- PhysariumSolver: use biologically inspired routing and edge length update.

The two algorithm only differ in the way flows are routed along the networks. 

### [Label Propagation](https://github.com/fqueyroi/tulip_plugins/tree/master/LabelPropagation)

A simple, fast and efficient graph clustering algorithm. For a weighted graph G=(V,E,w), it produces a *partition* of the vertices V. 

The algorithm iteratively change vertices labels (that correspond to cluster). A vertex will take as label the label that occurs the most in this neighborhood. The algoithm will stop after a sufficient number of iterations.

The method originaly appeared in :

For more details see:

>Usha Nandini Raghavan, Réka Albert, and Soundar Kumara. *Near linear time
algorithm to detect community structures in large-scale networks*. Physical Review
E, 76(3) :036106, 2007.

and 

>Ian XY Leung, Pan Hui, Pietro Lìo, and Jon Crowcroft. *Towards real-time community
detection in large networks.* Physical Review E, 79(6) :066107, 2009.


### [Minimum Spanning Tree](https://github.com/fqueyroi/tulip_plugins/tree/master/MinimumSpanningTree)

Compute the [Minimum Spanning Tree](https://en.wikipedia.org/wiki/Minimum_spanning_tree) of the graph (Python script) using a [Union-Find](https://en.wikipedia.org/wiki/Kruskal%27s_algorithm) data structure. 

### [Vertex Cycle Cover and Secret Santa!](https://github.com/fqueyroi/tulip_plugins/tree/master/VertexCycleCover)

![Christmas!](https://cdn.pixabay.com/photo/2013/07/13/13/58/holly-161840_960_720.png)

[Secret Santa](https://en.wikipedia.org/wiki/Secret_Santa) is a way for people to exchange gifts but where each person only knows who he/she has to give a gift to. Traditionnaly Secret Santa assignments rame done randomly: each participant pick the name of another participant. 

But one can assume that not all assignments are possible. For example, one can want an assignment where parents can be assigned to own children or where colleagues in a company have to be assigned to people working in another department. 

If we assume the the various possibilities are summed up by an graph G (an arc between A->B indicates that "A must offer a gift to B" is a possibility) then the problem is to find a [Vertex Cycle Cover](https://en.wikipedia.org/wiki/Vertex_cycle_cover) of G. This problem can be solved by using a maximum matching algorithm.

However, the Secret Santa has to be ... secret. Therefore, the Tulip Python plugin "SecretSanta" find a solution where the knowledge a given assignment can not be used to infer another assignment. This is an implementation of the algorithm by [Liberti and Raimondi](https://link.springer.com/chapter/10.1007/978-3-540-68880-8_26) (2008).
