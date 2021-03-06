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

#include "cpp2cxx/Parser.h"
#include "cpp2cxx/CondParser.h"
#include "cpp2cxx/DemacroficationScheme.h"
#include "cpp2cxx/Demacrofier.h"
#include "cpp2cxx/DepAnalyzer.h"
#include "cpp2cxx/Macro.h"
#include "cpp2cxx/MacroStat.h"
#include "cpp2cxx/RlParser.h"
#include "cpp2cxx/ValidatorMap.h"

#include "general_utilities/vector_utils.hpp"

// Read the file name and then look for macros.
// Take the line and give it to boost_wave for tokenizing.

#include <algorithm>
#include <cassert>
#include <fmt/ostream.h>
#include <fstream>

Parser::Parser(const DemacroficationScheme& demacrofication_scheme, std::ostream& log_file,
        std::ostream& macro_list_file)
        : pDemacroficationScheme(&demacrofication_scheme),
          rp(std::make_unique<RlParser>(demacrofication_scheme, log_file)),
          demac(std::make_unique<Demacrofier>()),
          logFile(log_file),
          mlFile(macro_list_file)
{
    /// TODO: dispatch components of demacrofication_scheme to relevant
    /// classes.
    if(!pDemacroficationScheme->globalMacrosFormatted.empty())
    {
        fileGlobalMacros = pDemacroficationScheme->globalMacrosFormatted;
        ReadGlobalMacros(fileGlobalMacros);
    }
    else
    {
        /// TODO: supply the file name of formatted global macros
        /// from the main function, hardcoding the name here is not a good idea
        /// TODO: check:: previously the file name was gMacros.dat. I've changed it.
        fileGlobalMacros = "gConditions.h";
        ParseNewGlobalMacros(pDemacroficationScheme->globalMacrosRaw);
    }
    // passing the file containing global macros so that it can make a list
    // which is useful in determining if a conditional macro uses global macros
    // FIXME: We're reading the global macros twice by doing this.
    cp = std::make_unique<CondParser>(fileGlobalMacros);
}

void Parser::Parse(const std::filesystem::path& file_name, ASTMacroStat_t* p, InvocationStat_t* is)
{
    assert(p && "AST does not give any information");
    pASTMacroStat = p;
    pInvocationStat = is;
    Parse(file_name);

    //after parsing localMacros contains all the macros parsed
    //putting all the macros in this file
    fmt::print(mlFile,
            "- Total_Macros: {}\n"
            "- Total_Function_Like_Macros: {}\n"
            "- Total_Object_Like_Macros: {}\n",
            macro_count, function_like_count, object_like_count);
    PrintMacroStats(mlFile, vec_macro_stat);
}

/// @todo complete this function
void Parser::Parse(const std::filesystem::path& file_name)
{
    logFile << file_name << ":\n";
    /// TODO: not clearing the list of local macros for a file
    /// and using the existing macros might become useful
    /// to find the macros in a header file included by the the current file
    localMacros.clear();

    std::ifstream instream(file_name, std::ios_base::in);

    if(!instream.is_open())
    {
        logFile << "  - error: " << file_name << " couldn't be opened\n";
        throw ExceptionHandler("Input file could not be opened");
    }

    /// NOTE: the following part is not to be taken into ParseLocalMacros
    /// because the position_type requires the file name
    instream.unsetf(std::ios::skipws);

    ifileStr = std::string(
            std::istreambuf_iterator<char>(instream.rdbuf()), std::istreambuf_iterator<char>());
    //just in case the file has only macros and the
    //replacement list of last macro doesn't find a newline
    // FIXME: Is this necessary?
    ifileStr += "\n";

    position_type pos(file_name.string(), 1Ui64, 1Ui64);

    ParseLocalMacros(ifileStr, pos);
}

