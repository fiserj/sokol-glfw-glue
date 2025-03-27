#if !__has_feature(objc_arc)
#  error Objective-C's ARC is off. Use "-fobjc-arc" compiler flag to enable it.
#endif

#include "sokol_glfw_glue.h"

#include <assert.h> // assert
#include <stddef.h> // NULL
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

#include "sokol_glfw_glue_common.h"

static struct {
  sgg_environment_desc desc;
  CAMetalLayer*        layer;
  id<MTLDevice>        device;
  id<CAMetalDrawable>  drawable;

} g_state = {0};

sg_environment sgg_environment(const sgg_environment_desc* desc) {
  assert(desc != NULL);
  assert(desc->window != NULL);
  assert(desc->backbuffer_min_width >= 0);
  assert(desc->backbuffer_min_height >= 0);

  if (g_state.desc.window == NULL) {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.opaque        = YES;
    layer.device        = device;
    layer.pixelFormat   = MTLPixelFormatBGRA8Unorm;

    NSWindow* ns_window              = glfwGetCocoaWindow(desc->window);
    ns_window.contentView.layer      = layer;
    ns_window.contentView.wantsLayer = YES;

    g_state.desc     = *desc;
    g_state.layer    = layer;
    g_state.device   = device;
    g_state.drawable = nil;
  }

  assert(g_state.desc.window == desc->window);

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

static CGFloat resolve_size(CGFloat current_size, CGFloat requested_size, CGFloat min_size, bool never_downsize) {
  if (min_size > requested_size) {
    requested_size = min_size;
  }

  if (requested_size < current_size && never_downsize) {
    return current_size;
  }

  return requested_size;
}

sg_swapchain sgg_swapchain(void) {
  assert(g_state.desc.window != NULL);

  int width, height;
  glfwGetFramebufferSize(g_state.desc.window, &width, &height);

  // clang-format off
  CGFloat drawable_width  = resolve_size(g_state.layer.drawableSize.width , width , g_state.desc.backbuffer_min_width , g_state.desc.backbuffer_never_downsize);
  CGFloat drawable_height = resolve_size(g_state.layer.drawableSize.height, height, g_state.desc.backbuffer_min_height, g_state.desc.backbuffer_never_downsize);
  // clang-format on

  if (drawable_width != g_state.layer.drawableSize.width || drawable_height != g_state.layer.drawableSize.height) {
#ifndef NDEBUG
    printf("Drawable resized: %4.0f x %4.0f px\n", drawable_width, drawable_height);
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

void sgg_present(void) {
  assert(g_state.desc.window != NULL);
}

void sgg_shutdown(void) {
  assert(g_state.desc.window != NULL);
}
