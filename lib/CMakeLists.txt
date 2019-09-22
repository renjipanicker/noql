include(FetchContent)

set(ENABLE_LEMONEX ON)

if(NOT DEFINED ENABLE_SPDLOG)
    set(ENABLE_SPDLOG ON)
endif()

if(NOT DEFINED ENABLE_JSON)
    set(ENABLE_JSON ON)
endif()

if(NOT DEFINED ENABLE_MONGOC)
    set(ENABLE_MONGOC OFF)
endif()

if(USE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    endif()
endif()

# fetch lemonex from github
if(ENABLE_LEMONEX)
    FetchContent_Declare(
    lemonex
    GIT_REPOSITORY https://github.com/renjipanicker/lemonex.git
    )

    FetchContent_GetProperties(lemonex)
    if(NOT lemonex_POPULATED)
        FetchContent_Populate(lemonex)
        file(COPY ${lemonex_SOURCE_DIR}/lempar.c DESTINATION ${lemonex_BINARY_DIR})
        add_subdirectory(${lemonex_SOURCE_DIR} ${lemonex_BINARY_DIR})
    endif()
else()
    #TODO: error out
endif()

# find and/or download spdlog library
# always download from github because we need v0.17 and above
if(ENABLE_SPDLOG)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
    )

    FetchContent_GetProperties(spdlog)
    if(NOT spdlog_POPULATED)
        message("-- Downloading spdlog")
        FetchContent_Populate(spdlog)
        add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
    endif()
    include_directories(${spdlog_SOURCE_DIR}/include)
    message("-- Enabled spdlog for this build: ${spdlog_SOURCE_DIR}")
    set(spdlog_FOUND ON)
else()
    set(spdlog_FOUND OFF)
    message("-- Disabled spdlog for this build")
endif()

# find and/or download json library
if(ENABLE_JSON)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
    )

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        message("-- Downloading json")
        FetchContent_Populate(json)
        add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR})
    endif()
    set(json_FOUND ON)
    include_directories(${json_SOURCE_DIR}/include)
    message("-- Enabled json for this build: ${json_SOURCE_DIR}")
else()
    set(json_FOUND OFF)
    message("-- Disabled json for this build")
endif()

# find and/or download mongoc library
if(ENABLE_MONGOC)
    # find_package(libmongoc-1.0 CONFIG QUIET)
    # if(libmongoc-1.0_FOUND)
    #     set(NOQ_LINK_LIBS
    #         ${NOQ_LINK_LIBS}
    #         "${MONGOC_LIBRARIES}"
    #     )
    #     message("-- Enabled mongoc for this build: ${mongoc_SOURCE_DIR}")
    # else()
    #     message("-- Enabled mongoc but not found on system")
    # endif()
else()
    set(mongoc_FOUND OFF)
    message("-- Disabled mongoc for this build")
endif()

# pull in std filesystem library
# temporary hack until std::filesystem is integrated into stdc++ library
set(NOQ_LINK_LIBS
    ${NOQ_LINK_LIBS}
    stdc++fs
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# this is required by noqParser below
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# build noqParser
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/noqParser.cpp
    COMMAND lemonex -e.cpp -q ${CMAKE_CURRENT_SOURCE_DIR}/noqParser.y
    COMMENT "Creating parser"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/noqParser.y
)

set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/noq.cpp
    PROPERTIES
    GENERATED TRUE
)

# define lib project
set(NOQ_LIB_SOURCE
    Module.cpp
    Query.cpp
    ExprEval.cpp
    Connection.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/noqParser.cpp
)

if(json_FOUND)
    set(NOQ_LIB_SOURCE
        ${NOQ_LIB_SOURCE}
        JsonAdaptor.cpp
    )
endif()

if(mongoc_FOUND)
    set(NOQ_LIB_SOURCE
        ${NOQ_LIB_SOURCE}
        MongoAdaptor.cpp
    )
endif()

add_library(${PROJECT_NAME} ${NOQ_LIB_SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE "${NOQ_LINK_LIBS}")

if(spdlog_FOUND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_SPDLOG=1)
endif()

if(json_FOUND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_NLOHMANN_JSON=1)
endif()

if(mongoc_FOUND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_MONGOC=1)
    target_include_directories(${PROJECT_NAME} PRIVATE "${MONGOC_INCLUDE_DIRS}")
    target_compile_definitions(${PROJECT_NAME} PRIVATE "${MONGOC_DEFINITIONS}")
endif()
