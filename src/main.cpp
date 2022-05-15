#include <cstddef>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <memory>
#include <vector>
#include <concepts>
#include <array>

#include "util.hpp"

#ifndef NO_DISCARD
#   define NO_DISCARD [[nodiscard]]
#endif

inline static constexpr std::array<char, 2> COMMENT_CHARS = { '#', ';' };

enum class KV_PAIR_VALUE : int8_t {
    ERR  = -1,
    BOOL =  1,
    INT,
    UINT,
    FLOAT,
    STRING,
    ARRAY
};
inline static const std::map<KV_PAIR_VALUE, std::string_view> 
KV_PAIR_VALUE_STR = 
{
    { KV_PAIR_VALUE::ERR,    "ERR"    },
    { KV_PAIR_VALUE::BOOL,   "BOOL"   },
    { KV_PAIR_VALUE::INT,    "INT"    },
    { KV_PAIR_VALUE::UINT,   "UINT"   },
    { KV_PAIR_VALUE::FLOAT,  "FLOAT"  },
    { KV_PAIR_VALUE::STRING, "STRING" },
    { KV_PAIR_VALUE::ARRAY,  "ARRAY"  }
};

namespace kv {

struct value {
    using self_type = value;

    constexpr value() 
        : type(KV_PAIR_VALUE::ERR),
          v(-1LL)
    {
          
    }

    constexpr ~value() {

    }

    self_type& operator=(bool b) {
        v.b = b;
        type = KV_PAIR_VALUE::BOOL;
        return *this;
    }

    self_type& operator=(std::size_t i) {
        v.ui = i;
        type = KV_PAIR_VALUE::UINT;
        return *this;
    }

    self_type& operator=(std::intmax_t i) {
        v.si = i;
        type = KV_PAIR_VALUE::INT;
        return *this;
    }

    self_type& operator=(long double d) {
        v.f = d;
        type = KV_PAIR_VALUE::FLOAT;
        return *this;
    }

    self_type& operator=(const std::string& str) {
        s = str;
        type = KV_PAIR_VALUE::STRING;
        return *this;
    }

    self_type& operator=(const std::vector<self_type>& arr) {
        a = arr;
        type = KV_PAIR_VALUE::ARRAY;
        return *this;
    }
    
    KV_PAIR_VALUE type;
    // union for trivial types
    union value_union {
        bool b;
        std::size_t ui;
        std::intmax_t si;
        long double f;

        constexpr value_union() : value_union(-1LL) { }
        constexpr value_union(std::intmax_t i) : si(i) { }
        constexpr ~value_union() { }
    } v;
    // non-trivial types declared separately
    std::string s;
    std::vector<self_type> a;
};

struct pair {
    using self_type = pair;
    using key_type = std::string;
    using value_type = value;

    pair() = default;
    pair(const self_type&) = default;
    pair(self_type&&) = default;

    self_type& operator=(const self_type&) = default;
    self_type& operator=(self_type&&) = default;

    key_type key;
    value_type val;
};

} // namespace kv

struct section {
    section() : section(nullptr) { }
    section(section* p) : parent(std::shared_ptr<section>(p)) { }
    ~section() { parent.reset(); }

    std::string name;
    std::shared_ptr<section> parent;
    std::vector<section> children;
    std::vector<kv::pair> kvs;
};

template<class CharT> struct fmt::formatter<kv::value, CharT> :
    fmt::formatter<int, CharT> 
{
    template<typename FormatContext>
    auto format(kv::value v, FormatContext& fc) {
        if (v.type == KV_PAIR_VALUE::ERR)
            throw std::invalid_argument("cannot format error type.");
        else if(v.type == KV_PAIR_VALUE::BOOL)
            return fmt::format_to(fc.out(), "{}", v.v.b);
        else if(v.type == KV_PAIR_VALUE::INT) 
            return fmt::format_to(fc.out(), "{}", v.v.si);
        else if(v.type == KV_PAIR_VALUE::UINT)
            return fmt::format_to(fc.out(), "{}", v.v.ui);
        else if (v.type == KV_PAIR_VALUE::FLOAT)
            return fmt::format_to(fc.out(), "{}", v.v.f);
        else if (v.type == KV_PAIR_VALUE::STRING)
            return fmt::format_to(fc.out(), "{}", v.s);
        else if (v.type == KV_PAIR_VALUE::ARRAY)
            throw std::invalid_argument(
                "array formatting not yet implemented.");
        throw std::invalid_argument("cannot format invalid type.");
    }
};

template<class CharT> struct fmt::formatter<kv::pair, CharT> : 
    fmt::formatter<int, CharT> 
{
    template<typename FormatContext>
    auto format(kv::pair kv, FormatContext& fc) {
        return fmt::format_to(fc.out(), 
                              "key=\"{}\"\nvalue=\"{}\" (t={})", 
                              kv.key, 
                              kv.val, 
                              KV_PAIR_VALUE_STR.at(kv.val.type));
    }
};

