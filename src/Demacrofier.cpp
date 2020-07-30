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

#include "cpp2cxx/Demacrofier.h"
#include "cpp2cxx/DemacBoostWaveIncludes.h"
#include "cpp2cxx/DepGraph.h"
#include "cpp2cxx/ExceptionHandler.h"
#include "cpp2cxx/Macro.h"
#include "cpp2cxx/Parser.h"
#include "cpp2cxx/RlCategory.h"
#include "general_utilities/debug.h"
#include "general_utilities/string_utils.hpp"

#include <fmt/core.h>

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

Demacrofier::Demacrofier() noexcept
        : headerGuard("#if defined(__cplusplus) && defined(__GXX_EXPERIMENTAL_CXX0X__)"), count(0)
{
}

std::string Demacrofier::Translate(
        PPMacro const* m_ptr, std::ostream& stat, bool cleanup, bool demacrofy)
{
    bool postponed = false;
    std::string demacrofied_str;
    std::stringstream demacrofied_fstream;
    //std::string instr;
    std::string outstr;
    const std::string unique_macro_switch = GenerateUniqueMacroSwitch(m_ptr);
    std::stringstream macro_iden;
    std::stringstream err_msg;

    //in case not demacrofiable return the original_str
    //take the function str and the replacement list from *m_ptr
    std::string original_str = fmt::format("#define {} {} \n", m_ptr->get_identifier_str(),
            m_ptr->get_replacement_list_str_with_comments());

    macro_iden << m_ptr->get_identifier().get_value();

    //during the cleanup phase see if the macro was validated or not
    if(cleanup && (pValidaMacros->find(unique_macro_switch) == pValidaMacros->end()))
    {
        demacrofy = false;
    }

    //test its transformability
    if(IsDemacrofiable(*m_ptr) && demacrofy)
    {
        //if transformable then collect the transformed string
        postponed = CollectDemacrofiedString(m_ptr, demacrofied_str);
        demacrofied_fstream << demacrofied_str;
    }
    else
    {
        return original_str;
    }
    //endif RlCCat and RlDCat test

    //should be after the previous if stmt,
    // as the previous one tests for boolean demacrofy.
    if(cleanup)
    {
        outstr = GenerateTranslation(
                macro_iden.str(), unique_macro_switch, demacrofied_fstream.str());
        stat << "  - id:" << macro_iden.str() << "\n";
    }
    else
    {
        outstr = SuggestTranslation(unique_macro_switch, demacrofied_fstream.str(), original_str);
        stat << "  - macro" << std::setw(sizeof(double)) << std::setfill('0') << count++ << ":\n"
             << "    - id: " << m_ptr->get_identifier_str() << "\n"
             << "    - category: " << m_ptr->get_macro_category() << "\n"
             << "    - header_guard_string: " << unique_macro_switch << "\n";
    }

    if(postponed)
    {
        //std::cout << "\nPutting macro: "
        //          << macro_iden.str()
        //          << ", into ready queue:\n" << outstr;
        InsertToReadyQueue(macro_iden, outstr);
        return "";
    }

    return outstr;
}

std::string Demacrofier::DemacrofyFunctionLike(PPMacro const* m_ptr) const
{
    std::stringstream demacrofied_line;
    std::stringstream template_arg;
    std::stringstream arg_str;
    int parameter_count = 0;
    std::vector<std::pair<token_type, unsigned int>>::const_iterator p_it;
    //TODO: make check for function like macro
    //breaks in case when the function doesn't have parameters F(,,,)
    const RlTokType token_cat = m_ptr->get_replacement_list().get_replacement_list_token_type();

    // F(X) {/**/}
    // F(X) X = X+1
    // F(X) X+Y;
    if(token_cat.braces_type)
    {
        return DemacrofyMultipleStatements(m_ptr);
    }

    if(token_cat.assignment_type || token_cat.statement_type)
    {
        return DemacrofyStatementType(m_ptr);
    }

    if(!m_ptr->get_identifier_parameters().empty())
    {
        //for the first argument in the function like macro
        p_it = m_ptr->get_identifier_parameters().begin();
        parameter_count++;
        template_arg << "template <"
                     << "class _T" << p_it->second;
        if(p_it->first != boost::wave::T_EOF)
        {
            arg_str << "_T" << p_it->second << " " //space
                    << p_it->first.get_value();
        }
        //std::cout<<"the dummy value is "<< (*p_it).first.get_value()<<std::endl;
        //for the 2nd till the last argument in the function like macro
        while(++p_it != m_ptr->get_identifier_parameters().end())
        {
            //  parameter_count++;
            template_arg << ", "; //comma then space
            template_arg << "class _T" << p_it->second;

            arg_str << ", "; //comma then space
            if(p_it->first != boost::wave::T_EOF)
            {
                arg_str << "_T" << p_it->second << " " //space
                        << p_it->first.get_value();
            }
        }
        template_arg << ">";
        demacrofied_line << template_arg.str();
    }
    demacrofied_line << "\nauto " //auto then space
                     << m_ptr->get_identifier().get_value() << "(" << arg_str.str() << ")"
                     << " -> decltype"
                     << "(" << m_ptr->get_replacement_list_str() << ")"
                     << "\n{\n"
                     << "  return " << m_ptr->get_replacement_list_str() << ";\n}\n";

    return demacrofied_line.str();
}

