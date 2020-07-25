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

#ifndef TUPLE3_T
#define TUPLE3_T

#include <ostream>

template <typename T1, typename T2, typename T3>
class Tuple3_t
{
public:
    T1 first;
    T2 second;
    T3 third;
    Tuple3_t(T1 const& f, T2 const& s, T3 const& t) : first(f), second(s), third(t)
    {
    }
    void SetFirst(T1 const& f)
    {
        first = f;
    }
    void SetSecond(T2 const& s)
    {
        second = s;
    }
    void SetThird(T3 const& t)
    {
        third = t;
    }
    T1& GetFirst()
    {
        return first;
    }
    T2& GetSecond()
    {
        return second;
    }
    T3& GetThird()
    {
        return third;
    }
};

//3 tuple to be printed in yaml format
template <typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, Tuple3_t<T1, T2, T3> const& t)
{
    os << "  -id: " << t.first << "\n"  //macro identifier
       << "  -rt: " << t.second << "\n" //macro replacement text
       << "  -ct: " << t.third << "\n"; //macro category(object like or function like)
    return os;
}

template <typename T1, typename T2, typename T3, typename T4>
class Tuple4_t
{
public:
    T1 first;
    T2 second;
    T3 third;
    T4 fourth;
    Tuple4_t(T1 const& f, T2 const& s, T3 const& t, T4 const& ft)
            : first(f), second(s), third(t), fourth(ft)
    {
    }
    void SetFirst(T1 const& f)
    {
        first = f;
    }
    void SetSecond(T2 const& s)
    {
        second = s;
    }
    void SetThird(T3 const& t)
    {
        third = t;
    }
    void SetThird(T4 const& ft)
    {
        fourth = ft;
    }
    T1& GetFirst()
    {
        return first;
    }
    T2& GetSecond()
    {
        return second;
    }
    T3& GetThird()
    {
        return third;
    }
    T4& GetFourth()
    {
        return fourth;
    }
};

//4 tuple to be printed in yaml format
template <typename T1, typename T2, typename T3, typename T4>
std::ostream& operator<<(std::ostream& os, Tuple4_t<T1, T2, T3, T4> const& t)
{
    os << t.first << ":\n"
       << "  -id: " << t.second << "\n"  //macro identifier
       << "  -rt: " << t.third << "\n"   //macro replacement text
       << "  -ct: " << t.fourth << "\n"; //macro category(object like or function like)
    return os;
}

#endif /*TUPLE3_T*/
