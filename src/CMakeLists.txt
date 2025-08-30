add_library(libskydebug libskydebug.cpp)
add_library(skydebug::libskydebug ALIAS libskydebug)

set_target_properties(
    libskydebug
    PROPERTIES OUTPUT_NAME skydebug
)

target_compile_features(
    libskydebug
    PUBLIC cxx_std_17
)

target_include_directories(
    libskydebug
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/include
)

include(GNUInstallDirs)
install(TARGETS libskydebug 
EXPORT skydebug-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
DIRECTORY ${PROJECT_SOURCE_DIR}/include/
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
EXPORT skydebug-targets
FILE skydebug-config.cmake
NAMESPACE skydebug::
DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skydebug
)