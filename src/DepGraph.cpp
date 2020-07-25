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

#include "cpp2cxx/DepGraph.h"
#include "cpp2cxx/ExceptionHandler.h"
#include "general_utilities/debug.h"

#include <string>
#include <utility>
#include <vector>

Node* MacTree::GetParent(Node* const np)
{
    return np->parent;
}

Vertex_t MacTree::GetParent(Vertex_t const v)
{
    //in a uni-directed graph incoming edge can't be accessed
    Node* cn = depGraph[v];
    Node* pn = cn->parent;

    DEBUG_TREE(dbgs() << "Parent nodeIndex: " << pn->nodeIndex << "\n";);

    auto nodeMap_iter = nodeMap.find(pn);
    return nodeMap_iter->second;
}

Node const* MacTree::GetRoot() const
{
    return depGraph[startVertex];
}
/*
Node* MacTree::GetFirstChild(Vertex_t const currV)
{
  return depGraph([boost::out_edges(currV,depGraph)).first];
}
*/
std::pair<OutEdgeIterator_t, OutEdgeIterator_t> MacTree::GetSiblings(Vertex_t const vd)
{
    Vertex_t v = GetParent(vd);
    return boost::out_edges(v, depGraph);
}

std::pair<OutEdgeIterator_t, OutEdgeIterator_t> MacTree::GetChildren(Vertex_t const vd)
{
    return boost::out_edges(vd, depGraph);
}

int MacTree::GetNumNodes() const
{
    return nodeIndex;
}

bool MacTree::IsRoot(Vertex_t const vd) const
{
    return vd == startVertex;
}

//sibling of the currently pointed vertex
bool MacTree::MakeSibling(Node& rn)
{
    DEBUG_TREE(dbgs() << "\nMaking sibling: " << rn.key.get_value() << "\t"
                      << rn.key.get_position().get_line() << "\n";);

    Vertex_t v_dummy{}; //faking

    // @TODO: Replace with smart pointer?
    Node* pn = new Node;
    *pn = rn;
    if(IsRoot(currVertex))
    {
        // @TODO: Remove this.
        throw ExceptionHandler("No sibling for root vertex.\n");
    }
    //this line should be after the previous assignment of *pn=rn !
    Vertex_t u = GetParent(currVertex);
    pn->parent = depGraph[u];
    pn->nodeIndex = ++nodeIndex;
    NodeMap_t::iterator nodeMap_iter;
    //map::insert() returns pair<iterator,bool>
    nodeMap_iter = nodeMap.insert(std::make_pair(pn, v_dummy)).first;
    //add the pointer to node to the multimap
    //and capturing the returned vertex descriptor
    Vertex_t v = boost::add_vertex(nodeMap_iter->first, depGraph);
    nodeMap_iter->second = v; //now assigning the vertex descriptor
    //return if the edge was created or it was already there
    //add edge returns a pair <edge_descriptor,bool>
    //Vertex_t u = GetParent(currVertex);
    bool new_edge = boost::add_edge(u, v, depGraph).second;
    currVertex = nodeMap_iter->second;

    DEBUG_TREE(
            dbgs() << "Node Number: " << depGraph[v]->nodeIndex << "\n";
            dbgs() << "n_ptr: allocated: " << pn << "\tinserted: " << nodeMap_iter->first << "\n";);
    return new_edge;
}

//child to the currently pointed vertex
bool MacTree::MakeChild(Node& rn)
{
    DEBUG_TREE(dbgs() << "\nMaking child: " << rn.key.get_value() << "\t"
                      << rn.key.get_position().get_line() << "\n";);

    Vertex_t v_dummy{}; //faking
    // @TODO: Replace with smart pointer?
    Node* pn = new Node;
    *pn = rn;
    //this line should be after the previous one!
    pn->parent = depGraph[currVertex];
    pn->nodeIndex = ++nodeIndex;
    NodeMap_t::iterator nodeMap_iter;
    nodeMap_iter = nodeMap.insert(std::make_pair(pn, v_dummy)).first;
    Vertex_t v = boost::add_vertex(nodeMap_iter->first, depGraph);
    nodeMap_iter->second = v; //now assigning the vertex descriptor
    //return if the edge was created or it was already there
    //add edge returns a pair <edge_descriptor,bool>
    bool new_edge = boost::add_edge(currVertex, v, depGraph).second;
    currVertex = nodeMap_iter->second;
    DEBUG_TREE(
            dbgs() << "Node Number: " << depGraph[v]->nodeIndex << "\n";
            dbgs() << "n_ptr: allocated: " << pn << "\tinserted: " << nodeMap_iter->first << "\n";);
    return new_edge;
}

bool MacTree::MakeChild(Vertex_t parentV, Vertex_t childV)
{
    currVertex = childV;
    DEBUG_TREE(dbgs() << "\nMaking child\n";);
    //return if the edge was created or it was already there
    return boost::add_edge(parentV, childV, depGraph).second;
}

