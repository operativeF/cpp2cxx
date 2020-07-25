#ifndef DEMACBOOSTWAVEINCLUDES_H
#define DEMACBOOSTWAVEINCLUDES_H

/**
  *  @file DemacBoostWaveIncludes.h
  *  @brief contains the typedefs included in the files using the boost::wave
  *  library. Also contains the functor to sort the tokens
  *  useful in case where tokens are used as index types in a map
  *  @version 2.0
  *  @author Aditya Kumar, Thomas Figueroa
  */

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
