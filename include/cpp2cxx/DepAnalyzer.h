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

#ifndef DEPANALYZER_H
#define DEPANALYZER_H

#include "ExceptionHandler.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>


/// @todo handle multiple definitions
/// how does multimap handle Compare functor

/**
 * @class DepAnalyzer
 * analyzes the dependencies between the macros based on the replacement list
 */
template <typename Vertex_t>
class DepAnalyzer
{
private:
    struct VertexOrder
    {
        bool operator()(const Vertex_t* v1, const Vertex_t* v2) const
        {
            //comparing the pointers to handle multiple definitions
            return v1 < v2;
        }
    };
    //for the hash map
    struct VertexEqual
    {
        bool operator()(const Vertex_t* v1, const Vertex_t* v2) const
        {
            return v1->identifier_str == v2->identifier_str;
        }
    };

    // vector of pairs to retain the order in which they occur
    //also defined in the DepGraph.h
    using DepList_t = std::vector<std::pair<Vertex_t*, std::vector<Vertex_t*>>>;

    //also defined in the DepGraph.h
    using Graph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, Vertex_t*>;

    using Vd_t = typename boost::graph_traits<Graph_t>::vertex_descriptor;
    using MapVertexVd_t = std::map<Vertex_t*, Vd_t, VertexOrder>;

public:
    //check if the map has the object type that is less than convertible
    DepAnalyzer(DepList_t const& adjList) : pDepList(&adjList)
    {
        depGraph = pDepList->size();
    }

    void Analyze(DepList_t const& adjList);

    void MakeGraph();
    void DoTopologicalSort(std::vector<Vertex_t*>& v);
    void PrintVertices();
    void PrintEdges();

    void DoTopologicalSort(std::vector<Vd_t>& topo_order);
    void CheckTotalOrder(std::ostream& os);

private:
    void MakeVertices();
    void MakeEdges();

private:
    DepList_t const* pDepList;
    MapVertexVd_t mapVertexVd;
    Graph_t depGraph;
};

template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::Analyze(DepList_t const& adjList)
{
    pDepList = &adjList;
    MakeGraph();
    CheckTotalOrder();
}

