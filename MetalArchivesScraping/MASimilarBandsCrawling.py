# When the plugin development is finished, you can copy the associated Python file 
# to /PATH/TO/.Tulip-5.3/plugins/python
# or /PATH/TO/TULIP/INSTALL/lib/tulip/python/
# and it will be automatically loaded at Tulip startup

from tulip import tlp
import tulipplugins

import MABandScrapping

class MASimilarBandsCrawling(tlp.ImportModule):
  def __init__(self, context):
    tlp.ImportModule.__init__(self, context)
    self.addStringParameter("url","Metal-archives url for the base band","https://www.metal-archives.com/bands/Weedeater/")
    self.addIntegerParameter("min score","Minimum score for bands to be considered similar","20")
    self.addIntegerParameter("max depth","Maximum level of recursion for crawling (distance to base band)","1")
    self.addStringParameter("logo folder path","Path for the folder in which the band logos are stored (leave blank to discard logos)","/home/queyroi-f/projects/metal_archive/scrapping/data/logos/")
    self.base_url = "https://www.metal-archives.com/bands/Weedeater/"
    self.min_score = 20
    self.max_depth = 1
    self.logo_folder = "/home/queyroi-f/projects/metal_archive/scrapping/data/logos/"
    
  def addBand(self,band):
    id_     = self.graph.getStringProperty("id")
    name   = self.graph.getStringProperty("name")
    size    = self.graph.getSizeProperty("viewSize")
    logo    = self.graph.getStringProperty("viewTexture")
    color   = self.graph.getColorProperty("viewColor")
    
    n = self.graph.addNode()
    id_[n]   = band.ma_id
    name[n] = band.name
    logo[n]    = band.logo_file_path
    return n   
  
  def crawlSimilarBands(self,bandNodes,url,current_level):
    if current_level > self.max_depth:
      return
    print "Scrapping url: {}".format(url)

    band = MABandScrapping.getBand(url,self.logo_folder, find_reviews = False, find_similar = True)
    if band is None:
      print "Error: url {} not valid.".format(url)
      return
    n = self.addBand(band)
    bandNodes[band.ma_id] = (band, n)
    
    for id_sim, url_sim, score_sim in band.similar_bands:
#      id_sim, url_sim, score_sim = info_sim_bands
      if score_sim >= self.min_score:
        self.crawlSimilarBands(bandNodes,url_sim,current_level + 1) 
  
  
  def importGraph(self):
    # get parameters
    self.base_url = self.dataSet["url"]
    self.min_score = self.dataSet["min score"]
    self.max_depth = self.dataSet["max depth"]
    self.logo_folder = self.dataSet["logo folder path"]
    
    # init properties for logos
    if self.logo_folder != '':
      color   = self.graph.getColorProperty("viewColor")
      color.setAllNodeValue(tlp.Color(255,255,255,255))
      shape   = self.graph.getIntegerProperty("viewShape")
      shape.setAllNodeValue(0)

    # crawl bands info
    bandNodes = {} # MA id to Band object, Tulip node
    self.crawlSimilarBands(bandNodes,self.base_url,0)
      
    # add graph edges
    score = self.graph.getDoubleProperty("score")

    for band, n in bandNodes.values():
      for id_sim, url_sim, score_sim in band.similar_bands:
        if id_sim in bandNodes.keys():
          sim_b, sim_n = bandNodes[id_sim]
          e = self.graph.addEdge(n, sim_n)
          score[e] = score_sim
    return True

# The line below does the magic to register the plugin into the plugin database
# and updates the GUI to make it accessible through the menus.
tulipplugins.registerPluginOfGroup("MASimilarBandsCrawling", "Similar Metal Bands", "Francois Queyroi", "02/12/2019", "Crawling of metal-archives.org visiting bands similar to the given band.", "1.0", "Python")
