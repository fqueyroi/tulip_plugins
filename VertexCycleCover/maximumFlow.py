from tulip import *

##############################################
def getPath(G,n,t,path,residual,visited) :
	if n==t :
		return True;
	visited[n]=True;
	found=False;	
	for e in G.getOutEdges(n) :
		o=G.opposite(e,n);
		if (residual[e]>0) and (not visited[o]) :
			if getPath(G,o,t,path,residual,visited) :
				path.append(e);
				return True;
	return False;
##############################################
def residualGraph(G,path,minC,residual,capacity,flow) :
	for e in path :
		flow[e]+=minC;		
		residual[e]=capacity[e]-flow[e];
		eo = G.existEdge(G.target(e),G.source(e),True);
		flow[eo]=-flow[e]
		residual[eo]=capacity[eo]-flow[eo];
##############################################
def getMaxFlow(G,s,t,capacity,flow) :
	residual=tlp.DoubleProperty(G);
	flow.setAllEdgeValue(0);
	for e in G.getEdges() :
		residual[e]=capacity[e];
		
	visited=tlp.BooleanProperty(G);
	visited.setAllNodeValue(False);
	path=[];
	getPath(G,s,t,path,residual,visited);
	while len(path)>0 :
		minC=residual[path[0]];
		for e in path :
			if residual[e]<minC :
				minC=residual[e];
		#print "P : "+str(path);
		#print "minC = "+str(minC);
		residualGraph(G,path,minC,residual,capacity,flow)
		path=[];
		visited.setAllNodeValue(False);
		getPath(G,s,t,path,residual,visited);
