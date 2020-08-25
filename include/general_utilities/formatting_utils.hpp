/**
cpp2cxx is an open source software distributed under terms of the
Apache2.0 licence.

Copyrights remain with the original copyright holders.
Use of this material is by permission and/or license.

Copyright [2020] Thomas Figueroa
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

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <string_view>
#include <type_traits>

namespace fmt
{

// Adds an additional join function that takes a container, a separater,
// and an invocable function / member. The invocable object is called on
// each item in the container.

template <typename It, typename Sentinel, typename Char, typename Func>
struct arg_invoke_join : detail::view
{
    It begin;
    Sentinel end;
    basic_string_view<Char> sep;
    Func func;

    arg_invoke_join(It b, Sentinel e, basic_string_view<Char> s, Func&& f)
            : begin(b), end(e), sep(s), func(std::forward<Func>(f))
    {
    }
};

template <typename It, typename Sentinel, typename Char, typename F>
struct formatter<arg_invoke_join<It, Sentinel, Char, F>, Char>
        : formatter<typename std::iterator_traits<std::add_pointer_t<
                            std::invoke_result_t<F, std::remove_pointer_t<It>>>>::value_type,
                  Char>
{
    template <typename FormatContext>
    auto format(const arg_invoke_join<It, Sentinel, Char, F>& value, FormatContext& ctx)
            -> decltype(ctx.out())
    {
        using base =
                formatter<typename std::iterator_traits<std::add_pointer_t<
                                  std::invoke_result_t<F, std::remove_pointer_t<It>>>>::value_type,
                        Char>;
        auto it = value.begin;
        auto out = ctx.out();
        if(it != value.end)
        {
            out = base::format(std::invoke(value.func, *it++), ctx);
            while(it != value.end)
            {
                out = std::copy(value.sep.begin(), value.sep.end(), out);
                ctx.advance_to(out);
                out = base::format(std::invoke(value.func, *it++), ctx);
            }
        }
        return out;
    }
};

template <typename Range, typename F>
arg_invoke_join<detail::iterator_t<const Range>, detail::sentinel_t<const Range>, char, F>
invoke_join(const Range& range, string_view sep, F&& f)
{
    return invoke_join(std::begin(range), std::end(range), sep, std::forward<F>(f));
}

template <typename Range, typename F>
arg_invoke_join<detail::iterator_t<const Range>, detail::sentinel_t<const Range>, wchar_t, F>
invoke_join(const Range& range, wstring_view sep, F&& f)
{
    return invoke_join(std::begin(range), std::end(range), sep, std::forward<F>(f));
}

template <typename T, typename F>
arg_invoke_join<const T*, const T*, char, F> invoke_join(
        std::initializer_list<T> list, string_view sep, F&& f)
{
    return invoke_join(std::begin(list), std::end(list), sep, std::forward<F>(f));
}

template <typename T, typename F>
arg_invoke_join<const T*, const T*, wchar_t, F> invoke_join(
        std::initializer_list<T> list, wstring_view sep, F&& f)
{
    return invoke_join(std::begin(list), std::end(list), sep, std::forward<F>(f));
}

template <typename It, typename Sentinel, typename F>
arg_invoke_join<It, Sentinel, char, F> invoke_join(It begin, Sentinel end, string_view sep, F&& f)
{
    return { begin, end, sep, std::forward<F>(f) };
}

template <typename It, typename Sentinel, typename F>
arg_invoke_join<It, Sentinel, wchar_t, F> invoke_join(
        It begin, Sentinel end, wstring_view sep, F&& f)
{
    return { begin, end, sep, std::forward<F>(f) };
}

} // end namespace fmt