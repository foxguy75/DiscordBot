add_executable( DnDDiscordBot main.cxx )

set_property(TARGET DnDDiscordBot PROPERTY CXX_STANDARD 20)

target_include_directories( DnDDiscordBot PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    )