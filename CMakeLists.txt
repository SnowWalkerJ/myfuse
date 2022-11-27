cmake_minimum_required(VERSION 3.23)
project(myfuse)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
set(CMAKE_CXX_STANDARD 17)

add_library(Path src/Path.cpp include/Path.h)
add_library(BufferIO src/BufferIO.cpp include/BufferIO.h)
add_library(MemFS src/MemFS.cpp include/MemFS.h include/FileSystem.h)

add_executable(myfuse src/main.cpp)
target_link_libraries(myfuse fuse Path BufferIO MemFS)

add_executable(test_fs src/test_fs.cpp)
target_link_libraries(test_fs Path BufferIO MemFS)
