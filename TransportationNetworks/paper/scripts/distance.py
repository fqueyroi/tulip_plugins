from tulip import tlp

def distances(G,length,src):
  visited = tlp.BooleanProperty(G)
  visited.setAllNodeValue(False);
  dist = tlp.DoubleProperty(G);
  dist.setAllNodeValue(10e+20);
  dist[src]=0.;  

  while True: 
    min_node = tlp.node();
    for u in visited.getNodesEqualTo(False):
      if not min_node.isValid() :
        min_node = u;
      elif dist[u] < dist[min_node]:
          min_node = u;

    if not min_node.isValid():
      break;

    visited[min_node]=True;

    for e in G.getInOutEdges(min_node) :
      weight = dist[min_node] + length[e];
      o=G.opposite(e,min_node);
      if weight < dist[o]:
        dist[o] = weight
  return dist;
