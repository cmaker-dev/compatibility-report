
cmake_minimum_required( VERSION 3.10 )
project(
  cmaker-compat-test
  VERSION 0.0.1
  
  LANGUAGES CXX
)
  
  

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR}
  "src/**.cpp"
  "src/**.c"
  "src/**/**.cpp"
  "src/**/**.c"
)

message("Sources: ${SOURCES}")

include_directories($CMAKE_SOURCE_DIR/include)
set(HEADER_DIR $CMAKE_SOURCE_DIR/include)

if(NOT DEFINED RELEASE)
  set(RELEASE 0)
endif()

add_executable(cmaker-compat-test ${SOURCES})
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions(-DRelease)

  set_target_properties(cmaker-compat-test PROPERTIES COMPILE_FLAGS "-O2 ")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDebug)

  set_target_properties(cmaker-compat-test PROPERTIES COMPILE_FLAGS "-g")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Test")
  add_definitions(-DTest)

  set_target_properties(cmaker-compat-test PROPERTIES COMPILE_FLAGS "-g")
endif()
set(BUILD_DIR build)
set_target_properties(cmaker-compat-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY build)


target_link_libraries(cmaker-compat-test nlohmann_json)
install(TARGETS cmaker-compat-test DESTINATION bin)

