/**
cpp2cxx is an open source software distributed under terms of the
Apache2.0 licence.

Copyrights remain with the original copyright holders.
Use of this material is by permission and/or license.

Copyright [2012] Aditya Kumar, Andrew Sutton, Bjarne Stroustrup

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

#include "cpp2cxx/ValidatorMap.h"
#include "cpp2cxx/ConfigScheme.h"

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

void ConfigScheme::SetFileManagerScheme(const std::vector<std::filesystem::path>& input_files,
        const std::vector<std::filesystem::path>& output_files, const std::vector<std::filesystem::path>& search_paths,
        const std::filesystem::path& input_directory, const std::filesystem::path& output_directory,
        const std::filesystem::path& backup_directory, const std::filesystem::path& cleanup_directory,
        const std::filesystem::path& validator_file, std::ostream* log_file,
        std::ostream* demacrofied_macro_stat_file, std::ostream* macro_stat_file)
{
    pFileManagerScheme.inputFiles = input_files;
    pFileManagerScheme.outputFiles = output_files;
    pFileManagerScheme.searchPaths = search_paths;
    pFileManagerScheme.inputDirectory = input_directory;
    pFileManagerScheme.outputDirectory = output_directory;
    pFileManagerScheme.backupDirectory = backup_directory;
    pFileManagerScheme.cleanup_directory = cleanup_directory;
    pFileManagerScheme.validator_file = validator_file;
    pFileManagerScheme.pLogFile = log_file;
    pFileManagerScheme.pDemacrofiedMacroStatFile = demacrofied_macro_stat_file;
    pFileManagerScheme.pMacroStatFile = macro_stat_file;
}

void ConfigScheme::SetDemacroficationScheme(std::string const& demac_gran,
        std::vector<std::string> const& mac_prev_demac, bool enable_warning,
        std::string const& global_mac_raw, std::string const& global_mac_formatted,
        bool multiple_definitions, bool cleanup)
{
    pDemacroficationScheme.demacroficationGranularity = demac_gran;
    pDemacroficationScheme.enableWarningFlag = enable_warning;
    pDemacroficationScheme.globalMacrosRaw = global_mac_raw;
    pDemacroficationScheme.globalMacrosFormatted = global_mac_formatted;

    std::for_each(mac_prev_demac.begin(), mac_prev_demac.end(), [this](std::string const& str) {
        this->pDemacroficationScheme.macrosPreventingDemacrofication.insert(str);
    });
    
    pDemacroficationScheme.multipleDefinitions = multiple_definitions;
    pDemacroficationScheme.performCleanup = cleanup;

    if(cleanup)
    {
        pDemacroficationScheme.validatorMap.InitValidatorMap(pFileManagerScheme.validator_file.string());
    }
}

void ConfigScheme::SetBuildScheme(std::string const& make_command)
{
    pBuildScheme.makeCommand = make_command;
}

FileManagerScheme& ConfigScheme::GetFileManagerScheme()
{
    return pFileManagerScheme;
}

DemacroficationScheme& ConfigScheme::GetDemacroficationScheme()
{
    return pDemacroficationScheme;
}

BuildScheme& ConfigScheme::GetBuildScheme()
{
    return pBuildScheme;
}
