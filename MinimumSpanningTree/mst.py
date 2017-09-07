from tulip import *
import unionfind as uf

####################################################
def mst(G,weight,selection,inverse=False) :
	selection.setAllEdgeValue(False);
	
	edges=[];
	for e in G.getEdges() :
		if inverse :
			edges.append([e,weight.getEdgeMax()-weight[e]]);
		else :
			edges.append([e,weight[e]]);
	edges.sort(key=lambda ed: ed[1]);
	
	X=uf.UnionFind();
	layout=G.getLayoutProperty("viewLayout");
	mest_length=0.;	
	for ed in edges :
		e=ed[0];
		if X[G.source(e)]!=X[G.target(e)] :
			X.union(G.source(e),G.target(e));
			selection[e]=True;
			mest_length+=layout[G.source(e)].dist(layout[G.target(e)]);
	print "MEST Length Metric = ", mest_length
####################################################
def main(G) :
	selection=G.getBooleanProperty("viewSelection");
	weight=G.getDoubleProperty("viewMetric");
	mst(G,weight,selection,True)
	
	
