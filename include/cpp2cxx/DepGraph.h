/**
cpp2cxx is an open source software distributed under terms of the
Apache2.0 licence.

Copyrights remain with the original copyright holders.
Use of this material is by permission and/or license.

Copyright [2012] Aditya Kumar, Andrew Sutton, Bjarne Stroustrup
          [2020] Thomas Figueroa
          
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef DEPGRAPH_H
#define DEPGRAPH_H

/**
 *  @file DepGraph.h
 *  @brief contains the node as well as the tree class for building
 *   the dependency graph. This graph can then be traversed for dependency analysis etc.
 *  @version 1.0
 *  @author Aditya Kumar
 *  @note
 *  compiles with g++-4.5 or higher,
 *  for compiling pass -std=c++0x to the compiler
 */
/** @todo how can the AST of if-else block be made generic
  * basically the graph has the following properties:
  * 1. siblings are connected.
  * 2. each level is a representation of nest depth.
  * 3. at one level each node will contain all the code/text that lies in between
  *    it and it's sibling.
  * 4.
  */
#include "DemacBoostWaveIncludes.h"
#include "Macro.h"
#include "UseCaseState.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>

#include <algorithm>
#include <map>
#include <vector>

/**
 * @class Node
 * @brief Node of the macro tree used to store macros
 */
struct Node
{
    //public:
    Node() noexcept : key(boost::wave::T_UNKNOWN), nodeIndex(0), condCat(CondCategory::local), parent(nullptr)
    {
    }
    ~Node()
    {
        std::for_each(vecMacro.begin(), vecMacro.end(), [](PPMacro* mac) {
            delete mac;
            mac = NULL;
        });
    }
    std::vector<token_type> const& GetCondStmt()
    {
        return condStmt;
    }
    //put the macro in the free store
    void PushBackMacro(PPMacro& mac)
    {
        // @TODO: Replace with smart pointer?
        PPMacro* m = new PPMacro(mac);
        //*m = mac;
        vecMacro.push_back(m);
    }
    bool operator==(Node& n)
    {
        return (condStmt == n.condStmt) && (parent == n.parent) && (vecMacro == n.vecMacro);
    }

    token_type key; //this key may be #if, #ifdef, ifndef, endif
    int nodeIndex;
    //whether config macros used or local macros tested for
    CondCategory condCat;
    //the complete condition text including the #if etc...
    std::vector<token_type> condStmt;
    //Macros within the condition block
    std::vector<PPMacro*> vecMacro; //keep only the pointer to macro
    Node* parent;
};

/**
 * @struct NodeOrder
 * @brief To sort the Node
 */
struct NodeOrder
{
    bool operator()(Node const* n1, Node const* n2 = NULL) const
    {
        if(n2 == NULL)
            return true;
        return n1->nodeIndex < n2->nodeIndex;
    }
};

using TokenMacroMap_t = std::multimap<token_type, PPMacro*, TokenOrder>;
using PairMacroIter_t = std::pair<TokenMacroMap_t::iterator, TokenMacroMap_t::iterator>;

//first arg: type of adjacency list, second arg: backbone of graph
//Also defined in DepAnalyzer.h
using Graph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, Node*>;

//Also defined in DepAnalyzer.h
using Vertex_t = boost::graph_traits<Graph_t>::vertex_descriptor;

using VertexIterator_t = boost::graph_traits<Graph_t>::vertex_iterator;
using OutEdgeIterator_t = boost::graph_traits<Graph_t>::out_edge_iterator;

//since we are storing pointers no need of a multimap
using NodeMap_t = std::map<Node*, Vertex_t, NodeOrder>;

// vector of pairs to retain the order in which they occur
//also defined in the DepAnalyzer.h
using DepList_t = std::vector<std::pair<PPMacro*, std::vector<PPMacro*>>>;

//not used rite now
class DFSVisitor : public boost::default_dfs_visitor
{
public:
    void discover_vertex(Vertex_t v, const Graph_t& g) const
    {
        /*    for_each(g[v]->vecMacro.begin(),g[v]->vecMacro.End(),
      [&linearOrder](PPMacro mac) {
        linearOrder.push_back(&mac);
      });*/
    }
};

/**
 * @class MacTree
 * @brief The tree class having Node as vertices of the trees
 */
class MacTree
{

public:
    MacTree() noexcept : nodeIndex(0), depGraph(1)
    {
        // @TODO: Replace with smart pointer?
        Node* pn = new Node;
        //currVertex is dummy here
        NodeMap_t::iterator nodeMap_iter = nodeMap.insert(std::make_pair(pn, currVertex)).first;
        currVertex = boost::add_vertex(nodeMap_iter->first, depGraph);
        nodeMap_iter->second = currVertex;
        startVertex = currVertex;
    }
    ~MacTree()
    {
        DeleteNodes();
    }

    /// @brief get the root node
    Node const* GetRoot() const;

    /// @brief get the parent node
    static Node* GetParent(Node* const np);

    /// @brief return the first child of the node, not implemented
    Node* GetFirstChild(Vertex_t const v);

    /// @brief get the total number of nodes
    int GetNumNodes() const;

    /// @brief is the node a root node
    bool IsRoot(Vertex_t const v) const;

    /// @brief sibling of the currently pointed vertex
    bool MakeSibling(Node& rn);

    /// @brief child to the currently pointed vertex
    bool MakeChild(Node& rn);

    /// @brief specific to the macro insertion
    void PushBackMacro(PPMacro& mac);

    /// @brief returns the macro associated having the identifier token as tok
    PairMacroIter_t GetMacro(const token_type& tok);

    /// @brief sets the pointer of the current node to the parent node
    void GotoParent();

    /// @brief builds the dependency list, and returns a refernce to it
    const DepList_t& BuildMacroDependencyList();

    /// @brief gets the vertex descriptor to the parent of the current vertex
    Vertex_t GetParent(Vertex_t const v);

    /// @brief returns the pair of iterators to the siblings
    std::pair<OutEdgeIterator_t, OutEdgeIterator_t> GetSiblings(Vertex_t const vd);

    /// @brief returns the pair of iterators to the children
    std::pair<OutEdgeIterator_t, OutEdgeIterator_t> GetChildren(Vertex_t const vd);

    bool MakeChild(Vertex_t parentV, Vertex_t childV);
    bool MakeSibling(Vertex_t firstV, Vertex_t secondV);
    bool DeleteVertex(Vertex_t v);
    void DeleteNodes();
    bool IsMacro(token_type const& tok);
    void CheckToken(const token_iterator& tok_iter);

    /// not defined: for future use
    Vertex_t GetAncestor(int nesting_level);

    /// algorithm
    void Dfs(Vertex_t const startV);
    /// algorithm
    void Bfs(Vertex_t const startV);
    /// algorithm
    void TopSort(std::vector<Graph_t::vertex_descriptor>& topo_order);

private:
    int nodeIndex;
    NodeMap_t nodeMap;
    std::vector<PPMacro*> linearOrder; //keeps the macro order as they occur
    UseCaseState macroUseCaseState;
    TokenMacroMap_t tokenMacroMap;
    DepList_t macroDepList;
    Graph_t depGraph;
    Vertex_t currVertex;
    Vertex_t startVertex; //the root node
};

#endif /*DEPGRAPH_H*/
