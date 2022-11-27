cmake_minimum_required(VERSION 3.23)
project(myfuse)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
set(CMAKE_CXX_STANDARD 17)

add_executable(myfuse main.cpp BufferIO.cpp BufferIO.h FileSystem.h MemFS.cpp MemFS.h Path.cpp Path.h)
target_link_libraries(myfuse fuse)

add_executable(test_fs test_fs.cpp BufferIO.cpp BufferIO.h FileSystem.h MemFS.cpp MemFS.h Path.cpp Path.h)
