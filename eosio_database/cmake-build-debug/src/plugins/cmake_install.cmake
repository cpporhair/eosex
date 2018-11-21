# Install script for directory: /Users/yangwenyu/workspace/eosex/eosio_database/src/plugins

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/bnet_plugin/cmake_install.cmake")
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/net_plugin/cmake_install.cmake")
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/producer_plugin/cmake_install.cmake")
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/chain_plugin/cmake_install.cmake")
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/http_plugin/cmake_install.cmake")
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/history_plugin/cmake_install.cmake")
  include("/Users/yangwenyu/workspace/eosex/eosio_database/cmake-build-debug/src/plugins/http_client_plugin/cmake_install.cmake")

endif()

