add_executable(test_client main.cpp)

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but can not be found")
endif()

target_link_libraries(
        test_client
        protocol
        ${PROTOBUF_LIBRARIES}
        network
        boost_system
)