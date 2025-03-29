# ------------------------------------------------------------------------------
# CMAKE SETUP
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.19)

project("Sokol-GLFW Glue")

# TODO : Add desktop OpenGL.
if(NOT APPLE AND NOT WIN32)
  message(FATAL_ERROR "Unsupported SGG platform. Must be either macOS, or Windows.")
endif()

if(CMAKE_VERSION VERSION_LESS 3.21)
  get_property(PARENT_SOURCE_DIRECTORY DIRECTORY PROPERTY PARENT_DIRECTORY)

  if(NOT PARENT_SOURCE_DIRECTORY)
    set(PROJECT_IS_TOP_LEVEL true)
  endif()
endif()

option(SGG_BUILD_EXAMPLE "Build SGG Example" ${PROJECT_IS_TOP_LEVEL})
option(SGG_CHECK_DEPENDENCIES "Check SGG Dependencies" ON)

if(SGG_BUILD_EXAMPLE)
  add_subdirectory(example)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sokol_glfw_glue_example)
endif()

if(SGG_CHECK_DEPENDENCIES)
  if(NOT TARGET glfw)
    message(FATAL_ERROR "Ensure that \"glfw\" is available as a target.")
  endif()
  if(NOT sokol_SOURCE_DIR)
    message(FATAL_ERROR "Ensure that \"sokol_SOURCE_DIR\" is defined.")
  endif()
endif()

# ------------------------------------------------------------------------------
# MAIN TARGET / SGG LIBRARY
# ------------------------------------------------------------------------------

set(NAME sokol_glfw_glue)

add_library(${NAME} INTERFACE)

target_include_directories(${NAME} INTERFACE .)
