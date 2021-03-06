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

#ifndef DEMACROFIER
#define DEMACROFIER


/**
 *  @file Demacrofier.h
 *  @brief demacrofies the macros and returns the demacrofied PPMacro as a string
 *  @version 1.0
 *  @author Aditya Kumar
 *  @details
 *  compiles with g++-4.5 or higher,
 *  for compiling pass -std=c++0x to the compiler
 */

#include "clang_interface/FunctionInfo.h"

#include <iosfwd>
#include <map>
#include <optional>
#include <string>

/**
 * @class Parser
 * @brief forward declaration
 */
class Parser;

/**
 * @class PPMacro
 * @brief forward declaration
 */
class PPMacro;

/**
 * @class Demacrofier
 * @brief demacrofies the macro on a per macro basis
 */

// Also defined in ValidatorMap.h
using ValidMacros_t = std::set<std::string>;

class Demacrofier
{
public:
    /// @TODO change pointer to const ref
    /// when cleaning up, the translations are put without any conditionals
    /// The \e Translate function will generate the demacrofication without
    /// any conditionals wrapped around it. The code generated will have
    /// several formatting modifications as compared to the original code.

    /// when cleanup = false, translations are put with #ifdef conditionals
    /// and by default they are disabled; the macros are enabled.
    // @TODO: Replace bools in this function.
    std::string Translate(PPMacro const* m_ptr, std::ostream& stat, bool cleanup, bool demacrofy);

    void SetMacroInvocationStat(InvocationStat_t* pInvocationStat);
    void SetASTStat(ASTMacroStat_t* pASTMacroStat);
    void SetValidator(ValidMacros_t const* v_macros);

private:
    /**
     * #define FXY(X,Y,Z) ((X) + (Y))
     * auto FXY = [](decltype(i) X, decltype(j) Y, decltype(k) Z)
     *  { return ((X)+(Y)); }
     * int i = FXY(i,j,k);
     * @todo if type can be deduced from clang then do it
     * for statement like macros use void function
     */
    // lambda function transformation

    void InsertToReadyQueue(std::string const& macro_iden, std::string const& outstr);
    bool CollectDemacrofiedString(PPMacro const* m_ptr, std::string& demacrofied_str) const;

private:
    // it can change the readyQueue
    friend class Parser;

    // keeps a list of postponed macros according to the line number
    using ReadyQueue_t = std::map<std::size_t, std::string>;

    ReadyQueue_t readyQueue;
    // contains all the line numbers where macros were invoked in current file
    InvocationStat_t* pInvocationStat{ nullptr };
    // containes all the information about macros in current file a/c clang
    ASTMacroStat_t* pASTMacroStat{ nullptr };

    //pointer to the container having all the valid macros
    //to be used only when the cleanup is in process
    ValidMacros_t const* pValidaMacros;
    int count{ 0 };
    // FIXME: Add getter / setter.
public:
    static constexpr std::string_view headerGuard =
            "#if defined(__cplusplus) && defined(__GXX_EXPERIMENTAL_CXX0X__)";
};

std::string DemacrofyFunctionLike(const PPMacro* m_ptr);
std::optional<std::string> GetFunctionClosure(const PPMacro* m_ptr);
std::optional<std::string> GetFunctionArgs(const PPMacro* m_ptr);
std::string GetFunctionBody(const PPMacro* m_ptr);
std::string DemacrofyObjectLike(const PPMacro* m_ptr);
std::string DemacrofyMultipleStatements(const PPMacro* m_ptr);
std::string DemacrofyStatementType(const PPMacro* m_ptr);
std::string GenerateUniqueMacroSwitch(const PPMacro* m_ptr);
std::string DemacrofyFunctionLikePostponed(const PPMacro* m_ptr);
std::string DemacrofyObjectLikePostponed(const PPMacro* m_ptr);
std::string SuggestTranslation(std::string_view unique_macro_switch,
        std::string_view demacrofied_fstream, std::string_view original_str);
std::string GenerateTranslation(std::string_view macro_iden, std::string_view unique_macro_switch,
        std::string_view demacrofied_fstream);
bool IsDemacrofiable(PPMacro const& mac);

#endif /*DEMACROFIER*/
