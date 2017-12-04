# When the plugin development is finished, you can copy the associated Python file 
# to /PATH/TO/.Tulip-5.0/plugins/python
# or /PATH/TO/lib/tulip/python/
# and it will be automatically loaded at Tulip startup

from tulip import tlp
import tulipplugins
######################################################################
import unionfind
######################################################################
# Compute the Minimum Spanning Tree of the graph
# using Kruskal algorithm
# see: https://en.wikipedia.org/wiki/Kruskal%27s_algorithm
######################################################################
class MinimumSpanningTree(tlp.BooleanAlgorithm):
  ######################################################################
  def __init__(self, context):
    tlp.BooleanAlgorithm.__init__(self, context)
    self.addDoublePropertyParameter("weight", "Edge weights used", "",True);
  ######################################################################
  def check(self):
    return (True, "")
  ######################################################################
  def run(self):
    # get parameters
    weight=self.dataSet["weight"];
    # init result
    self.result.setAllEdgeValue(False);
    	# sort edge in increasing order of weight
    edges=[];
    for e in self.graph.getEdges() :
      edges.append([e,weight.getEdgeMax()-weight[e]]);
    edges.sort(key=lambda ed: ed[1]);
    	# init Union-Find data structure
    uf=unionfind.UnionFind();
	
    for ed in edges :
      e=ed[0];
      if uf[self.graph.source(e)]!=uf[self.graph.target(e)] :
        uf.union(self.graph.source(e),self.graph.target(e));
        self.result[e]=True;
    return True
  ######################################################################
# The line below does the magic to register the plugin into the plugin database
# and updates the GUI to make it accessible through the menus.
tulipplugins.registerPluginOfGroup("MinimumSpanningTree", "Minimum Spanning Tree", "Francois Queyroi", "04/12/2017", "Compute a Minimum Spanning Tree", "1.0", "selection")
