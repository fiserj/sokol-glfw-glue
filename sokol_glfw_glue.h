#ifndef SOKOL_GLFW_GLUE_H
#define SOKOL_GLFW_GLUE_H

#include <stdbool.h> // bool

#ifdef __cplusplus
extern "C" {
#endif

struct GLFWwindow;

struct sg_environment;

struct sg_swapchain;

// TODO : Add support for depth buffer, MSAA, etc., as needed.
typedef struct sgg_environment_desc {
  // Window to render to.
  struct GLFWwindow* window;

  // Minimum width of the backbuffer. Set to 0 to use the current window size.
  int backbuffer_min_width;

  // Minimum height of the backbuffer. Set to 0 to use the current window size.
  int backbuffer_min_height;

  // If true, the backbuffer size will never be downsized. This is useful if you
  // want to have smooth rendering during window resizing (assuming the GLFW
  // window is resizable).
  bool backbuffer_never_downsize;

} sgg_environment_desc;

// Initializes the backend for a given window, and returns the sokol environment
// descriptor used in `sg_setup` call.
struct sg_environment sgg_environment(const sgg_environment_desc* desc);

// Returns the swapchain descriptor used in `sg_setup` call on every frame.
struct sg_swapchain sgg_swapchain(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GLFW_GLUE_H
