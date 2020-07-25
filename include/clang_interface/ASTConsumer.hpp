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

#include "FunctionInfo.h"
#include "TrackMacro.hpp"

#include "general_utilities/map_utils.hpp"
#include "general_utilities/vector_utils.hpp"

#include "llvm/Support/Host.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"

#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/Utils.h"

#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"

#include "clang/Parse/ParseAST.h"
#include "clang/Parse/Parser.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>

/******************************************************************************
 *
 *****************************************************************************/
class MyASTConsumer : public clang::ASTConsumer
{

public:
    virtual bool HandleTopLevelDecl(clang::DeclGroupRef d);
    int InitializeCI(clang::CompilerInstance& ci, const std::vector<std::string>& search_paths);

    void DumpContent(const std::string& file_name);
    void PrintSourceLocation(clang::FunctionDecl* fd);
    void PrintSourceLocation(clang::SourceManager& sm, clang::SourceLocation loc);
    void PrintStats();
    void VerifyMacroScope(bool use_fast);
    ASTMacroStat_t GetMacroStat();
    InvocationStat_t* GetInvocationStat()
    {
        return track_macro->GetInvocationStat();
    }

private:
    clang::CompilerInstance* pci;

    //MyASTConsumer *astConsumer;
    /// \brief clang::TrackMacro is defined by me
    /// the delete operator should not be called on track_macro
    /// because it is added to a reference counted Preprocessor class
    clang::TrackMacro* track_macro;

    std::string current_file;

    /// \brief clang::Info is defined by me
    std::map<std::string, ParsedDeclInfo> FunctionInfo;
};

std::ostream& operator<<(std::ostream& os, const ParsedDeclInfo& inf);
