#ifndef ITERATIVECYCLESOLVER_H
#define ITERATIVECYCLESOLVER_H

#include <tulip/Graph.h>
#include <tulip/DoubleProperty.h>
#include <tulip/BooleanProperty.h>
#include <tulip/MutableContainer.h>
#include <tulip/TlpTools.h>
#include <tulip/ConnectedTest.h>

#include <stack>
#include <queue>

using namespace tlp;
using namespace std;
//======================================================================================================
// Methods and classes to sort edges
//======================================================================================================
struct WEdge {
    WEdge ( const double w = DBL_MAX, const tlp::edge e= tlp::edge()) : w(w),e(e) {}
    bool operator==(const WEdge &b) const {return e == b.e;}
    bool operator!=(const WEdge &b) const {return e != b.e;}
    double w;
    tlp::edge e;
};
//================================================================================
struct HighWEdge {
    bool operator()(const WEdge&  a,const WEdge&  b ) const {
        if (fabs(a.w - b.w) > 1.E-9)
            return (a.w > b.w);
        else
            return (a.e.id < b.e.id);
    }
};
//================================================================================
struct LessWEdge {
    bool operator()(const WEdge&  a,const WEdge&  b )  const{
        if (fabs(a.w - b.w) > 1.E-9)
            return (a.w < b.w);
        else
            return (a.e.id < b.e.id);
    }
};
//======================================================================================================
// Compute the Minimum Spanning Tree of g using weights w and store the result in property in_tree
//======================================================================================================
void mst(Graph* g,DoubleProperty* w,BooleanProperty& in_tree){

    in_tree.setAllNodeValue(true);
    in_tree.setAllEdgeValue(false);

    DoubleProperty id_subtree(g);
    node u;
    forEach(u,g->getNodes()){
        id_subtree.setNodeValue(u,u.id);
    }

    // Sort the edges not in _T in increasing order of w
    set<WEdge,LessWEdge> outtree_edges;
    edge ee;
    forEach(ee,g->getEdges()){
        if(in_tree.getEdgeValue(ee)==false){
            outtree_edges.insert(WEdge(w->getEdgeValue(ee),ee));
        }
    }

    set<WEdge,LessWEdge>::const_iterator it_we;
    for(it_we=outtree_edges.begin();it_we!=outtree_edges.end();++it_we){
        WEdge we=*it_we;
        node a=g->source(we.e);
        node b=g->target(we.e);
        if(id_subtree.getNodeValue(a)!=id_subtree.getNodeValue(b)){
            // set subtree id of a to nodes with subtree id b
            double id_sub_b=id_subtree.getNodeValue(b);
            node u;
            forEach(u,g->getNodes()){
                if(id_subtree.getNodeValue(u)==id_sub_b)
                    id_subtree.setNodeValue(u,id_subtree.getNodeValue(a));
            }
            // add e to the tree
            in_tree.setEdgeValue(we.e,true);
        }
    }
}
//======================================================================================================
// Modify the orietation of edge using a BFS search in tree so that "root" is the root of T
//======================================================================================================
void makeTreeRooted(Graph* tree,node root,BooleanProperty* orient_changed){
    orient_changed->setAllEdgeValue(false);
    BooleanProperty visited(tree);
    visited.setAllNodeValue(false);
    visited.setNodeValue(root,true);
    queue<node> Q;
    Q.push(root);

    while(!Q.empty()){
        node u = Q.front();
        Q.pop();
        edge e;
        forEach(e,tree->getInOutEdges(u)){
            node v=tree->opposite(e,u);
            if(!visited.getNodeValue(v)){
                if(tree->source(e)==u){
                    tree->reverse(e);
                    orient_changed->setEdgeValue(e,true);
                }
                Q.push(v);
                visited.setNodeValue(v,true);
            }
        }
    }
}
//======================================================================================================
/** Approximation of optimal electrical flows on a support graph
 *
 *
 * Kelner, J. A., Orecchia, L., Sidford, A., & Zhu, Z. A. (2013, June).
 * "A simple, combinatorial algorithm for solving SDD systems in nearly-linear time."
 *  In Proceedings of the forty-fifth annual ACM symposium on Theory of computing (pp. 911-920).
 * ACM.
 *
 *
 *  \note 2017 Version 1.0: Initial release by François Queyroi, Géographie-Cités
 *
 */
//======================================================================================================
class IterativeCycleSolver{
public :
    IterativeCycleSolver(Graph* support,
                         DoubleProperty* length,
                         DoubleProperty* diameter,
                         double eps);
    ~IterativeCycleSolver();


    bool solve(Graph* transfer_g,
               DoubleProperty* transfers,
               node tgt,
               DoubleProperty* res_flow,
               string& errMsg);     // main method : compute a feisable flow for input transfers
                                    // and copy it to res_flow
    bool compute(string& errMsg);   //initialize the solver by computing _T and stretch values

private:
    Graph* _G;          //suport graph
    DoubleProperty* _r; //resistance (edge weight)
    double _eps;        //approximation threshold >0