//when we don't need to parse the global macros, just read the globalMacros
//from the file formattedGlobalMacroList
//initializing the variable condCat is just for namesake at this point
//because while assigning the condCat the gMacros is checked
void Parser::ReadGlobalMacros(const std::filesystem::path& global_macro_file_name)
{
    //first parse the global macros and put into a file which can be used by
    //conditional parser for looking up the list of global macros
    try
    {
        globalMacros.clear();

        std::ifstream gMacros(global_macro_file_name, std::ios_base::in);

        if(!gMacros.is_open())
        {
            fmt::print(logFile, "  - error: {} could not be opened.\n",
                    global_macro_file_name.string());
            throw ExceptionHandler("global macro file could not be opened");
        }

        // @TODO: Replace with filesystem
        gMacros.seekg(0, std::ios::beg);

        //to ignore the newline at the end of the file
        for(std::string fc, sc; std::getline(gMacros, fc), std::getline(gMacros, sc);)
        {
            globalMacros.insert({ fc, sc });
        }

        gMacros.close();
    }
    catch(ExceptionHandler& e)
    {
        fmt::print(logFile, "  - error: {}\n", e.GetExMessage());
        throw;
    }
    //should be after parsing the global macros
    //passing the file containing global macros so that it can make a list
    //which is useful in determining if a conditional macro uses global macros
    //cp = new CondParser(fileGlobalMacros);
    //demac = new Demacrofier;
}

//when we need to parse the global macros, and write the globalMacros
//into the file formattedGlobalMacroList
void Parser::ParseNewGlobalMacros(const std::string& raw_global_macro_file_name)
{
    //  const char* infile = "gConditions.h";
    globalMacros.clear();

    std::ifstream gmstream(raw_global_macro_file_name, std::ios_base::in);

    if(!gmstream.is_open())
    {
        throw ExceptionHandler("file: " + raw_global_macro_file_name + " couldn't be opened\n");
    }

    gmstream.unsetf(std::ios::skipws);

    std::string instr = std::string(
            std::istreambuf_iterator<char>(gmstream.rdbuf()), std::istreambuf_iterator<char>());
    //just in case the file has only macros and the
    //replacement list of last macro doesn't find a newline
    instr += "\n";

    position_type pos(raw_global_macro_file_name, 1Ui64, 1Ui64);

    it = token_iterator(instr.begin(), instr.end(), pos,
            boost::wave::language_support(
                    boost::wave::support_cpp | boost::wave::support_option_long_long));

    it_begin = it;

    it_end = token_iterator();

    //first parse the global macros and put into a file which can be used by
    //conditional parser for looking up the list of global macros
    try
    {
        //writing into fileGlobalMacros
        ParseMacros(globalMacros);

        std::ofstream gMacros(fileGlobalMacros, std::ios_base::out);

        if(!gMacros.is_open())
        {
            throw ExceptionHandler("file: " + fileGlobalMacros.string() + " couldn't be opened\n");
        }

        std::for_each(globalMacros.begin(), globalMacros.end(),
                [&gMacros](const std::pair<std::string, std::string>& gm) {
                    gMacros << gm.first << "\n" << gm.second << "\n";
                });

        gMacros.close();
    }
    catch(ExceptionHandler& e)
    {
        logFile << "  - error: " << e.GetExMessage() << "\n";
        throw;
    }
}