NO_DISCARD std::ifstream open_file(std::string_view path) {
    std::ifstream f(path.data());
    if (!f)
        throw std::runtime_error("failed to open file.");
    return f;
}

NO_DISCARD std::stringstream strip_comments(std::ifstream& f) {
    std::string s;
    std::stringstream ss;

    while (!f.eof()) {
        std::getline(f, s);
        if (f.fail())
            throw std::runtime_error("error reading from file.");
        std::size_t comment_pos = std::string::npos;
        for (const auto& r : COMMENT_CHARS) {
            const std::size_t pos = s.find(r);
            if (pos < comment_pos)
                comment_pos = pos;
        }
        ss << s.substr(0, comment_pos) << std::endl;
    }
    return ss;
}

NO_DISCARD constexpr bool LINE_CONTAINS_KV(std::string_view s) noexcept {
    return s.find('=') != std::string::npos;
}

NO_DISCARD constexpr int
KV_STRING_CONTAINS_INVALID_WHITESPACE(std::string_view s) noexcept {
    if (ERROR(LINE_CONTAINS_KV(s)))
        return -1;

    s = util::parse::remove_leading_and_trailing_whitespace(s);

    const std::size_t eq_pos = s.find('=');

    // if first whitespace is after eq_pos, there is no leading whitespace

    const std::size_t key_end = s.find_first_of(" =");
    std::string_view k = s.substr(0, key_end);
    if (util::parse::STRING_CONTAINS_WHITESPACE(k))
        return -2;

    const std::size_t value_begin = s.find_first_not_of(" \n", eq_pos + 1);
    std::size_t i = value_begin + 1;
    if (s.at(value_begin) == '"') {
        while ((i = s.find('"', i)) != std::string::npos) {
            if (s.at(i - 1) != '\\')
                break;
        }
        if (i == std::string::npos)
            return -3;
    }

    const std::size_t value_end = s.find_first_of(" \n", value_begin);
    std::size_t value_whitespace_begin = s.find_first_of(" \n", value_end);
    if (value_whitespace_begin < i)
        value_whitespace_begin = i;

    if (value_whitespace_begin == std::string::npos)
        return 0;

    std::size_t trailing_nonwhitespace_begin =
        s.find_first_not_of(" \n", value_whitespace_begin);
    if (trailing_nonwhitespace_begin != std::string::npos)
        return -3;

    if (s.find('\n') < value_whitespace_begin)
        return -4;

    // else all tests pass, return success
    return 0;
}

NO_DISCARD constexpr bool LINE_IS_WHITESPACE(std::string_view s) noexcept {
    return s.find_first_not_of(" \n") == std::string::npos;
}

NO_DISCARD constexpr bool 
LINE_CONTAINS_SECTION_HEADER(std::string_view s) noexcept {
    return s.find('[') != std::string::npos &&
           s.find(']', s.find('[')) != std::string::npos;
}

NO_DISCARD std::stringstream read_file(std::string_view path) {
    std::ifstream f = open_file(path);
    return strip_comments(f);
}

constexpr bool parse_kv_value_as_bool(std::string_view s) {
    s = util::parse::remove_leading_and_trailing_whitespace(s);
    std::string v = to_lower(s);
    if (v == "true")
        return true;
    else if (v == "false")
        return false;
    else
        throw std::invalid_argument(
            util::format(
                "parse_kv_value_as_bool(): value is invalid (v={}).", s));
}

std::size_t parse_kv_value_as_unsigned_int(const std::string& s) {
    std::string v = s;
    v = util::parse::remove_leading_and_trailing_whitespace(s);
    if (util::parse::STRING_HAS_SIGN_PREFIX(v)) {
        if (s.at(0) == '-')
            throw std::invalid_argument(
                "parse_kv_value_as_unsigned_int(): value is negative.");
    }

    try {
        return std::stoull(v, nullptr, 0);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(
            util::format("parse_kv_value_as_unsigned_int(): parse error: {}",
                          e.what()));
    }
}

std::intmax_t parse_kv_value_as_signed_int(const std::string& s) {
    std::string v = s;
    v = util::parse::remove_leading_and_trailing_whitespace(s);

    try {
        return std::stoll(v, nullptr, 0);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(
            util::format("parse_kv_value_as_signed_int(): parse error: {}",
                         e.what()));
    }
}

long double parse_kv_value_as_float(const std::string& s) {
    std::string v = s;
    v = util::parse::remove_leading_and_trailing_whitespace(s);

    if (util::parse::STRING_HAS_OCTAL_PREFIX_OR_POSTFIX(s))
        throw std::invalid_argument(
            "parse_kv_value_as_float(): can't parse octal value as float.");

    try {
        return std::stold(v, nullptr);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument(
            util::format("parse_kv_value_as_float(): parse error: {}", 
                         e.what()));
    }
}

