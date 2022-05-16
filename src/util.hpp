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
#   define FMT_HEADER_ONLY
#   include <fmt/core.h>
#endif

namespace util {
template<typename ...Args> 
NO_DISCARD std::string format(std::string_view fmt_str, Args&&... args) {
    return fmt::vformat(fmt_str, fmt::make_format_args(args...));
}

// print formatted output to stream
template<typename ...Args> 
void sprint(std::ostream& os, std::string_view fmt_str, Args&&... args) {
    os << format(fmt_str, args...);
}

// print single item to stream
template<typename T> void sprint(std::ostream& os, const T& t) {
    os << format("{}", t);
}

// print formatted output to std::cout
template<typename ...Args> 
void print(std::string_view fmt_str, Args&&... args) {
    sprint(std::cout, fmt_str, args...);
}

// print single item to std::cout
template<typename T> void print(const T& t) {
    sprint(std::cout, t);
}

// alias to sprint()
template<typename ...Args> 
void print(std::ostream& os, std::string_view fmt_str, Args&&... args) {
    sprint(os, fmt_str, args...);
}

// alias to sprint()
template<typename T> void print(std::ostream& os, const T& t) {
    sprint(os, t);
}

// print formatted output to std::cout, with newline appended
template<typename ...Args> 
void log(const std::string& fmt_str, Args&&... args) {
    print(fmt_str + '\n', args...);
}

// log single item
template<typename T> void log(const T& t) {
    log("{}", t);
}
#ifdef DEBUG
// log if in debug mode
template<typename ...Args> 
void dlog(const std::string& fmt_str, Args&&... args) {
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
template<typename ...Args> 
void error(const std::string& fmt_str, Args&&... args) {
    print(std::cerr, fmt_str + '\n', args...);
}

// error with single item
template<typename T> void error(const T& t) {
    error("{}", t);
}
#ifdef DEBUG
// error if in debug mode
template<typename ...Args> 
void derror(const std::string& fmt_str, Args&&... args) {
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

namespace parse {

NO_DISCARD constexpr bool 
STRING_HAS_HEX_PREFIX_OR_POSTFIX(std::string_view s) noexcept;

NO_DISCARD constexpr bool STRING_IS_NUMERIC(std::string_view s) noexcept;

NO_DISCARD constexpr std::string_view 
remove_leading_and_trailing_whitespace(std::string_view s) {
    const std::size_t first_nonwhitespace = s.find_first_not_of(" \n");
    const std::size_t last_nonwhitespace = s.find_last_not_of(" \n");
    return s.substr(first_nonwhitespace, 
                    last_nonwhitespace - first_nonwhitespace + 1);
}

NO_DISCARD constexpr std::string_view 
remove_leading_and_trailing_whitespace(const std::string& s) {
    std::string_view v = s;
    return remove_leading_and_trailing_whitespace(v);
}

NO_DISCARD constexpr std::string_view 
remove_sign_prefix(std::string_view s) {
    if (s.starts_with('+') || s.starts_with('-'))
        s = s.substr(1);

    return s;
}

NO_DISCARD std::string_view remove_hex_prefix_or_postfix(std::string_view s) {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));

    if (s.starts_with("0x"))
        s = s.substr(2);

    if (s.ends_with('h'))
        s = s.substr(0, s.size() - 1);

    return s;
}

NO_DISCARD std::string_view 
remove_octal_prefix_or_postfix(std::string_view s) {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));

    if (STRING_HAS_HEX_PREFIX_OR_POSTFIX(s))
        throw std::runtime_error("string is hex, not octal.");

    if (s.starts_with('o'))
        s = s.substr(1);

    if (s.starts_with('0'))
        s = s.substr(1);

    if (s.ends_with('o'))
        s = s.substr(0, s.size() - 1);

    return s;
}

