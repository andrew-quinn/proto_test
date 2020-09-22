cmake_minimum_required(VERSION 3.18)
project(proto_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)
find_package(fmt REQUIRED)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

set(proto_filename test_spec)
get_filename_component(proto_abs ${proto_filename}.proto ABSOLUTE)
get_filename_component(proto_path "${proto_abs}" PATH)

set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/${proto_filename}.pb.cc")
set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${proto_filename}.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/${proto_filename}.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${proto_filename}.grpc.pb.h")

add_custom_command(
        OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
             --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
             -I "${proto_path}"
             --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
             "${proto_abs}"
        DEPENDS "${proto_abs}"
)

include_directories("/home/walrus/.local/include/")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_executable(test_client test_client.cpp)
add_executable(test_server test_server.cpp)

foreach(_target test_client test_server)
    message(STATUS "Adding ${_target} to build with ${proto_srcs} and ${grpc_srcs}")
    target_sources(${_target} PRIVATE
            ${proto_srcs}
            ${grpc_srcs}
            name_num.h)

    target_link_libraries(${_target}
            ${_REFLECTION}
            ${_GRPC_GRPCPP}
            ${_PROTOBUF_LIBPROTOBUF})
endforeach()

target_link_libraries(test_client fmt::fmt)
target_link_libraries(test_server fmt::fmt)
