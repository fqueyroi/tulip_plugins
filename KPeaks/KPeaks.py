# When the plugin development is finished, you can copy the associated Python file 
# to /PATH/TO/.Tulip-5.2/plugins/python
# or /PATH/TO/tulip/install/lib/tulip/python/
# and it will be automatically loaded at Tulip startup
#######################################################################################################
from tulip import tlp
import tulipplugins
#######################################################################################################
class KPeaks(tlp.DoubleAlgorithm):
  #######################################################################################################
  def __init__(self, context):
    tlp.DoubleAlgorithm.__init__(self, context)
    self.addDependency("K-Cores","1.0");
    self.addStringCollectionParameter("type","This parameter indicates the direction used to compute k-peak values","InOut;In;Out");
    self.addNumericPropertyParameter("metric","Edge-Weighted version of k-peaks decomposition","",False);
    self.addFloatParameter("Core Dependency","In [0,1], indicates the average dependency to highest core value (1 : high dependency, 0 : no dependency)","-1",False,False,True)
  #######################################################################################################
  def check(self):
    return (True, "")
  #######################################################################################################
  def run(self):
    clone = self.graph.addCloneSubGraph("clone");    
    cores = tlp.DoubleProperty(clone);
    self.pluginProgress.setComment("Computing K-Peaks ...");
    while clone.numberOfNodes() > 0 :
      self.pluginProgress.progress(self.graph.numberOfNodes() - clone.numberOfNodes() , self.graph.numberOfNodes());
      clone.applyDoubleAlgorithm("K-Cores",cores,self.dataSet);
      d = cores.getNodeMax();
      for v in clone.getNodes() :
         if cores[v] == d :
            self.result[v] = d;
            clone.delNode(v);             
    self.graph.delAllSubGraphs(clone);

    ## compute core dependancy
    self.pluginProgress.setComment("Computing Core Dependency...");
    base_cores = tlp.DoubleProperty(self.graph);
    self.graph.applyDoubleAlgorithm("K-Cores",base_cores,self.dataSet);
    core_dep = 0.;
    for n in self.graph.getNodes() :
      if base_cores[n] > 0 :
        core_dep += (base_cores[n] - self.result[n])/base_cores[n];
    core_dep /= self.graph.numberOfNodes() + 0.;
    self.dataSet["Core Dependency"] = core_dep;
    return True
#########################################################################################################
tulipplugins.registerPluginOfGroup("KPeaks", "K-Peaks", "François Queyroi", "21/03/2019", "Compute the k-peak decomposition of the networks. See</br> J. Abello and F. Queyroi.<br/><b>Network decomposition into fixed points of degree peeling.</b><br/>Social Network Analysis and Mining, 2014, vol. 4, no 1, p. 191. (2014)", "1.0", "Measure")