void Parser::ParseMacros(MacroList_t& macro_list)
{
    InitializeMacTree();
    token_iterator ti_mac_begin;
    token_iterator ti_mac_end;
    bool PPDEFINED = true;
    bool PPNOTDEFINED = false;

    while(it != it_end)
    {
        //so that we get a new tempNode and macro everytime
        Node tempNode;
        PPMacro mac;
        condStmt.clear();

        // FIXME: What happens if there is no endif after the ifs?
        switch(auto id = boost::wave::token_id(*it); id)
        {
        case boost::wave::T_PP_DEFINE:
            ti_mac_begin = it;
            PPDefineHandler(macro_list, mac);
            ti_mac_end = it;
            pTree->PushBackMacro(mac);
            break;
        case boost::wave::T_PP_IF:
            ++nesting_level;
            tempNode.key = *it;
            PPIfHandler(tempNode);
            pTree->MakeChild(tempNode);
            break;
        case boost::wave::T_PP_IFDEF:
            ++nesting_level;
            tempNode.key = *it;
            PPIfHandler(tempNode, PPDEFINED);
            pTree->MakeChild(tempNode);
            break;
        case boost::wave::T_PP_IFNDEF:
            ++nesting_level;
            tempNode.key = *it;
            PPIfHandler(tempNode, PPNOTDEFINED);
            pTree->MakeChild(tempNode);
            break;
        case boost::wave::T_PP_ELIF:
            tempNode.key = *it;
            PPIfHandler(tempNode);
            pTree->MakeSibling(tempNode);
            break;
        case boost::wave::T_PP_ELSE:
            tempNode.key = *it;
            //since there is no condition only the #else will goto condStmt
            condStmt.push_back(*it);
            tempNode.condStmt = condStmt;
            tempNode.condCat = condCat; //same as of immediate IF block
            pTree->MakeSibling(tempNode);
            break;
        case boost::wave::T_PP_ENDIF:
            tempNode.key = *it;
            //since there is no condition only the #endif will go to condStmt
            condStmt.push_back(*it);
            tempNode.condStmt = condStmt;
            tempNode.condCat = condCat; //same as of immediate IF block
            pTree->MakeSibling(tempNode);
            --nesting_level;
            //logFile<<"nesting level = "<<nesting_level<<"\n";
            if(nesting_level < 0)
            {
                throw ExceptionHandler(*it, "misnesting of macro");
            }
            pTree->GotoParent();
            break; //end the nearest if block
        case boost::wave::T_PP_INCLUDE:
        case boost::wave::T_PP_QHEADER:
        case boost::wave::T_PP_HHEADER:
        case boost::wave::T_PP_ERROR:
            //FIXME: what happens if #error is followed by #define in the same line
        case boost::wave::T_PP_LINE:
        case boost::wave::T_PP_PRAGMA:
            break;
        case boost::wave::T_PP_UNDEF:
            PPUndefHandler(macro_list, mac);
            //create a new macro object which has undef as the operation
            pTree->PushBackMacro(mac);
            break;
        case boost::wave::T_PP_WARNING:
            //FIXME: what happens if #error is followed by #define in the same line
            break;
        default:
            //logFile<<"  - log: checking token: "<<it->get_value()<<"\n";
            pTree->CheckToken(it);
            break; //goto next line of the code
        }

        it++;
    }
}

bool Parser::PPCheckIdentifier(std::string const& id_value) const
{
    return (localMacros.find(id_value) != localMacros.end())
           || (globalMacros.find(id_value) != localMacros.end());
}

bool Parser::PPCheckIdentifier(std::string const& id_str, MacroList_t const& macro_list) const
{
    return macro_list.find(id_str) != macro_list.end();
}

