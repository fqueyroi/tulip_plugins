from tulip import tlp
import math
import distance
#######################################################################
## The total length (cost) of the network
#######################################################################
def length_network(G,length) :
  sum_speed=0.;
  for e in G.getEdges() :
    sum_speed=sum_speed+length[e];
  return sum_speed;
#######################################################################
## The maximum distance between two nodes in the networks
#######################################################################
def diameter(roads,length) :
  res=0.;
  for u in roads.getNodes() :
    dist=distance.distances(roads,length,u);
    for v in roads.getNodes() :
      res=max(res,dist[v]);
  return res;
#######################################################################
## Performance of the network (based on average weighted distances)
#######################################################################
def performance(G,is_road,flow_value,length,in_net,dia,sum_flow) :
  tempG=G.addCloneSubGraph();
  todel=tlp.BooleanProperty(tempG);
  todel.setAllNodeValue(True);
  for e in G.getEdges() :
    if in_net[e] :
      todel[G.source(e)]=False;
      todel[G.target(e)]=False;
  
  for n in G.getNodes() :
    if todel[n] :
      tempG.delNode(n);
      
  ds=tlp.getDefaultPluginParameters("Flow Betweenness");
  ds["is road"]=is_road;
  ds["flow value"]=flow_value;
  ds["length"]=length;
  metric=tempG.getDoubleProperty("viewMetric");
  tempG.applyDoubleAlgorithm("Flow Betweenness",metric,ds);
  res=0.;
  for e in tempG.getEdges() :
    res=res+metric[e]*length[e];
  G.delSubGraph(tempG);   
  return 1.-res/(dia*sum_flow);
#######################################################################
## Fault tolerance of the network 
## (percentage of the transfers that are not possible due to a fault in the network)
#######################################################################
def fault_tolerance(flow,flow_value,net,sum_flow) :
  res=0.;  
  id_comp=tlp.DoubleProperty(net);
  for e in net.getEdges() :
    temp_net=net.addCloneSubGraph();
    temp_net.delEdge(e);
    temp_net.applyDoubleAlgorithm("Connected Component",id_comp);
    flow_intra_comp=0.;
    for e in flow.getEdges() :
      s=flow.source(e);
      t=flow.target(e);
      if temp_net.isElement(s) and temp_net.isElement(t) :
        if id_comp[s]==id_comp[t] :
          flow_intra_comp=flow_intra_comp+flow_value[e];    
    res=res+flow_intra_comp/sum_flow;
    net.delAllSubGraphs(temp_net);
  return res/(net.numberOfEdges()+0.);
#######################################################################
## Difference between two networks (number of edges in common over the size of the union)
#######################################################################
def difference_net(roads,in_net1,in_net2) :
  inter=0.;
  union=0.;
  for e in roads.getEdges() :
    if in_net1[e] and in_net2[e] :
      inter=inter+1.;
    if in_net1[e] or in_net2[e] :
      union=union+1.;
  return inter/union;
