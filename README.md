![Tulip Software](http://tulip.labri.fr/TulipDrupal/sites/default/files/logo_web.png)

# Graph Algorithms for [Tulip Software](http://tulip.labri.fr/TulipDrupal/)

> Tulip is an information visualization framework dedicated to the analysis and visualization of relational data. Tulip aims to provide the developer with a complete library, supporting the design of interactive information visualization applications for relational data that can be tailored to the problems he or she is addressing.

Find more info [here](http://tulip.labri.fr/TulipDrupal/).

This page contains Tulip plugins using [Tulip API](http://tulip.labri.fr/Documentation/current/doxygen/html/index.html) or [Tulip Python API](http://tulip.labri.fr/Documentation/current/tulip-python/html). They are implementation of well-known (orless know) graph algorithms and network analysis methods.

## Available Plugins


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



