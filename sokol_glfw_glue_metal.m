#if !__has_feature(objc_arc)
#  error Objective-C's ARC is off. Use "-fobjc-arc" compiler flag to enable it.
#endif

#include "sokol_glfw_glue.h"

#include <assert.h>  // assert
#include <stdbool.h> // bool
#include <stddef.h>  // NULL
#ifndef NDEBUG
#  include <stdio.h> // printf
#endif

#import <Cocoa/Cocoa.h>             // NSWindow
#import <Metal/Metal.h>             // MTLCreateSystemDefaultDevice
#import <QuartzCore/CAMetalLayer.h> // CAMetalDrawable, CAMetalLayer

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>       // glfw*
#include <GLFW/glfw3native.h> // glfwGetCocoaWindow

#include <sokol_gfx.h> // sg_environment, sg_swapchain

static struct {
  GLFWwindow*         window;
  CAMetalLayer*       layer;
  id<MTLDevice>       device;
  id<CAMetalDrawable> drawable;

} g_state = {0};

void sgg_init_for_window(GLFWwindow* window) {
  assert(g_state.window == NULL);

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();

  CAMetalLayer* layer = [CAMetalLayer layer];
  layer.opaque        = YES;
  layer.device        = device;
  layer.pixelFormat   = MTLPixelFormatBGRA8Unorm;

  NSWindow* ns_window              = glfwGetCocoaWindow(window);
  ns_window.contentView.layer      = layer;
  ns_window.contentView.wantsLayer = YES;

  g_state.window   = window;
  g_state.layer    = layer;
  g_state.device   = device;
  g_state.drawable = nil;
}

sg_environment sgg_environment(void) {
  assert(g_state.window != NULL);

  return (sg_environment){
    .defaults = {
      .color_format = SG_PIXELFORMAT_BGRA8,
      .depth_format = SG_PIXELFORMAT_NONE,
      .sample_count = 1,
    },
    .metal = {
      .device = (__bridge const void*)g_state.device,
    }};
}

sg_swapchain sgg_swapchain(void) {
  assert(g_state.window != NULL);

  int width, height;
  glfwGetFramebufferSize(g_state.window, &width, &height);

  g_state.layer.drawableSize = CGSizeMake(width, height);
  g_state.drawable           = [g_state.layer nextDrawable];

  return (sg_swapchain){
    .width                  = width,
    .height                 = height,
    .sample_count           = 1,
    .color_format           = SG_PIXELFORMAT_BGRA8,
    .depth_format           = SG_PIXELFORMAT_NONE,
    .metal.current_drawable = (__bridge const void*)g_state.drawable,
  };
}

sg_swapchain sgg_swapchain_with_full_monitor_size(void) {
  assert(g_state.window != NULL);

  int width, height;
  glfwGetFramebufferSize(g_state.window, &width, &height);

  int drawable_width  = g_state.layer.drawableSize.width;
  int drawable_height = g_state.layer.drawableSize.height;

  if (width > drawable_width) {
    drawable_width = width * 2;
  }
  if (height > drawable_height) {
    drawable_height = height * 2;
  }

  if (drawable_width != g_state.layer.drawableSize.width || drawable_height != g_state.layer.drawableSize.height) {
#ifndef NDEBUG
    printf("Drawable resized: %4d x %4d px\n", drawable_width, drawable_height);
#endif
    g_state.layer.drawableSize = CGSizeMake(drawable_width, drawable_height);
  }

  g_state.drawable = [g_state.layer nextDrawable];

  return (sg_swapchain){
    .width                  = width,
    .height                 = height,
    .sample_count           = 1,
    .color_format           = SG_PIXELFORMAT_BGRA8,
    .depth_format           = SG_PIXELFORMAT_NONE,
    .metal.current_drawable = (__bridge const void*)g_state.drawable,
  };

  return (sg_swapchain){0};
}
