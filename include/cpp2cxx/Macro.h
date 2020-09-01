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

#ifndef MACRO_HPP
#define MACRO_HPP

#include "MacroScopeClassifier.h"
#include "MacroStat.h"
#include "ReplacementList.h"
#include "RlCategory.h"

#include "DemacBoostWaveIncludes.h"

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

// (c) Copyright
// ALL RIGHTS RESERVED
/**
*  @file Macro.h
*  @brief contains the PPMacro class and various enum classes useful
* for classification of macros into different categories.
*  @version 1.0
*  @author Aditya Kumar
*  @details
*  compiles with g++-4.5 or higher,
*  for compiling pass -std=c++0x to the compiler
*/

/** forward declaration
 * @class MacroStat
 */
struct MacroStat;

/**
 * @class PPMacro
 * The class to keep all the details of a macro
 */
class PPMacro
{
public:
    //for the object like macro 1first == last for the use_case
    // keep only the first use case
    // as a complete list of use case is already returned by clang
    void set_use_case(const std::pair<token_iterator, token_iterator>& token_iter_range);
    void SetUseCaseStr(const std::vector<std::string>& vec_string);

    void dump() const;

    std::vector<token_type> get_replacement_list_dep_idlist() const;

    /// @brief keeps important details about macro for printing to a file
    // TODO: Remove this function. Create a separate struct for stats or create stats that link to macros as a separate data structure.
    void SetMacroStat();

    bool operator==(PPMacro const& mac) const;
    //bool operator==(token_type const& tok) const;
    //no less than operator should be defined but why??
    bool operator<(PPMacro const& mac) const;

    MacroCategory m_cat{ MacroCategory::none }; //function like or object like etc...
    //the complete identifier string including arguments
    std::string identifier_str; // Unique to each macro
    // the macro identifier token
    token_type identifier{ boost::wave::T_UNKNOWN }; // Unique to each macro
    std::pair<token_iterator, token_iterator> use_case;
    //keep the function_like PPMacro's arguments and their position
    vpTokInt identifier_parameters;                // Unique to each macro
    PPOperation operation{ PPOperation::unknown }; //define or undefine etc...
    MacroScopeCategory m_scat;                     // inside function, inside class, etc...
    ReplacementList rep_list;                      // Unique to each macro
    CondCategory cond_cat{ CondCategory::config };
    std::vector<std::string> invoArgs; // Unique to each macro
    bool use_case_set{ false };
    MacroStat m_stat;
};

struct FunctionLike
{
    static std::string demacrofy()
    {
        return "";
    }
};

struct ObjectLike
{
    static std::string demacrofy()
    {
        return "";
    }
};


struct MacroStore
{
    // std::vector<PPMacro<FunctionLike>> func_macros;
    // std::vector<PPMacro<ObjectLike>> obj_macros;
};

template<typename... MacroBunches>
static void demacro(const MacroBunches&... bunches)
{
    // TODO: Transform, instead of for_each, so we can shove the results in another set of vectors.
    (std::for_each(bunches.begin(), bunches.end(), [](auto&& mac){ mac.demacrofy(); }), ...);
}

static bool HasLowerCase(const PPMacro& mac)
{
    return std::any_of(mac.identifier_str.begin(), mac.identifier_str.end(),
            [](unsigned char c) { return std::islower(c) != 0; });
}

static bool HasLeadingUnderscore(const PPMacro& mac)
{
    return mac.identifier_str.starts_with("_");
}

static bool IsFunctionLike(const PPMacro& mac)
{
    return mac.m_cat == MacroCategory::function_like;
}

static bool IsObjLike(const PPMacro& mac)
{
    return mac.m_cat == MacroCategory::object_like;
}

// TODO: Change this behavior. It's misleading, as we're writing to a log here.
// Also, this generally isn't very well written.
static void AnalyzeIdentifier(const PPMacro& mac)
{
    if(const bool contains_lower_case = HasLowerCase(mac),
            begins_with_underscore    = HasLeadingUnderscore(mac);
            contains_lower_case || begins_with_underscore)
    {
        std::string warning_msg = fmt::format("  - line number: {}\t: {}\n",
                mac.identifier.get_position().get_line(), mac.identifier_str);

        if(contains_lower_case)
        {
            warning_msg += "  - warning: lower case letters:\n";
        }

        if(begins_with_underscore)
        {
            warning_msg += "  - warning: leading underscore(s):\n";
        }
        // TODO: Store to a container and print out at once?
        //fmt::print(mac.logFile, "{}", warning_msg);
        //throw ExceptionHandler(warning_msg);
    }
}

/**
 * @struct MacroOrder
 * @details Used to sort the macros in a map, the sorting is based on pointers
 * so as to handle multiple definitions
 */
struct MacroOrder
{
    bool operator()(const PPMacro* m1, const PPMacro* m2 = NULL) const
    {
        //comparing the pointers to handle multiple definitions
        if(m2 == NULL)
            return true;
        return m1 < m2;
    }
};

#endif /*MACRO_HPP*/
