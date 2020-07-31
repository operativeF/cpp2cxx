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

#include "cpp2cxx/MacroStat.h"
#include "cpp2cxx/Macro.h"

#include <fmt/ostream.h>
#include <iomanip>
#include <ostream>

static unsigned int macro_count = 0;

void PrintMacroStat(std::ostream& aStream, const MacroStat& stat)
{

   fmt::print(aStream, "- macro {:d}:\n - m_id: {}\n - m_cat: {}\n - c_cat: {}\n - d_cat: {}\n",
         macro_count, stat.id_string, stat.m_cat, stat.rl_ccat, stat.rl_dcat);
   ++macro_count;
}

void PrintMacroStats(std::ostream& aStream, const std::vector<MacroStat>& stats)
{
   for(auto&& stat : stats)
   {
      PrintMacroStat(aStream, stat);
   }
}
