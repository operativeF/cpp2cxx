// Definitions.

#include "general_utilities/debug.h"

// TODO: Remove this abomination.
std::ostream *dbg_stream = &std::cout;

auto dbgs() -> std::ostream&
{
  return *dbg_stream;
}

void set_dbg_stream(std::ostream &os)
{
  dbg_stream = &os;
}
