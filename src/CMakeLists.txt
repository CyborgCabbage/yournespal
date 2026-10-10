find_package(Eigen3 CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

add_library (main STATIC "App/app.cpp" "Asset/sprite.cpp" "Core/core.cpp" "Asset/pixel.cpp" "Asset/palette.cpp")
target_link_libraries(main PRIVATE Eigen3::Eigen)
target_link_libraries(main PUBLIC lodepng)
target_link_libraries(main PRIVATE tomlplusplus::tomlplusplus)
target_precompile_headers(main PRIVATE "common.h")

target_include_directories(main PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(main PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)