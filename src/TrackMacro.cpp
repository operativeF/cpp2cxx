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

#include "clang_interface/TrackMacro.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <map>
#include <string>

// @TODO: Replace with filesystem

std::ostream& operator<<(std::ostream& os, const CollectedMacroInfo& cmi)
{
    os << "::";
    os << cmi.s_cat;
    os << "  ";
    os << cmi.op;
    os << "  ";
    os << cmi.m_cat;
    os << "  ";
    os << cmi.c_cat;
    os << "  ";
    os << cmi.defined_line;
    os << "  ";
    os << cmi.invoked_lines;
    os << "  ";
    return os;
}

namespace clang
{

void TrackMacro::MacroExpands(const Token& MacroNameTok, const MacroInfo* MI, SourceRange Range)
{
    /// Testing if the macro is defined in the same file
    /// so that global macros can be skipped
    if(sm->isInMainFile(MI->getDefinitionLoc()))
    {
        const PresumedLoc presumed = sm->getPresumedLoc(Range.getBegin(), true);

        //NOTE PresumedLoc can be modified by the LINE directive

        ASTMacroStat[MacroNameTok.getIdentifierInfo()->getNameStart()].invoked_lines.push_back(
                presumed.getLine());

        // put the line number of invocation in m_istat
        // i think it is not useful anymore
        m_istat->insert(presumed.getLine());

        //if(MacroIsLocal(Range.getBegin())) {
        if(MI->isFunctionLike())
        {
            /*      std::cout<<"The macro "
               <<MacroNameTok.getIdentifierInfo()->getNameStart()
               <<" is function like\n";      */
        }
    }
    /*
    if(MI->isObjectLike())
      std::cout<<"The macro is object like\n";
    if(MI->isVariadic())
      std::cout<<"The macro is variadic\n";
  }*/
}

/// PPCallback
void TrackMacro::MacroDefined(const Token& MacroNameTok, const MacroDirective* MD)
{
    // TODO: Check MD for nullness?
    const MacroInfo* MI = MD->getMacroInfo();
    //if(MacroIsLocal(MI->getDefinitionLoc())) {
    CollectedMacroInfo cmi;
    if(sm->isInMainFile(MI->getDefinitionLoc()))
    {
        const PresumedLoc presumed = sm->getPresumedLoc(MI->getDefinitionLoc(), true);

        cmi.defined_line = presumed.getLine();
        //std::cout<<"Macro "<<tok::getTokenName(MacroNameTok.getKind())<<" is defined here\n\t";
        if(MI->isFunctionLike())
        {
            cmi.m_cat = MacroCategory::function_like;
            //std::cout<<"The macro is function like: ";
        }
        if(MI->isObjectLike())
        {
            cmi.m_cat = MacroCategory::object_like;
            //std::cout<<"The macro is object like\n";
        }
        if(MI->isVariadic())
        {
            cmi.m_cat = MacroCategory::variadic;
            //std::cout<<"The macro is variadic\n";
        }
        ASTMacroStat[MacroNameTok.getIdentifierInfo()->getNameStart()] = cmi;
    }
}

/// PPCallback
bool TrackMacro::FileNotFound(StringRef FileName, SmallVectorImpl<char>& RecoveryPath)
{
    std::string path = "/usr/include";
    RecoveryPath.append(path.begin(), path.end());
    /*      for(int i = 0; i<path.size(); ++i)
    RecoveryPath.push_back(path[i]);*/
    return true;
}

///if the macro is local to the current file being processed
bool TrackMacro::MacroIsLocal(SourceLocation loc)
{
    /// the source manager can decode the SourceLocation information
    //clang::FileID ID = sm->getFileID(loc);
    //std::cout<<"file id = "<<ID.getHashValue()<<"\n";
    //std::cout<<"Filea name is: "<<file_name;
    return file_name == sm->getBufferName(loc, nullptr);
    //PresumedLoc presumed = sm->getPresumedLoc(loc);
    //if(file_name == presumed.getFilename())
}

void TrackMacro::SetFileName(std::string_view f)
{
    file_name = f;
    //std::cout<<"File name in TrackMacro set to: "<<file_name;
    SetLocParams();
    //InitializeMacroInvocationStat();
}

/// @TODO: Use make_unique
void TrackMacro::InitializeMacroInvocationStat()
{
    //m_istat = new InvocationStat_t;
}

const std::string& TrackMacro::GetFileName()
{
    return file_name;
}

/// called everytime the file name is changed
/// to get the file currently being processed
void TrackMacro::SetLocParams()
{
    sm = &(pci->getSourceManager());
}

void TrackMacro::SetCompilerInstance(const CompilerInstance* p)
{
    pci = p;
}

void TrackMacro::PrintStats()
{
    //using namespace general_utilities;
    std::map<std::string, CollectedMacroInfo>::const_iterator i = ASTMacroStat.begin();
    for(; i != ASTMacroStat.end(); ++i)
    {
        // FIXME: Replace with fmt::print
        // std::cout << i->first;
        // std::cout << "\t";
        // std::cout << i->second;
        // std::cout << "\n";
    }
    //std::cout<<MacroStat;
}

void TrackMacro::VerifyMacroScopeFast(std::map<std::string, ParsedDeclInfo> const& FunctionInfo)
{
    auto fi_iter = FunctionInfo.begin();
    std::map<std::string, CollectedMacroInfo>::iterator ms;
    std::map<int, int> FunctionDefinitionRange;
    std::map<int, int>::iterator fdr_iter;
    const int start = 0;
    const int end = 1;
    /// collect all the start line numbers of function, and sort them
    for(; fi_iter != FunctionInfo.end(); ++fi_iter)
    {
        FunctionDefinitionRange[(fi_iter->second).start_line] = start;
        FunctionDefinitionRange[(fi_iter->second).end_line] = end;
    }
    // adding a dummy entry at the end so that all the entries will
    // have a lower bound
    FunctionDefinitionRange[std::numeric_limits<int>::max()] = start;
    /// compare if the macro is defined after the function start line
    /// if yes then is it before the function end line.
    ms = ASTMacroStat.begin();
    for(; ms != ASTMacroStat.end(); ++ms)
    {
        fdr_iter = FunctionDefinitionRange.lower_bound((ms->second).defined_line);
        // no need to check if fdr_iter is FunctionDefinitionRange.end() or not
        // because of an extra entry

        if(fdr_iter->second == end)
        {
            (ms->second).s_cat = MacroScopeCategory::inside_function;
        }
    }
}

void TrackMacro::VerifyMacroScope(std::map<std::string, ParsedDeclInfo> const& FunctionInfo)
{
    auto fi = FunctionInfo.begin();
    std::map<std::string, CollectedMacroInfo>::iterator ms;
    for(; fi != FunctionInfo.end(); ++fi)
    {
        ms = ASTMacroStat.begin();
        for(; ms != ASTMacroStat.end(); ++ms)
        {
            if((ms->second).defined_line > (fi->second).start_line
                    && (ms->second).defined_line < (fi->second).end_line)
            {
                (ms->second).s_cat = MacroScopeCategory::inside_function;
            }
        }
    }
}

ASTMacroStat_t& TrackMacro::GetMacroStat()
{
    return ASTMacroStat;
}

/// collect all the line numbers where the macros have been invoked in a file
/// the collection will be sorted in a multimap
/// this will help in verifying if two macros are invoked on the same line
/// if the macros are invoked on the same line I assume that they
/// are nested and hence not transform it.

/// return by value, relying on the move semantics of C++11
InvocationStat_t* TrackMacro::GetInvocationStat()
{
    return m_istat;
    /*
  //std::cout<<"Inserting:\n";
  // this will leak make a smart pointer
  InvocationStat_t* is = new InvocationStat_t;
  auto iter = ASTMacroStat.begin();
  for(; iter!=ASTMacroStat.end(); ++iter) {
    /// for each macro collect all the points of invocation
    std::for_each((iter->second).invoked_lines.begin(),
             (iter->second).invoked_lines.end(),
             [&is](int i) {
                  //std::cout<<i<<"\n";
              is->insert(i);
            });
  }
return is;*/
}

} // namespace clang
