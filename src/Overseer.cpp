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

#include "cpp2cxx/Overseer.h"
#include "cpp2cxx/DepGraph.h"
#include "cpp2cxx/ExceptionHandler.h"
#include "cpp2cxx/FileManager.h"
#include "cpp2cxx/Parser.h"

#include "general_utilities/file_type.hpp"

#include <fmt/ostream.h>

#include <memory>
#include <string>

Overseer::Overseer(ConfigScheme&& config_scheme)
        : configScheme(config_scheme),
          pFileManager(std::make_unique<FileManager>(
                  GetFileManagerScheme(), GetDemacroficationScheme())),
          pParser(std::make_unique<Parser>(
                  GetDemacroficationScheme(), GetLogFile(), GetMacroStatFile()))
{
}

void Overseer::ConfigureFileManager()
{
    pFileManager->Configure(configScheme.GetFileManagerScheme());
}

/**
  * @function StartProcessing
  * @details processing is to be done in the following steps
  * 1. configure file manager
  * 2. get one file from file manager
  * 3. call the parser
  * 4. get the macro count
  * 5. run the dependency analyzer,print total order
  * 6. call the demacrofier as per the scheme
  * 7. update the demacrofied string to the file manager
  * 8. call the build class
  * 9. goto step2
  * 10. catch any errors
  */

void Overseer::StartProcessing(bool demacrofy)
{   
    /// @todo after demacrofication maintain a list of all the macro
    /// identities so that each identifier can be defined in the make file
    /// and subsequently compiled
    try
    {
        for(auto&& aFile : GetInputFiles())
        {
            GenerateExternalASTHandler(aFile.string());
            //GetInformationFromExternalSource(*v_iter);
            RunParser(aFile.string());

            if(demacrofy)
            {
                RunDemacrofier();
                RunDependencyAnalyzer();
            }
        }
    }
    catch(ExceptionHandler& e)
    {
        //std::cout<<"There was some error in start processing";
        fmt::print(GetLogFile(), "Error: {}\n", e.GetExMessage());
    }
}

const std::vector<std::filesystem::path>& Overseer::GetInputFiles()
{
    return pFileManager->GetInputFiles();
}

void Overseer::RunParser(const std::filesystem::path& log_file_name)
{
    /// pass the data collected from clang front end
    pParser->Parse(log_file_name, &ASTMacroStat, nullptr);
    //pMacTree = pParser->GetMacTree();
}

void Overseer::RunDependencyAnalyzer()
{
    //pass the tree pointer
    //pass the log_file pointer
    //pass a call_back function(observer) which will give the
    //return the relevant error message to the dependency analyser
    //or may be it can only take the relevant error message and put that into
    //log file
    //
    pParser->PPAnalyzeMacroDependency(GetLogFile());
    //
    //dependency analyzer should do the analysis and then put the analysis
    //result into the log_file
}

void Overseer::PrintTotalOrder()
{
    //call the dep_analyzer to print the total order
    //to the file_pointer passed to the dependency_analyzer
}

void Overseer::RunDemacrofier()
{
    //if(one_file_at_a_time)
    //std::string demac_string = demacrofy(tree_ptr,input_file_ptr)
    //else if(one_macro_at_a_time)
    //std::string demac_string = demacrofy(mac_ptr)
    //else invalid option
    pParser->Demacrofy(GetStatFile(), configScheme.GetDemacroficationScheme().multipleDefinitions);
    UpdateFileManager();
}

void Overseer::UpdateFileManager()
{
    //std::cout<<output_file;
    pFileManager->UpdateFile(*this);
}

/// @brief called by the FileManager class to update the file
void Overseer::WriteOutputFile(std::ostream& os) const
{
    pParser->GetDemacrofiedFile(os);
}

std::ostream& Overseer::GetLogFile()
{
    return pFileManager->GetLogFile();
}

std::ostream& Overseer::GetMacroStatFile()
{
    return pFileManager->GetMacroStatFile();
}

std::ostream& Overseer::GetStatFile()
{
    return pFileManager->GetDemacrofiedMacroStatFile();
}

DemacroficationScheme& Overseer::GetDemacroficationScheme()
{
    return configScheme.GetDemacroficationScheme();
}

FileManagerScheme& Overseer::GetFileManagerScheme()
{
    return configScheme.GetFileManagerScheme();
}

void Overseer::GenerateExternalASTHandler(const std::string& filename)
{
    /// generally the function definitions are not in header files. This has been done to make things faster and minimize
    /// clang errors. As there are cases where header files are not self contained e.g. poco-1.4.3p1/XML$ vi src/xmlrole.h
    ///
    if(general_utilities::isHeaderFile(filename))
    {
        return;
    }

    clang::CompilerInstance ci(std::make_shared<clang::PCHContainerOperations>(), nullptr);

    //InvocationStat_t inv_stat;

    pASTConsumer.InitializeCI(ci, pFileManager->GetSearchPaths());

    pASTConsumer.DumpContent(filename);

    //ASTConsumer.PrintStats();
    pASTConsumer.VerifyMacroScope(true);
    ASTMacroStat = pASTConsumer.GetMacroStat();
    //inv_stat = ASTConsumer.GetInvocationStat();
}

/*
void Overseer::GenerateExternalASTHandler()
{
  clang::CompilerInstance *pci = new clang::CompilerInstance;
  pASTConsumer = new MyASTConsumer;
  pASTConsumer->Initialize(*pci);
}

void Overseer::GetInformationFromExternalSource(const std::string& filename)
{
  pASTConsumer->DumpContent(filename);
  pASTConsumer->PrintStats();
  pASTConsumer->VerifyMacroScope();
  delete pASTConsumer;
  pASTConsumer = NULL;
}
*/