    bool _solver_ready;
    Graph* _T;                              //low-strecht spanning tree of _G
    DoubleProperty* _tau;                   //stretch of edges in E(_G) \ _T
    node _root;                             //arbitrary root node of _T
    BooleanProperty* _orientation_changed;  //save the orientation of edges of _T in the support Graph
    double _K;                              //number of cycle update needed to reach epsilon-approximation
    DoubleProperty* _f;                     //computed feisable flow on _G (result)

    double stretch(edge e);                         //compute the stretch of edge e in E(_G) \ _T
    void initializeFlow(Graph* transfer_g,
                        DoubleProperty* transfers,
                        node tgt);                  //compute feisable flow according to input transfers on _T

    //updating flow methods of _T
    double queryTree(node a,DoubleProperty* w);                 //sum of w(e')*_r(e') for edges on the path from a to _root in _T
    void updateFlowTree(node a,double alpha,DoubleProperty* w); //add alpha to w for edges on the path from a to _root in _T

};

//======================================================================================================
IterativeCycleSolver::IterativeCycleSolver(Graph *support,
                                           DoubleProperty *length,
                                           DoubleProperty *diameter,
                                           double eps){
    _G=support;
    _r = new DoubleProperty(_G);
    // We set edge resistance as length / diameter
    edge e;
    forEach(e,_G->getEdges()){
        if(diameter->getEdgeValue(e)>1.E-9)
            _r->setEdgeValue(e,length->getEdgeValue(e)/diameter->getEdgeValue(e));
        else
            _r->setEdgeValue(e,DBL_MAX);
    }
    _eps=eps;
    _solver_ready = false;
    _root=node();
    _T = NULL;
    _tau = new DoubleProperty(_G);
    _orientation_changed = new BooleanProperty(_G);
    _K=0.;
    _f = new DoubleProperty(_G);
}
//======================================================================================================
IterativeCycleSolver::~IterativeCycleSolver(){
    if(_T!=NULL){
        // cancel the change in edges' orientation done to create tree _T
        edge e;
        forEach(e,_T->getEdges()){
            if(_orientation_changed)
                _G->reverse(e);
        }
        _G->delSubGraph(_T);
    }
    delete(_r);
    delete(_tau);
    delete(_f);
    delete(_orientation_changed);
}
//======================================================================================================
// Returns the sum of of w(e)*r(e) for each edge e on the path form a to _root in _T
//======================================================================================================
double IterativeCycleSolver::queryTree(node a,DoubleProperty* w){
    double sum_r=0.;
    node prev=a;
    while(prev!=_root){
        edge out;
        forEach(out,_T->getOutEdges(prev)){
            sum_r+=w->getEdgeValue(out)*_r->getEdgeValue(out);
            prev=_T->opposite(out,prev);
            break;
        }
    }
    return sum_r;
}
//======================================================================================================
// Set w(e)=w(e)+alpha for each edge e on the path form a to _root in _T
//======================================================================================================
void IterativeCycleSolver::updateFlowTree(node a, double alpha,DoubleProperty* w){
    node prev=a;
    while(prev!=_root){
        edge out;
        forEach(out,_T->getOutEdges(prev)){
            w->setEdgeValue(out,w->getEdgeValue(out)+alpha);
            prev=_T->opposite(out,prev);
            break;
        }
    }
}
//======================================================================================================
// Returns the stretch of edge e=(a,b) for the spanning tree _T
// i.e. the sum of of r(e) for each edge e on the path form a to b in _T
//======================================================================================================
double IterativeCycleSolver::stretch(edge e){
    if(_T->isElement(e))
        return 0.;
    DoubleProperty temp_f(_T);
    temp_f.setAllEdgeValue(0.);
    node a=_G->source(e);
    node b=_G->target(e);
    updateFlowTree(a,-1.,&temp_f);
    updateFlowTree(b,1.,&temp_f);
    double Re=queryTree(b,&temp_f)-queryTree(a,&temp_f)+_r->getEdgeValue(e);
    return Re;
}
//======================================================================================================
// Initalize the slover by creating the Spanning tree and computing the stretch
// of the edges outside the tree.
//======================================================================================================
bool IterativeCycleSolver::compute(string& errMsg){
    //take mst as low-stretch spanning tree
    BooleanProperty to_keep(_T);
    mst(_G,_r,to_keep);

    // create spanning tree as subgraph
    _T=_G->addSubGraph(&to_keep,"sp spanning treee");
    if(!tlp::ConnectedTest::isConnected(_T)){
        errMsg="Error: _T not connected";
        return false;
    }

    //set as root of _T a node with minimum resistance
    double min_wr=DBL_MAX;
    node u;
    forEach(u,_G->getNodes()){
        double wr=0.;
        edge e;
        forEach(e,_G->getInOutEdges(u))
            wr+=_r->getEdgeValue(e);
        if(wr<min_wr){
            min_wr=wr;
            _root=u;
        }
    }

    //modify orietation of edge in _T so that _root is root of _T
    makeTreeRooted(_T,_root,_orientation_changed);

    //compute stretch for all edge in E(_G)
    edge e;
    forEach(e,_G->getEdges()){
        _tau->setEdgeValue(e,stretch(e));
    }

    //compute tree condition number tau(T) + total stretch st(e);
    double tree_cond_number=0.;
    double total_stretch=0.;
    edge eg;
    forEach(eg,_G->getEdges()){
        tree_cond_number+=_tau->getEdgeValue(eg)/_r->getEdgeValue(eg);
        if(_tau->getEdgeValue(eg)>0.)
            total_stretch+=(_tau->getEdgeValue(eg)-_r->getEdgeValue(eg))/_r->getEdgeValue(eg);
    }

    //compute number of step needed to reach precision _eps
    _K=ceil(tree_cond_number*log(tree_cond_number*total_stretch/_eps));
    cout << "   K = " << _K << endl;

    //Solver is now ready
    _solver_ready=true;
    return true;
}
//======================================================================================================
// Initialize _f as a feasible flow on _T according to the input
//======================================================================================================
void IterativeCycleSolver::initializeFlow(Graph *transfer_g, DoubleProperty *transfers,tlp::node tgt){
    //for each edge e=(a,n) in transfer_g, add quantity transfers[e] to the path from a to b in _T
    edge fe;
    forEach(fe,transfer_g->getInEdges(tgt)){
        node src=transfer_g->source(fe);
        double fe_w=transfers->getEdgeValue(fe);
        updateFlowTree(src,fe_w,_f);
        updateFlowTree(tgt,-fe_w,_f);
    }
}
//======================================================================================================
// Main method : compute a _eps-approximation of an electrical flow
//======================================================================================================
bool IterativeCycleSolver::solve(Graph *transfer_g, DoubleProperty *transfers,tlp::node tgt,
                                 DoubleProperty *res_flow, string &errMsg){
    if(!_solver_ready){
        errMsg="The solver is not ready. Call compute() first.";
        return false;
    }
    //init random sequence
    tlp::initRandomSequence();

    //Initialize flow on _T by mapping transfers values
    _f->setAllEdgeValue(0.);
    initializeFlow(transfer_g,transfers,tgt);

    //Sort edge in decreasing order of stretch / r (probability to pick the edge at each update)
    set<WEdge,HighWEdge> tcn_table;
    edge e;
    forEach(e,_G->getEdges()){
        if(!_T->isElement(e))
            tcn_table.insert(WEdge(_tau->getEdgeValue(e)/_r->getEdgeValue(e),e));
    }

    // Compute tree condition number (sum of stretch / r for each edge not in _T)
    double tree_cond_number=0.;
    edge eg;
    forEach(eg,_G->getEdges()){
        tree_cond_number+=_tau->getEdgeValue(eg)/_r->getEdgeValue(eg);
    }

    // Main loop:
    for(unsigned int i=0;i<_K;++i){
        //pick random edge accourding to distribution of stretch / r : tau(e)/_r(e)
        double rand_d = randomDouble(tree_cond_number);
        double cumsum_tau=0.;
        set<WEdge,HighWEdge>::iterator tcn_it;
        WEdge te;
        for(tcn_it=tcn_table.begin();tcn_it!=tcn_table.end();++tcn_it){
            te=*tcn_it;
            cumsum_tau+=te.w;
            if(cumsum_tau>=rand_d)
                break;
        }
        //cycle update of the flow based on e
        edge e=te.e;
        node a=_G->source(e);
        node b=_G->target(e);
        //Compute the flow to remove on the route
        double delta_fc = _f->getEdgeValue(e)*_r->getEdgeValue(e);
        delta_fc-=((queryTree(a,_f)-queryTree(b,_f)));
        double alpha=delta_fc/_tau->getEdgeValue(e);

        //Add a flow of alpha to e
        _f->setEdgeValue(e,_f->getEdgeValue(e)-alpha);

        //Add a flow of alpha on the route from a to b in _T
        updateFlowTree(a,alpha,_f);
        updateFlowTree(b,-alpha,_f);
    }

    //Update the result property (res_flow) by adding _f
    edge fe;
    forEach(fe,_G->getEdges()){
        res_flow->setEdgeValue(fe,res_flow->getEdgeValue(fe)+fabs(_f->getEdgeValue(fe)));
    }
    return true;
}
//======================================================================================================
#endif //ITERATIVECYCLESOLVER_H
