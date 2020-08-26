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

#ifndef MACROSCOPECLASSIFIER_H
#define MACROSCOPECLASSIFIER_H

#include <ostream>
/**
 * @enum PPOperation
 */
enum class PPOperation
{
    define,
    undef,
    conditional,
    includes,
    pragma,
    warning,
    line,
    error,
    unknown
};

inline std::ostream& operator<<(std::ostream& os, PPOperation const& oper)
{
    switch(oper)
    {
    case PPOperation::define:
        os << "define";
        break;
    case PPOperation::undef:
        os << "undef";
        break;
    case PPOperation::conditional:
        os << "conditional";
        break;
    case PPOperation::includes:
        os << "includes";
        break;
    case PPOperation::pragma:
        os << "pragma";
        break;
    case PPOperation::warning:
        os << "warning";
        break;
    case PPOperation::line:
        os << "line";
        break;
    case PPOperation::error:
        os << "error";
        break;
    default:
        os << "unknown";
        break;
    }
    return os;
}

/**
 * @enum MacroCategory
 */
enum class MacroCategory
{
    none,
    null_define, //#define X i.e. without replacement_list
    object_like,
    function_like,
    variadic
};

inline std::ostream& operator<<(std::ostream& os, MacroCategory const& m_cat)
{
    switch(m_cat)
    {
    case MacroCategory::none:
        os << "none";
        break;
    case MacroCategory::null_define:
        os << "null_define";
        break;
    case MacroCategory::object_like:
        os << "object_like";
        break;
    case MacroCategory::function_like:
        os << "function_like";
        break;
    case MacroCategory::variadic:
        os << "variadic";
        break;
    }
    return os;
}

/**
 * @enum  MacroScopeCategory
 */
enum class MacroScopeCategory
{
    predefined,
    local,
    inside_function,
    inside_class
};

inline std::ostream& operator<<(std::ostream& os, MacroScopeCategory const& m_cat)
{
    switch(m_cat)
    {
        case MacroScopeCategory::predefined:
            os << "predefined\t";
            break;
        case MacroScopeCategory::local:
            os << "local\t";
            break;
        case MacroScopeCategory::inside_function:
            os << "inside_function\t";
            break;
        case MacroScopeCategory::inside_class:
            os << "inside_class\t";
            break;
    }

    return os;
}

/**
 * @enum CondCategory
 */
enum class CondCategory
{
    config,
    local
};
inline std::ostream& operator<<(std::ostream& os, CondCategory const& c_cat)
{
    switch(c_cat)
    {
    case CondCategory::config:
        os << "config";
        break;
    case CondCategory::local:
        os << "local";
        break;
    }
    return os;
}

#endif // MACROSCOPECLASSIFIER_H
