# When the plugin development is finished, you can copy the associated Python file 
# to /PATH/TO/.Tulip-5.2/plugins/python
# or /PATH/TO/TULIP/INSTALL/lib/tulip/python/
# and it will be automatically loaded at Tulip startup
#######################################################################################################
from tulip import tlp
import tulipplugins
#######################################################################################################
import priority_dict as pd
#######################################################################################################
class BetweennessGravityModel(tlp.DoubleAlgorithm):
	#######################################################################################################
	def __init__(self, context):
		tlp.DoubleAlgorithm.__init__(self, context)
		self.addStringCollectionParameter("type",
		                                  "This parameter indicates the direction used to compute the flow betweenness",
		                                  "InOut;In;Out")
		self.addNumericPropertyParameter("weight", "Nodes weight", "", False)
		self.addNumericPropertyParameter("cost", "Edges length", "", False)
		self.addLayoutPropertyParameter("layout", "Nodes position", "", False)
		self.addFloatParameter("average path length",
		                       "The sum of flow betweenness for edges (average path length with the gravity model)", "-1",
		                       False, False, True)
		self.addFloatParameter("total flow",
													"The total amount of flow in the network", "-1",
													False, False, True)
		self.weight = None
		self.cost = None
		self.position = None
	
	#######################################################################################################
	def check(self):
		return (True, "")
	
	#######################################################################################################
	def flow_amount(self, u, v):
		if u.id == v.id :
			return 0.
		d = 1.
		if self.position is not None:
			d = self.position[u].dist(self.position[v])
		w_u, w_v = 1., 1.
		if self.weight is not None:
			w_u, w_v = self.weight[u], self.weight[v]
		return w_u * w_v / (d * d)

	#######################################################################################################
	def shortestPaths(self, src, it_edges):
		distances = {}
		queue = pd.priority_dict({})
		queue[src.id] = 0
		## priority_dict requires __lt__ operator
		## which is undefined for tlp.node
		ancestors = {src : []}
		stack = []
		nb_paths = tlp.DoubleProperty(self.graph)
		nb_paths.setAllNodeValue(1)
		nb_paths[src] = 1

		while len(queue) > 0:
			v_id = queue.smallest()
			d = queue[v_id]
			v = tlp.node(v_id)
			queue.pop_smallest()
			
			stack.append(v)
			distances[v] = d
			for e in it_edges(v):
				val = 1.
				if self.cost is not None:
					val = self.cost[e]
				w = self.graph.opposite(e, v)
				alt = d + val
				if w in distances:
					if alt < distances[w]:
						self.pluginProgress.setError("Dijkstra: found better path to already-final vertex")
				else:
					if (w.id not in queue) or (alt < queue[w.id]):
						queue[w.id] = alt
						ancestors[w] = [v]
						nb_paths[w] = nb_paths[v]
					elif (alt == queue[w.id]) :
						ancestors[w].append(v)
						nb_paths[w] = nb_paths[w] + nb_paths[v]
		return ancestors, stack, nb_paths


#######################################################################################################
	def run(self):
		self.weight = self.dataSet["weight"]
		self.cost = self.dataSet["cost"]
		self.position = self.dataSet["layout"]
	
		iterators_edges = {"InOut": self.graph.getInOutEdges, "In": self.graph.getInEdges, "Out": self.graph.getOutEdges}
		chossen_iterator = iterators_edges[self.dataSet["type"]]
	
		self.result.setAllNodeValue(0.)
		self.result.setAllEdgeValue(0.)
	
		avg_path_length = 0.
		total_flow      = 0.
		nb_nodes_done   = 0
		delta = tlp.DoubleProperty(self.graph)
		for src in self.graph.getNodes():
			self.pluginProgress.progress(nb_nodes_done,self.graph.numberOfNodes())
			nb_nodes_done += 1

			delta.setAllNodeValue(0.)
			ancestors, stack, nb_paths = self.shortestPaths(src, chossen_iterator)
			
			while len(stack) > 0:
				w = stack.pop()
				flow_sw = self.flow_amount(src, w)/2.
				# flow divided by two since flows are undirected
				total_flow += flow_sw
				if w != src :
					self.result[w] += delta[w]
				for v in ancestors[w] :
					inc_delta = (nb_paths[v]/nb_paths[w])*(flow_sw + delta[w])
					delta[v] += inc_delta
					e = self.graph.existEdge(v, w, False)
					if self.dataSet["type"] == "Out":
						e = self.graph.existEdge(v, w, True)
					if self.dataSet["type"] == "In":
						e = self.graph.existEdge(w, v, True)
					self.result[e] += inc_delta
					
					val = 1.
					if self.cost is not None:
						val = self.cost[e]
					avg_path_length += val * inc_delta
	
		self.dataSet["average path length"] = avg_path_length
		self.dataSet["total flow"] = total_flow
		return True
#########################################################################################################
tulipplugins.registerPlugin("BetweennessGravityModel", "Betweenness Gravity Model", "Francois Queyroi", "02/04/2019", "", "1.0")
