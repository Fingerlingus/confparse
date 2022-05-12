#pragma once

#include <iostream>
#include <string>
#include <string_view>

#ifndef NO_DISCARD
#   define NO_DISCARD [[nodiscard]]
#endif

#ifdef _MSC_VER
namespace fmt = std;
#include <format>
#else
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#endif

namespace util {

// name prefixed with underscore due to overload resolution error with std::format() in MSVC
template<typename ...Args> NO_DISCARD std::string _format(std::string_view fmt_str, Args&&... args) {
    return fmt::vformat(fmt_str, fmt::make_format_args(args...));
}

// print formatted output to stream
template<typename ...Args> void sprint(std::ostream& os, std::string_view fmt_str, Args&&... args) {
    os << _format(fmt_str, args...);
}

// print formatted output to std::cout
template<typename ...Args> void print(std::string_view fmt_str, Args&&... args) {
    sprint(std::cout, fmt_str, args...);
}

// alias to sprint()
template<typename ...Args> void print(std::ostream& os, std::string_view fmt_str, Args&&... args) {
    sprint(os, fmt_str, args...);
}

// print formatted output to std::cout, with newline appended
template<typename ...Args> void log(const std::string& fmt_str, Args&&... args) {
    print(fmt_str + '\n', args...);
}

// print formatted output to std::cerr, with newline appended
template<typename ...Args> void error(const std::string& fmt_str, Args&&... args) {
    print(std::cerr, fmt_str + '\n', args...);
}

} // namespace util
