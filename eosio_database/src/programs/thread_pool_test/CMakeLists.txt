add_executable(thread_pool_test main.cpp)


target_link_libraries(
        thread_pool_test
        boost_system
        boost_thread-mt
)