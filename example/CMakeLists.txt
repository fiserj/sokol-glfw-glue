# ------------------------------------------------------------------------------
# DEPENDENCIES
# ------------------------------------------------------------------------------

include(FetchContent)

if(NOT COMMAND TURN_OFF)
  macro(TURN_OFF option)
    set(${option} OFF CACHE INTERNAL "" FORCE)
  endmacro()
endif()

TURN_OFF(GLFW_BUILD_DOCS)
TURN_OFF(GLFW_BUILD_EXAMPLES)
TURN_OFF(GLFW_BUILD_TESTS)
TURN_OFF(GLFW_DOCUMENT_INTERNALS)
TURN_OFF(GLFW_INSTALL)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        e7ea71be039836da3a98cea55ae5569cb5eb885c
)

FetchContent_Declare(
  sokol
  GIT_REPOSITORY https://github.com/floooh/sokol.git
  GIT_TAG        a9a2a5cea26701fc4b7fb8e2f95ac85d3f23ba9d
)

FetchContent_Declare(
  sokol-tools-bin
  GIT_REPOSITORY https://github.com/floooh/sokol-tools-bin.git
  GIT_TAG        9b5a3e2b57fe9783ba4d1f3249059bc4720b592f
)

FetchContent_MakeAvailable(
  glfw
  sokol
  sokol-tools-bin
)

set(sokol_SOURCE_DIR "${sokol_SOURCE_DIR}" PARENT_SCOPE)

# ------------------------------------------------------------------------------
# SHADERS' COMPILATION
# ------------------------------------------------------------------------------

if(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(SHDC_BIN "${sokol-tools-bin_SOURCE_DIR}/bin/osx_arm64/sokol-shdc")
  else()
    set(SHDC_BIN "${sokol-tools-bin_SOURCE_DIR}/bin/osx/sokol-shdc")
  endif()
  set(SHDC_SLANG "metal_macos")
  set(SHDC_BYTECODE "--bytecode")
elseif(WIN32)
  set(SHDC_BIN "${sokol-tools-bin_SOURCE_DIR}/bin/win32/sokol-shdc.exe")
  set(SHDC_SLANG "hlsl5")
  set(SHDC_BYTECODE "--bytecode")
else()
  message(FATAL_ERROR "Unsupported platform.")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(SHDC_BYTECODE "")
endif()

message(STATUS "Shader compiler: ${SHDC_BIN}")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/shaders.h"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders.glsl"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMAND ${SHDC_BIN}
    --input=shaders.glsl
    --output=shaders.h
    --slang=${SHDC_SLANG}
    ${SHDC_BYTECODE}
)

# ------------------------------------------------------------------------------
# MAIN TARGET / SGG EXAMPLE
# ------------------------------------------------------------------------------

set(NAME sokol_glfw_glue_example)

add_executable(${NAME}
  shaders.h
  sokol_glfw_glue_example.c
)

target_include_directories(${NAME} PRIVATE
  ${sokol_SOURCE_DIR}
)

target_link_libraries(${NAME} PRIVATE
  glfw
  sokol_glfw_glue
)

if(APPLE)
  set_source_files_properties(
    sokol_glfw_glue_example.c
    PROPERTIES
    COMPILE_OPTIONS "-xobjective-c"
  )
  target_compile_options(${NAME} PRIVATE
    -fobjc-arc
  )
  target_link_libraries(${NAME} PRIVATE
    "-framework Metal"
  )
endif()

if(MSVC)
  target_compile_definitions(${NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
  )
  target_compile_options(${NAME} PRIVATE
    /W4
  )
else()
  target_compile_options(${NAME} PRIVATE
    -pedantic
    -Wall
    -Wextra
    -Wno-missing-field-initializers
  )
endif()

set_target_properties(${NAME} PROPERTIES
  C_STANDARD 99
  C_EXTENSIONS OFF
  C_STANDARD_REQUIRED ON
  DEBUG_POSTFIX "_d"
)

add_custom_command(
  TARGET ${NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:${NAME}>"
)
