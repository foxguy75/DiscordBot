cmake_minimum_required( VERSION 3.20 )

project( DnDDiscordBot LANGUAGES CXX VERSION 0.0.1 )

set( CONAN_DISABLE_CHECK_COMPILER true )

# include(${PROJECT_SOURCE_DIR}/build/conanbuildinfo.cmake)
# conan_basic_setup()

add_subdirectory( src )
add_subdirectory( config )
