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
  GIT_TAG        b35641f4a3c62aa86a0b3c983d163bc0fe36026d
)

FetchContent_Declare(
  sokol
  GIT_REPOSITORY https://github.com/floooh/sokol.git
  GIT_TAG        bcdf25ae58c4fe82cd444ea1ce3f1b8f2532c7ed
)

FetchContent_Declare(
  sokol-tools-bin
  GIT_REPOSITORY https://github.com/floooh/sokol-tools-bin.git
  GIT_TAG        a86d0d152dd6a8488f892a223fa377a278a78195
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
# MAIN TARGET
# ------------------------------------------------------------------------------

set(NAME sokol_glfw_glue_test)

add_executable(${NAME}
  shaders.h
  sokol_glfw_glue_test.c
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
    sokol_glfw_glue_test.c
    PROPERTIES
    COMPILE_OPTIONS "-xobjective-c"
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
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${NAME}> "${CMAKE_SOURCE_DIR}/test/bin/$<TARGET_FILE_NAME:${NAME}>"
)
