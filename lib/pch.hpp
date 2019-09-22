#pragma once

#include "noql/pch.hpp"

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>

#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/file_sinks.h>
#endif

#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#ifdef USE_MONGOC
#include <mongoc/mongoc.h>
#endif
