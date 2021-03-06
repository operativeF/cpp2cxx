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

// @TODO: Replace filesystem stuff

#include "cpp2cxx/FileManager.h"
#include "cpp2cxx/ExceptionHandler.h"
#include "cpp2cxx/FileManagerScheme.h"
#include "cpp2cxx/Overseer.h" //observable

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace fs = std::filesystem;

FileManager::FileManager(FileManagerScheme const& fs, DemacroficationScheme const& ds)
        : fileManagerScheme(fs), demacroficationScheme(ds), inputFileIndex(0), outputFileIndex(0)
{
    PrepareMacroStatFile();
    PrepareDemacrofiedMacroStatFile();
}

void FileManager::Configure(FileManagerScheme const& fs)
{
    //fileManagerScheme = fs;
}

const std::vector<fs::path>& FileManager::GetOutputFiles()
{
    return fileManagerScheme.outputFiles;
}

const std::vector<fs::path>& FileManager::GetInputFiles()
{
    return fileManagerScheme.inputFiles;
}

const fs::path& FileManager::GetOutputDirectory()
{
    return fileManagerScheme.outputDirectory;
}

const fs::path& FileManager::GetInputDirectory()
{
    return fileManagerScheme.inputDirectory;
}

//check whether this file is there in the list of output_files or not
void FileManager::UpdateFile(std::ostream& fp, std::string_view file_str)
{
    fp << file_str;
}

void FileManager::UpdateFile(std::string_view file_str)
{
    auto output_file = GetCurrentOutputFile();

    if(fs::exists(output_file))
    {
        std::ofstream fp(output_file, std::ios_base::out);

        if(fp.is_open())
        {
            UpdateFile(fp, file_str);
            fp.close();
        }
        else
        {
            // @TODO: Remove unsafe bounds here.
            std::string error_msg = fmt::format("Could not open the file: {} for writing.",
                    fileManagerScheme.outputFiles[outputFileIndex].string());
            WriteLog(error_msg);
            throw ExceptionHandler(error_msg);
        }
    }
    else
    {
        ///For no output file name output shall be
        ///redirected to the standard output";
        // @TODO: Remove unsafe bounds here.
        WriteLog(
                fmt::format("No output file was found for input file: {}."
                            "Output shall be redirected to the standard output",
                        fileManagerScheme.inputFiles[outputFileIndex].string()));

        UpdateFile(std::cout, file_str);
    }
}

// @TODO: Figure out where to increment the file index.
void FileManager::UpdateFile(Overseer const& overseer)
{
    auto output_file = GetCurrentOutputFile();

    if(fs::exists(output_file))
    {
        std::ofstream fp(output_file, std::ios_base::out);

        if(fp.is_open())
        {
            overseer.WriteOutputFile(fp);
            fp.close();
        }
        else
        {
            /// @brief if output file is null then the output will be
            /// printed on the screen
            // @TODO: Remove unsafe bounds here.
            std::string error_msg = fmt::format(
                    "Could not open the file {} for writing to output.\n"
                    "Output shall be redirected to the standard output",
                    fileManagerScheme.outputFiles[outputFileIndex].string());
            WriteLog(error_msg);
            overseer.WriteOutputFile(std::cout);
        }
    }
    else
    {
        ///For no output file name output shall be
        ///redirected to the standard output";
        // @TODO: Remove unsafe bounds here.
        std::string log_msg = fmt::format(
                "For input file: {} no output file was found.\n"
                "Output shall be directed to the standard output.",
                fileManagerScheme.inputFiles[outputFileIndex].string());
        WriteLog(log_msg);
        overseer.WriteOutputFile(std::cout);
    }
}


fs::path FileManager::GetCurrentOutputFile()
{
    // @TODO: Remove unsafe bounds here.
    auto output_file_name = fileManagerScheme.outputFiles[outputFileIndex];
    // ++outputFileIndex;
    return output_file_name;
}

const std::vector<fs::path>& FileManager::GetSearchPaths() const
{
    return fileManagerScheme.searchPaths;
}

const std::vector<fs::path>& FileManager::GetInputFiles() const
{
    return fileManagerScheme.inputFiles;
}

void FileManager::WriteLog(std::string_view str)
{
    fmt::print(*(fileManagerScheme.pLogFile), "{}\n", str);
}

void FileManager::PrepareDemacrofiedMacroStatFile()
{
    if(demacroficationScheme.performCleanup)
    {
        GetDemacrofiedMacroStatFile()
                << "###################################################################\n"
                   "#This file contains the details of each macro processed by the demacrofier\n"
                   "#The file can be read by any yaml parser\n"
                   "#The format is as follows:\n"
                   "#file-name:"
                   "#  - id: identifier string\n"
                   "###################################################################\n";
    }
    else
    {
        GetDemacrofiedMacroStatFile()
                << "###################################################################\n"
                   "#This file contains the details of each macro processed by the demacrofier\n"
                   "#The file can be read by any yaml parser\n"
                   "#The format is as follows:\n"
                   "#macro<count>\n"
                   "#  - id: identifier string\n"
                   "#  - category: macro_category\n"
                   "#  - header_guard_string: string\n"
                   "###################################################################\n";
    }
}

void FileManager::PrepareMacroStatFile()
{
    GetMacroStatFile()
            << "###################################################################\n"
               "#This file contains the details of each macro present in the files processed\n"
               "#The file can be read by any yaml parser\n"
               "#The format is as follows:\n"
               "#macro<count>\n"
               "#  - m_id : identifier string\n"
               "#  - m_cat: macro_category\n"
               "#  - c_cat: if the replacement text maps to C++ expression then c_cat is complete otherwise partial\n"
               "#  - d_cat: if the replacement text contains free variable(s) then d_cat is dependent otherwise closed\n"
               "###################################################################\n";
}

std::ostream& FileManager::GetLogFile()
{
    return *(fileManagerScheme.pLogFile);
}

std::ostream& FileManager::GetMacroStatFile()
{
    return *(fileManagerScheme.pMacroStatFile);
}

std::ostream& FileManager::GetDemacrofiedMacroStatFile()
{
    return *(fileManagerScheme.pDemacrofiedMacroStatFile);
}