constexpr std::string_view parse_kv_value_as_string(std::string_view s) {
    s = util::parse::remove_leading_and_trailing_whitespace(s);
    if (s.at(0) != '"')
        return s.substr(0, s.find_first_of(" \n"));
    
    std::size_t i = 0;
    while((i = s.find('"', i + 1)) != std::string::npos) {
        if (i == std::string::npos)
            throw std::invalid_argument(
                "parse_kv_value_as_string(): value has no non-escaped closing double-quote.");
        else if (s.at(i - 1) != '\\')
            return s.substr(1, i - 1); // don't include quotes
    }
    throw std::invalid_argument("parse_kv_value_as_string(): unknown error.");
}

NO_DISCARD kv::pair parse_kv(const std::string& s) {
    if (ERROR(LINE_CONTAINS_KV(s)))
        throw std::runtime_error(
            "parse_kv: string does not contain a valid KV-pair");

    if (ERROR(KV_STRING_CONTAINS_INVALID_WHITESPACE(s)))
        throw std::runtime_error(
            "parse_kv: string contains invalid whitespace.");

    const std::size_t delim_pos = s.find('=');
    const std::size_t key_begin = s.find_first_not_of(' ');
    std::size_t key_end = s.find(' ', key_begin);
    if (key_end > delim_pos)
        key_end = delim_pos;
    std::string k = s.substr(key_begin, key_end - key_begin);
    std::string v;
    const std::size_t value_begin = s.find_first_not_of(" \n", delim_pos + 1);
    kv::pair kv;
    kv.key = k;
    // if value is not multi-word string
    if (s.at(value_begin) == '"') {
        std::size_t i = value_begin;
        while ((i = s.find('"', i + 1)) != std::string::npos) {
            if (s.at(i - 1) != '\\')
                break;
        }
        v = s.substr(value_begin, i - value_begin + 1);
    } else {
        const std::size_t value_whitespace_begin =
            s.find_first_of(" \n", value_begin);
        v = s.substr(value_begin, value_whitespace_begin - value_begin);
    }

    try {
        bool b = parse_kv_value_as_bool(v);
        kv.val = b;
        return kv;
    } catch (const std::invalid_argument& e) {
        util::dlog("val is not bool (v=\"{}\", e={}).", v, e.what());
    }

    if (!util::parse::STRING_IS_FLOAT(v)) {
        try {
            std::size_t i = parse_kv_value_as_unsigned_int(v);
            kv.val = i;
            return kv;
        } catch (const std::invalid_argument& e) {
            util::dlog(
                "val is not unsigned int (v=\"{}\", e={}).", 
                v, 
                e.what());
        }

        try {
            std::intmax_t i = parse_kv_value_as_signed_int(v);
            kv.val = i;
            return kv;
        } catch (const std::invalid_argument& e) {
            util::dlog("val is not signed int (v=\"{}\", e={}).", v, e.what());
        }
    } else {
        try {
            long double f = parse_kv_value_as_float(v);
            kv.val = f;
            return kv;
        } catch (const std::invalid_argument& e) {
            util::dlog("val is not float (v=\"{}\", e={}).", v, e.what());
        }
    }

    try {
        std::string_view sv = parse_kv_value_as_string(v);
        kv.val = std::string(sv);
        return kv;
    } catch (const std::invalid_argument& e) {
        util::dlog("val is not valid string (v=\"{}\", e={}).", v, e.what());
    }

    throw std::invalid_argument(
        util::format("val did not match to a known type (v=\"{}\").", v));
}

NO_DISCARD section parse_global_kvs(std::stringstream& ss) {
    section global;
    global.name = "";
    global.parent = nullptr;
    global.children.clear();

    std::string s;
    int i = 1;
    while (i++) {
        std::getline(ss, s);
        if (LINE_CONTAINS_SECTION_HEADER(s)) {
            ss << s;
            return global;
        }

        if (LINE_IS_WHITESPACE(s))
            continue;

        kv::pair p;
        try {
            p = parse_kv(s);
        } catch (const std::invalid_argument& e) {
            util::dlog(
                "parse_global_kvs: encountered invalid kv, skipping (s={}, e={}).",
                s, 
                e.what());

            continue;
        }

        global.kvs.push_back(p);
    }
    return global;
}

int main(int argc, char** argv) {
    if (argv[argc] != nullptr)
        return -1;

    if (argc == 0)
        return -2;

    if (argv[0] == nullptr)
        return -3;

    try {
        std::string_view s = argc > 1 && argv[1] != nullptr ?
            argv[1] :
            "../../../test.conf";

        std::stringstream ss = read_file(s);
        section sec_global = parse_global_kvs(ss);

        for (const auto& r : sec_global.kvs)
            util::dlog("{}\n", r);

        util::log("Done.");
    } catch (const std::exception& e) {
        util::log("main: uncaught exception: {}", e.what());
        return -5;
    }
}