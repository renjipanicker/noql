#pragma once

namespace noq {
    struct Exception : public std::runtime_error {
        inline Exception(const std::string& msg) : runtime_error(msg) {}
    };
}
