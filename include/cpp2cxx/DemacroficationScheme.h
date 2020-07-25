#ifndef DEMACROFICATIONSCHEME_H
#define DEMACROFICATIONSCHEME_H

/**
  *  @file DemacroficationScheme.h
  *  @brief contains the details which will be used by the Parser class
  *  and other subclasses for configuration
  *  @version 2.0
  *  @author Aditya Kumar, Thomas Figueroa
  */

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
