cmake_minimum_required(VERSION 3.17)
project(steam_matchmaking C)

set(deps ../../deps/)
set(CMAKE_C_STANDARD 99)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(isDebug TRUE)
else()
  message("no debug")
  set(isDebug FALSE)
endif()

if(isDebug)
  message("Debug build detected")
  add_compile_definitions(CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)
  set(CONFIGURATION_DEBUG 1)
endif()


if(WIN32)
  file(GLOB_RECURSE platform_src FOLLOW_SYMLINKS "${platform}/windows/*.c")

  set(OS_WINDOWS 1)
elseif(APPLE)
  message("macos detected")
  set(OS_MAC 1)
endif()

file(GLOB_RECURSE deps_src FOLLOW_SYMLINKS "${deps}/*/*/src/lib/*.c")

message("found ${deps_src}")

add_executable(steam_matchmaking_example main.c ${deps_src})

if(OS_MAC)
  target_compile_definitions(steam_matchmaking_example PRIVATE TORNADO_OS_MACOS)
elseif(OS_WINDOWS)
  target_compile_definitions(steam_matchmaking_example
                             PRIVATE TORNADO_OS_WINDOWS)
endif()

target_include_directories(steam_matchmaking_example
                           PRIVATE ${deps}piot/clog/src/include)
target_include_directories(steam_matchmaking_example
                           PRIVATE ${deps}piot/tiny-libc/src/include)

if(OS_WINDOWS)
  target_link_libraries(steam_matchmaking_example steam_matchmaking)
else()
  target_link_libraries(steam_matchmaking_example steam_matchmaking m)
endif()
