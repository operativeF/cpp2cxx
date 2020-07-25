#ifndef UTILS_FILETYPE_HPP
#define UTILS_FILETYPE_HPP

/*
 *  @author Aditya Kumar 
 *
 *  This file is distributed under the MIT License. See 
 *  LICENCE.txt attached with this project or visit
 *  http://www.opensource.org/licenses/mit-license.php 
 *  for terms and conditions.
 */

#include <string_view>

namespace general_utilities
{

static constexpr bool isHeaderFile(std::string_view filename)
{
    return filename.ends_with(".h") || filename.ends_with(".hpp");
}

static constexpr bool isCppFile(std::string_view filename)
{
    return filename.ends_with(".cpp") || filename.ends_with(".cxx") || filename.ends_with(".c");
}

} // namespace general_utilities

#endif // FILETYPE_HPP