std::string Demacrofier::DemacrofyStatementType(PPMacro const* m_ptr)
{
    std::stringstream demacrofied_line;
    std::stringstream template_arg;
    std::stringstream arg_str;
    int parameter_count = 0;
    std::vector<std::pair<token_type, unsigned int>>::const_iterator p_it;

    if(!m_ptr->get_identifier_parameters().empty())
    {
        //for the first argument in the function like macro
        p_it = m_ptr->get_identifier_parameters().begin();
        parameter_count++;
        template_arg << "template <"
                     << "class _T" << p_it->second;
        if(p_it->first != boost::wave::T_EOF)
        {
            arg_str << "_T" << p_it->second << " && " //space
                    << p_it->first.get_value();
        }

        //for the 2nd till the last argument in the function like macro
        while(++p_it != m_ptr->get_identifier_parameters().end())
        {
            //  parameter_count++;
            template_arg << ", "; //comma then space
            template_arg << "class _T" << p_it->second;

            arg_str << ", "; //comma then space
            if(p_it->first != boost::wave::T_EOF)
            {
                arg_str << "_T" << p_it->second << " && " //space
                        << p_it->first.get_value();
            }
        }

        template_arg << "> "; // '>' then space
        demacrofied_line << template_arg.str();
    }

    demacrofied_line << "\nvoid " //auto then space
                     << m_ptr->get_identifier().get_value() << "(" << arg_str.str() << ")\n{\n"
                     << m_ptr->get_formatted_replacement_list_str()
                     //semicolon has been added because many use cases
                     //do not have semicolon. for the ones which have it doesn't hurt.
                     << ";\n}\n";

    return demacrofied_line.str();
}


std::string Demacrofier::DemacrofyMultipleStatements(PPMacro const* m_ptr)
{
    std::stringstream demacrofied_line;
    std::stringstream template_arg;
    std::stringstream arg_str;
    int parameter_count = 0;
    std::vector<std::pair<token_type, unsigned int>>::const_iterator p_it;

    if(!m_ptr->get_identifier_parameters().empty())
    {
        //for the first argument in the function like macro
        p_it = m_ptr->get_identifier_parameters().begin();
        parameter_count++;
        template_arg << "template <"
                     << "class _T" << p_it->second;
        if(p_it->first != boost::wave::T_EOF)
        {
            arg_str << "_T" << p_it->second << " && " //space
                    << p_it->first.get_value();
        }
        //std::cout<<"the dummy value is "<< (*p_it).first.get_value()<<std::endl;
        //for the 2nd till the last argument in the function like macro
        while(++p_it != m_ptr->get_identifier_parameters().end())
        {
            //  parameter_count++;
            template_arg << ", "; //comma then space
            template_arg << "class _T" << p_it->second;

            arg_str << ", "; //comma then space
            if(p_it->first != boost::wave::T_EOF)
            {
                arg_str << "_T" << p_it->second << " && " //space
                        << p_it->first.get_value();
            }
        }
        template_arg << "> "; // '>' then space
        demacrofied_line << template_arg.str();
    }

    demacrofied_line << "\nvoid " //auto then space
                     << m_ptr->get_identifier().get_value() << "(" << arg_str.str() << ")\n"
                     << m_ptr->get_formatted_replacement_list_str() << "\n";

    return demacrofied_line.str();
}

