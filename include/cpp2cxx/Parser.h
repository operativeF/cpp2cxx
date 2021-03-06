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

#ifndef PARSER_H
#define PARSER_H

/**
 *  @file Parser.h
 *  @brief reads in the file passed in the constructor,
 *   it then parses the macros, collects the information about all the macros,
 *   and maintains the information,
 *   provides as interface to the demacrofier,
 *   dependency analyzer etc.
 *   and finally generates the demacrofied file.
 *  @version 1.0
 *  @author Aditya Kumar
 *  @note
 *  compiles with g++-4.5 or higher,
 *  for compiling pass -std=c++0x to the compiler
 */

#include "CondParser.h"
#include "Demacrofier.h"
#include "DepGraph.h"
#include "MacroStat.h"
#include "RlParser.h"
#include "clang_interface/FunctionInfo.h"

#include <iosfwd> //for void Parser::GetDemacrofiedFile(std::ostream os)
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


// @TODO: Replace std::map
using MacroList_t = std::map<std::string, //identifier
        std::string>;                     //replacement text

/** forward declaration
 * @struct DemacroficationScheme
 */
struct DemacroficationScheme;

/** forward declaration
 * @class PPMacro
 */
class PPMacro;

/**
 * @class Parser
 */
class Parser
{

public:
    Parser(DemacroficationScheme const& demacrofication_scheme, std::ostream& log_file,
            std::ostream& macro_list_file);

    void Parse(const std::filesystem::path& file_name);
    void Parse(const std::filesystem::path& file_name, ASTMacroStat_t* p,
            InvocationStat_t* is = nullptr);

    void ParseNewGlobalMacros(const std::string& raw_global_macro_file_name);
    void ReadGlobalMacros(const std::filesystem::path& global_macro_file_name);
    //Parser(std::string instr, position_type pos);

    MacTree const* GetMacTree();
    /** @function Configure
      * for loading the configuration file dynamically
      */
    void Configure(DemacroficationScheme const& demacrofication_scheme);

    void ParseMacros(MacroList_t& macro_list);
    void ParseGlobalMacros();
    void ParseLocalMacros(std::string ifileStr, const position_type& pos);

    bool PPCheckIdentifier(std::string const& id_value) const;
    bool PPCheckIdentifier(std::string const& id_str, MacroList_t const& macro_list) const;
    void GetDemacrofiedFile(std::ostream& os);
    //std::stringstream const& GetDemacrofiedFile();
    void PPAnalyzeMacroDependency(std::ostream& os);
    // when multiple occurences of same macros are allowed multiple_definitions=true
    void Demacrofy(std::ostream& stat, bool multiple_definitions = false);

private:
    void InitializeMacTree();
    void PPDefineHandler(MacroList_t& macro_list, PPMacro& macro_ref);
    std::string PPUndefHandler(MacroList_t& macro_list, PPMacro& macro_ref);
    void PPIfHandler(Node& node);
    void PPIfHandler(Node& node, bool def);
    void PPBuildMacroDependencyList(std::ostream& os);
    static token_iterator GoPastMacro(token_iterator it);
    //std::string FillCondTokens(std::vector<token_type> const& cs);

private:
    /**
     * Friend Classes:
     *     @class CondParser;
     *     @class Demacrofier;
     */
    /// @todo friend to be removed from CondParser
    friend class Demacrofier;

    /// @brief pointer to the map passed by the Overseer class,
    /// this contains information collected by the clang front end.
    ASTMacroStat_t* pASTMacroStat{ nullptr };

    /// @brief contains line numbers whre the macros are invoked
    InvocationStat_t* pInvocationStat{ nullptr };

    /**
     * Variables initialized in the body of the constructor
     *     @var token_iterator it;//current token
     *     @var token_iterator it_begin;//first token
     *     @var token_iterator it_end;//one past the last token
     */

    std::unique_ptr<CondParser> cp;
    std::unique_ptr<RlParser> rp;
    std::unique_ptr<Demacrofier> demac;

    token_iterator it;       //current token
    token_iterator it_begin; //first token
    token_iterator it_end;   //one past the last token
    /**
      * variables to be passed to the constructor to be called by the
      * Overseer class
      */
    /// information for the parser and demacrofier on how to demacrofy
    DemacroficationScheme const* pDemacroficationScheme;
    /// log file to store all the errors and warnings etc.
    std::ostream& logFile;
    /// file to store the complete macro(identifier+repl_text)
    std::ostream& mlFile;
    /**
     * Variables initialized in the initializer list of the constructor
     *     @var int nesting_level;
     *     @var CondCategory condCat;
     *     @var std::string fileGlobalMacros;
     */

    /**
     * @var int nesting_level
     * @details nesting level is 0 for the predefined macros and
     * increases by 1 with each #if style block
     */
    int nesting_level{ 0 };
    /**
     * @details useful for passing around the conditional category(config/local)
     */
    CondCategory condCat{ CondCategory::local };

    /**
     * @var std::string fileGlobalMacros;
     * @details file containing global macros
     */
    std::filesystem::path fileGlobalMacros;

    /**
     * Other Variables:
     * @var std::vector<token_type> condStmt;
     * @var MacroList_t localMacros;
     * @var MacroList_t globalMacros;
     * @var MacTree tree;
     * @var std::stringstream outStream;
     * @var std::string ifileStr;
     */
    /// @brief useful for passing around the conditional statement
    std::vector<token_type> condStmt;
    /// @brief list of macros in the current file
    MacroList_t localMacros;
    /// @brief list of predefined macros
    // FIXME: Make this static.
    MacroList_t globalMacros;
    /// @brief make this a pointer to tree and dynamically allocate
    MacTree* pTree{ nullptr };
    /// @brief demacrofied file
    std::stringstream outStream;
    /// @brief the input file contents
    std::string ifileStr;
    /// @brief statistics
    std::size_t macro_count{ 0 };
    std::size_t object_like_count{ 0 };
    std::size_t function_like_count{ 0 };
    std::vector<MacroStat> vec_macro_stat;
};

#endif /*PARSER_H*/
