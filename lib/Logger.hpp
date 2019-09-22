#pragma once

namespace noq {
    struct Logger {
        static constexpr auto logname = "noql";
        template <typename... ArgsT>
        inline void error(const std::string& fmt, ArgsT... args){
#if defined(USE_SPDLOG)
            spdlog::get(logname)->error(fmt.c_str(),args...);
#endif
        }

        template <typename... ArgsT>
        inline void info(const std::string& fmt, ArgsT... args){
#if defined(USE_SPDLOG)
            spdlog::get(logname)->info(fmt.c_str(),args...);
#endif
        }

        template <typename... ArgsT>
        inline void warn(const std::string& fmt, ArgsT... args){
#if defined(USE_SPDLOG)
            spdlog::get(logname)->warn(fmt.c_str(),args...);
#endif
        }

        template <typename... ArgsT>
        inline void debug(const std::string& fmt, ArgsT... args){
#if defined(USE_SPDLOG)
            spdlog::get(logname)->debug(fmt.c_str(),args...);
#endif
        }

        template <typename... ArgsT>
        inline void trace(const std::string& fmt, ArgsT... args){
#if defined(USE_SPDLOG)
            spdlog::get(logname)->trace(fmt.c_str(),args...);
#endif
        }

        inline Logger() {}
        inline Logger(const std::string& src) {}

        static inline void initFile(const std::string& filename) {
#if defined(USE_SPDLOG)
            auto logger = spdlog::basic_logger_mt(logname, filename, true);
#ifndef NDEBUG
            logger->set_level(spdlog::level::trace);
#endif
            logger->info("############## Start run ###############");
#endif
        }
    };
}
