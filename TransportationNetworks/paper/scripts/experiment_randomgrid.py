from tulip import tlp
import stats_experiments
import random
import numpy as np
import itertools
import time
import math
#######################################################################
def gen_grid(G,nb_points) :
  layout = G.getLayoutProperty("viewLayout")
  for i in range(nb_points) :
    c=tlp.Coord(random.random()*10.,random.random()*10.);
    n=G.addNode();
    layout[n]=c;
  
  G.applyAlgorithm("Delaunay triangulation");
  length=G.getDoubleProperty("length");
  for e in G.getEdges() :
    length[e]=layout[G.source(e)].dist(layout[G.target(e)]);
  is_road=G.getBooleanProperty("is_road");
  is_road.setAllEdgeValue(False);
  for e in G.getEdges() :
    is_road[e]=True;
#######################################################################
def visGraph(G) :
  flow=G.getDoubleProperty("flow");
  color=G.getColorProperty("viewColor");
  color.setAllNodeValue(tlp.Color(0,0,0,0));
  size=G.getSizeProperty("viewSize");
  
  for u in G.getNodes() :
    if flow[u]>0 :
      color[u]=tlp.Color(255,0,0,255);
      u_s = (flow[u]-flow.getNodeMin())/(flow.getNodeMax()-flow.getNodeMin())*1.+0.05;
      size[u]=tlp.Size(u_s,u_s,1.);
    else :
      size[u]=tlp.Size(0.001,0.001,1.);
#######################################################################
def gen_flow_unif(G,nb_srcs) :
  grid=G.getSubGraph("Delaunay");
  flow=G.getSubGraph("Original graph");
  
  flow_value=G.getDoubleProperty("flow");
  nodes=[];
  for n in G.getNodes() :
    nodes.append(n);
  
  srcs=random.sample(nodes,nb_srcs);

  for n in srcs :
    flow_value[n]=1.;
  
  for n in flow.getNodes() :
    if flow_value[n]<=0 :
      flow.delNode(n); 
      
  for n1 in flow.getNodes() :
    for n2 in flow.getNodes() :
      if n1.id < n2.id :
        e=flow.addEdge(n1,n2);
        flow_value[e]=1.;
#######################################################################
def gen_flow_zipf(G,nb_srcs,epsilon) :
  grid=G.getSubGraph("Delaunay");
  flow=G.getSubGraph("Original graph");
  
  flow_value=G.getDoubleProperty("flow");
  nodes=[];
  for n in G.getNodes() :
    nodes.append(n);
  
  srcs=random.sample(nodes,nb_srcs);

  max_pop=100;
  inode=0;
  for n in srcs :
    flow_value[n]=max_pop/math.pow(1.5,inode);
    inode=inode+1;
    
  
  for n in flow.getNodes() :
    if flow_value[n]<=0 :
      flow.delNode(n); 
  
  layout=G.getLayoutProperty("viewLayout");
  for n1 in flow.getNodes() :
    for n2 in flow.getNodes() :
      if n1.id < n2.id :
        d=layout[n1].dist(layout[n2]);
        val_f=flow_value[n1]*flow_value[n2]/pow(d,1.2);
        e=flow.addEdge(n1,n2);
        flow_value[e]=val_f;
  sum_flow=0.;
  for e in flow.getEdges() :
    sum_flow=sum_flow+flow_value[e];
  for e in flow.getEdges() :
    flow_value[e]=flow_value[e]/sum_flow;
  
  for n in flow.getNodes() :
    if flow.deg(n)==0 :
      flow_value[n]=0;
      flow.delNode(n); 
#######################################################################
def applyRoutingAlgorithms(G,mu,alpha,decay,epsilon) :
  flow_value=G.getDoubleProperty("flow");
  is_road=G.getBooleanProperty("is_road");
  length=G.getDoubleProperty("length");
  ds= tlp.getDefaultPluginParameters("Shortest-Path Routing", G)
  ds["is road"]=is_road;
  ds["flow value"]=flow_value;
  ds["length"]=length;
  ds["mu"]=mu;
  ds["alpha"]=alpha;
  ds["decay"]=decay;  
  ds["epsilon"]=epsilon;  
  ds2=ds;
  
  res_sp=G.getDoubleProperty("speed_sp");
  start_time = time.time()
  G.applyDoubleAlgorithm("Shortest-Path Routing",res_sp,ds);
  time_sp = (time.time() - start_time);
  n_steps_sp = ds["final nb steps"]
  
  res_bio=G.getDoubleProperty("speed_bio");
  start_time = time.time()
  [finish_bio,error_msg]=G.applyDoubleAlgorithm("Physarum Solver",res_bio,ds2);
  if not finish_bio :
    print error_msg
  time_bio = (time.time() - start_time);
  n_steps_bio = ds2["final nb steps"]
  
  return [res_sp,n_steps_sp,time_sp,res_bio,n_steps_bio,time_bio];