/**
  *  @brief  calls the functions MakeVertices() and MakeEdges()
  *  to generate the graph
  *  @param  none
  *  @return void
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::MakeGraph()
{
    MakeVertices();
    MakeEdges();
}

/**
  *  @brief  topological sorting for dependency analysis
  *  @param  v vector of pointer to vertices in which the topologically sorted
  *  vertices will be inserted
  *  @return void
  *
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::DoTopologicalSort(std::vector<Vertex_t*>& v)
{
    //typename is required since the type Vd_t depends upon the type Vertex_t
    typename std::vector<Vd_t> topo_order;
    boost::topological_sort(depGraph, std::back_inserter(topo_order));
    std::for_each(topo_order.begin(), topo_order.end(),
            [&v, this](Vd_t vd) { v.push_back(this->depGraph[vd]); });
}


template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::DoTopologicalSort(std::vector<Vd_t>& topo_order)
{
    boost::topological_sort(depGraph, std::back_inserter(topo_order));
}

/** @todo improve the code by removing the portion of code specific to
  * PPMacro. Generalize the code.
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::CheckTotalOrder(std::ostream& os)
{
    //typename is required since the type Vd_t depends upon the type Vertex_t
    typename std::vector<Vd_t> topo_order;
    typename std::vector<Vd_t>::iterator ordered_vertices_iter;
    typename boost::graph_traits<Graph_t>::edge_iterator ei, ei_end;

    //puts the topologically sorted vertices into topo_order
    DoTopologicalSort(topo_order);
    std::stringstream err_msg;
    //will point to the vertices in the order they were inserted
    typename boost::graph_traits<Graph_t>::vertex_iterator vi, vi_end;
    std::unordered_map<Vertex_t*, int, std::hash<Vertex_t*>, VertexEqual> orig_order_map,
            topo_order_map;

    int i = 0;
    //int num_macros;
    //inserts the macros in hash table with value as integer
    for(std::tie(vi, vi_end) = boost::vertices(depGraph); vi != vi_end; ++vi)
    {
        orig_order_map[depGraph[*vi]] = ++i;
        //std::cout<<depGraph[*vi]->identifier_str<<"\n";
    }
    //num_macros = i;
    i = 0;
    //will point to the vertices in the topological order
    ordered_vertices_iter = topo_order.begin();
    for(; ordered_vertices_iter != topo_order.end(); ordered_vertices_iter++)
    {
        //inserts the macros in hash table with value as integer
        topo_order_map[depGraph[*ordered_vertices_iter]] = ++i;
        //std::cout<<depGraph[*ordered_vertices_iter]->identifier_str<<"\n";
    }

    ordered_vertices_iter = topo_order.begin();
    if(ordered_vertices_iter != topo_order.end())
    {
        os << "  - log: checking the dependency order of macro "
           << depGraph[*ordered_vertices_iter]->identifier_str << "\n";
    }
    for(; ordered_vertices_iter != topo_order.end(); ordered_vertices_iter++)
    {
        //if the vertex in topologically sorted list occurs before(w.r.t. position)
        //the macro in the original list of vertices, then it's okay
        if(orig_order_map[depGraph[*ordered_vertices_iter]]
                > topo_order_map[depGraph[*ordered_vertices_iter]])
        {
            //try to remove get_identifier to the exception handler
            //making sure the processing continues even after an out of order
            //macro is found, ask Andrew

            err_msg << "  - "
                    << depGraph[*ordered_vertices_iter]->identifier.get_position().get_file()
                    << ":"
                    << depGraph[*ordered_vertices_iter]->identifier.get_position().get_line()
                    << ":"
                    << depGraph[*ordered_vertices_iter]
                               ->identifier
                               .get_position()
                               .get_column()
                    << ":\n";
            err_msg << "    - warning: ";
            os << err_msg.str() << "macro '"
               << depGraph[*ordered_vertices_iter]->identifier_str
               << "' is used before it is defined\n";
            err_msg.str(std::string());
            /// set the out_of_order_dependent_type token type, to true
            depGraph[*ordered_vertices_iter]
                    ->rep_list
                    .set_replacement_list_dependency_category(true);
            boost::tie(ei, ei_end) = boost::edges(depGraph);
            //finding the macro which depend upon this out of order macros
            for(; ei != ei_end; ++ei)
            {
                //if the target vertex is the current vertex i.e. we have a match
                if(target(*ei, depGraph) == *ordered_vertices_iter)
                {
                    //print the source vertex
                    os << "    - note: used at: "
                       << depGraph[source(*ei, depGraph)]
                                    ->identifier
                                    .get_position()
                                    .get_file()
                       << ":"
                       << depGraph[source(*ei, depGraph)]
                                    ->identifier
                                    .get_position()
                                    .get_line()
                       << ":"
                       << depGraph[source(*ei, depGraph)]
                                    ->identifier
                                    .get_position()
                                    .get_column()
                       << ":"
                       << " with macro: " << depGraph[source(*ei, depGraph)]->identifier_str
                       << "\n";
                    /// this macro depends upon a macro that is defined after its use
                    depGraph[source(*ei, depGraph)]
                            ->rep_list
                            .set_replacement_list_dependency_category(true);
                }
            }
            //err_msg = depGraph[*ordered_vertices_iter]->identifier_str;
            //std::cerr << "macro: " << err_msg << ": out of order\n";
        }
    }
}


/**
  *  @brief  makes the vertices in the graph
  *  @param  none
  *  @return void
  *  @details
  *  inserts the vertices in the graph as well as in the property map
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::MakeVertices()
{
    DepList_t const& depList = *pDepList;

    //add vertices
    typename boost::graph_traits<Graph_t>::vertex_iterator v_iter;
    v_iter = vertices(depGraph).first;
    typename DepList_t::const_iterator dl_iter;
    dl_iter = depList.begin();
    for(; dl_iter != depList.end(); dl_iter++, v_iter++)
    {
        //vd = boost::add_vertex(map_iter->first,depGraph);
        depGraph[*v_iter] = dl_iter->first;
        mapVertexVd.insert(std::make_pair(dl_iter->first, *v_iter));
    }
}

/**
  *  @brief  makes the edges in the graph
  *  @param  none
  *  @return void
  *  @details adds the edges according to the adjacency list
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::MakeEdges()
{
    DepList_t const& depList = *pDepList;
    /*    DepList_t depList;
    MapVertexVd_t mapVertexVd;
    Graph_t depGraph;*/
    //iterator to the depList mapped_type
    typename std::vector<Vertex_t*>::const_iterator dt_iter;
    //iterator to the depList
    typename DepList_t::const_iterator dl_iter;
    //iterator to the map of vertex to vertex descriptor
    typename MapVertexVd_t::iterator pm_iter = mapVertexVd.begin();
    // @TODO: Initialize these.
    Vd_t u, v; //ordered pair (u,v)
    for(dl_iter = depList.begin(); dl_iter != depList.end(); dl_iter++)
    {
        //find the iterator to the mapVertex where macro is
        pm_iter = mapVertexVd.find(dl_iter->first);
        if(pm_iter != mapVertexVd.end())
        {
            //the vertex with the in edge in the ordered pair (u,v)
            v = pm_iter->second;
        }
        else
        {
            std::string err_msg =
                    "vertex for " + dl_iter->first->identifier_str + " not added\n";
            throw ExceptionHandler(err_msg);
        }
        dt_iter = dl_iter->second.begin();
        for(; dt_iter != dl_iter->second.end(); dt_iter++)
        {
            // the vertex with the out edge in the ordered pair (u,v)
            pm_iter = mapVertexVd.find(*dt_iter);
            if(pm_iter != mapVertexVd.end())
            {
                u = pm_iter->second;
                //for topological_sort insert the edges in opposite direction
                //since this topological_sort uses BFS
                boost::add_edge(v, u, depGraph);
            }
            else
            { //first vertex not found => error
                std::string err_msg = "macro: " + (*dt_iter)->identifier_str + " not found\n";
                throw ExceptionHandler(err_msg);
            }
        }
    }
}

