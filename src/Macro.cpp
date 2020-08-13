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

// @TODO: Replace with smart pointer
PPMacro::PPMacro(std::ostream& log_file)
        : identifier(boost::wave::T_UNKNOWN),
          m_cat(MacroCategory::none),
          logFile(log_file),
          use_case_set(false)
{
}

void PPMacro::set_identifier(const token_type& tok)
{
    identifier = tok;
    //    std::cout<<"identifier: "<<(*it).get_value()<<std::endl;
}

void PPMacro::set_identifier_parameters(const token_type& tok, unsigned int parameter_count)
{
    //token mytoken(it);
    //std::cout<<"identifier_parameter iD: "<<tok.get_name()<<std::endl;
    identifier_parameters.emplace_back(token_instances(tok, parameter_count));
}

void PPMacro::set_identifier_str(const std::string& ident_str)
{
    identifier_str = ident_str;
    //    std::cout<<"iden_string: "<<identifier_str<<std::endl;
}

void PPMacro::set_replacement_list(const token_type& tok)
{
    rep_list.set_replacement_list(tok);
    //ReplacementList.push_back(*it);
    //    std::cout<<"ReplacementList: "<<(*it).get_value()<<std::endl;
}

void PPMacro::set_replacement_list_str(const std::string& replacement_str, RlParser& rl_parser)
{
    rep_list.set_replacement_list_str(replacement_str, identifier_parameters);
    //    std::cout<<"repl_string: "<<replacement_list_str<<std::endl;
    fmt::print(logFile, " - log: parsing replacement list: '{}' of macro: {}\n", replacement_str,
            identifier.get_value());
    set_replacement_list_category(rl_parser);
}


void PPMacro::set_operation(PPOperation op)
{
    operation = op;
}


void PPMacro::set_macro_category(MacroCategory cat)
{
    m_cat = cat;
}


void PPMacro::set_replacement_list_category(RlParser& rl_parser)
{
    rep_list.set_replacement_list_category(rl_parser);
}


void PPMacro::set_conditional_category(CondCategory ccat)
{
    condCat = ccat;
}


void PPMacro::set_macro_scope_category(MacroScopeCategory scat)
{
    m_scat = scat;
}

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
    if(vec_string.size() != identifier_parameters.size())
    {
        fmt::print(logFile,
                "  - log: definition and invocation does not have equal number of args,"
                " perhaps multiple definition of same macro.\n"
                "  - log: Not demacrofying: {}\n",
                identifier_str);
        rep_list.get_replacement_list_token_type().reject_type = true;
    }

    if(!use_case_set)
    {
        invoArgs = vec_string;
        use_case_set = true;
    }
}

void PPMacro::AnalyzeIdentifier() const
{
    if(bool contains_lower_case = HasLowerCase(), begins_with_underscore = HasLeadingUnderscore();
            contains_lower_case || begins_with_underscore)
    {
        std::string warning_msg = fmt::format("  - line number: {}\t: {}\n",
                identifier.get_position().get_line(), identifier_str);

        if(contains_lower_case)
        {
            warning_msg += "  - warning: lower case letters:\n";
        }

        if(begins_with_underscore)
        {
            warning_msg += "  - warning: leading underscore(s):\n";
        }

        logFile << warning_msg;
        throw ExceptionHandler(warning_msg);
    }
}

bool PPMacro::HasLowerCase() const
{
    return std::any_of(identifier_str.begin(), identifier_str.end(),
            [](unsigned char c) { return std::islower(c) != 0; });
}

bool PPMacro::HasLeadingUnderscore() const
{
    return identifier_str.starts_with("_");
}

token_type PPMacro::get_identifier() const
{
    return identifier;
}

const std::string& PPMacro::get_identifier_str() const
{
    return identifier_str;
}


void PPMacro::dump() const
{
    std::cout << identifier_str;
}

std::string PPMacro::get_replacement_list_str() const
{
    return rep_list.get_replacement_list_str();
}


const std::string& PPMacro::get_formatted_replacement_list_str() const
{
    return rep_list.get_formatted_replacement_list_str();
}


const std::string& PPMacro::get_replacement_list_str_with_comments() const
{
    return rep_list.get_replacement_list_str_with_comments();
}


const vpTokInt& PPMacro::get_identifier_parameters() const
{
    return identifier_parameters;
}


ReplacementList& PPMacro::get_replacement_list()
{
    return rep_list;
}


const ReplacementList& PPMacro::get_replacement_list() const
{
    return rep_list;
}


PPOperation PPMacro::get_operation() const
{
    return operation;
}


MacroCategory PPMacro::get_macro_category() const
{
    return m_cat;
}


RlDCat PPMacro::get_replacement_list_dependency_category() const
{
    return rep_list.get_replacement_list_dependency_category();
}


RlCCat PPMacro::get_replacement_list_closure_category() const
{
    return rep_list.get_replacement_list_closure_category();
}


CondCategory PPMacro::get_conditional_category() const
{
    return condCat;
}


std::vector<token_type> PPMacro::get_replacement_list_idlist() const
{
    return rep_list.get_replacement_list_idlist();
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

const std::vector<std::string>& PPMacro::get_use_case_string() const
{
    return invoArgs;
}

MacroScopeCategory PPMacro::get_macro_scope_category() const
{
    return m_scat;
}
//comparison is based on the token id's to determine their equivalence

bool PPMacro::IsEquivalent(const std::pair<token_iterator, token_iterator>& token_iter_range)
{
    return true;
    /*
  typedef std::vector<token_type>::const_iterator tok_iter;
  token_iterator first = token_iter_range.first;
  token_iterator second = token_iter_range.second;
  tok_iter this_mac_tok_iter = macro_tokens.begin();
  tok_iter this_mac_tok_iter_end = macro_tokens.end();

  for( ;(this_mac_tok_iter!=this_mac_tok_iter_end) && (first != second);
      ++first,++this_mac_tok_iter) {

    if(boost::wave::token_id(*first) != boost::wave::token_id(*this_mac_tok_iter))
      return false;
  }
return true;
*/
}

void PPMacro::SetMacroStat()
{
    m_stat.id_string = identifier_str;
    m_stat.rep_list = rep_list.get_replacement_list_str();
    m_stat.m_cat = m_cat;
    m_stat.rl_ccat = rep_list.get_replacement_list_closure_category();
    m_stat.rl_dcat = rep_list.get_replacement_list_dependency_category();
}

MacroStat PPMacro::GetMacroStat() const
{
    return m_stat;
}

bool PPMacro::operator==(PPMacro const& mac) const
{ //return this->identifier == mac.identifier;
    return this->identifier_str == mac.identifier_str;
}

/*bool PPMacro::operator==(token_type const& tok) const
{
  return (this->identifier == tok);
}*/


bool PPMacro::operator<(PPMacro const& mac) const
{
    return (this->identifier_str.compare(mac.identifier_str) < 0);
}
