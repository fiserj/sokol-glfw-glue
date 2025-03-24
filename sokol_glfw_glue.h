#ifndef SOKOL_GLFW_GLUE_H
#define SOKOL_GLFW_GLUE_H

#ifdef __cplusplus
extern "C" {
#endif

struct GLFWwindow;

struct sg_environment;

struct sg_swapchain;

// Initializes the framework for the provided window.
void sgg_init_for_window(struct GLFWwindow* window);

// Returns the `sg_environment` environment descriptor used in `sg_setup` call.
struct sg_environment sgg_environment(void);

// Returns the swapchain descriptor used in `sg_setup` call on every frame.
struct sg_swapchain sgg_swapchain(void);

// Same as above, but keeps the backing framebuffer size equal to the monitor
// size the window is on. This is mainly useful, if you want to have smooth
// rendering during window resizing (assuming the GLFW window is resizable).
struct sg_swapchain sgg_swapchain_with_full_monitor_size(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GLFW_GLUE_H
