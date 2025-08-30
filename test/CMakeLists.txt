add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE skydebug::libskydebug Catch2::Catch2WithMain)
