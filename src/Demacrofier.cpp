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
#include "general_utilities/string_utils.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// TODO: Remove line continuations that are on their own line.

template <>
struct fmt::formatter<token_instances>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if(it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const token_instances& p, FormatContext& ctx)
    {
        // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
        // ctx.out() is an output iterator to write to.
        return format_to(ctx.out(), "{}", p.param_count);
    }
};

std::string concat_func_arg(const std::vector<token_instances>& the_vec, std::string modifier)
{
    std::vector<std::string> temp_v;

    for(auto&& tin : the_vec)
    {
        temp_v.push_back(fmt::format("_T{}{} {}", tin.param_count, modifier, tin.arg.get_value()));
    }

    return fmt::format("{}", fmt::join(temp_v, ", "));
};

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

// @TODO: Remove the out argument, it's confusing.
// FIXME: Put the postponement inside the macro?
// FIXME: Return the demacrofied_string
bool Demacrofier::CollectDemacrofiedString(PPMacro const* m_ptr, std::string& demacrofied_str) const
{
    bool postponed = false;
    if(IsFunctionLike(*m_ptr))
    {
        if(m_ptr->m_scat == MacroScopeCategory::inside_function)
        {
            demacrofied_str = DemacrofyFunctionLikePostponed(m_ptr);
            postponed = true;
        }
        else
        {
            demacrofied_str = DemacrofyFunctionLike(m_ptr);
        }
    }
    else if(IsObjLike(*m_ptr))
    {
        const RlTokType token_cat = m_ptr->rep_list.get_replacement_list_token_type();
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

//
void Demacrofier::InsertToReadyQueue(std::string const& macro_iden, std::string const& outstr)
{
    // each macro has an entry in the pASTMacroStat
    const auto ast_macro_iter = pASTMacroStat->find(macro_iden);

    if((ast_macro_iter != pASTMacroStat->end()) && !ast_macro_iter->second.invoked_lines.empty())
    {
        auto line_no = ast_macro_iter->second.invoked_lines.front();
        readyQueue.insert(std::make_pair(line_no, outstr));
    }
    //std::cout<<"\nmacro was not found in the ASTConsumer:"<<macro_iden.str();
}

// TODO: Clean up this function a bit.
// Demacrofy and cleanup are global input variables.
// Either make them part of the macro, or make them static const members.
std::string Demacrofier::Translate(
        PPMacro const* m_ptr, std::ostream& stat, bool cleanup, bool demacrofy)
{
    const std::string unique_macro_switch = GenerateUniqueMacroSwitch(m_ptr);

    //in case not demacrofiable return the original_str
    //take the function str and the replacement list from *m_ptr
    const std::string original_str = fmt::format("#define {} {}\n", m_ptr->identifier_str,
            m_ptr->rep_list.get_replacement_list_str_with_comments());

    //during the cleanup phase see if the macro was validated or not
    if(cleanup && (pValidaMacros->find(unique_macro_switch) == pValidaMacros->end()))
    {
        demacrofy = false;
    }

    std::string demacrofied_str;
    bool postponed = false;

    if(IsDemacrofiable(*m_ptr) && demacrofy) // test its transformability
    {
        //if transformable then collect the transformed string
        postponed = CollectDemacrofiedString(m_ptr, demacrofied_str);
    }
    else
    {
        return original_str;
    }
    //endif RlCCat and RlDCat test

    //should be after the previous if stmt,
    // as the previous one tests for boolean demacrofy.
    std::string outstr;
    const std::string macro_iden = m_ptr->identifier.get_value();

    if(cleanup)
    {
        outstr = GenerateTranslation(macro_iden, unique_macro_switch, demacrofied_str);
        fmt::print(stat, "  - id: {}\n", macro_iden);
    }
    else
    {
        outstr = SuggestTranslation(unique_macro_switch, demacrofied_str, original_str);
        fmt::print(stat,
                "  - macro{:0{}}:\n"
                "     -id: {}\n"
                "     - category: {}\n"
                "     - header_guard_string: {}\n",
                count++, sizeof(double), m_ptr->identifier_str,
                fmt::format("{}", m_ptr->m_cat), unique_macro_switch);
    }

    if(postponed)
    {
        InsertToReadyQueue(macro_iden, outstr);
        // TODO: Should an empty string really be returned here?
        return "";
    }

    return outstr;
}


// FIXME: Refactor out repeated code in the functions below.
// FIXME: Create a single Demacrofy that is parameterized on the macro category type.
std::string DemacrofyFunctionLike(PPMacro const* m_ptr)
{
    //TODO: make check for function like macro
    // FIXME: breaks in case when the function doesn't have parameters F(,,,)
    const RlTokType token_cat = m_ptr->rep_list.get_replacement_list_token_type();

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

    std::string arg_str;
    std::string template_arg;

    // FIXME: Use empty base class so we can handle cases like this.
    if(!m_ptr->identifier_parameters.empty())
    {
        arg_str = concat_func_arg(m_ptr->identifier_parameters, "");

        template_arg = fmt::format("template <class _T{}>",
                fmt::join(m_ptr->identifier_parameters, ", class _T"));
    }


    // FIXME: Use names for variables for clarity here.
    // TODO: This is simple substitution and can be improved by type deduction
    // from the tokens. This also means that types are deduced even for functions
    // that don't return anything, which looks a bit nonsensical.
    return fmt::format("{}\nauto {}({}) -> decltype({})\n{{\n return {};\n}}\n", template_arg,
            m_ptr->identifier.get_value(), arg_str, m_ptr->rep_list.get_replacement_list_str(),
            m_ptr->rep_list.get_replacement_list_str());
}

std::string DemacrofyStatementType(PPMacro const* m_ptr)
{
    std::string arg_str;
    std::string template_arg;

    if(!m_ptr->identifier_parameters.empty())
    {
        arg_str = concat_func_arg(m_ptr->identifier_parameters, "&&");

        template_arg = fmt::format("template <class _T{}>",
                fmt::join(m_ptr->identifier_parameters, ", class _T"));
    }

    return fmt::format("{}\nvoid {}({})\n{{\n{};\n}}\n", template_arg,
            m_ptr->identifier.get_value(), arg_str,
            m_ptr->rep_list.get_formatted_replacement_list_str());
}

// TODO: Determine the appropriate variable value / reference / pointer
// somehow.
std::string DemacrofyMultipleStatements(PPMacro const* m_ptr)
{
    std::string arg_str;
    std::string template_arg;

    if(!m_ptr->identifier_parameters.empty())
    {
        arg_str = concat_func_arg(m_ptr->identifier_parameters, "&&");

        template_arg = fmt::format("template <class _T{}>",
                fmt::join(m_ptr->identifier_parameters, ", class _T"));
    }

    return fmt::format("{}\nvoid {}({})\n{}\n", template_arg, m_ptr->identifier.get_value(),
            arg_str, m_ptr->rep_list.get_formatted_replacement_list_str());
}

std::string DemacrofyObjectLike(PPMacro const* m_ptr)
{
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
    // TODO: Determine viability of using static or inline here.
    return fmt::format("constexpr auto {} = {};\n", m_ptr->identifier_str,
            m_ptr->rep_list.get_replacement_list_str());
}

std::string DemacrofyObjectLikePostponed(const PPMacro* m_ptr)
{
    return fmt::format("auto {} = [{}]()->void {{ {}; }};\n", m_ptr->identifier.get_value(),
            GetFunctionClosure(m_ptr).value_or(""), GetFunctionBody(m_ptr));
    // if it already has a semicolon or not
    // if(!(m_ptr->get_replacement_list().get_replacement_list_token_type()).statement_type)
    //  demacrofied_line << ";";
}

// FIXME: This should be part of the macro, or at least a nonmember in the macro.cpp file.
bool IsDemacrofiable(PPMacro const& mac)
{
    bool demacrofiable = false;
    const RlTokType token_cat = mac.rep_list.get_replacement_list_token_type();

    if(mac.rep_list.get_replacement_list_closure_category() == RlCCat::closed)
    {
        const MacroCategory m_cat = mac.m_cat;
        /// @brief no demacrofication for null_define, variadic or other types
        if(m_cat == MacroCategory::object_like)
        {
            demacrofiable = !(token_cat.keyword_type
                              || (token_cat.assignment_type
                                      && (mac.m_scat
                                              != MacroScopeCategory::inside_function))
                              || token_cat.braces_type || token_cat.reject_type
                              || token_cat.special_type || token_cat.unknown_type
                              || token_cat.out_of_order_dependent_type);
        }
        else if(m_cat == MacroCategory::function_like)
        {
            //demacrofy braces type for the function like PPMacro
            demacrofiable = !(token_cat.reject_type || token_cat.unknown_type
                              || token_cat.out_of_order_dependent_type);
            /// @brief if we couldnot capture any use case then
            /// it is not possible to apply the lambda function txform
            if((mac.m_scat == MacroScopeCategory::inside_function)
                    && mac.invoArgs.empty())
            {
                demacrofiable = false;
            }
        }
    }
    //}
    //if(mac.get_replacement_list_dependency_category == RlDCat::independent)

    return demacrofiable;
}

std::string DemacrofyFunctionLikePostponed(const PPMacro* m_ptr)
{
    return fmt::format("auto {} = [{}]({}) {{ return {}; }};\n",
            m_ptr->identifier.get_value(), GetFunctionClosure(m_ptr).value_or(""),
            GetFunctionArgs(m_ptr).value_or(""), GetFunctionBody(m_ptr));
}

std::optional<std::string> GetFunctionClosure(const PPMacro* m_ptr)
{
    if(!m_ptr->get_replacement_list_dep_idlist().empty())
    {
        std::string closure_str;

        for(auto&& dep : m_ptr->get_replacement_list_dep_idlist())
        {
            fmt::format_to(std::back_inserter(closure_str), "&{}", dep.get_value());
        }

        return closure_str;
    }

    return std::nullopt;
}

// TODO: Replace regular (basic) types with their actual
// names vs using decltype. This should be just a matter of
// getting the correct token ID.
std::optional<std::string> GetFunctionArgs(const PPMacro* m_ptr)
{
    if(!m_ptr->identifier_parameters.empty())
    {
        std::vector<std::string> func_args;
        func_args.reserve(m_ptr->invoArgs.size());
        auto ip_iter = m_ptr->identifier_parameters.begin();

        for(auto&& invok : m_ptr->invoArgs)
        {
            auto& part_arg = func_args.emplace_back("decltype(" + invok + ") ");
            part_arg += ip_iter->arg.get_value();
            ip_iter = std::next(ip_iter);
        }

        return fmt::format("{}", fmt::join(func_args, ", "));
    }

    return std::nullopt;
}

std::string GetFunctionBody(const PPMacro* m_ptr)
{
    return m_ptr->rep_list.get_replacement_list_str();
}

std::string GenerateUniqueMacroSwitch(PPMacro const* m_ptr)
{
    // FIXME: This gets called everytime, even for the same file.
    // Store the filename so it gets called only once per file.
    std::string file_name = general_utilities::keep_alpha_numeric(
            m_ptr->identifier.get_position().get_file());

    return fmt::format("USE_{}_{}_{}_{}", m_ptr->identifier.get_value(), file_name,
            m_ptr->identifier.get_position().get_line(),
            m_ptr->identifier.get_position().get_column());
}

std::string SuggestTranslation(std::string_view unique_macro_switch,
        std::string_view demacrofied_fstream, std::string_view original_str)
{
    // FIXME: Not sane access, change this.
    return fmt::format("{} && defined({})\n{}#else\n{}#endif\n\n", Demacrofier::headerGuard,
            unique_macro_switch, demacrofied_fstream, original_str);
}

std::string GenerateTranslation(std::string_view macro_iden, std::string_view unique_macro_switch,
        std::string_view demacrofied_fstream)
{
    return fmt::format("\n/** Demacrofication for the macro {} with unique identifier {}*/\n{}",
            macro_iden, unique_macro_switch, demacrofied_fstream);
}