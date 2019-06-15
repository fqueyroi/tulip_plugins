# When the plugin development is finished, you can copy the associated Python file 
# to /home/queyroi/.Tulip-5.2/plugins/python
# or /home/queyroi/dev/tulip/install/lib/tulip/python/
# and it will be automatically loaded at Tulip startup
#############################################################################################
from tulip import tlp
import tulipplugins
#############################################################################################
from datetime import datetime
import math
#############################################################################################
import PathwayDetectionSimple
#############################################################################################  
def appendToVectorProp(prop,elem,value) :
  vect = prop[elem];
  vect.append(value);
  prop[elem] = vect;
#############################################################################################
def LongLatDistance(position,n1,n2) :
  ### Distance (in km) between n1 and n2 localized by position (LayoutProperty)
  ### using Haversine formula
  p1 = (math.radians(position[n1].getY()),math.radians(position[n1].getX()))
  p2 = (math.radians(position[n2].getY()),math.radians(position[n2].getX()))
  d_long = (p2[0] - p1[0])
  d_latt = (p2[1] - p1[1])
  a = math.sin(d_latt/2)**2 + math.cos(p1[1]) * math.cos(p2[1]) * math.sin(d_long/2)**2
  c = 2 * math.asin(math.sqrt(a))
  return 6371 * c
#############################################################################################

