cmake_minimum_required(VERSION 4.2)
project(Cpp2MusicPrompt)

set(CMAKE_CXX_STANDARD 20)

set(LLVM_DIR "/opt/homebrew/opt/llvm/lib/cmake/llvm")
set(Clang_DIR "/opt/homebrew/opt/llvm/lib/cmake/clang")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(SYSTEM
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

add_definitions(${LLVM_DEFINITIONS})

add_executable(Cpp2MusicPrompt
        main.cpp
)

target_link_libraries(Cpp2MusicPrompt
        clangTooling
        clangBasic
        clangASTMatchers
)