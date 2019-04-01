#include "flowbetweenness.h"

#include <tulip/ForEach.h>

using namespace tlp;
using namespace std;
//================================================================================
// Methods and classes to sort nodes according to their degree
//================================================================================
struct NodeDeg {
    NodeDeg ( const double d = DBL_MAX, const tlp::node n= tlp::node()) : d(d),n(n) {}
    bool operator==(const NodeDeg &b) const {return n == b.n;}
    bool operator!=(const NodeDeg &b) const {return n != b.n;}
    double d;
    tlp::node n;
};
//================================================================================
struct HighNodeDeg {
    bool operator()(const NodeDeg&  a,const NodeDeg&  b ) const {
        if (fabs(a.d - b.d) > 1.E-9)
            return (a.d > b.d);
        else
            return (a.n.id < b.n.id);
    }
};
//================================================================================
//Sort the nodes of g in decreasing order of g.deg
//================================================================================
void sortNodesByDeg(Graph* g,vector<node>& sorted){
    sorted.clear();
    //sort nodes decreasingly by weighted degree
    set<NodeDeg,HighNodeDeg> sorted_nd;
    node u;
    forEach(u,g->getNodes()){
            sorted_nd.insert(NodeDeg(g->deg(u),u));
    }
    set<NodeDeg,HighNodeDeg>::const_iterator it;
    for(it=sorted_nd.begin();it!=sorted_nd.end();++it){
        sorted.push_back((*it).n);
    }
}
//================================================================================
PLUGIN(FlowBetweenness)
//================================================================================
// Methods and classes for shortest-path computation
//================================================================================
struct DikjstraElement {
    DikjstraElement ( const double dist = DBL_MAX, const tlp::node n = tlp::node()) :
        dist(dist),
        n(n) {}

    bool operator==(const DikjstraElement &b) const {
        return n == b.n;
    }
    bool operator!=(const DikjstraElement &b) const {
        return n != b.n;
    }
    double dist;
    tlp::node   n;
};
//================================================================================
struct LessDikjstraElement {
    bool operator()(const DikjstraElement * const a, const DikjstraElement * const b ) {
        if (fabs(a->dist - b->dist) > 1.E-9)
            return (a->dist < b->dist);
        else
            return (a->n.id < b->n.id);
    }
};
//================================================================================
void FlowBetweenness::computePaths(tlp::Graph* g,
                                   tlp::DoubleProperty* _length,
                                   tlp::node src,
                                   tlp::MutableContainer<std::list<node> > &ancestors,
                                   tlp::DoubleProperty& nb_paths,
                                   std::stack<node>& S){

    assert(src.isValid());
    set<DikjstraElement *, LessDikjstraElement> dikjstraTable;
    MutableContainer<DikjstraElement *> mapDik;
    mapDik.setAll(0);
    node n;
    forEach (n, g->getNodes()) {
        //init all nodes to +inf
        if (n != src) {
            DikjstraElement *tmp = new DikjstraElement(DBL_MAX / 2. + 10., n);
            dikjstraTable.insert(tmp);
            mapDik.set(n.id, tmp);
        }
        //init starting node to 0
        else {
            DikjstraElement * tmp = new DikjstraElement(0,n);
            dikjstraTable.insert(tmp);
            mapDik.set(n.id, tmp);
        }
    }
    ancestors.setAll(list<node>());
    nb_paths.setAllNodeValue(0);
    nb_paths.setNodeValue(src,1);


    while (!dikjstraTable.empty()) {
        //select the first element in the list the one with min value
        set<DikjstraElement *, LessDikjstraElement>::iterator it = dikjstraTable.begin();
        DikjstraElement &u = *(*it);
        dikjstraTable.erase(it);
        S.push(u.n);


        edge e;
        forEach(e, g->getInOutEdges(u.n)) {
            node v = g->opposite(e, u.n);
            DikjstraElement & dEle = *mapDik.get(v.id);
            assert(_length->getEdgeValue(e) > 0);

            //new shortest path found
            if ( (u.dist + _length->getEdgeValue(e) ) < dEle.dist) {
                dikjstraTable.erase(&dEle);
                dEle.dist = u.dist + _length->getEdgeValue(e);
                dikjstraTable.insert(&dEle);
                ancestors.set(v.id,list<node>());
                //                ancestor.set(v.id,u.n);
                nb_paths.setNodeValue(v,0);
            }
            //add path if shortest
            if((u.dist + _length->getEdgeValue(e) ) == dEle.dist){
                list<node> src_c=ancestors.get(v.id);
                src_c.push_back(u.n);
                ancestors.set(v.id,src_c);
                nb_paths.setNodeValue(v,nb_paths.getNodeValue(v)+nb_paths.getNodeValue(u.n));
            }
        }
    }

    node tmpN;
    forEach(tmpN, g->getNodes()) {
        DikjstraElement *dEle = mapDik.get(tmpN.id);
        delete dEle;
    }
}

