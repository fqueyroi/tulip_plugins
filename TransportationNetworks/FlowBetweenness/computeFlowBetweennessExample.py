from tulip import tlp

""" Example of a computation of the Flow betweenness of a graph
using a gravity model of flow i.e.
the weight of a pairs of nodes (u,v) is not 1 (as in the betweenness centrality)
but W[u] * W[v] / dist(u,v) ^ 2
"""
def main(graph):
  # Nodes weight and position
  weight   = graph.getDoubleProperty("weight")
  position = graph.getLayoutProperty("viewLayout");
  # Edges length
  length   = graph.getDoubleProperty("length")

  # Create temp graph properties
  is_road = tlp.BooleanProperty(graph);
  is_road.setAllEdgeValue(True)
  flow_val = tlp.DoubleProperty(graph);

  # Compute the flow between nodes pairs
  for n1 in graph.getNodes() :
    for n2 in graph.getNodes() :
      if n1.id < n2.id :        
        d_12 = position[n1].dist(position[n2])
        if d_12 > 0 :
            e = graph.addEdge(n1,n2);
            is_road[e] = False;
            flow_val[e] = weight[n1] * weight[n2] / (d_12 * d_12);

  # Compute Flow Betweenness
  ds = tlp.getDefaultPluginParameters("Flow Betweenness")
  ds["is road"] = is_road
  ds["flow value"] = flow_val
  ds["length"] = length
  fbc = graph.getDoubleProperty("fbc");
  graph.applyDoubleAlgorithm("Flow Betweenness",fbc,ds);
  
  # Clean up the graph
  for e in graph.getEdges() :
    if not is_road[e] :
      graph.delEdge(e);
 
  # Output Average path length and total flow values
  print("Average path length: ",ds["Average path length"]);
  print("Total flow: ",ds["Total flow"])