class TVGLlyodsImport(tlp.ImportModule):
  
  #############################################################################################
  def __init__(self, context):
    tlp.ImportModule.__init__(self, context)
    self.addFileParameter("moves filepath", True, "path to Llyods moves file")
    self.addFileParameter("ports filepath", True, "path to Llyods ports file")
    self.addStringCollectionParameter("Type", "Network type: Space L/P or A (see doc)",
                                      "SpaceL;SpaceP;SpaceA")
    self.addBooleanParameter("keep all moves?", "If false,  remove P and I moves", "False", True)
    self.addBooleanParameter("keep all ports?", "If false,  remove ports with type diff than 1")
    self.addFloatParameter("min date", "Minimum date included", "14335", False)
    self.addFloatParameter("max date", "Maximum date excluded", "14455", False)
    self.addFloatParameter("min freq","Reccursively Filter Nodes such that every port as a frequency of arrival and departures higher than the given thresold",
                           "0.143",False)
    self.keep_all_moves = False
    self.keep_all_ports = False
    self.min_date = 0
    self.max_date = 99999
    self.net_type = "SpaceL"
    self.netTypeFunction = {"SpaceL": self.spaceLIndexes,
                            "SpaceP": self.spacePIndexes,
                            "SpaceA": self.spaceANoRepIndexes}
    self.min_freq = 0.143
  #############################################################################################
  def readPorts(self, port_filepath):
    id_ = self.graph.getStringProperty("id_")
    name = self.graph.getStringProperty("name")
    label = self.graph.getStringProperty("viewLabel")
    country = self.graph.getStringProperty("country")
    continent = self.graph.getStringProperty("continent")
    pos = self.graph.getLayoutProperty("viewLayout")
    
    portsNodes = {}
    excluded_ports = set()
    ports_file = open(port_filepath, 'r')
    ports_file.readline()
    for line in ports_file:
      s = line.strip().split(',')
      if (not self.keep_all_ports) and (int(s[8]) != 1):
        excluded_ports.add(s[4])
        continue
      n = self.graph.addNode()
      portsNodes[s[4]] = n
      id_[n] = s[4]
      name[n] = s[5]
      label[n] = s[5]
      country[n] = s[3]
      continent[n] = s[0]
      if s[6] != "" and s[7] != "":
        pos[n] = (float(s[6]), float(s[7]))
    return [portsNodes, excluded_ports]  
  #############################################################################################
  def cleanPathShip(self,list_port, portsNodes) :
    list_port.sort(key = lambda x: x[1])
    res = [];
    i = 0
    while i < len(list_port):
      ## check if the port exist as a node
      if list_port[i][0] in portsNodes.keys() :
        ## if dest and origin are different
        ## and arrival after departure
        if len(res) == 0 or (list_port[i][0] != res[-1][0] and list_port[i][1] >= res[-1][2]):          
          res.append(list_port[i])          
      i += 1
    return res;
  #############################################################################################
  def spaceLIndexes(self,ports,deps,arrs):
    return [(i,i+1) for i in range(len(ports) - 1)];
  #############################################################################################
  def spaceANoRepIndexes(self,ports,deps,arrs):
    res = []
    for i in range(len(ports) - 1) :
      visited = set()
      for j in range(i + 1,len(ports)) :
        if ports[i] == ports[j] :
          break;
        if ports[j] not in visited :
          visited.add(ports[j])
          res.append((i,j))
    return res
  #############################################################################################
  def spacePIndexes(self,ports,deps,arrs):
    decomp = PathwayDetectionSimple.find_valid_subpath_decomp(ports)
    res = []
    start_index = 0
    for ind_sp in range(len(decomp)) :
      subpath = decomp[ind_sp]
      if len(subpath) > 1:
        res.extend([(start_index + i,start_index + j) 
                    for i in range(len(subpath)-1) for j in range(i+1,len(subpath)) if subpath[i] != subpath[j]]);
      if ind_sp < len(decomp) - 1:
        res.append((start_index + len(subpath) - 1, start_index + len(subpath)))
      start_index += len(subpath)   
    return res  
    
  #############################################################################################
  def addTripsForShip(self,ship,list_port, portsNodes):
    arrivals       = self.graph.getDoubleVectorProperty("arrivals")
    departures     = self.graph.getDoubleVectorProperty("departures")
    arrivals_str   = self.graph.getStringVectorProperty("arr_str")
    departures_str = self.graph.getStringVectorProperty("dep_str")
    ship_id        = self.graph.getStringVectorProperty("ship_id")
    nb_stops       = self.graph.getDoubleVectorProperty("nb_stops")

    ports   = [p for p, a, d, astr, dstr in list_port]
    arr_day = [a for p, a, d, astr, dstr in list_port]
    dep_day = [d for p, a, d, astr, dstr in list_port]
        
    for i,j in self.netTypeFunction[self.net_type](ports,dep_day,arr_day) :
      n_p1, n_p2   = portsNodes[ports[i]], portsNodes[ports[j]]
      e = self.graph.existEdge(n_p1, n_p2, True) 
      if not e.isValid():
        e = self.graph.addEdge(n_p1, n_p2)
      appendToVectorProp(departures,e,dep_day[i]);
      appendToVectorProp(arrivals,e,arr_day[j]);
      appendToVectorProp(departures_str,e,list_port[i][4]);
      appendToVectorProp(arrivals_str,e,list_port[j][3]);
      appendToVectorProp(ship_id,e,ship);
      appendToVectorProp(nb_stops,e, j - (i + 1) );
  #############################################################################################
  def readMoves(self, moves_filepath, portsNodes, excluded_ports):
    self.pluginProgress.progress(0, 1)
    self.pluginProgress.setComment("Reading Trajectory file...")
    ships = {}
    moves_file = open(moves_filepath, 'r')
    moves_file.readline()
    for line in moves_file:
      s = line.strip().split(',')
      if not self.keep_all_moves and (s[12] == 'P' or s[12] == 'I'):
        continue
      
      [id_ship, id_port, arr_date_str,dep_date_str] = [s[0], s[3], s[4], s[8]];
      if not self.keep_all_ports and id_port in excluded_ports:
        continue     
      if arr_date_str == "" :
        continue;
      if dep_date_str == "" :
        dep_date_str == arr_date_str
      arr_date = datetime.strptime(arr_date_str, '%d/%m/%Y')
      dep_date = datetime.strptime(dep_date_str, '%d/%m/%Y')
      arr_nb_days = abs(arr_date - datetime(1970, 1, 1)).days
      dep_nb_days = abs(dep_date - datetime(1970, 1, 1)).days
      if arr_nb_days < self.min_date or arr_nb_days > self.max_date :
        continue
      if dep_nb_days < arr_nb_days :
        dep_nb_days = arr_nb_days;
        dep_date_str = arr_date_str;
      if id_ship not in ships.keys():
        ships[id_ship] = []
      ships[id_ship].append((id_port, arr_nb_days, dep_nb_days, arr_date_str, dep_date_str))
    self.pluginProgress.progress(1, 1)

    counter_loop = 0
    max_loop = len(ships.keys())
    for ship in ships.keys():
      self.pluginProgress.setComment("Adding graph edges: Ship id "+str(ship))
      self.pluginProgress.progress(counter_loop, max_loop)
      counter_loop += 1
      list_port = ships[ship]
      if len(list_port) > 1:  
        list_port = self.cleanPathShip(list_port, portsNodes)
        if len(list_port) > 1 :         
          self.addTripsForShip(ship,list_port, portsNodes)
  #############################################################################################
  def correctDurationEdge(self):
    avg_duration = self.graph.getDoubleProperty("avg_duration")
    dist_ends    = self.graph.getDoubleProperty("dist_points")
    
    coef_cor = 21. / 12774.5575 ## coef given by the ratio between the
                                ## average number of days of travel between Shanghai and Rottendam 
                                ## and the geodesic distance between Shanghai and Rottendam 
    cor_duration = self.graph.getDoubleProperty("cor_duration")
    for e in self.graph.getEdges():
      cor_duration[e] = avg_duration[e]
      ## if the observed average duration is lower than a quarter 
      ## the expected dist we replace the observed average
      if dist_ends[e] == 0 :
        print(self.graph["name"][self.graph.source(e)]+" - "+self.graph["name"][self.graph.target(e)]+" : "+str(avg_duration[e])+" null points dist")
        continue
      if avg_duration[e] / (coef_cor * dist_ends[e]) < 0.25:
        cor_duration[e] = coef_cor * dist_ends[e]
  #############################################################################################
  def computeStats(self):
    arrivals       = self.graph.getDoubleVectorProperty("arrivals")
    departures     = self.graph.getDoubleVectorProperty("departures")
    ship_id        = self.graph.getStringVectorProperty("ship_id")
    nb_stops       = self.graph.getDoubleVectorProperty("nb_stops")
    position       = self.graph.getLayoutProperty("viewLayout")

    freq         = self.graph.getDoubleProperty("Freq")    
    nb_ships     = self.graph.getDoubleProperty("nb_ships")
    logfreq      = self.graph.getDoubleProperty("logfreq")
    avg_duration = self.graph.getDoubleProperty("avg_duration")
    nb_uses      = self.graph.getDoubleProperty("nb_uses")
    dist_ends    = self.graph.getDoubleProperty("dist_points")

    self.pluginProgress.setComment("Computing Stats: Nodes");
    count_n = 0;
    for n in self.graph.getNodes() :
      self.pluginProgress.progress(count_n,self.graph.numberOfNodes())
      count_n += 1
      
      set_ships = set();
      for e in self.graph.getInOutEdges(n) :
        stops = nb_stops[e]
        ships = ship_id[e]
        dep   = departures[e]
        arr   = arrivals[e]
        for i in range(len(dep)):
          set_ships.add(ships[i])
          if stops[i] == 0:
            freq[n] += 1;
      freq[n] /= (self.max_date - self.min_date);
      if freq[n] > 0 :
        logfreq[n] = math.log(freq[n])
      nb_ships[n] = len(set_ships)

    self.pluginProgress.setComment("Computing Stats: Edges");
    count_e = 0;
    for e in self.graph.getEdges() :
      self.pluginProgress.progress(count_e,self.graph.numberOfEdges())
      count_e += 1
      
      stops = nb_stops[e]
      ships = ship_id[e]
      dep   = departures[e]
      arr   = arrivals[e]

      set_ships = set(ships)
      nb_ships[e] = len(set_ships)
      nb_uses[e]  = len(ships)
      
      dist_ends[e] = LongLatDistance(position,self.graph.source(e),self.graph.target(e))

      for i in range(len(dep)) :
        avg_duration[e] += arr[i] - dep[i]
      avg_duration[e] /= len(dep)
  #############################################################################################
  def inOutFreq(self,n,selec):
    nb_stops       = self.graph.getDoubleVectorProperty("nb_stops")
    infreq, outfreq = 0.,0.
    for e in self.graph.getInEdges(n):
      if not selec[self.graph.opposite(e,n)]:
        infreq += len([s for s in nb_stops[e] if s == 0])
    for e in self.graph.getOutEdges(n):
      if not selec[self.graph.opposite(e,n)]:
        outfreq += len([s for s in nb_stops[e] if s == 0])
    return infreq / (self.max_date - self.min_date + 0.), outfreq / (self.max_date - self.min_date + 0.)
  
  #############################################################################################
  def filterFreq(self):
    selec = tlp.BooleanProperty(self.graph)
    selec.setAllNodeValue(False)
    nb_del = self.graph.numberOfNodes()
    nb_del_tot = 0
    while nb_del > 0:
      nb_del = 0
      for n in self.graph.getNodes():
        if not selec[n]:
          freqs = self.inOutFreq(n,selec)
          if freqs[0] < self.min_freq or freqs[1] < self.min_freq:
            selec[n] = True
            nb_del += 1
            nb_del_tot += 1
    
    print("% ports filtered: "+str(100.* nb_del_tot / self.graph.numberOfNodes()))
    for n in self.graph.getNodes():
      if selec[n]:
        self.graph.delNode(n)
  #############################################################################################
  def importGraph(self):
    
    moves_filename = self.dataSet['moves filepath']
    ports_filename = self.dataSet['ports filepath']
    self.keep_all_moves = self.dataSet['keep all moves?']
    self.keep_all_ports = self.dataSet['keep all ports?']
    self.min_date = self.dataSet['min date']
    self.max_date = self.dataSet['max date']
    self.net_type = self.dataSet['Type']
    self.min_freq = self.dataSet['min freq']
    
    self.pluginProgress.progress(0, 1)
    self.pluginProgress.setComment("Reading ports info...")
    [portsNodes, excluded_ports] = self.readPorts(ports_filename)
    
    self.pluginProgress.progress(1, 1)
    self.readMoves(moves_filename, portsNodes, excluded_ports)
    
    self.graph.setName("Llyods_"+self.net_type+"("+str(int(self.min_date))+":"+str(int(self.max_date))+")")
    if self.min_freq > 0:
      self.pluginProgress.setComment("Filtering ports");
      self.filterFreq()
    self.computeStats()
    self.correctDurationEdge()
    return True

#############################################################################################
tulipplugins.registerPluginOfGroup("TVGLlyodsImport",  "TVG Llyods Import",  "Francois Queyroi",  "17/03/2019",  "",  "1.0",  "Python")
