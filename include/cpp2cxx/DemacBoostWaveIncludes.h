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

#ifndef DEMACBOOSTWAVEINCLUDES_H
#define DEMACBOOSTWAVEINCLUDES_H

#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>


#include <iostream>
#include <map>
#include <vector>


using token_type = boost::wave::cpplexer::lex_token<>;
using token_iterator = boost::wave::cpplexer::lex_iterator<token_type>;
using position_type = token_type::position_type;

/**
 * @struct TokenOrder
 * @brief To sort tokens based on their lexical order
 */
struct TokenOrder
{
    bool operator()(const token_type t1, const token_type t2) const
    {
        return t1.get_value() < t2.get_value();
    }
};

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, std::map<T1, T2> const& m)
{
    std::for_each(m.begin(), m.end(), [&os](std::pair<T1, T2> const& elem) {
        os << elem.first << " " << elem.second << "\n";
    });
    return os;
}

inline std::ostream& operator<<(std::ostream& os, token_type const& tok)
{
    os << tok.get_value();
    return os;
}

#endif /*DEMACBOOSTWAVEINCLUDES_H*/
