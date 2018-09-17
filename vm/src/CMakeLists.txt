add_library(http_parser
        ${PROJECT_ROOT_PATH}/third_party/http-parser/http_parser.h
        ${PROJECT_ROOT_PATH}/third_party/http-parser/http_parser.c)

add_executable(boxed
        main.cpp
        service/http_service.hpp
        service/listener.hpp utils/connection.hpp utils/unique_ptr_wrapper.hpp utils/http.hpp vm/api_console.hpp vm/api_aware_base.hpp utils/null_terminated_ptr.hpp)
message(STATUS,${CMAKE_CURRENT_BINARY_DIR})

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    target_compile_options(boxed PUBLIC
            -std=gnu++17 -g
            -Wall -Werror
            -Wno-error=deprecated-declarations
            -fvisibility=hidden -Wno-attributes -pthread
            -I${SEASTAR_PATH} -U_FORTIFY_SOURCE
            -I${SEASTAR_PATH}/fmt
            -I${PROJECT_ROOT_PATH}
            -I${PROJECT_ROOT_PATH}/third_party
            -DBOOST_TEST_DYN_LINK -Wno-ignored-attributes
            -Wno-overloaded-virtual
            -Wno-uninitialized
            -Wno-mismatched-tags
            -Wno-error=cpp
            -Wno-unused-lambda-capture
            -O0 -g
            #-Wno-stringop-overflow
            #        -DSEASTAR_HAVE_GCC6_CONCEPTS
            -DSEASTAR_HAVE_HWLOC
            -DSEASTAR_HAVE_NUMA
            -DSEASTAR_HAS_MEMBARRIER -DSEASTAR_HAVE_LZ4_COMPRESS_DEFAULT -DSEASTAR_HAVE_ASAN_FIBER_SUPPORT
            #        -O2
            -I${SEASTAR_PATH}/build/release/gen
            -I${SEASTAR_PATH}/build/release/c-ares
            )
    target_link_libraries(boxed PUBLIC
            -L${SEASTAR_PATH}/build/release/
            -L${SEASTAR_PATH}/build/release/fmt/fmt
            -L${CMAKE_CURRENT_BINARY_DIR}
            -Wl,--whole-archive,-lseastar,--no-whole-archive
            -lfmt -std=gnu++17 -g
            -Wall -Werror -Wno-error=deprecated-declarations -fvisibility=hidden -Wno-attributes
            -pthread
            -I${SEASTAR_PATH}
            -U_FORTIFY_SOURCE
            -I${SEASTAR_PATH}/fmt
            -DBOOST_TEST_DYN_LINK
            -Wno-ignored-attributes
            -Wno-overloaded-virtual
            -Wno-uninitialized
            -Wno-error=cpp
            -Wno-unused-lambda-capture
            #-Wno-stringop-overflow
            #        -DSEASTAR_HAVE_GCC6_CONCEPTS
            -DSEASTAR_HAVE_HWLOC
            -DSEASTAR_HAVE_NUMA
            -DSEASTAR_HAS_MEMBARRIER -DSEASTAR_HAVE_LZ4_COMPRESS_DEFAULT
            -DSEASTAR_HAVE_ASAN_FIBER_SUPPORT
            -Wl,--no-as-needed
            -lboost_program_options
            -lboost_system -lboost_filesystem -lstdc++ -lm -lstdc++fs
            -lboost_thread -lcryptopp -lrt -lgnutls -lgnutlsxx -llz4 -lprotobuf -ldl -lgcc_s
            -lyaml-cpp -lhwloc -lnuma -lpciaccess -lxml2 -lz
            -lcares-seastar
            -lfmt
            -lhttp_parser
            )
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(boxed PUBLIC
            -std=gnu++17 -g
            -Wall -Werror -Wno-error=deprecated-declarations
            -fvisibility=hidden -Wno-attributes -pthread
            -I${SEASTAR_PATH} -U_FORTIFY_SOURCE
            -fconcepts
            -I${SEASTAR_PATH}/fmt
            -I${PROJECT_ROOT_PATH}
            -I${PROJECT_ROOT_PATH}/third_party
            -DBOOST_TEST_DYN_LINK -Wno-ignored-attributes
            -Wno-overloaded-virtual -Wno-maybe-uninitialized -Wno-error=cpp -Wno-stringop-overflow
            #        -DSEASTAR_HAVE_GCC6_CONCEPTS
            -DSEASTAR_HAVE_HWLOC
            -DSEASTAR_HAVE_NUMA
            -DSEASTAR_HAS_MEMBARRIER -DSEASTAR_HAVE_LZ4_COMPRESS_DEFAULT -DSEASTAR_HAVE_ASAN_FIBER_SUPPORT
            #        -O2
            -I${SEASTAR_PATH}/build/release/gen
            -I${SEASTAR_PATH}/build/release/c-ares
            )
    target_link_libraries(boxed PUBLIC
            -L${SEASTAR_PATH}/build/release/
            -L${SEASTAR_PATH}/build/release/fmt/fmt
            -L${CMAKE_CURRENT_BINARY_DIR}
            -Wl,--whole-archive,-lseastar,--no-whole-archive
            -lfmt -std=gnu++17 -g
            -Wall -Werror -Wno-error=deprecated-declarations -fvisibility=hidden -Wno-attributes
            -pthread
            -I${SEASTAR_PATH}
            -U_FORTIFY_SOURCE
            -fconcepts
            -I${SEASTAR_PATH}/fmt
            -DBOOST_TEST_DYN_LINK -Wno-ignored-attributes -Wno-overloaded-virtual
            -Wno-maybe-uninitialized -Wno-error=cpp -Wno-stringop-overflow
            #        -DSEASTAR_HAVE_GCC6_CONCEPTS
            -DSEASTAR_HAVE_HWLOC
            -DSEASTAR_HAVE_NUMA
            -DSEASTAR_HAS_MEMBARRIER -DSEASTAR_HAVE_LZ4_COMPRESS_DEFAULT
            -DSEASTAR_HAVE_ASAN_FIBER_SUPPORT
            -Wl,--no-as-needed
            -lboost_program_options
            -lboost_system -lboost_filesystem -lstdc++ -lm -lstdc++fs
            -lboost_thread -lcryptopp -lrt -lgnutls -lgnutlsxx -llz4 -lprotobuf -ldl -lgcc_s
            -lyaml-cpp -lhwloc -lnuma -lpciaccess -lxml2 -lz
            -lcares-seastar
            -lfmt
            -lhttp_parser
            )
ENDIF()

