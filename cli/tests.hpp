#pragma once

namespace noq {
    namespace unittest {
        struct Result {
            size_t total = 0;
            size_t failed = 0;
            std::function<void(const std::string&)> fn;
            std::string tdir;
            std::string tsrc;
        };
        void runTest(const std::string& toTest, Result& result);
    }
}
