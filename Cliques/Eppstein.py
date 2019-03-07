from tulip import *
import time
import random
from collections import OrderedDict
#########################################################		
def createCliqueSubgraph(G,K) :
	nodes=[];
	for n in K :
		nodes.append(n);
	sg = G.inducedSubGraph(nodes);
	sg.setName("Clique"+str(G.numberOfSubGraphs()));
#########################################################	
def getNeighborhoodSet(G,u) :
	N = set([]);
	for v in G.getInOutNodes(u) :
		N.add(v);
	return N;
#########################################################		
def choosePivot(G,C) :
	pivot = tlp.node();
	maxinter = 0;
	for u in C :
		val =  len(C & getNeighborhoodSet(G,u));
		if val>=maxinter :
			pivot = u;
			maxinter = val;
	return pivot;
#########################################################				
def maxCliquePivot(G,P,R,X) :
	C=P|X;
	if len(C)==0 :
		createCliqueSubgraph(G,R);
	else :
		pivot = choosePivot(G,C);
		A=P-getNeighborhoodSet(G,pivot);
		for x in A :
			maxCliquePivot(G,P & getNeighborhoodSet(G,x),R | set([x]),X & getNeighborhoodSet(G,x));							
			P=P-set([x]);
			X=X|set([x]);
#########################################################	
def maxCliquePeeling(G) :
	peel=tlp.DoubleProperty(G);
	G.applyDoubleAlgorithm("K-Cores",peel);
	peelMap=OrderedDict();
	for u in G.getNodes() :
		peelMap[u]=peel[u];

	sortedMap = OrderedDict(sorted(peelMap.items(), key=lambda x: x[1]))
	order = list(sortedMap.keys())
	
	for i in range(len(order)) :
		Nu=getNeighborhoodSet(G,order[i]);
		if i==len(order)-1 :
			P=set([]);
		else :
			P= Nu & set(order[(i+1):len(order)]);
		if i==0 :
			X=set([]);
		else :
			X= Nu & set(order[0:i]);
		maxCliquePivot(G,P,set([order[i]]),X); 
	
		
#########################################################	
def main(graph): 
	maxCliquePeeling(graph);
	

