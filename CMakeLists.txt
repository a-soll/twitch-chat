cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

project(twitchchat VERSION 1.0)
include(GNUInstallDirs)

add_library(twitchchat
    irc/main_irc.c
    irc/parse.c
)

set_target_properties(twitchchat PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(twitchchat PROPERTIES PUBLIC_HEADER twitchchat.h)
set_target_properties(twitchchat PROPERTIES PRIVATE_HEADER "irc/main_irc.h;irc/parse.h")

install(TARGETS twitchchat LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/twitchchat
    PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/twitchchat/irc
)