// FIXME: Change this to a factory-like function.
void Parser::PPDefineHandler(MacroList_t& macro_list, PPMacro& macro_ref)
{
    //PPMacro macro_ref;
    ++macro_count;
    macro_ref.operation = PPOperation::define;
    // logFile<<"  - log: in define \n";
    //identifier string
    std::stringstream id_value;
    std::vector<token_type> macro_tokens;
    const token_type dummy_token(boost::wave::T_UNKNOWN);
    boost::wave::token_id id{};
    //get rid of the space
    while((id = *(++it)) == boost::wave::T_SPACE)
    {
        ;
    }
    id_value << it->get_value();
    //keep the identifier token string for later use
    std::string iden_token_str = id_value.str();
    macro_ref.identifier = *it;
    macro_tokens.push_back(*it);
    //move to the next token to check if it is function like PPMacro or not
    it++;
    //set the PPMacro to be object like by default
    MacroCategory m_cat = MacroCategory::object_like;
    id = boost::wave::token_id(*it);
    //left paren just after the identifier means function_like PPMacro
    if(id == boost::wave::T_LEFTPAREN)
    {
        id_value << it->get_value();
        macro_tokens.push_back(*it);
        m_cat = MacroCategory::function_like;
        ++function_like_count;
        unsigned int left_paren_count = 1;
        unsigned int parameter_count = 0;
        unsigned int comma_count = 0;
        //handle function like PPMacro
        while(left_paren_count != 0U)
        {
            //logFile<<"  - log: \nfound token: "<<id_value.str();
            // causes memory leaks
            //id = boost::wave::token_id(*(++it));
            id = *(++it);
            macro_tokens.push_back(*it);
            id_value << it->get_value();

            switch(id)
            {
            case boost::wave::T_NEWLINE:
            case boost::wave::T_SPACE:
                // cout<<"\nfound white_space\n";
                break;
            case boost::wave::T_LEFTPAREN:
                // cout<<"\nfound left paren\n";
                left_paren_count++;
                break;
            case boost::wave::T_RIGHTPAREN:
                //logFile<<"  - log: \nfound right paren\n";
                left_paren_count--;
                if(parameter_count == comma_count && parameter_count != 0)
                {
                    parameter_count++;
                    macro_ref.identifier_parameters.push_back({dummy_token, parameter_count});
                }
                break;
                //read next function argument
            case boost::wave::T_COMMA:
                if(parameter_count == comma_count)
                {
                    parameter_count++;
                    macro_ref.identifier_parameters.push_back({dummy_token, parameter_count});
                }
                comma_count++;
                // cout<<"\nfound comma :: parameter_count "<<parameter_count;
                break;
            case boost::wave::T_IDENTIFIER:
                parameter_count++;
                macro_ref.identifier_parameters.push_back({*it, parameter_count});
                break;
            /// make the MacroCategory as struct with member variables
            /// or think of other scheme because the membership is not
            /// exclusive to one
            case boost::wave::T_ELLIPSIS:
                m_cat = MacroCategory::variadic;
                break;
            default:
                /// to bail out of unforseen situations e.g. when the token is a keyword
                /// it emits an error
                m_cat = MacroCategory::none;
                logFile << "  - error: Parsing token " << id_value.str() << "\n";
                break;
                //throw ExceptionHandler("Error parsing function_like macro");
            }
        }
        it++;
    }
    else
    {
        ++object_like_count;
    }

    macro_ref.identifier_str = id_value.str();

    try
    {
        if(macro_list == localMacros && pDemacroficationScheme->enableWarningFlag)
        {
            AnalyzeIdentifier(macro_ref);
        }
    }
    catch(ExceptionHandler& e)
    {
        logFile << "  - error: " << e.GetExMessage() << "\n";
    }

    //skip spaces until we find the replacement text
    while((id = *it) == boost::wave::T_SPACE)
    {
        it++;
    }

    id = boost::wave::token_id(*it);
    std::stringstream rep_list_str;

    //handle for replacement list with more than one tokens/parameters
    while(id != boost::wave::T_NEWLINE)
    {

        //when c comment or cpp comment stop putting the tokens into
        //replacement text and put a newline at the end of the replacement text
        if(id == boost::wave::T_CONTLINE)
        {
            rep_list_str << "\\\n";
            logFile << "  - log: found contline\n";
        }
        else
        {
            rep_list_str << it->get_value();
        }

        macro_ref.rep_list.set_replacement_list(*it);

        if(id == boost::wave::T_CPPCOMMENT)
        {
            //making a new token to insert into the replacement text
            //because boost::T_CPPCOMMENT = // ... \n
            //token_type token_newline(boost::wave::T_NEWLINE, "\n", it->get_position());
            //macro_ref.set_replacement_list(token_newline);
            break;
        }
        /*
        else if(id == boost::wave::T_CCOMMENT) {
            rep_list_str<<it->get_value();
        }
        else {
            rep_list_str<<it->get_value();
        if(id != boost::wave::T_SPACE)
            macro_ref.set_replacement_list(*it);
        /// @todo contline is not detected
        //       if(id == boost::wave::T_CONTLINE)
        //         logFile<<"found a contline\n";
        }
        */

        //accumulate all the tokens in replacement list in a string
        //id = boost::wave::token_id(*(++it));
        id = *(++it);
    }

    /** The order of following two statements should not be changed
    If the last token was a CPP comment then we insert a token which will
    make replacement string have 1 token, and hence would not pass the
    null_define criteria */
    if(rep_list_str.str().empty())
    {
        m_cat = MacroCategory::null_define;
    }
    //if the last token encountered was not the CPP comment
    //then put the next token i.e. the newline at the end,
    //since for CPP comment the newline token is consumed by the T_CPPCPMMENT
    //and hence the newline token is inserted separately there itself
    if(id != boost::wave::T_CPPCOMMENT)
    {
        macro_ref.rep_list.set_replacement_list(*it);
    }
    /** **********************************************************************/

    // set the scope category of macro as found from the clang front end
    //const std::string iden_token_str = macro_ref.get_identifier().get_value();
    //using the operator [] to read the value may insert a new entry
    if(pASTMacroStat->find(iden_token_str) != pASTMacroStat->end())
    {
        //std::cout<<"setting the scope category for macro: "<<iden_token_str<<"\n";
        macro_ref.m_scat = (*pASTMacroStat)[iden_token_str].s_cat;
    }

    macro_ref.m_cat = m_cat;
    macro_ref.rep_list.set_replacement_list_str(rep_list_str.str(), macro_ref.identifier_parameters);
    macro_ref.rep_list.set_replacement_list_category(*rp);
    //get the list of identifiers from the replacement_list
    //and insert into the localMacros
    //macro_ref.get_replacement_list_idlist();
    macro_list.insert(std::make_pair(id_value.str(), rep_list_str.str()));
    macro_ref.SetMacroStat();
    vec_macro_stat.push_back(macro_ref.m_stat);
}