std::string Demacrofier::DemacrofyObjectLike(PPMacro const* m_ptr)
{
    // @TODO: Check m_ptr for nullness?
    /*  std::stringstream template_arg;
  std::stringstream arg_str;
  //if it is a statement_type&&assignment_type
  //inside a function  --> lambda function
  //outside a function --> void fun()
  if((m_ptr->get_replacement_list().get_replacement_list_token_type()).assignment_type){
   std::list<token_type> dep_list = m_ptr->get_replacement_list_dep_idlist();
   std::list<token_type>::const_iterator dep_list_iter = dep_list.begin();
   int count = 1;
     if(!dep_list.empty()){
       template_arg << "template <"
                    << "class _T"
                    << count;
       if(*dep_list_iter != boost::wave::T_EOF) {
         arg_str << "_T"
                 << count
                 << " && "  //space
                 << dep_list_iter->get_value();
       }
       while(++dep_list_iter != dep_list.end()){
         ++count;
         template_arg << ", "; //comma then space
         template_arg << "class _T"
                      << count;

         arg_str << ", ";//comma then space
         if(*dep_list_iter != boost::wave::T_EOF) {
           arg_str << "_T"
                   << count
                   << " && "  //space
                   << dep_list_iter->get_value();
         }
      }
      template_arg << "> "; // '>' then space
      demacrofied_line << template_arg.str();
    }
     demacrofied_line << "\ninline void "
                      << m_ptr->get_identifier().get_value()
                      << "(" << arg_str.str()<< ")\n{\n"
                      << m_ptr->get_formatted_replacement_list_str()
                      << ";\n}\n";
     return demacrofied_line.str();
  }*/

    // @TODO: do constexpr only for integer literals.
    // figure out how to identify integer literals...
    return fmt::format("constexpr auto {} = {};\n", m_ptr->get_identifier_str(),
            m_ptr->get_replacement_list_str());
}

std::string Demacrofier::DemacrofyObjectLikePostponed(const PPMacro* m_ptr) const
{
    return fmt::format("auto {} = [{}]()->void {{ {}; }};\n",
            m_ptr->get_identifier().get_value().c_str(), GetFunctionClosure(m_ptr),
            GetFunctionBody(m_ptr));
    // if it already has a semicolon or not
    // if(!(m_ptr->get_replacement_list().get_replacement_list_token_type()).statement_type)
    //  demacrofied_line << ";";
}

// @TODO: I don't think this is a sane implementation.
bool Demacrofier::IsDemacrofiable(PPMacro const& mac)
{
    bool demacrofiable = false;
    const RlTokType token_cat = mac.get_replacement_list().get_replacement_list_token_type();
    //std::cout<<"testing..."<<mac.get_identifier_str()<<"\n";
    //if(mac.get_conditional_category() == CondCategory::local) {
    //std::cout<<"condCat: local\n";
    //std::cout<<"RlCCat: closed\n";
    if(mac.get_replacement_list_closure_category() == RlCCat::closed)
    {
        const MacroCategory m_cat = mac.get_macro_category();
        /// @brief no demacrofication for null_define, variadic or other types
        if(m_cat == MacroCategory::object_like)
        {
            if(token_cat.keyword_type
                    || (token_cat.assignment_type
                            && !mac.get_macro_scope_category().inside_function)
                    || token_cat.braces_type || token_cat.reject_type || token_cat.special_type
                    || token_cat.unknown_type || token_cat.out_of_order_dependent_type)
            {
                demacrofiable = false;
            }
            else
            {
                demacrofiable = true;
            }
        }
        else if(m_cat == MacroCategory::function_like)
        {
            //demacrofy braces type for the function like PPMacro
            if(token_cat.reject_type || token_cat.unknown_type
                    || token_cat.out_of_order_dependent_type)
            { // || token_cat.braces_type
                demacrofiable = false;
            }
            else
            {
                demacrofiable = true;
            }
            /// @brief if we couldnot capture any use case then
            /// it is not possible to apply the lambda function txform
            if(mac.get_macro_scope_category().inside_function && mac.get_use_case_string().empty())
            {
                demacrofiable = false;
            }
        }
    }
    //}
    //if(mac.get_replacement_list_dependency_category == RlDCat::independent)

    return demacrofiable;
}

void Demacrofier::SetMacroInvocationStat(InvocationStat_t* stat)
{
    pInvocationStat = stat;
}

void Demacrofier::SetASTStat(ASTMacroStat_t* stat)
{
    pASTMacroStat = stat;
}

void Demacrofier::SetValidator(ValidMacros_t const* v_macros)
{
    pValidaMacros = v_macros;
}

std::string Demacrofier::DemacrofyFunctionLikePostponed(const PPMacro* m_ptr) const
{
    return fmt::format("auto {} = [{}]({}) {{ return {}; }};\n",
            m_ptr->get_identifier().get_value().c_str(), GetFunctionClosure(m_ptr),
            GetFunctionArgs(m_ptr), GetFunctionBody(m_ptr));
}

