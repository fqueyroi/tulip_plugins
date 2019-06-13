# Networks from Sequential Data

This folder contains code and data for 


Archive `llyods_ships_moves2009.zip` contains a sample of Llyod's Intelligence database of ships movement and ports info for the year 2009.
See the webpage of [the WorldSeastems ERC project](http://www.world-seastems.cnrs.fr/) for more details.

## Details on computations

- The networks can be generated using the [Tulip-python Import Module](http://tulip.labri.fr/Documentation/current/tulip-python/html/tulipreference.html?highlight=importmodule#tulip.tlp.ImportModule) ["TVG Llyods Import"](https://github.com/fqueyroi/tulip_plugins/blob/master/papers/NetFromSequentialData/TVGLlyodsImport.py) with files found in file `llyods_ships_moves2009.zip` using defaults parameters. Note that the nodes are filtered out in order to have a network where the frequency of interactions (sending or receiving ships) correspond to a amount of at least one ship every week over the period. 

- The [result folder](https://github.com/fqueyroi/tulip_plugins/tree/master/papers/NetFromSequentialData/results) contains the results computed for three types of sequential data aggregation (Space-L, Space-P and Pathways). Some values such as the time-dependent fastest travel durations (`travel_time`) can only be found in file Space-L (the value is the same for the two other type of aggregation). 

- The travel durations are computed with no cost of stopover and a cost of 2 days (variables with suffix `"*_c0"` and `"*_c2"` respectively). The average of travel durations is taken between 20 days after the start of the period (*i.e.* 04/01/2009) and 20 days before the end (*i.e.* 07/31/2009) in order to avoid border effects.

- The travel durations are computed using the plugin ["Average Travel Time"](https://github.com/fqueyroi/tulip_plugins/tree/master/TimeVaryingFastestPaths) while the Closeness centralities (average path length for each node) are computed using Tulip plugin "Eccentricity". 

