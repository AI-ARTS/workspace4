
cmake_minimum_required(VERSION 3.5)
project(sylar)

include_directories(
/home/aldd/project/workspace4
/home/aldd/project/workspace4/sylar
)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -g -std=c++17 -Wall -Wno-deprecated -Werror -Wno-unused-function")

# 构建共享库的文件
set(LIB_SRC
    sylar/logger.cpp
    sylar/config.cpp
    sylar/util.cpp
    sylar/thread.cpp
    sylar/schedule.cpp
    sylar/iomanager.cpp
    sylar/Timer.cpp
    sylar/hook.cpp
    sylar/fd_manager.cpp
    sylar/hook.cpp
		sylar/http/http.cpp # 测试
    sylar/socket.cpp # 测试
    sylar/address.cpp # 测试
    sylar/server.cpp# 测试
)

add_library(sylar SHARED ${LIB_SRC})

# 添加可执行文件及其依赖项和链接库
add_executable(test tests/main.cpp)
add_dependencies(test sylar)
target_link_libraries(test PRIVATE sylar)


add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber sylar)
target_link_libraries(test_fiber PRIVATE sylar)

add_executable(test_schedule tests/test_schedule.cpp)
add_dependencies(test_schedule sylar)
target_link_libraries(test_schedule PRIVATE sylar)


add_executable(test_iomanager tests/test_iomanager.cpp)
add_dependencies(test_iomanager sylar)
target_link_libraries(test_iomanager PRIVATE sylar)

add_executable(test_Timer tests/test_Timer.cpp)
add_dependencies(test_Timer sylar)
target_link_libraries(test_Timer PRIVATE sylar)

add_executable(test_hook tests/test_hook.cpp)
add_dependencies(test_hook sylar)
target_link_libraries(test_hook PRIVATE sylar)


add_executable(test_http tests/test_http.cpp)
add_dependencies(test_http sylar)
target_link_libraries(test_http PRIVATE sylar)

add_executable(test_address tests/test_address.cpp)
add_dependencies(test_address sylar)
target_link_libraries(test_address PRIVATE sylar)

add_executable(test_sock tests/test_sock.cpp)
add_dependencies(test_sock sylar)
target_link_libraries(test_sock PRIVATE sylar)

add_executable(test_server tests/test_server.cpp)
add_dependencies(test_server sylar)
target_link_libraries(test_server PRIVATE sylar)



# boost和yaml加上
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})
target_link_libraries(test PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_fiber PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_schedule PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_iomanager PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_Timer PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_hook PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_http PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_address PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_sock PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(test_server PRIVATE yaml-cpp::yaml-cpp)




find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Boost REQUIRED COMPONENTS coroutine)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(test PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_fiber PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_schedule PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_iomanager PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_Timer PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_hook PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_http PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_address PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_sock PRIVATE ${Boost_LIBRARIES})
target_link_libraries(test_server PRIVATE ${Boost_LIBRARIES})



# openssl
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(test PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_fiber PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_schedule PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_iomanager PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_Timer PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_hook PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_http PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_address PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_sock PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test_server PRIVATE OpenSSL::SSL OpenSSL::Crypto)



# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