bool MacTree::MakeSibling(Vertex_t firstV, Vertex_t secondV)
{
    currVertex = secondV;
    DEBUG_TREE(dbgs() << "Making sibling\n";);
    //return if the edge was created or it was already there
    Vertex_t v = GetParent(firstV);
    return boost::add_edge(v, secondV, depGraph).second;
}

void MacTree::PushBackMacro(PPMacro& mac)
{
    DEBUG_TREE(dbgs() << "Pushing Macro: " << mac.get_identifier_str() << "\n";);
    depGraph[currVertex]->PushBackMacro(mac);
    //get the pointer to the macro
    PPMacro* m_ptr = depGraph[currVertex]->vecMacro.back();
    linearOrder.push_back(m_ptr);
    tokenMacroMap.insert(std::pair<token_type, PPMacro*>(m_ptr->get_identifier(), m_ptr));
}

const DepList_t& MacTree::BuildMacroDependencyList()
{
    std::vector<std::pair<std::string, std::string>> missing_macros;

    //putting the macros in macroDepList as they occur
    for(const auto& mp_iter : linearOrder)
    {
        //every loop should have a new instance to do away with emptying
        std::vector<PPMacro*> vec_mp;

        DEBUG_TREE(dbgs() << "Processing Macro: " << mp_iter->get_identifier_str() << "\n";);

        for(const auto& anId : mp_iter->get_replacement_list_dep_idlist())
        {
            auto tmm_iter = tokenMacroMap.find(anId);

            //check if the token was not found
            if(tmm_iter != tokenMacroMap.end())
            {
                vec_mp.push_back(tmm_iter->second);
            }
            else
            {
                const auto line_val = std::to_string(anId.get_position().get_line());
                const auto val = std::string(anId.get_value().c_str());
                missing_macros.emplace_back(std::make_pair(line_val, val));
            }
        }

        macroDepList.push_back(std::make_pair(mp_iter, vec_mp));
    }

    for(const auto& mmacro : missing_macros)
    {
        std::string err_msg = "Exception Line Number: " + mmacro.first
                              + ", No macro found for token: " + mmacro.second + "\n";

        std::cout << err_msg;
    }

    return macroDepList;
}

void MacTree::GotoParent()
{
    DEBUG_TREE(dbgs() << "Going to parent\n";);
    currVertex = GetParent(currVertex);
}

void MacTree::DeleteNodes()
{
    //for each out_edges call the delete
    //vertices(g) returns the pair of vertex iterators
    //the first one points to the first vertex
    //the second points past the end of the last vertex
    //dereferencing the vertex iterator gives the vertex object
    for(auto vp = vertices(depGraph); vp.first != vp.second; ++vp.first)
    {
        delete depGraph[*(vp.first)];
    }
}

bool MacTree::DeleteVertex(Vertex_t v)
{
    //first delete all the macros in the node as they are allocated on the store
    if(startVertex == v)
    {
        return false;
    }
    if(currVertex == v)
    {
        currVertex = GetParent(v); //point to the parent now
    }
    //not deleting the node now,
    //will delete the node during destruction via nodeMap
    boost::clear_vertex(v, depGraph);
    boost::remove_vertex(v, depGraph);
    return true;
}

PairMacroIter_t MacTree::GetMacro(token_type const& tok)
{
    return tokenMacroMap.equal_range(tok);
}

bool MacTree::IsMacro(const token_type& tok)
{
    //if there is atleast a macro with the token_type tok as identifier
    return GetMacro(tok).first != GetMacro(tok).second;
}

//checks the token to get the use case of macro
void MacTree::CheckToken(const token_iterator& tok_iter)
{
    PairMacroIter_t pm_iter = GetMacro(*tok_iter);

    //if there is no macro corresponding to this token

    if(pm_iter.first == pm_iter.second)
    {
        DEBUG_MACRO_USE_CASE(dbgs() << "Putting token into the existing state:\n";);
        //are we collecting tokens in a macro invocation
        if(macroUseCaseState.DoneCollection())
        {
            return;
        }

        {
            macroUseCaseState.PutToken(tok_iter);
        }
    }
    else
    {
        /// @todo this token is a macro we are taking the first entry
        /// of the multimap which keeps macro
        /// *********collecting the args of function like macro **********
        if(pm_iter.first->second->is_function_like())
        {
            macroUseCaseState.PutArgBegin(tok_iter, pm_iter.first->second);
        }
    }
}

//algorithms
void MacTree::Dfs(Vertex_t const startV)
{
    //boost::depth_first_search(depGraph);
}

void MacTree::Bfs(Vertex_t const startV)
{
}

void MacTree::TopSort(std::vector<Graph_t::vertex_descriptor>& topo_order)
{
    boost::topological_sort(depGraph, std::back_inserter(topo_order));
}
