# CMakeList.txt : CMake project for SDLPlatformer, include source and define
# project specific logic here.
#
find_package(SDL3 REQUIRED)

# Add source to this project's executable.
add_executable (SDLPlatformer "SDLPlatformer.cpp" "SDLPlatformer.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SDLPlatformer PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(SDLPlatformer PRIVATE SDL3::SDL3)
# TODO: Add tests and install targets if needed.
