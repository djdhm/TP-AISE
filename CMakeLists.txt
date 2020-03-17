cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)

project(mymalloc C)
find_package (Threads)

add_subdirectory(lib)
add_executable(myapp main.c)
target_link_libraries(myapp mymalloc)
target_link_libraries (myapp ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS myapp DESTINATION bin)