/**
 * FIXME: not inserting the identifier into the macro_list as of now.
 * look into the macro_list if the macro is predefined or local,
 * or if the macro was even present or not.
 */
// FIXME: This does too many things to the input variables.
// FIXME: Change this to a factory-like function.
std::string Parser::PPUndefHandler(MacroList_t& macro_list, PPMacro& macro_ref)
{
    std::string mac_stmt = it->get_value();
    macro_ref.operation = PPOperation::undef;

    while((*(++it)) == boost::wave::T_SPACE)
    {
        fmt::format_to(std::back_inserter(mac_stmt), "{}", it->get_value());
    }

    fmt::format_to(std::back_inserter(mac_stmt), "{}", it->get_value());
    macro_ref.identifier = *it;
    macro_ref.identifier_str = it->get_value();
    macro_ref.m_cat = MacroCategory::null_define; // no replacement list
    //macro_ref.set_replacement_list_str("");
    return mac_stmt;
}

/** @todo to check how the incomplete expressions are tested
 * what if at the end of the expression there is space instead of new line
 * is taken care of by Match, as we skip all the space everytime a new
 * token is scanned
 */
void Parser::PPIfHandler(Node& node)
{
    //asking the parser to parse the condition statement
    //provide the it positions etc. and put the condition statement into node
    cp->ParseConditions(node, it);
    it = cp->GetTokenPosition();
}

//used when '#ifdef or #ifndef' is called
void Parser::PPIfHandler(Node& node, bool def)
{
    condStmt.push_back(*it);

    while((*(++it)) == boost::wave::T_SPACE)
    {
        condStmt.push_back(*it);
    }

    condStmt.push_back(*it);
    // FIXME: Why don't we just use this function to return a condcat?
    if(PPCheckIdentifier(it->get_value(), globalMacros))
    {
        condCat = CondCategory::config;
    }
    else
    {
        condCat = CondCategory::local;
    }

    node.condStmt = condStmt;
    node.condCat = condCat;

    //not useful as of now
    if(def)
    {
        PPCheckIdentifier(it->get_value());
    }
}

void Parser::ParseLocalMacros(std::string ifileStr, const position_type& pos)
{
    it = token_iterator(ifileStr.begin(), ifileStr.end(), pos,
            boost::wave::language_support(boost::wave::support_cpp
                                          | boost::wave::support_option_long_long
                                          | boost::wave::support_option_emit_contnewlines));
    it_begin = it;
    it_end = token_iterator();

    ParseMacros(localMacros);
}

void Parser::PPAnalyzeMacroDependency(std::ostream& os)
{
    PPBuildMacroDependencyList(os);
}

void Parser::PPBuildMacroDependencyList(std::ostream& os)
{
    std::vector<PPMacro*> v;

    //vector of pairs to retain the order in which they occur
    //also defined in the DepGraph.h
    //typedef std::vector<std::pair<PPMacro*,std::vector<PPMacro*> > > DepList_t;

    DepList_t const& dl = pTree->BuildMacroDependencyList();

    try
    {
        DepAnalyzer<PPMacro> macro_dep(dl);
        macro_dep.MakeGraph();

        macro_dep.DoTopologicalSort(v);
        macro_dep.CheckTotalOrder(os);
    }
    catch(ExceptionHandler& e)
    {
        logFile << "  - error: " << e.GetExMessage() << "\n";
    }
}

void Parser::GetDemacrofiedFile(std::ostream& os)
{
    os << outStream.str();
    outStream.str(std::string());
}

