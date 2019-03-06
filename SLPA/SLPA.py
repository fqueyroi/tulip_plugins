# When the plugin development is finished, you can copy the associated Python file 
# to /home/queyroi/.Tulip-5.2/plugins/python
# or /home/queyroi/dev/tulip/install/lib/tulip/python/
# and it will be automatically loaded at Tulip startup

from tulip import tlp
import tulipplugins

class SLPA(tlp.Algorithm):
  def __init__(self, context):
    tlp.Algorithm.__init__(self, context)
    # You can add parameters to the plugin here through the following syntax:
    # self.add<Type>Parameter("<paramName>", "<paramDoc>", "<paramDefaultValue>")
    # (see the documentation of class tlp.WithParameter to see what parameter types are supported).

  def check(self):
    # This method is called before applying the algorithm on the input graph.
    # You can perform some precondition checks here.
    # See comments in the run method to know how to have access to the input graph.

    # Must return a tuple (Boolean, string). First member indicates if the algorithm can be applied
    # and the second one can be used to provide an error message.
    return (True, "")

  def run(self):
    # This method is the entry point of the algorithm when it is called
    # and must contain its implementation.

    # The graph on which the algorithm is applied can be accessed through
    # the "graph" class attribute (see documentation of class tlp.Graph).

    # The parameters provided by the user are stored in a dictionnary
    # that can be accessed through the "dataSet" class attribute.

    # The method must return a boolean indicating if the algorithm
    # has been successfully applied on the input graph.
    return True

# The line below does the magic to register the plugin into the plugin database
# and updates the GUI to make it accessible through the menus.
tulipplugins.registerPluginOfGroup("SLPA", "SLPA Clustering", "Francois Queyroi", "06/03/2019", "Overlapping clustering using label propagation", "1.0", "Clustering")
