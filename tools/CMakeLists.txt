add_executable(skydebug skydebug.cpp)
target_link_libraries(skydebug PRIVATE skydebug::libskydebug PkgConfig::readline)

include(GNUInstallDirs)
install(
TARGETS skydebug
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

