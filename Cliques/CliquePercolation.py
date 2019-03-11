# You can copy the Python file
# to /PATH/TO/.Tulip-VERSION/plugins/python
# or /PATH/TO/lib/tulip/python/
# and it will be automatically loaded at Tulip startup
###################################################################################################################
from tulip import tlp
import tulipplugins
###################################################################################################################
class CliquePercolation(tlp.Algorithm):
  def __init__(self, context):
    tlp.Algorithm.__init__(self, context);
    self.addFloatParameter("k","Mininum overlap for which cliques should be merged","3",True);
    self.addBooleanParameter("Create subgraphs?","If a subgraph should be created for each cluster","True",True);
    self.addBooleanParameter("Create property?","Create a property to store cluster memberships","True",True);
    self.addDependency("Cliques Enumeration","1.0");
    self.k = 3;
    self.prop_memb = True;
    self.cliques_sg = True;
###################################################################################################################
  def check(self):
    return (True, "")
###################################################################################################################
  def buildCliqueGraph(self,cg,memb) :
    count = cg.getDoubleProperty("count");
    label = cg.getIntegerProperty("label");
    mapCNodes = {};
    for n in self.graph.getNodes() :
      for lab in memb[n] :
        if lab not in mapCNodes :
          cn = cg.addNode();
          mapCNodes[lab] = cn;
          label[cn] = lab;
        count[mapCNodes[lab]] = count[mapCNodes[lab]] + 1;
      
      for i in range(len(memb[n])-1) :
        for j in range(i+1,len(memb[n])) :
          e = cg.existEdge(mapCNodes[memb[n][i]],mapCNodes[memb[n][j]]);
          if not e.isValid() :
            e = cg.addEdge(mapCNodes[memb[n][i]],mapCNodes[memb[n][j]]);
          count[e] = count[e] + 1;
    return [count,label]
###################################################################################################################
  def percolation(self,cg,count,label) :
    for e in cg.getEdges() :
      [c1,c2] = cg.ends(e);
      if count[e] < self.k - 1 :
        cg.delEdge(e);
    
    connected_comp = tlp.ConnectedTest.computeConnectedComponents(cg);  
    mapLabClust = {};
    for i in range(len(connected_comp)) :
      for n in connected_comp[i] :
        mapLabClust[label[n]] = i;
        
    return mapLabClust;
###################################################################################################################
  def run(self):    
    # get parameters
    self.k = self.dataSet["k"];    
    self.create_sg = self.dataSet["Create subgraphs?"];
    self.create_prop = self.dataSet["Create property?"];
    
    ## compute max cliques
    ds = tlp.DataSet();
    ds["Create subgraphs?"] = False;
    ds["Create property?"]  = True;
    [suc,errmsg] = self.graph.applyAlgorithm("Cliques Enumeration",ds);
    if not suc :
      return False;
    
    ## build clique graph
    cG = tlp.newGraph();
    prop_memb  = self.graph.getIntegerVectorProperty("clique_memb");
    [count,label] = self.buildCliqueGraph(cG,prop_memb);
    
    ## Perform percolation
    mapLabClust = self.percolation(cG,count,label);    
    
    ## Save results
    if self.create_prop :
      final_clust = self.graph.getIntegerVectorProperty("clust_percolation_k="+str(self.k));
      for n in self.graph.getNodes() :
        final_clust.resizeNodeValue(n,0);
        set_clust = set();
        for lab in prop_memb[n] : 
          set_clust.add(mapLabClust[lab]);
        for sc in set_clust :
          final_clust.pushBackNodeEltValue(n,sc);
    
    if self.create_sg :
      mapLabNodes = {}
      for n in self.graph.getNodes() :
        for lab in prop_memb[n] : 
          if mapLabClust[lab] not in mapLabNodes.keys() :
            mapLabNodes[mapLabClust[lab]] = [];
          mapLabNodes[mapLabClust[lab]].append(n);
      for lab,cc in mapLabNodes.items() :
        sgg = self.graph.inducedSubGraph(cc);
        sgg.setName("Cluster_"+str(lab));
    
    ## Delete temp variables
    self.graph.delLocalProperty("clique_memb");
    return True
###################################################################################################################
# The line below does the magic to register the plugin into the plugin database
# and updates the GUI to make it accessible through the menus.
tulipplugins.registerPluginOfGroup("CliquePercolation", "Clique Percolation", "François Queyroi", "07/03/2019", "Clique Percolation Algorithm", "1.0", "Clustering")
