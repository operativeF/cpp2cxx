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

#ifndef FILEMANAGERSCHEME_H
#define FILEMANAGERSCHEME_H

#include <iostream>
#include <string>
#include <vector>

/**
*  @struct FileManagerScheme
*  @brief defines the configuration for the FileManager
*/
struct FileManagerScheme
{
    std::ostream* pLogFile;
    std::ostream* pDemacrofiedMacroStatFile;
    std::ostream* pMacroStatFile;
    std::vector<std::string> inputFiles;
    std::vector<std::string> outputFiles;
    std::vector<std::string> searchPaths;
    std::string inputDirectory;
    std::string outputDirectory;
    std::string backupDirectory;
    std::string cleanup_directory;
    std::string validator_file;
};

#endif // FILEMANAGERSCHEME_H
