#pragma once

#include <iostream>
#include <string>
#include <string_view>

#ifndef NO_DISCARD
#   define NO_DISCARD [[nodiscard]]
#endif

#if defined _MSC_VER
#   define IS_MSVC
#elif defined __GNU_C__
#   define IS_GCC_OR_CLANG
#endif

#define CPP17 201703L
#define CPP20 202002L

#if __cplusplus > CPP17 && __cplusplus <= CPP20
#   define IS_CPP20 1
#elif __cplusplus > CPP20 && __cplusplus < 202300L
#   define IS_CPP_LATEST 1
#endif

#if defined IS_MSVC && defined IS_CPP_LATEST
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

// print single item to stream
template<typename T> void sprint(std::ostream& os, const T& t) {
    os << _format("{}", t);
}

// print formatted output to std::cout
template<typename ...Args> void print(std::string_view fmt_str, Args&&... args) {
    sprint(std::cout, fmt_str, args...);
}

// print single item to std::cout
template<typename T> void print(const T& t) {
    sprint(std::cout, t);
}

// alias to sprint()
template<typename ...Args> void print(std::ostream& os, std::string_view fmt_str, Args&&... args) {
    sprint(os, fmt_str, args...);
}

// alias to sprint()
template<typename T> void print(std::ostream& os, const T& t) {
    sprint(os, t);
}

// print formatted output to std::cout, with newline appended
template<typename ...Args> void log(const std::string& fmt_str, Args&&... args) {
    print(fmt_str + '\n', args...);
}

// log single item
template<typename T> void log(const T& t) {
    log("{}", t);
}
#ifdef DEBUG
// log if in debug mode
template<typename ...Args> void dlog(const std::string& fmt_str, Args&&... args) {
    log(fmt_str, args...);
}

template<typename T> void dlog(const T& t) {
    log(t);
}
#else
// else do nothing
template<typename ...Args> void dlog(const std::string&, Args&&...) { }
template<typename T> void dlog(const T&) { }
#endif

// print formatted output to std::cerr, with newline appended
template<typename ...Args> void error(const std::string& fmt_str, Args&&... args) {
    print(std::cerr, fmt_str + '\n', args...);
}

// error with single item
template<typename T> void error(const T& t) {
    error("{}", t);
}
#ifdef DEBUG
// error if in debug mode
template<typename ...Args> void derror(const std::string& fmt_str, Args&&... args) {
    error(fmt_str, args...);
}

template<typename T> void derror(const T& t) {
    error(t);
}
#else
// else do nothing
template<typename ...Args> void derror(const std::string&, Args&&...) { }
template<typename T> void derror(const T&) { }
#endif

} // namespace util

NO_DISCARD constexpr bool ERROR(bool b) noexcept {
    return b == false;
}

template<std::integral T> NO_DISCARD constexpr bool ERROR(T i) noexcept {
    return i != 0;
}

NO_DISCARD constexpr std::string to_lower(std::string_view s) {
    std::string l = "";
    for (const auto& c : s)
        l += static_cast<std::string::value_type>(std::tolower(c));
    return l;
}