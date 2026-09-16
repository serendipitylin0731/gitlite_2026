cmake_minimum_required(VERSION 3.10)
project(gitlite CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
                  "${CMAKE_CURRENT_SOURCE_DIR}/src/commands/*.cpp")

add_executable(gitlite ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCES})

# The executable must land in gitlite/build for the tester to find it.
set_target_properties(gitlite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(
TARGET gitlite POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:gitlite>
        ${CMAKE_SOURCE_DIR}/code
)