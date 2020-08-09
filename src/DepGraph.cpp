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

#include <fmt/core.h>
#include <fmt/format.h>

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

    // FIXME: What if nothing is found here?
    const auto nodeMap_iter = nodeMap.find(pn);
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
    const bool new_edge = boost::add_edge(u, v, depGraph).second;
    currVertex = nodeMap_iter->second;

    return new_edge;
}

//child to the currently pointed vertex
bool MacTree::MakeChild(Node& rn)
{
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
    const bool new_edge = boost::add_edge(currVertex, v, depGraph).second;
    currVertex = nodeMap_iter->second;

    return new_edge;
}

bool MacTree::MakeChild(Vertex_t parentV, Vertex_t childV)
{
    currVertex = childV;
    //return if the edge was created or it was already there
    return boost::add_edge(parentV, childV, depGraph).second;
}

bool MacTree::MakeSibling(Vertex_t firstV, Vertex_t secondV)
{
    currVertex = secondV;
    //return if the edge was created or it was already there
    Vertex_t v = GetParent(firstV);
    return boost::add_edge(v, secondV, depGraph).second;
}

void MacTree::PushBackMacro(PPMacro& mac)
{
    PPMacro* m_ptr = depGraph[currVertex]->PushBackMacro(mac);
    linearOrder.push_back(m_ptr);
    tokenMacroMap.insert({m_ptr->get_identifier(), m_ptr});
}

const DepList_t& MacTree::BuildMacroDependencyList()
{
    //putting the macros in macroDepList as they occur
    for(auto&& mp_iter : linearOrder)
    {
        //every loop should have a new instance to do away with emptying
        std::vector<PPMacro*> vec_mp;

        for(auto&& anId : mp_iter->get_replacement_list_dep_idlist())
        {
            const auto tmm_iter = tokenMacroMap.find(anId);

            //check if the token was not found
            if(tmm_iter != tokenMacroMap.end())
            {
                vec_mp.push_back(tmm_iter->second);
            }
            else
            {
                fmt::print("Exception line number: {}, no macro found for token: {}\n", anId.get_position().get_line(),
                    anId.get_value());
            }
        }

        macroDepList.push_back(std::make_pair(mp_iter, vec_mp));
    }

    return macroDepList;
}

void MacTree::GotoParent()
{
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
    const PairMacroIter_t pm_iter = GetMacro(*tok_iter);

    //if there is no macro corresponding to this token

    if(pm_iter.first == pm_iter.second)
    {
        //are we collecting tokens in a macro invocation
        if(macroUseCaseState.DoneCollection())
        {
            return;
        }
        else
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