#######################################################################
def final_network(G,roads,speed,threshold,alpha) :
  in_net=G.getBooleanProperty(speed.getName()+"_net_"+str(alpha));
  for e in roads.getEdges() :
    if speed[e]>threshold :
      in_net[e]=True;
      in_net[roads.source(e)]=True;
      in_net[roads.target(e)]=True;
  res_g=G.addSubGraph(in_net,speed.getName()+"_net_"+str(alpha));
  return [res_g,in_net];
#######################################################################
def main(G):
	## Parameters for grid and transfers generation 
	use_unif_transfers = True;
	size_grid=150;  
	## Parameters for the transportation network computation
	mu=1.8;
	epsilon=0.001;
	decay=0.5;
	
	## Number of experiments
	nb_test=200;
  
	file_res = open("./res.csv","w") 
	header="i,size_grid,nb_srcs,alpha,mu,decay,epsilon,time_sp,time_bio,n_steps_sp,n_steps_bio,perf_sp,perf_bio,cost_sp,cost_bio,ft_sp,ft_bio,diffset";
	print header
	file_res.write(header+"\n");

	for i in range(nb_test) :
		## Clean previous test
		for sg in G.getSubGraphs() :
			G.delAllSubGraphs(sg);
		for n in G.getNodes() :
			G.delNode(n);
    
		## Generate test grid
		gen_grid(G,size_grid);
    
		## Generate random flow
		nb_srcs=8;
		if use_unif_transfers :
			gen_flow_unif(G,nb_srcs);
		else :
			gen_flow_zipf(G,nb_srcs,epsilon);
		## Print the graph
		visGraph(G)    
    
    ## Get roads/flow graph and properties
		roads=G.getSubGraph("Delaunay");
		flow=G.getSubGraph("Original graph");
          
		flow_value=G.getDoubleProperty("flow");
		is_road=G.getBooleanProperty("is_road");
		length=G.getDoubleProperty("length");
      
    ## Compute diameter and sum flow
		dia=stats_experiments.diameter(roads,length);
		sum_flow=0.;
		for e in flow.getEdges() :
			sum_flow=sum_flow+flow_value[e];
    
		## Loop over alpha value
		for alpha in [0,1,2,4,8,16,32,64] :
          
      ## Apply routing algorithms     
			[res_sp,n_steps_sp,time_sp,res_bio,n_steps_bio,time_bio]=applyRoutingAlgorithms(G,mu,alpha,decay,epsilon)
          
      ## Compute the result networks
			[net_sp,in_net_sp]=  final_network(G,roads,res_sp,epsilon,alpha);
			[net_bio,in_net_bio]=final_network(G,roads,res_bio,epsilon,alpha);    
      
      ## Compute Cost of the networks
			total_length=0.;
			for e in roads.getEdges() :
				total_length=total_length+length[e]
			cost_sp = stats_experiments.length_network(net_sp,length)/total_length
			cost_bio = stats_experiments.length_network(net_bio,length)/total_length
      
      ## Compute Performance of the networks
			perf_sp =  stats_experiments.performance(G,is_road,flow_value,length,in_net_sp,dia,sum_flow);
			perf_bio = stats_experiments.performance(G,is_road,flow_value,length,in_net_bio,dia,sum_flow);
          
      ## Compute Fault Tolerance
			ft_sp =   stats_experiments.fault_tolerance(flow,flow_value,net_sp,sum_flow);
			ft_bio =  stats_experiments.fault_tolerance(flow,flow_value,net_bio,sum_flow);      
          
      ## Compute the differece between the final networks
			diffset=stats_experiments.difference_net(roads,in_net_sp,in_net_bio);
          
      ## Print results
			stats=[i+1,size_grid,nb_srcs,alpha,mu,decay,epsilon,round(time_sp,3),round(time_bio,3),n_steps_sp,n_steps_bio,round(perf_sp,5),round(perf_bio,5),round(cost_sp,5),round(cost_bio,5),round(ft_sp,5),round(ft_bio,5),round(diffset,3)];
			print ','.join(map(str,stats))
			file_res.write(','.join(map(str,stats))+"\n");
    
	file_res.close();
  