NO_DISCARD constexpr bool STRING_HAS_HEX_PREFIX(std::string_view s) noexcept {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));
    if (!STRING_IS_NUMERIC(s))
        return false;

    return s.starts_with("0x");
}

NO_DISCARD constexpr bool STRING_HAS_HEX_POSTFIX(std::string_view s) noexcept {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));
    if (!STRING_IS_NUMERIC(s))
        return false;

    return s.ends_with('h');
}

NO_DISCARD constexpr bool 
STRING_HAS_HEX_PREFIX_OR_POSTFIX(std::string_view s) noexcept {
    return STRING_HAS_HEX_PREFIX(s) || STRING_HAS_HEX_POSTFIX(s);
}

NO_DISCARD constexpr bool 
STRING_CONTAINS_WHITESPACE(std::string_view s) noexcept {
    return s.find_first_of(" \n") != std::string::npos;
}

NO_DISCARD constexpr bool STRING_HAS_SIGN_PREFIX(std::string_view s) noexcept {
    s = remove_leading_and_trailing_whitespace(s);
    return s.starts_with('-') || s.starts_with('+');
}

NO_DISCARD constexpr bool STRING_IS_NUMERIC(std::string_view s) noexcept {
    std::string_view search = "0123456789abcdef,_.'+-";
    for (const auto c : s) {
        if (search.find(
                static_cast<std::string::value_type>(std::tolower(c))
           ) == std::string::npos)
            return false;
    }
    return true;
}

NO_DISCARD constexpr bool 
STRING_HAS_OCTAL_PREFIX(std::string_view s) noexcept {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));
    if (!STRING_IS_NUMERIC(s))
        return false;

    return s.starts_with('0');
}

NO_DISCARD constexpr bool 
STRING_HAS_OCTAL_POSTFIX(std::string_view s) noexcept {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));
    if (!STRING_IS_NUMERIC(s))
        return false;

    return s.ends_with('o');
}

NO_DISCARD constexpr bool 
STRING_HAS_OCTAL_PREFIX_OR_POSTFIX(std::string_view s) noexcept {
    return STRING_HAS_OCTAL_PREFIX(s) || STRING_HAS_OCTAL_POSTFIX(s);
}

NO_DISCARD constexpr bool STRING_IS_FLOAT(std::string_view s) noexcept {
    s = remove_sign_prefix(remove_leading_and_trailing_whitespace(s));

    if (!STRING_IS_NUMERIC(s))
        return false;

    bool is_hex = false;
    bool is_octal = false;
    if (STRING_HAS_HEX_PREFIX(s) || STRING_HAS_OCTAL_PREFIX(s)) {
        s = s.substr(1);
        is_hex = true;
    }
    else if (STRING_HAS_HEX_POSTFIX(s) || STRING_HAS_OCTAL_POSTFIX(s)) {
        s = s.substr(0, s.size() - 1);
        is_octal = true;
    }

    const std::size_t decimal_pos = s.find('.');
    if (decimal_pos == std::string::npos)
        return false;

    if (s.find('.', decimal_pos + 1) != std::string::npos)
        return false; // float can't have multiple decimal points

    const std::string_view search = is_hex ? "0123456789abcdef" :
        is_octal ? "01234567" : "0123456789";
    // ensure at least one digit exists before or after decimal point 
    // (i.e. .1 or 1., not just 1.0/0.1, should be valid)
    if (s.find_first_of(search) > decimal_pos &&
        s.find_first_of(search, decimal_pos + 1) == std::string::npos) {
        return false;
    }

    return true;
}

} // namespace parse
} // namespace util

NO_DISCARD constexpr bool ERROR(bool b) noexcept {
    return b == false;
}

template<std::integral T> NO_DISCARD constexpr bool ERROR(T i) noexcept {
    return i != 0;
}

NO_DISCARD std::string to_lower(std::string_view s) {
    std::string l = "";
    for (const auto c : s)
        l += static_cast<decltype(l)::value_type>(std::tolower(c));
    return l;
}
