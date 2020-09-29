# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include(FetchContent)
FetchContent_Declare(
  esp-ena
  GIT_REPOSITORY https://github.com/Lurkars/esp-ena.git
  GIT_TAG        origin/component
)
FetchContent_MakeAvailable(esp-ena)
set(EXTRA_COMPONENT_DIRS ${esp-ena_SOURCE_DIR}/components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp-ena-device)