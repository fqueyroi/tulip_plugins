#include "flowbetweenness.h"

#include <tulip/ForEach.h>

using namespace tlp;
using namespace std;
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
                                   bool directed,
                                   tlp::MutableContainer<std::list<node> > &ancestors,
                                   tlp::DoubleProperty& nb_paths,
                                   std::stack<node>& S){

    assert(src.isValid());
    set<DikjstraElement *, LessDikjstraElement> dikjstraTable;
    MutableContainer<DikjstraElement *> mapDik;
    mapDik.setAll(0);

    DikjstraElement * tmp = new DikjstraElement(0,src);
    dikjstraTable.insert(tmp);
    mapDik.set(src.id, tmp);
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
        forEach(e, (directed ? g->getOutEdges(u.n) : g->getInOutEdges(u.n)) ) {
            node v = g->opposite(e, u.n);
            assert(_length->getEdgeValue(e) > 0);

            DikjstraElement* dEle = mapDik.get(v.id);

            //new shortest path found
            if( dEle == nullptr ){
                dEle = new DikjstraElement(u.dist + _length->getEdgeValue(e),v);
                dikjstraTable.insert(dEle);
                mapDik.set(v.id, dEle);
                list<node> src_c = ancestors.get(v.id);
                src_c.push_back(u.n);
                ancestors.set(v.id,src_c);
                nb_paths.setNodeValue(v,nb_paths.getNodeValue(u.n));
            }else{
                if ( u.dist + _length->getEdgeValue(e) < dEle->dist ) {
                    dikjstraTable.erase(dEle);
                    dEle->dist = u.dist + _length->getEdgeValue(e);
                    dikjstraTable.insert(dEle);
                    ancestors.set(v.id,list<node>());
                    nb_paths.setNodeValue(v,0);
                }
                //add path if shortest
                if( u.dist + _length->getEdgeValue(e) == dEle->dist ){
                    list<node> src_c = ancestors.get(v.id);
                    src_c.push_back(u.n);
                    ancestors.set(v.id,src_c);
                    nb_paths.setNodeValue(v,nb_paths.getNodeValue(v)+nb_paths.getNodeValue(u.n));
                }
            }
        }
    }

    node tmpN;
    forEach(tmpN, g->getNodes()) {
        DikjstraElement *dEle = mapDik.get(tmpN.id);
        if( dEle != nullptr )
            delete dEle;
    }
}

//================================================================================
namespace{
const char * paramHelp[] = {
    // directed
    "Indicates if the transportation network should be considered as directed or not.",
    // are road
    "True for edges in the graph that correspond to the transportation network.<br>"
    "False for the edges that correspond to flow value.",
    // flow value
    "Flow exchanged on the flow edges.",
    // length
    "Length of edges in the transportation network."
};
}

FlowBetweenness::FlowBetweenness(const PluginContext *context): DoubleAlgorithm(context){
    addInParameter<bool>("directed",paramHelp[0],"false",true);
    addInParameter<BooleanProperty>("is road",paramHelp[1],"",true);
    addInParameter<DoubleProperty>("flow value",paramHelp[2],"",true);
    addInParameter<DoubleProperty>("length",paramHelp[3],"",false);
}
//================================================================================
bool FlowBetweenness::check(string &errMsg){
    errMsg="";
    return true;
}
//================================================================================
bool FlowBetweenness::run(){
    // Get Parameters
    bool directed;
    DoubleProperty* input_length;
    DoubleProperty* flow_value;
    BooleanProperty* is_road;

    if(dataSet!=0){
        dataSet->get("directed",directed);
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
    result->setAllNodeValue(0.);
    result->setAllEdgeValue(0.);

    for(auto itn_src=source_nodes.begin();itn_src!=source_nodes.end();++itn_src){
        node src=*itn_src;
        MutableContainer<list<node> > ancestors;
        DoubleProperty nb_paths(graph);
        stack<node> S;

        //Compute Path tree from src to others nodes in G
        computePaths(roads,&length,src,directed,ancestors,nb_paths,S);

        DoubleProperty delta(graph);
        delta.setAllNodeValue(0.);

        while(!S.empty()){
            node w = S.top();
            S.pop();
            list<node> anc = ancestors.get(w);

            double sw_flow = 0.;
            if(index_sink_nodes.getNodeValue(w)>=0){
                sw_flow=flow_mat[index_src_nodes.getNodeValue(src)][index_sink_nodes.getNodeValue(w)];
            }
            if(w.id != src.id)
                result->setNodeValue(w,result->getNodeValue(w) + delta.getNodeValue(w));
            for(auto itn=anc.begin();itn!=anc.end();++itn){
                node v = *itn;
                double inc_delta = nb_paths.getNodeValue(v)/nb_paths.getNodeValue(w)*(sw_flow+delta.getNodeValue(w));
                delta.setNodeValue(v,delta.getNodeValue(v)+inc_delta);
                edge e  = roads->existEdge(v,w,directed);
                assert(e.isValid());
                result->setEdgeValue(e, result->getEdgeValue(e) + inc_delta);
            }

        }
    }
    graph->delSubGraph(roads);
    graph->delSubGraph(flow);
    return true;
}
