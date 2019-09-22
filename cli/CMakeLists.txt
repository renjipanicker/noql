include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# define shell project
set(NOQ_CLI_SOURCE
    main.cpp
    tests.cpp
)

add_executable(${PROJECT_NAME}-cli ${NOQ_CLI_SOURCE})
target_link_libraries(${PROJECT_NAME}-cli
    ${PROJECT_NAME}
    ${NOQ_LINK_LIBS}
    pthread
)
