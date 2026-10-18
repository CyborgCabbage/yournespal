find_package(lodepng CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

add_library (main STATIC "App/app.cpp" "App/config.cpp" "Asset/image.cpp" "Asset/palette.cpp" "Asset/pixel.cpp" "Asset/tileset.cpp" "Util/compute.cpp" "Util/log.cpp" "Util/validation.cpp" "Util/mdvector.h")
target_link_libraries(main PUBLIC lodepng)
target_link_libraries(main PRIVATE tomlplusplus::tomlplusplus)
target_precompile_headers(main PRIVATE "common.h")

target_include_directories(main PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