/// @todo when the multiple_definitions variable is already
/// then what is the use of passing it as a separate parameter
// FIXME: Move this function out of Parser.
void Parser::Demacrofy(std::ostream& stat, bool multiple_definitions_allowed)
{
    /// @note put the file name, when putting stat
    stat << it_begin->get_position().get_file() << ":\n";

    // settings for the demacrofier
    demac->SetASTStat(pASTMacroStat);
    demac->SetMacroInvocationStat(pInvocationStat);
    demac->SetValidator(&pDemacroficationScheme->validatorMap.GetValidMacros());

    it = it_begin;

    using TokenMacroMap_t = std::multimap<token_type, PPMacro*>;
    using PairMacroIter_t = std::pair<TokenMacroMap_t::iterator, TokenMacroMap_t::iterator>;

    while(it != it_end)
    {
        std::size_t defn_counter = 0;

        if(auto id = boost::wave::token_id(*it); id == boost::wave::T_PP_DEFINE)
        {
            it++;
            it++; //one for #define and the other for space
            /// find the macro associated with this token and then pass the macro
            /// find the macro corresponding to this identifier from the tree
            /// since the tree returns entries from multimap, we have to
            /// find the exact match by matching the location(position)
            /// FIXME: check if works for object_like macro only
            //  modify for function_like macro--check
            auto pm_iter = pTree->GetMacro(*it);
            /// to be looked for exact macro
            PPMacro* m_ptr = nullptr;

            std::for_each(pm_iter.first, pm_iter.second,
                    [this, &m_ptr, &defn_counter](const std::pair<token_type, PPMacro*>& tm_pair) {
                        ++defn_counter;
                        if(tm_pair.first.get_position() == this->it->get_position())
                        {
                            m_ptr = tm_pair.second;
                        }
                    });

            // if there was no match then throw error, although it is unlikely
            // FIXME: How unlikely?
            if(m_ptr == nullptr)
            {
                std::string err_msg = fmt::format(
                        "  - error: no macro with identifier: {} found.\n", it->get_value());
                throw ExceptionHandler(err_msg);
            }
            /// multiple definitions should not actually matter
            /// because if they are not in the conditionals that means
            /// the programmer has messed up in the first place

            std::string demacrofied_string;

            if(defn_counter > 1 && !multiple_definitions_allowed)
            {
                logFile << "  - error: "
                        << "multiple definitions of macro: \'" << it->get_value()
                        << "\' found, so not demacrofying. "
                        << "to demacrofy set appropriate configuration.\n";
                demacrofied_string = demac->Translate(
                        m_ptr, stat, pDemacroficationScheme->performCleanup, false);
            }
            else
            {
                /// now we have the exact match, so demacrofy
                demacrofied_string =
                        demac->Translate(m_ptr, stat, pDemacroficationScheme->performCleanup, true);
            }

            outStream << demacrofied_string;

            /// @todo call the observer class here for the compilation
            it = GoPastMacro(it);
        }
        else
        {
            //search if there is a postponed demacrofication for this line number

            auto post_demac_iter = demac->readyQueue.find(it->get_position().get_line());

            if(post_demac_iter != demac->readyQueue.end())
            {
                //std::cout<<"\nline number:"<<it->get_position().get_line();
                outStream << post_demac_iter->second;

                // remove that postponed demac string from the readyQueue
                demac->readyQueue.erase(post_demac_iter);
            }
            /// handle postponed demacrofication here
            outStream << it->get_value();
            //std::cout<<"\nPrinting the demacrofied string::\n"<<demacrofied_string<<"\n\n";
        }

        it++;
    }
}

token_iterator Parser::GoPastMacro(token_iterator it)
{
    auto id = boost::wave::token_id(*it);

    while(id != boost::wave::T_NEWLINE)
    {
        if(id == boost::wave::T_CPPCOMMENT || id == boost::wave::T_CCOMMENT)
        {
            return it; //go back to point to the end of macro
        }

        id = *(++it);
    }

    return it; //go back to point to the end of macro
}

MacTree const* Parser::GetMacTree()
{
    assert(pTree);
    return pTree;
}

// FIXME: Memory leak.
void Parser::InitializeMacTree()
{
    if(pTree != nullptr)
    {
        //delete pTree;
        //pTree = NULL;
    }
    pTree = new MacTree;
}