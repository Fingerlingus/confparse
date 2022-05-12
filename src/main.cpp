#include <cstddef>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "util.hpp"
#include "result.hpp"

#ifndef NO_DISCARD
#   define NO_DISCARD [[nodiscard]]
#endif

NO_DISCARD result<std::ifstream, int> open_file(std::string_view path) {
    std::ifstream f(path.data());
    if (!f)
        return error(-1);
    return f;
}

NO_DISCARD result<std::stringstream, int> strip_comments(std::ifstream& f) {
    std::string s;
    std::stringstream ss;

    while (!f.eof()) {
        std::getline(f, s);
        if (f.fail())
            return error(-1);
        ss << s.substr(0, s.find(';')) << std::endl;
    }
    return std::move(ss);
}

NO_DISCARD result<std::stringstream, int> read_file(std::string_view path) {
    auto e = open_file(path);
    if (e.is_err())
        return error(-1);

    std::ifstream f = e.value();
    auto e2 = strip_comments(f);
    if (e.is_err())
        return error(-2);

    std::stringstream ss = e2.value();
    return ss;
}

int main(int argc, char** argv) {
    std::string_view s;
    if (argc > 1 && argv[1] != nullptr)
        s = argv[1];
    else
        s = "../../../test.conf";
    auto e = read_file(s);

    if (e.is_err()) {
        if (e.err() == -1) {
            util::log("failed to open file.");
            return -1;
        }
        else if (e.err() == -2) {
            util::log("error reading from file.");
            return -2;
        } else {
            util::log("unknown error.");
            return -3;
        }

    }
    std::stringstream ss = e.value();
    util::print(ss.str());
}