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

#ifndef DEMACROFICATIONSCHEME_H
#define DEMACROFICATIONSCHEME_H

#include "ValidatorMap.h"
#include "general_utilities/string_utils.hpp"

#include <set>
#include <string>

/**
  *  @struct DemacroficationScheme
  *  @brief contains the details which will be used by the Parser class
  *  and other subclasses for configuration
  */
struct DemacroficationScheme
{
    ///@var enableWarningFlag
    ///@details whether to enable the warning or not
    ///to be used instead of the ENABLE_WARNING macro
    ///if this flag is set then check for the struct warningLogFile
    bool enableWarningFlag;
    bool multipleDefinitions;
    bool performCleanup;

    std::set<std::string, general_utilities::SortString> macrosPreventingDemacrofication;

    std::string demacroficationGranularity;
    std::string globalMacrosRaw;
    std::string globalMacrosFormatted;
    ValidatorMap validatorMap;
};

#endif // DEMACROFICATIONSCHEME_H