std::string Demacrofier::GetFunctionClosure(const PPMacro* m_ptr)
{
    std::string closure_str;

    std::vector<token_type> dep_list = m_ptr->get_replacement_list_dep_idlist();
    std::vector<token_type>::const_iterator dep_list_iter = dep_list.begin();

    if(!dep_list.empty())
    {
        // taking all the parameters by reference
        closure_str = "&";
        closure_str += dep_list_iter->get_value().c_str();

        while(++dep_list_iter != dep_list.end())
        {
            closure_str += ", &";
            closure_str += dep_list_iter->get_value().c_str();
        }
    }

    return closure_str;
}

std::string Demacrofier::GetFunctionArgs(const PPMacro* m_ptr)
{
    std::string dtype = "decltype(";
    std::stringstream arg_string;

    auto invok_iter = m_ptr->get_use_case_string().begin();
    // identifier parameters iterator
    auto ip_iter = m_ptr->get_identifier_parameters().begin();
    // @TODO: Make this a lambda
    if(!m_ptr->get_identifier_parameters().empty())
    {
        arg_string << dtype << *invok_iter << ") " << ip_iter->first.get_value();

        while(++ip_iter != m_ptr->get_identifier_parameters().end())
        {
            arg_string << ", " << dtype << *(++invok_iter) << ") " << ip_iter->first.get_value();
        }
    }

    return arg_string.str();
}

std::string Demacrofier::GetFunctionBody(const PPMacro* m_ptr)
{
    return m_ptr->get_replacement_list_str();
}

// @TODO: Use {fmt}
std::string Demacrofier::GenerateUniqueMacroSwitch(PPMacro const* m_ptr)
{
    std::stringstream m_switch;

    // @TODO: This gets called everytime, even for the same file.
    // Store the filename so it gets called only once per file.
    std::string file_name = general_utilities::keep_alpha_numeric(
            m_ptr->get_identifier().get_position().get_file().c_str());

    return fmt::format("USE_{}_{}_{}_{}", m_ptr->get_identifier().get_value().c_str(), file_name,
            m_ptr->get_identifier().get_position().get_line(),
            m_ptr->get_identifier().get_position().get_column());
}

std::string Demacrofier::SuggestTranslation(const std::string_view unique_macro_switch,
        const std::string_view demacrofied_fstream, const std::string_view original_str) const
{
    return fmt::format("{} && defined({})\n{}#else\n{}#endif\n\n", headerGuard, unique_macro_switch,
            demacrofied_fstream, original_str);
}

std::string Demacrofier::GenerateTranslation(const std::string_view macro_iden,
        const std::string_view unique_macro_switch, const std::string_view demacrofied_fstream)
{
    return fmt::format("\n/** Demacrofication for the macro {} with unique identifier {}*/\n{}",
            macro_iden, unique_macro_switch, demacrofied_fstream);
}

void Demacrofier::InsertToReadyQueue(std::stringstream const& macro_iden, std::string const& outstr)
{
    // each macro has an entry in the pASTMacroStat
    const auto ast_macro_iter = pASTMacroStat->find(macro_iden.str());
    if((ast_macro_iter != pASTMacroStat->end()) && !ast_macro_iter->second.invoked_lines.empty())
    {
        // @TODO: Remove unsafe bounds here.
        auto line_no = ast_macro_iter->second.invoked_lines[0];
        readyQueue.insert(std::make_pair(line_no, outstr));
    }
    //std::cout<<"\nmacro was not found in the ASTConsumer:"<<macro_iden.str();
}

// @TODO: Remove the out argument.
bool Demacrofier::CollectDemacrofiedString(PPMacro const* m_ptr, std::string& demacrofied_str) const
{
    bool postponed = false;
    if(m_ptr->is_function_like())
    {
        if(m_ptr->get_macro_scope_category().inside_function)
        {
            demacrofied_str = DemacrofyFunctionLikePostponed(m_ptr);
            postponed = true;
        }
        else
        {
            demacrofied_str = DemacrofyFunctionLike(m_ptr);
        }
    }
    else if(m_ptr->is_object_like())
    {
        const RlTokType token_cat = m_ptr->get_replacement_list().get_replacement_list_token_type();
        // only statement like functions can have the lambda function tx
        if(token_cat.assignment_type || token_cat.statement_type || token_cat.braces_type)
        {
            demacrofied_str = DemacrofyObjectLikePostponed(m_ptr);
            postponed = true;
        }
        else
        {
            demacrofied_str = DemacrofyObjectLike(m_ptr);
        }
    }

    return postponed;
}
