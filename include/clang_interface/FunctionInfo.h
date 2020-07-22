#ifndef FUNCTION_INFO
#define FUNCTION_INFO


#include "cpp2cxx/MacroScopeClassifier.h"
#include "general_utilities/vector_utils.hpp"

#include <map>
#include <set>
#include <vector>

  struct ParsedDeclInfo{
    std::size_t start_line;
    std::size_t end_line;
  };

  struct CollectedMacroInfo{
    std::size_t defined_line;
    std::vector<std::size_t> invoked_lines;
    PPOperation op;
    MacroCategory m_cat;
    CondCategory c_cat;
    MacroScopeCategory s_cat;
  };
  
  using MacroNameStr = std::string;
  using ASTMacroStat_t = std::map<MacroNameStr, CollectedMacroInfo>;
  using InvocationStat_t = std::multiset<std::size_t>;
  
  //typedef std::multimap<int,MacroName> InvocationStat_t;
#endif // FUNCTION_INFO
