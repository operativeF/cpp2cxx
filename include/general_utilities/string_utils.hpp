#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP
/*
 *  @author Aditya Kumar 
 *
 *  This file is distributed under the MIT License. See 
 *  LICENCE.txt attached with this project or visit
 *  http://www.opensource.org/licenses/mit-license.php 
 *  for terms and conditions.
 */


#include <string>

namespace general_utilities
{
// instead of all these function i can use my mapper
// and then apply the predicate to each character in the string

/// @brief removes all occurences of character c in string str
inline std::string remove_char(const std::string& str, char remove_ch)
{
    std::string s_new = str;

    s_new.erase(std::remove_if(s_new.begin(), s_new.end(),
                        [remove_ch](unsigned char s_c) { return s_c == remove_ch; }),
            s_new.end());

    return s_new;
}

inline std::string keep_alpha_numeric(const std::string& str)
{
    std::string alphanum_str = str;

    alphanum_str.erase(std::remove_if(alphanum_str.begin(), alphanum_str.end(),
                               [](unsigned char c) { return !std::isalnum(c); }),
            alphanum_str.end());

    return alphanum_str;
}

struct SortString
{
    bool operator()(std::string const& s1, std::string const& s2) const
    {
        return s1.compare(s2) < 0;
    }
};

} // namespace general_utilities
#endif // UTILS_STRING_HPP
