cmake_minimum_required(VERSION 3.10)
project(redis LANGUAGES CXX)

# 1. Standard selection (like Rust editions)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Add your favorite safety flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# 3. Define your executables
add_executable(server src/server.cpp)
add_executable(client src/client.cpp)

# 4. Networking Tip:
# If you are on Windows, you usually need to link Winsock
if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif()
