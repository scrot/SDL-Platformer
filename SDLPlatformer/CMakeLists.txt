# CMakeList.txt : CMake project for SDLPlatformer, include source and define
# project specific logic here.
#
set(CMAKE_PREFIX_PATH /SDKs/tinyxml2-11.0.0)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(GLM REQUIRED)

# Add source to this project's executable.
add_executable (SDLPlatformer "SDLPlatformer.cpp" "include/SDLPlatformer.h" "include/timer.h" "include/animation.h" "include/gameobject.h" "include/tmx.h" "tmx.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SDLPlatformer PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(SDLPlatformer PRIVATE SDL3::SDL3 SDL3_image::SDL3_image glm::glm)

# TODO: Add tests and install targets if needed.