/**
  *  @brief  prints the vertices in the graph
  *  @param  none
  *  @return void
  *  @details uses the identifier string of the macro
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::PrintVertices()
{
    std::stringstream strm;
    typename boost::graph_traits<Graph_t>::vertex_iterator vi, vi_end;

    for(boost::tie(vi, vi_end) = boost::vertices(depGraph); vi != vi_end; ++vi)
    {
        strm << depGraph[*vi]->identifier_str << "\n";
    }
    std::cout << strm.str();
}

/**
  *  @brief  prints the edges in the graph
  *  @param  none
  *  @return void
  *  @details uses the identifier string of the macro
  */
template <typename Vertex_t>
void DepAnalyzer<Vertex_t>::PrintEdges()
{
    std::stringstream strm;
    typename boost::graph_traits<Graph_t>::edge_iterator ei, ei_end;

    for(boost::tie(ei, ei_end) = boost::edges(depGraph); ei != ei_end; ++ei)
    {
        strm << " (" << depGraph[source(*ei, depGraph)]->identifier_str << " , "
             << depGraph[target(*ei, depGraph)]->identifier_str << ")\n";
    }
    std::cout << strm.str();
}

#endif /*DEPANALYZER_H*/

/*
 * method: how to make a boost graph
 * how to add vertices and edges
void Parser::PPBuildMacroDependencyMap(macro const& mac)
{
  token_type macro_id = mac.get_identifier();
  std::vector<token_type> id_list = mac.get_replacement_list_dep_idlist();
  macroAdjList.insert(std::make_pair(macro_id,id_list));
}

//can be used to generate the statistics about macros
//e.g. how many macros are null defines, how many function like, object like
void Parser::PPAnalyzeMacroDependency()
{
  std::multimap<token_type,std::vector<token_type> >::iterator map_iter;
  std::stringstream err_msg;
  //std::cout<<"size of adjList = "<<macroAdjList.size();
  //first argument: out-edge, second argument: backbone of adj list
  typedef boost::adjacency_list<boost::listS, boost::vecS,
                                boost::directedS,token_type> graph;
  graph dep_graph(macroAdjList.size());
  //graph dep_graph;
  typedef boost::graph_traits<graph>::vertex_descriptor Vertex;
  boost::graph_traits<graph>::vertex_iterator vi;
  typedef std::multimap<token_type,Vertex> mapTokenVertex;
  mapTokenVertex tok_to_vd;

//add vertices
  vi = vertices(dep_graph).first;
  map_iter = macroAdjList.begin();
  for( ;map_iter != macroAdjList.end(); map_iter++, vi++) {
    //vd = boost::add_vertex(map_iter->first,dep_graph);
    dep_graph[*vi] = map_iter->first;
    //problem is here
    tok_to_vd.insert(std::make_pair(map_iter->first,*vi));
  }

  mapTokenVertex::iterator iter_tok_to_vd;

//add edges now
  iter_tok_to_vd = tok_to_vd.begin();//not required actually
  Vertex u,v;//the ordered pair (u,v)
  std::vector<token_type>::iterator tok_iter;
  map_iter = macroAdjList.begin();
  for( ; map_iter != macroAdjList.end(); map_iter++) {

    //the vertex with the out edge in the ordered pair (u,v)
    tok_iter = (map_iter->second).begin();
    for( ; tok_iter != (map_iter->second).end(); tok_iter++) {
      iter_tok_to_vd = std::find_if(tok_to_vd.begin(), tok_to_vd.end(),
                      boost::bind(&mapTokenVertex::value_type::first, _1)
                      == map_iter->first);
      if(iter_tok_to_vd != tok_to_vd.end()) {
        //get the vertex desctiptor of first vertex
        u = iter_tok_to_vd->second;

        iter_tok_to_vd = tok_to_vd.begin();//not required actually
        iter_tok_to_vd = std::find_if(tok_to_vd.begin(),tok_to_vd.end(),
                      boost::bind(&mapTokenVertex::value_type::first, _1)
                      == *tok_iter);
        if(iter_tok_to_vd != tok_to_vd.end()) {
          //get the vertex desctiptor of second vertex
          v = iter_tok_to_vd->second;

          //both vertices are found make the edge now
          //in topological_sort the direction of the edges are reversed
          boost::add_edge(u,v,dep_graph);
        }
        else {//second vertex not found => error
          err_msg << "Macro: " << tok_iter->get_value()
                  << " has no previous definition\n";
          throw ExceptionHandler(err_msg.str());
        }
      }
      else {//first vertex not found => error
        err_msg << "Macro: " << (map_iter->first).get_value()
                << " not found\n";
        throw ExceptionHandler(err_msg.str());
      }
    }//iteration over the dependency list of macro in macroAdjList

  }//iteration over the macroAdjList
  //container to hold the totally ordered vertices/macros
  std::vector<graph::vertex_descriptor> topo_order;
  boost::topological_sort(dep_graph,std::back_inserter(topo_order));
}
*/
