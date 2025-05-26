cmake_minimum_required(VERSION 3.10)
project(SquadroAI)
set(CMAKE_CXX_STANDARD 17)
set(SOURCES
src/main.cpp
src/GameState.cpp
src/AIEngine.cpp
src/Zobrist.cpp
src/utils.cpp
)
include_directories(include)
add_executable(SquadroAI ${SOURCES})

enable_testing()
add_subdirectory(test)