# When the plugin development is finished, you can copy the Python files 
# to /PATH/TO/.Tulip-5.0/plugins/python
# or /PATH/TO/lib/tulip/python/
# and it will be automatically loaded at Tulip startup

from tulip import tlp
import tulipplugins
####################################################################################
import random
####################################################################################
import maximumFlow
####################################################################################
# Find a vertex disjoint cycle cover (VDCC) of G (if any)
# using maximum matching 
#
# See 
# Tutte, W. T. (1954). A short proof of the factor theorem for finite graphs. Canad. J. Math, 6(1954), 347-352.
#
####################################################################################
class VertexCycleCover(tlp.BooleanAlgorithm):
	####################################################################################
	def __init__(self, context):
		tlp.BooleanAlgorithm.__init__(self, context);
		self.addBooleanParameter("oriented", "Should edge orientation be taken into account", "false")
		self.oriented=False;
	####################################################################################
	def check(self):
		return (True, "");
	####################################################################################
	# Transform the graph self.graph	by duplicating vertices according to in/out edges
	# in order to solve find a Vertex disjoint cycle cover (VDCC) of self.graph
	# via bipartite maximum matching computation. 
	# (Adjacencies are given a random ordering)
	# Returns: 
  # 	bipart_g: The transformed graph
	#		map_oends: map transformed graph arcs to original arcs
	#		s: dummy source node for maximum flow computation
	#		t: dummy sink node for maximum flow computation
	####################################################################################
	def bipartite_transformation(self) :		
		map_in_nodes={} ## vertices in self.graph -> corresponding duplicated vertices
		map_out_nodes={} ## vertices in self.graph -> corresponding duplicated vertices
		map_oends={}; ## arcs in bipartite graph -> edge in self.graph
		
		edges=[];
		for e in self.graph.getEdges() :
				edges.append(e);
		random.shuffle(edges);	# shuffling arcs
		
		bipart_g = tlp.newGraph();
		## duplicate vertices 
		for e in edges :
			n1,n2 = self.graph.ends(e);
			if n1 not in map_out_nodes.keys() :
				n_out = bipart_g.addNode();
				map_out_nodes[n1]=n_out;
			if n2 not in map_in_nodes.keys() :
				n_in = bipart_g.addNode();
				map_in_nodes[n2]=n_in;
			a=bipart_g.addEdge(map_out_nodes[n1],map_in_nodes[n2])
			map_oends[a]=e;		
	
		## connected the new nodes to source and sink nodes
		s= bipart_g.addNode();
		for n in map_out_nodes.values() :
			bipart_g.addEdge(s,n);
		t= bipart_g.addNode();
		for n in map_in_nodes.values() :
			bipart_g.addEdge(n,t);
		return [bipart_g,map_oends,s,t]
	####################################################################################
	def run(self):
		self.oriented=self.dataSet["oriented"];
		## get augmented bipartite graph
		[bipart_g,map_oends,s,t]=self.bipartite_transformation()

		## add edge in opposite direction and set edge capacities
		capacity=bipart_g.getDoubleProperty("capacity");
		capacity.setAllEdgeValue(0.);
		for e in bipart_g.getEdges() :
			eo=bipart_g.addEdge(bipart_g.target(e),bipart_g.source(e));
			capacity[e]=1.;
			
		## init flow
		flow=bipart_g.getDoubleProperty("flow");
		flow.setAllEdgeValue(0.);
		
		## compute the maximum matching in the bipartite graph using a maximum flow algorithm
		maximumFlow.getMaxFlow(bipart_g,s,t,capacity,flow);
		
		## get the results
		bipart_g.delNode(s);
		bipart_g.delNode(t);
		for e in bipart_g.getEdges() :
			if flow[e]>0 :
				if e not in map_oends.keys() :
					print "Error: unknown edge in maximum matching."
					return False;
				self.result[map_oends[e]]=True;
#		for n in bipart_g.getNodes() :
#			self.graph.delNode(n);
#		self.graph.delAllSubGraphs(bipart_g);
		return True
	####################################################################################
# The line below does the magic to register the plugin into the plugin database
# and updates the GUI to make it accessible through the menus.
tulipplugins.registerPlugin("VertexCycleCover", "Vertex-Disjoint Cycle Cover", "Francois Queyroi", "30/11/2017", "Find a Vertex-Disjoint Cycle Cover", "1.0")
