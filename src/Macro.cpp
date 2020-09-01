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

#include "cpp2cxx/Macro.h"
#include "cpp2cxx/ExceptionHandler.h"
#include "cpp2cxx/MacroStat.h"
#include "cpp2cxx/RlParser.h"

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <string>

// void PPMacro::set_identifier_parameters(const token_type& tok, unsigned int parameter_count)
// {
//     //token mytoken(it);
//     //std::cout<<"identifier_parameter iD: "<<tok.get_name()<<std::endl;
//     identifier_parameters.emplace_back(token_instances(tok, parameter_count));
// }

// void PPMacro::set_replacement_list(const token_type& tok)
// {
//     rep_list.set_replacement_list(tok);
//     //ReplacementList.push_back(*it);
//     //    std::cout<<"ReplacementList: "<<(*it).get_value()<<std::endl;
// }

// void PPMacro::set_replacement_list_str(std::string_view replacement_str, RlParser& rl_parser)
// {
//     rep_list.set_replacement_list_str(replacement_str, identifier_parameters);
//     //    std::cout<<"repl_string: "<<replacement_list_str<<std::endl;
//     fmt::print(logFile, " - log: parsing replacement list: '{}' of macro: {}\n", replacement_str,
//             identifier.get_value());
//     set_replacement_list_category(rl_parser);
// }


// void PPMacro::set_replacement_list_category(RlParser& rl_parser)
// {
//     rep_list.set_replacement_list_category(rl_parser);
// }

void PPMacro::set_use_case(const std::pair<token_iterator, token_iterator>& token_iter_range)
{
    if(!use_case_set)
    {
        use_case = token_iter_range;
    }
}

//capturing only the first macro invocation args
/// @brief if there is a mismatch in the number of arguments
/// then demacrofication is not done for that macro
void PPMacro::SetUseCaseStr(const std::vector<std::string>& vec_string)
{
    // FIXME: How likely is this?
    if(vec_string.size() != identifier_parameters.size())
    {
        // TODO: Store in container and print all at once?
        // fmt::print(logFile,
        //         "  - log: definition and invocation does not have equal number of args,"
        //         " perhaps multiple definition of same macro.\n"
        //         "  - log: Not demacrofying: {}\n",
        //         identifier_str);
        rep_list.get_replacement_list_token_type().reject_type = true;
    }

    if(!use_case_set)
    {
        invoArgs = vec_string;
        use_case_set = true;
    }
}

void PPMacro::dump() const
{
    fmt::print("{}", identifier_str);
}

std::vector<token_type> PPMacro::get_replacement_list_dep_idlist() const
{
    //will contain dependent identifiers in the ReplacementList
    //i.e. those which do not occur as function arguments
    if(m_cat == MacroCategory::null_define)
    {
        return std::vector<token_type>{};
    }

    std::vector<token_type> dep_idlist = rep_list.get_replacement_list_idlist();

    if(m_cat == MacroCategory::object_like)
    {
        return dep_idlist;
    }

    // FIXME: This could stand to be more efficient.
    //iterator over function_like PPMacro's arguments
    //iterator for the ReplacementList tokens
    //remove all those identifiers which are in the function argument
    for(auto&& id : identifier_parameters)
    {
        for(auto&& iter_rl_idlist = dep_idlist.begin(); iter_rl_idlist != dep_idlist.end(); )
        {
            if(id.arg == *iter_rl_idlist)
            {
                //remove the identifiers which are in the function argument
                iter_rl_idlist = dep_idlist.erase(iter_rl_idlist);
                //since the iterators will be invalidated after deletion
            }
            else
            {
                ++iter_rl_idlist;
            }
        }
    }

    return dep_idlist;
}

void PPMacro::SetMacroStat()
{
    m_stat.id_string = identifier_str;
    m_stat.rep_list = rep_list.get_replacement_list_str();
    m_stat.m_cat = m_cat;
    m_stat.rl_ccat = rep_list.get_replacement_list_closure_category();
    m_stat.rl_dcat = rep_list.get_replacement_list_dependency_category();
}

bool PPMacro::operator==(PPMacro const& mac) const
{ //return this->identifier == mac.identifier;
    return this->identifier_str == mac.identifier_str;
}

bool PPMacro::operator<(PPMacro const& mac) const
{
    return (this->identifier_str.compare(mac.identifier_str) < 0);
}
