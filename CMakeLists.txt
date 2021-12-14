# minimum CMake version required
cmake_minimum_required(VERSION 3.15)
# Project name, version and description
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

# Add DPP as dependency
include(FetchContent)

FetchContent_Declare(DPP
    GIT_REPOSITORY  https://github.com/brainboxdotcc/DPP.git
)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
)

FetchContent_MakeAvailable(DPP nlohmann_json)

# Create an executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Linking libraries
target_link_libraries(${PROJECT_NAME}
    dpp
    nlohmann_json
)

# Specify includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${dpp_SOURCE_DIR}/include
    ${nlohmann_json_SOURCE_DIR}/include
)

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)