add_executable( DnDDiscordBot main.cxx constants.hxx )

add_library( CommandHandlers hordcommandhandler.cxx hordcommandhandler.hxx constants.hxx )

set_property(TARGET DnDDiscordBot PROPERTY CXX_STANDARD 20)
set_property(TARGET CommandHandlers PROPERTY CXX_STANDARD 20)

find_package(fmt REQUIRED)

find_package(nlohmann_json 3.11.2 REQUIRED)

target_include_directories( DnDDiscordBot PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "E:/Repos/Dpp/include/dpp-10.0"
    )

target_include_directories( CommandHandlers PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "E:/Repos/Dpp/include/dpp-10.0"
    )

target_link_directories( DnDDiscordBot PUBLIC "E:/Repos/Dpp/lib/dpp-10.0" "E:/Repos/Dpp/bin" )

target_link_libraries( CommandHandlers fmt::fmt dpp nlohmann_json::nlohmann_json )

target_link_libraries( DnDDiscordBot fmt::fmt dpp CommandHandlers )