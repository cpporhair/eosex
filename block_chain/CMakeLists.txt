cmake_minimum_required(VERSION 3.10)
project(eos_chain_application)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    include
    /usr/local/Cellar/boost/1.67.0_1/include
)

link_directories(
    /usr/local/Cellar/boost/1.67.0_1/lib
)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/fc/CMakeModules)

include( GNUInstallDirs )
include( VersionMacros )
include( SetupTargetMacros )

configure_file(config.hpp.in config.hpp ESCAPE_QUOTES)




if ("${OPENSSL_ROOT_DIR}" STREQUAL "")
    if (NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
        set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR})
        set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    elseif (APPLE)
        set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
        set(OPENSSL_INCLUDE_DIR "/usr/local/opt/openssl/include")
    elseif(UNIX AND NOT APPLE)
        set(OPENSSL_ROOT_DIR "/usr/include/openssl")
        set(OPENSSL_INCLUDE_DIR "/usr/include/openssl/include")
    else()
        message(FATAL_ERROR "openssl not found and don't know where to look, please specify OPENSSL_ROOT_DIR")
    endif()
endif()

if ("${EOSIO_ROOT_KEY}" STREQUAL "")
    set(EOSIO_ROOT_KEY "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV")
endif()

add_subdirectory(src)

add_executable(eos_chain_application main.cpp)
target_link_libraries(
    eos_chain_application
    appbase
    chain_plugin
    net_plugin
    http_plugin
    bnet_plugin
    http_client_plugin
    producer_plugin
    history_plugin
    boost_filesystem
    boost_iostreams
)