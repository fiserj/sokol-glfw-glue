# ------------------------------------------------------------------------------
# CMAKE SETUP
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.19)

project("Sokol-GLFW Glue")

if(CMAKE_VERSION VERSION_LESS 3.21)
  get_property(PARENT_SOURCE_DIRECTORY DIRECTORY PROPERTY PARENT_DIRECTORY)

  if(NOT PARENT_SOURCE_DIRECTORY)
    set(PROJECT_IS_TOP_LEVEL true)
  endif()
endif()

# ------------------------------------------------------------------------------
# TEST EXECUTABLE
# ------------------------------------------------------------------------------

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(test)
else()
  if(NOT TARGET glfw)
    message(FATAL_ERROR "Ensure that \"glfw\" is available as a target.")
  endif()
  if(NOT sokol_SOURCE_DIR)
    message(FATAL_ERROR "Ensure that \"sokol_SOURCE_DIR\" is defined.")
  endif()
endif()

# ------------------------------------------------------------------------------
# MAIN LIBRARY
# ------------------------------------------------------------------------------

set(NAME sokol_glfw_glue)

add_library(${NAME} STATIC
  ${NAME}.h
)

target_include_directories(${NAME}
  PUBLIC
    .
  PRIVATE
    ${sokol_SOURCE_DIR}
)

target_link_libraries(${NAME} PRIVATE
  glfw
)

if(APPLE)
  target_sources(${NAME} PRIVATE
    ${NAME}_metal.m
  )
  target_compile_options(${NAME} PRIVATE
    -fobjc-arc
  )
  target_link_libraries(${NAME} PRIVATE
    "-framework Metal"
  )
else()
  message(FATAL_ERROR "Unsupported ${PROJECT_NAME} platform.")
endif()

set_target_properties(${NAME} PROPERTIES
  C_STANDARD 99
  C_EXTENSIONS OFF
  C_STANDARD_REQUIRED ON
)