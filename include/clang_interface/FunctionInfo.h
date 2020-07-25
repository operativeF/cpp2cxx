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

#ifndef FUNCTION_INFO
#define FUNCTION_INFO

#include "cpp2cxx/MacroScopeClassifier.h"
#include "general_utilities/vector_utils.hpp"

#include <map>
#include <set>
#include <vector>

struct ParsedDeclInfo
{
    std::size_t start_line;
    std::size_t end_line;
};

struct CollectedMacroInfo
{
    std::size_t defined_line;
    std::vector<std::size_t> invoked_lines;
    PPOperation op;
    MacroCategory m_cat;
    CondCategory c_cat;
    MacroScopeCategory s_cat;
};

using MacroNameStr = std::string;
using ASTMacroStat_t = std::map<MacroNameStr, CollectedMacroInfo>;
using InvocationStat_t = std::multiset<std::size_t>;

//typedef std::multimap<int,MacroName> InvocationStat_t;
#endif // FUNCTION_INFO
