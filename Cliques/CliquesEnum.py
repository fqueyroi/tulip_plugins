from tulip import tlp
import tulipplugins
from collections import OrderedDict
###################################################################################################################
class CliquesEnum(tlp.Algorithm):
  def __init__(self, context):
    tlp.Algorithm.__init__(self, context)
    self.addBooleanParameter("Create subgraphs?","If a subgraph should be created for each cliques","True",True);
    self.addBooleanParameter("Create property?","Create a property to store clique membership","True",True);
    self.prop_memb = True;
    self.cliques_sg = True;
    self.nb_cliques = 0;
  ################################################################################################################
  def check(self):
    return (True, "")
  #########################################################	
  def saveClique(self,G,R) :        
    if self.create_sg :
      nodes=[];
      for n in R :
        nodes.append(n);
      cliques_sg = self.graph.getSubGraph("Cliques")
      csg = cliques_sg.inducedSubGraph(nodes);
      csg.setName("clique_"+str(self.nb_cliques));
    if self.create_prop :
      prop_memb  = self.graph.getIntegerVectorProperty("clique_memb");
      for n in nodes :
        prop_memb.pushBackNodeEltValue(n,self.nb_cliques);
    self.nb_cliques = self.nb_cliques + 1;
  #########################################################
  def getNeighborhoodSet(self,G,u) :
    	N = set([]);
    	for v in G.getInOutNodes(u) :
    		N.add(v);
    	return N;
  #########################################################		
  def choosePivot(self,G,C) :
    	pivot = tlp.node();
    	maxinter = 0;
    	for u in C :
    		val =  len(C & self.getNeighborhoodSet(G,u));
    		if val >= maxinter :
      			pivot = u;
      			maxinter = val;
    	return pivot;
  ################################################################################################################
  def maxCliquePivot(self,G,P,R,X) :
    C = P | X;
    if len(C) == 0 :
      self.saveClique(G,R);
    else :
    		pivot = self.choosePivot(G,C);
    		A = P - self.getNeighborhoodSet(G,pivot);
    		for x in A :
    			self.maxCliquePivot(G,P & self.getNeighborhoodSet(G,x),R | set([x]),X & self.getNeighborhoodSet(G,x));							
    			P = P - set([x]);
    			X = X | set([x]);
  ################################################################################################################
  def run(self):
    # get parameters    
    self.create_sg = self.dataSet["Create subgraphs?"];
    self.create_prop = self.dataSet["Create property?"];
    
    ## init results
    self.nb_cliques = 0;
    prop_memb = None;
    cliques_sg = None;
    if self.create_sg :
      self.graph.addCloneSubGraph("Cliques");
    if self.create_prop :
      prop_memb  = self.graph.getIntegerVectorProperty("clique_memb");
      for n in self.graph.getNodes() :
        prop_memb.resizeNodeValue(n,0);
    
    ## compute the degeneracy ordering of the nodes
    peel=tlp.DoubleProperty(self.graph);
    self.graph.applyDoubleAlgorithm("K-Cores",peel);
    peelMap={};
    for u in self.graph.getNodes() :
    		peelMap[u]=peel[u];
    sortedMap = OrderedDict(sorted(peelMap.items(), key=lambda x: x[1]))
    order = list(sortedMap.keys())
    	
    	## start clique detection
    for i in range(len(order)) :
      Nu = self.getNeighborhoodSet(self.graph,order[i]);
      if i == len(order)-1 :
        P = set();
      else :
        P = Nu & set(order[(i+1):len(order)]);
      if i == 0 :
        X = set();
      else :
        X = Nu & set(order[0:i]);
      self.maxCliquePivot(self.graph,P,set([order[i]]),X);    		
    ## end
    return True
###################################################################################################################
tulipplugins.registerPluginOfGroup("CliquesEnum", "Cliques Enumeration", "François Queyroi", "07/03/2019", "Enumerate All Maximal Cliques using [Eppstein et al., 2010] method.", "1.0", "Clustering")
