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
# Find a Secret Santa assignment (selection of edges of the graph) if any
# i.e. a vertex disjoint cycle cover (VDCC) such that
# if someone know that one given edge is part of the assignmen, he/she can not
# infer the rest of the assignment.
#
# See 
# Liberti, L., & Raimondi, F. (2008). 
# "The secret santa problem". 
# In International Conference on Algorithmic Applications in Management (pp. 271-279). 
# Springer.
#
####################################################################################
class SecretSanta(tlp.BooleanAlgorithm):
	####################################################################################
	def __init__(self, context):
		tlp.BooleanAlgorithm.__init__(self, context);
		self.addBooleanParameter("oriented", "Should edge orientation be taken into account?", "false")
		self.oriented=False;
	####################################################################################
	def check(self):
		return (True,"");
	####################################################################################
	# Transform the graph G 	by duplicating vertices according to in/out edges
	# in order to solve find a Vertex disjoint cycle cover (VDCC) of G
	# via bipartite maximum matching computation. 
	# (Adjacencies are given a random ordering)
	# Input:
	#   G: the graph to transform
	#   forced_arc: arc that is assume to be part of the VDCC of G
	# Returns: 
  # 	bipart_g: The transformed graph
	#		map_oends: map transformed graph arcs to original arcs
	#		s: dummy source node for maximum flow computation
	#		t: dummy sink node for maximum flow computation
	####################################################################################
	def bipartite_transformation(self,G,forced_arc=tlp.edge()) :		
		map_in_nodes={} ## vertices in self.graph -> corresponding duplicated vertices
		map_out_nodes={} ## vertices in self.graph -> corresponding duplicated vertices
		map_oends={}; ## arcs in bipartite graph -> edge in G
		
		edges=[];
		for e in G.getEdges() :
			if e!=forced_arc :
				edges.append(e);  # forced_arc is assumed to be part of the assignment
		random.shuffle(edges);	# shuffling arcs
		
		bipart_g = tlp.newGraph();
		## duplicate vertices 
		for e in edges :
			n1,n2 = G.ends(e);
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
	# Check wether G as a Santa assignment 
	# i.e. if the size of the maximum vertex disjoint cycle cover (VDCC) == number of nodes of G
	# Input:
	#   G: the graph to transform
	#   forced_arc: arc that is assume to be part of the VDCC of G
	# Returns: 
  # 	Wether G (assuming forced_arc is part of the assignment) has a santa assignment or not.
	####################################################################################
	def has_assignment(self,G,res,forced_arc=tlp.edge()) :
		## get augmented bipartite graph
		[bipart_g,map_oends,s,t]=self.bipartite_transformation(G,forced_arc)

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
		nb_assignment=0.;
		for e in bipart_g.getEdges() :
			if flow[e]>0 :
				if e not in map_oends.keys() :
					print "Error: unknown edge in maximum matching."
					return False;
				res[map_oends[e]]=True;
				nb_assignment+=1;
		return nb_assignment==G.numberOfNodes();
	####################################################################################
	def run(self):
		## get parameters
		self.oriented=self.dataSet["oriented"];
		## copy the graph
		copy=self.graph.addCloneSubGraph("copy");
		G=self.graph.addCloneSubGraph("G");
		## If oriented, add reverse arcs to each connected pairs of nodes
		if not self.oriented :
			for e in G.getEdges() :
				u,v=G.ends(e);
				G.addEdge(v,u);
				
		base_number_of_arcs=G.numberOfEdges();
		result_found=False;
		## while there is enough arcs left for a santa assignment to be possible
		while not result_found and ((base_number_of_arcs - G.numberOfEdges()) <= base_number_of_arcs - G.numberOfNodes()) :
			self.pluginProgress.progress(base_number_of_arcs - G.numberOfEdges(), base_number_of_arcs - G.numberOfNodes());
			## check if an santa assignment is possible
			self.result.setAllEdgeValue(False);
			assignement_found = self.has_assignment(G,self.result);
			if not assignement_found :
				result_found=False;
				break;
			else :
				## chech if the assignment found is secret
				result_found=True
				for e1 in self.result.getEdgesEqualTo(True) :				
					s1,t1 = G.ends(e1);
					print "   Test edge ",e1," : ",s1,t1
					keep_e1=True;
					## assuming we know edge e1 is part of the assignment, is it possible to know for sure that edge e2 is also?
					for e2 in self.result.getEdgesEqualTo(True) :
						if (e1!=e2) :
							## copy the graph
							cg=G.addCloneSubGraph();
							cg_assign=cg.getBooleanProperty("cg_assign");
							## remove edge e2 (we assume e2 is not part of the solution)
							cg.delEdge(e2);						
							## Test if an assignment is still possible with these constraints
							assignement_secret = self.has_assignment(cg,cg_assign,e1);
							G.delAllSubGraphs(cg);
							if not assignement_secret :
								keep_e1=False;
								break;
					## if the assignment is not secret (due to e1) we remove e1 and look for a new assignment
					if not keep_e1 :
						result_found=False;
						G.delEdge(e1);
						break;
		
		## get the results
		if not self.oriented :
			for e in self.graph.getEdges() :
				if not copy.isElement(e) :
					if self.result[e] :
						s,t=self.graph.ends(e);
						oe=self.graph.existEdge(t,s,True);
						self.result[oe]=True;
					self.graph.delEdge(e);
		self.graph.delAllSubGraphs(G);
		self.graph.delAllSubGraphs(copy);
		if not result_found :
			self.pluginProgress.setError("Graph "+self.graph.getName()+" has no Secret Santa Assignment !");
			self.result.setAllEdgeValue(False);
		return result_found;
####################################################################################
# The line below does the magic to register the plugin into the plugin database
# and updates the GUI to make it accessible through the menus.
tulipplugins.registerPlugin("SecretSanta", "Secret Santa", "Francois Queyroi", "30/11/2017", "Find a Secret Santa Assignment", "1.0")
