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

#ifndef TRACK_MACRO_HPP
#define TRACK_MACRO_HPP

#include "FunctionInfo.h"
#include "cpp2cxx/MacroScopeClassifier.h"
#include "general_utilities/map_utils.hpp"
#include "general_utilities/vector_utils.hpp"

#include <clang/Basic/TokenKinds.h>
#include <clang/Frontend/CompilerInstance.h>

#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>

#include <map>
#include <string>

using namespace general_utilities;
/*
struct CollectedMacroInfo{
  int defined_line;
  std::vector<int> invoked_lines;
  PPOperation op;
  MacroCategory m_cat;
  CondCategory c_cat;
  MacroScopeCategory s_cat;
};
*/
//typedef std::map<std::string, CollectedMacroInfo> MacroStat_t;

namespace clang
{
//std::ostream& operator<<(std::ostream& os, const CollectedMacroInfo& cmi);
class TrackMacro : public PPCallbacks
{
public:
    TrackMacro() : m_istat(NULL)
    {
    }

    // @TODO: Get rid of manual memory usage
    ~TrackMacro()
    {
        delete m_istat;
    }

    /// PPCallback
    void MacroExpands(const Token& MacroNameTok, const MacroInfo* MI,
            SourceRange Range); //, MacroArgs* Args); in old version of 3.1

    /// PPCallback
    void MacroDefined(const Token& MacroNameTok, const MacroDirective* MD);

    /// PPCallback
    bool FileNotFound(StringRef FileName, SmallVectorImpl<char>& RecoveryPath);

    /// if the macro is local to the current file being processed
    bool MacroIsLocal(SourceLocation loc);

    void SetFileName(const std::string& f);

    const std::string& GetFileName();

    void PrintStats();

    void VerifyMacroScopeFast(std::map<std::string, ParsedDeclInfo> const& FunctionInfo);

    void VerifyMacroScope(std::map<std::string, ParsedDeclInfo> const& FunctionInfo);

    /// called everytime the file name is changed
    /// to get the file currently being processed
    void SetLocParams();

    void SetCompilerInstance(const CompilerInstance* p);

    void InitializeMacroInvocationStat();

    /// called by the MyASTConsumer class
    ASTMacroStat_t& GetMacroStat();

    InvocationStat_t* GetInvocationStat();
    //void CollectFunArgs(const MacroInfo* MI, SourceRange Range);
    /*    MacroArgs *ReadFunctionLikeMacroArgs(Token &MacroNameStr,
                                         MacroInfo *MI,
                                         SourceLocation &MacroEnd);
*/
private:
    std::string file_name;
    // contains all the information about macros in a file as collected by clang
    ASTMacroStat_t ASTMacroStat;
    //std::map<std::string, CollectedMacroInfo>ASTMacroStat;
    // contains the line numbers of all the macro invocations in a file
    InvocationStat_t* m_istat;
    const CompilerInstance* pci;
    SourceManager* sm;
};
} // namespace clang
#endif //TRACK_MACRO_HPP