//================================================================================
namespace{
const char * paramHelp[] = {
    // are road
    HTML_HELP_OPEN() \
    HTML_HELP_DEF( "type", "BooleanProperty" ) \
    HTML_HELP_BODY() \
    "is road" \
    HTML_HELP_CLOSE(),
    // flow value
    HTML_HELP_OPEN() \
    HTML_HELP_DEF( "type", "DoubleProperty" ) \
    HTML_HELP_BODY() \
    "flow length" \
    HTML_HELP_CLOSE(),
    // length
    HTML_HELP_OPEN() \
    HTML_HELP_DEF( "type", "DoubleProperty" ) \
    HTML_HELP_BODY() \
    "Edges length" \
    HTML_HELP_CLOSE()
};
}
//================================================================================
FlowBetweenness::FlowBetweenness(const PluginContext *context): DoubleAlgorithm(context){
    addInParameter<BooleanProperty>("is road",paramHelp[0],"",true);
    addInParameter<DoubleProperty>("flow value",paramHelp[1],"",true);
    addInParameter<DoubleProperty>("length",paramHelp[2],"",false);
}
//================================================================================
bool FlowBetweenness::check(string &errMsg){
    errMsg="";
    if(!tlp::ConnectedTest::isConnected(graph)){
        errMsg="The graph must be connected.";
        return false;
    }
    return true;
}
//================================================================================
bool FlowBetweenness::run(){
    // Get Parameters
    DoubleProperty* input_length;
    DoubleProperty* flow_value;
    BooleanProperty* is_road;

    if(dataSet!=0){
        dataSet->get("length",input_length);
        dataSet->get("is road",is_road);
        dataSet->get("flow value",flow_value);
    }

    // Check if edges have positive length
    edge e;
    forEach(e,graph->getEdges()){
        if(is_road->getEdgeValue(e)){
            if(input_length->getEdgeValue(e)<0){
                pluginProgress->setError("Edges should have positive length.");
                return false;
            }
        }
    }

    // Make flow and transport subgraph as subgraphs of graph
    Graph* flow = graph->addCloneSubGraph("flow");
    Graph* roads = graph->addCloneSubGraph("roads");

    forEach(e,graph->getEdges()){
        if(is_road->getEdgeValue(e)){
            flow->delEdge(e);
        }else{
            roads->delEdge(e);
        }
    }
    //Copy the length of edges
    DoubleProperty length(graph);
    if(input_length!=NULL){
        edge e;
        forEach(e,graph->getEdges()){
            length.setEdgeValue(e,input_length->getEdgeValue(e));
        }
    }else
        length.setAllEdgeValue(1.);

    // Find a small vertex cover of flow graph by changing edge orientations
    // so that for each edge (a,b), if deg(b) > deg(a) then a-> b
    vector<node> sorted_nodes;
    BooleanProperty visited(flow);
    visited.setAllNodeValue(false);
    sortNodesByDeg(flow,sorted_nodes);
    for(unsigned int i=0;i<sorted_nodes.size();++i){
        node u = sorted_nodes[i];
        visited.setNodeValue(u,true);
        edge e;
        forEach(e,flow->getOutEdges(u)){
            if(!visited.getNodeValue(flow->opposite(e,u))){
                flow->reverse(e);
            }
        }
    }

    // Get source and sink nodes
    vector<node> source_nodes;
    IntegerProperty index_src_nodes(graph);
    index_src_nodes.setAllNodeValue(-1);
    vector<node> sink_nodes;
    IntegerProperty index_sink_nodes(graph);
    index_sink_nodes.setAllNodeValue(-1);
    node u;
    forEach(u,flow->getNodes()){
        if(flow->outdeg(u)>0){
            source_nodes.push_back(u);
            index_src_nodes.setNodeValue(u,source_nodes.size()-1);
        }
        if(flow->indeg(u)>0){
            sink_nodes.push_back(u);
            index_sink_nodes.setNodeValue(u,sink_nodes.size()-1);
        }
    }

    // Make flow matrix
    vector<vector<double> > flow_mat;
    flow_mat.resize(source_nodes.size());
    forEach(u,graph->getNodes()){
        if(flow->outdeg(u)>0){
            flow_mat[index_src_nodes.getNodeValue(u)].resize(sink_nodes.size());
            edge e;
            forEach(e,flow->getOutEdges(u)){
                flow_mat[index_src_nodes.getNodeValue(u)][index_sink_nodes.getNodeValue(flow->opposite(e,u))]=flow_value->getEdgeValue(e);
            }
        }
    }

    // Main loop
    result->setAllEdgeValue(0.);
    vector<node>::const_iterator itn_src;
    for(itn_src=source_nodes.begin();itn_src!=source_nodes.end();++itn_src){
        node src=*itn_src;
        MutableContainer<list<node> > ancestors;
        DoubleProperty nb_paths(graph);
        stack<node> S;

        //Compute Path tree from src to others nodes in G
        computePaths(roads,&length,src,ancestors,nb_paths,S);

        DoubleProperty delta(graph);
        delta.setAllNodeValue(0.);

        while(!S.empty()){
            node w = S.top();
            S.pop();
            list<node> anc = ancestors.get(w);
            list<node>::const_iterator itn;
            double sw_flow = 0.;
            if(index_sink_nodes.getNodeValue(w)>=0){
                sw_flow=flow_mat[index_src_nodes.getNodeValue(src)][index_sink_nodes.getNodeValue(w)];
            }
            for(itn=anc.begin();itn!=anc.end();++itn){
                node v = *itn;
                double inc_delta = nb_paths.getNodeValue(v)/nb_paths.getNodeValue(w)*(sw_flow+delta.getNodeValue(w));
                delta.setNodeValue(v,delta.getNodeValue(v)+inc_delta);
                edge e  = roads->existEdge(v,w,false);
                assert(e.isValid());
                result->setEdgeValue(e, result->getEdgeValue(e) + inc_delta);
            }

        }
    }
    graph->delSubGraph(roads);
    graph->delSubGraph(flow);
    return true;
}
