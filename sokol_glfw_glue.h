#ifndef SOKOL_GLFW_GLUE_H
#define SOKOL_GLFW_GLUE_H

// sokol_glfw_glue.h -- glue helper functions to use sokol-gfx with GLFW
//
// Project URL: https://github.com/fiserj/sokol-glfw-glue
//
// Do this:
//   #define SOKOL_IMPL, or
//   #define SOKOL_GLFW_GLUE_IMPL
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// In the same place, define one of the following to select the rendering
// backend (the backends not listed here are not supported at the moment):
//   #define SOKOL_D3D11
//   #define SOKOL_METAL
//
// I.e., for the macOS, it could look like this:
//
// #include ...
// #include ...
// #define SOKOL_IMPL
// #define SOKOL_METAL
// #include "sokol_gfx.h"
// #include "sokol_glfw_glue.h"
//
// sokol_gfx DOES NOT:
// ===================
// - initialize GLFW, or create a GLFW window
//
// - set up sokol_gfx
//
//
// STEP BY STEP
// ============
// - initialize GLFW and create a new window:
//
//     glfwInit();
//
//     glfwDefaultWindowHints();
//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//     glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
//
//     GLFWwindow* window = glfwCreateWindow(...);
//
// - when setting up sokol_gfx, use the sgg_environment function to get the
//   environment descriptor, and pass it to the sg_setup function:
//
//     sg_setup(&(sg_desc){
//       // ...
//       .environment = sgg_environment(&(sgg_environment_desc){
//         // ...
//         .window = window,
//       }),
//     });
//
//   The window is the only required parameter in the sgg_environment_desc
//   struct.
//
// - when rendering, use the sgg_swapchain function to get the swapchain
//   descriptor, and pass it to the sg_begin_pass function:
//
//     sg_begin_pass(&(sg_pass){
//       // ...
//       .swapchain = sgg_swapchain(),
//     });
//
//   After finshing issuing sokol gfx commands, call sgg_present to present the
//   rendered frame to the window:
//
//     // ...
//     sg_commit();
//     sgg_present();
//
// - when shutting down, call sgg_shutdown after sg_shutdown, but before
//   destroying the GLFW window:
//
//     // ...
//     sg_shutdown();
//     sgg_shutdown();
//     glfwDestroyWindow(window);
//
//
// LICENSE
// =======
// MIT License
//
// Copyright (c) 2025 Jakub Fiser
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

  // Minimum required width of the backbuffer. Set to 0 to use the exact window
  // width.
  int backbuffer_min_width;

  // Minimum required height of the backbuffer. Set to 0 to use the exact window
  // height.
  int backbuffer_min_height;

  // If `true`, the backbuffer size will never be downsized. This is useful if
  // you want to have smooth rendering during window resizing (assuming the GLFW
  // window is resizable).
  bool backbuffer_never_downsize;

  // If `true`, vsync will be disabled at the start. Use `sgg_toggle_vsync` to
  // change it at runtime.
  bool vsync_disabled;

} sgg_environment_desc;

// Initializes the backend for a given window, and returns the sokol environment
// descriptor used in `sg_setup` call.
struct sg_environment sgg_environment(const sgg_environment_desc* desc);

// Returns the swapchain descriptor used in `sg_setup` call on every frame.
struct sg_swapchain sgg_swapchain(void);

// Presents the rendered frame to the window. Needs to be called after
// `sg_commit`.
void sgg_present(void);

// Destroys internal resources. Call this after `sg_shutdown` but before you
// destroy the GLFW window.
void sgg_shutdown(void);

// Toggles the vsync. The initial state is determined by the `vsync_disabled`
// field in the `sgg_environment_desc` struct.
void sgg_toggle_vsync(void);

// Helper function to retrieve the maximum size of any of the connected monitors
// (as per GLFW's reporting).
void sgg_max_monitor_size(int* width, int* height);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOKOL_GLFW_GLUE_H

// ---------------------- Implementation follows below ---------------------- //

#if defined(SOKOL_GLFW_GLUE_IMPL) || defined(SOKOL_IMPL)

#define SOKOL_GLFW_GLUE_IMPL_INCLUDED (1)

#if !(defined(SOKOL_D3D11) || defined(SOKOL_METAL))
#  error "Please select one of the supported backends: SOKOL_D3D11 or SOKOL_METAL"
#endif

// Note: Relying on the headers that are being included from sokol_gfx.h.

#ifdef SOKOL_DEBUG
#  include <stdio.h> // printf
#endif

#if defined(SOKOL_D3D11)
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <d3d11_1.h> // DXGI*, ID3D11*, IDXGI*
#  include <dxgi1_3.h> // DXGIGetDebugInterface1
#  ifdef SOKOL_DEBUG
#    include <dxgidebug.h> // ...
#  endif
#  ifdef _MSC_VER
#    pragma comment(lib, "dxguid")
#  endif
#elif defined(SOKOL_METAL)
#  if !__has_feature(objc_arc)
#    error Objective-C's ARC is off. Use "-fobjc-arc" compiler flag to enable it.
#  endif
#  define GLFW_EXPOSE_NATIVE_COCOA
#  import <Cocoa/Cocoa.h> // NSWindow
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>       // glfw*
#include <GLFW/glfw3native.h> // glfwGet*Window

// clang-format off
typedef struct {
  sgg_environment_desc    desc;
#if defined(SOKOL_D3D11)
  ID3D11Device*           base_device;
  ID3D11DeviceContext*    base_device_context;
  ID3D11Device1*          device;
  ID3D11DeviceContext1*   device_context;
  IDXGISwapChain1*        swapchain;
  DXGI_SWAP_CHAIN_DESC1   swapchain_desc;
  ID3D11RenderTargetView* render_target_view;
#elif defined(SOKOL_METAL)
  CAMetalLayer*           layer;
  id<MTLDevice>           device;
  id<CAMetalDrawable>     drawable;
#endif // SOKOL_* backend
} sgg__state;
// clang-format on

static sgg__state g_sgg_state = {0};

#if defined(SOKOL_D3D11)
// `sokol_gfx.h` doesn't define COBJMACROS before including D3D11 and DXGI
// headers, and now it's too late.
#  if !defined(__cplusplus) && !defined(COBJMACROS)
#    define ID3D11Device1_Release(This)                                                                                       ((This)->lpVtbl->Release(This))
#    define ID3D11Device_CreateRenderTargetView(This, pResource, pDesc, ppRTView)                                             ((This)->lpVtbl->CreateRenderTargetView(This, pResource, pDesc, ppRTView))
#    define ID3D11Device_QueryInterface(This, riid, ppvObject)                                                                ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))
#    define ID3D11Device_Release(This)                                                                                        ((This)->lpVtbl->Release(This))
#    define ID3D11DeviceContext1_QueryInterface(This, riid, ppvObject)                                                        ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))
#    define ID3D11DeviceContext1_Release(This)                                                                                ((This)->lpVtbl->Release(This))
#    define ID3D11DeviceContext_QueryInterface(This, riid, ppvObject)                                                         ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))
#    define ID3D11DeviceContext_Release(This)                                                                                 ((This)->lpVtbl->Release(This))
#    define ID3D11RenderTargetView_Release(This)                                                                              ((This)->lpVtbl->Release(This))
#    define ID3D11Texture2D_Release(This)                                                                                     ((This)->lpVtbl->Release(This))
#    define IDXGIAdapter1_GetParent(This, riid, ppParent)                                                                     ((This)->lpVtbl->GetParent(This, riid, ppParent))
#    define IDXGIAdapter1_Release(This)                                                                                       ((This)->lpVtbl->Release(This))
#    define IDXGIDevice1_Release(This)                                                                                        ((This)->lpVtbl->Release(This))
#    define IDXGIDevice_GetParent(This, riid, ppParent)                                                                       ((This)->lpVtbl->GetParent(This, riid, ppParent))
#    define IDXGIFactory2_CreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain) ((This)->lpVtbl->CreateSwapChainForHwnd(This, pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain))
#    define IDXGIFactory2_Release(This)                                                                                       ((This)->lpVtbl->Release(This))
#    define IDXGISwapChain1_GetBackgroundColor(This, pColor)                                                                  ((This)->lpVtbl->GetBackgroundColor(This, pColor))
#    define IDXGISwapChain1_GetBuffer(This, Buffer, riid, ppSurface)                                                          ((This)->lpVtbl->GetBuffer(This, Buffer, riid, ppSurface))
#    define IDXGISwapChain1_Present(This, SyncInterval, Flags)                                                                ((This)->lpVtbl->Present(This, SyncInterval, Flags))
#    define IDXGISwapChain1_Release(This)                                                                                     ((This)->lpVtbl->Release(This))
#    define IDXGISwapChain1_ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags)                        ((This)->lpVtbl->ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags))
#  endif // !__cplusplus && !COBJMACROS

static void sgg__platform_init(sgg__state* state) {
  D3D_FEATURE_LEVEL    feature_levels[] = {D3D_FEATURE_LEVEL_11_1};
  ID3D11Device*        base_device;
  ID3D11DeviceContext* base_device_context;

  UINT device_flags = 0;
  device_flags |= D3D11_CREATE_DEVICE_SINGLETHREADED; // TODO : Runtime configurable ?
  device_flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#  ifdef SOKOL_DEBUG
  device_flags |= D3D11_CREATE_DEVICE_DEBUG; // TODO : Runtime configurable ?
#  endif

  HRESULT hr = D3D11CreateDevice(
    NULL,
    D3D_DRIVER_TYPE_HARDWARE,
    NULL,
    device_flags,
    feature_levels,
    1,
    D3D11_SDK_VERSION,
    &base_device,
    NULL,
    &base_device_context);
  SOKOL_ASSERT(SUCCEEDED(hr));

  ID3D11Device1* device;
  hr = ID3D11Device_QueryInterface(base_device, &IID_ID3D11Device1, (void**)(&device));
  SOKOL_ASSERT(SUCCEEDED(hr));

  ID3D11DeviceContext1* device_context;
  hr = ID3D11DeviceContext_QueryInterface(base_device_context, &IID_ID3D11DeviceContext1, (void**)&device_context);
  SOKOL_ASSERT(SUCCEEDED(hr));

  IDXGIDevice1* dxgi_device;
  hr = ID3D11Device_QueryInterface(device, &IID_IDXGIDevice1, (void**)&dxgi_device);
  SOKOL_ASSERT(SUCCEEDED(hr));

  IDXGIAdapter1* adapter;
  hr = IDXGIDevice_GetParent(dxgi_device, &IID_IDXGIAdapter1, (void**)&adapter);
  SOKOL_ASSERT(SUCCEEDED(hr));

  IDXGIFactory2* factory;
  hr = IDXGIAdapter1_GetParent(adapter, &IID_IDXGIFactory2, (void**)&factory);
  SOKOL_ASSERT(SUCCEEDED(hr));

  DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
    .Format           = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc.Count = 1,
    .BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount      = 2,
    .Scaling          = DXGI_SCALING_NONE,
    .Flags            = 0,
    .SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD,
  };

  IDXGISwapChain1* swapchain;
  hr = IDXGIFactory2_CreateSwapChainForHwnd(
    factory,
    (IUnknown*)device,
    glfwGetWin32Window(state->desc.window),
    &swapchain_desc,
    NULL,
    NULL,
    &swapchain);
  SOKOL_ASSERT(SUCCEEDED(hr));

  IDXGIFactory2_Release(factory);
  IDXGIAdapter1_Release(adapter);
  IDXGIDevice1_Release(dxgi_device);

  state->base_device         = base_device;
  state->base_device_context = base_device_context;
  state->device              = device;
  state->device_context      = device_context;
  state->swapchain           = swapchain;
  state->swapchain_desc      = swapchain_desc;
}

static void sgg__platform_environment(const sgg__state* state, sg_environment* env) {
  env->d3d11.device         = state->base_device;
  env->d3d11.device_context = state->base_device_context;
}

static void sgg__platform_swapchain_backbuffer_size(sgg__state* state, double* width, double* height) {
  *width  = state->swapchain_desc.Width;
  *height = state->swapchain_desc.Height;
}

static void sgg__platform_resize_swapchain_backbuffer(sgg__state* state, int width, int height) {
  if (state->render_target_view) {
    ID3D11RenderTargetView_Release(state->render_target_view);
  }

  if (width == 0 || height == 0) {
    return;
  }

  state->swapchain_desc.Width  = (UINT)width;
  state->swapchain_desc.Height = (UINT)height;

  HRESULT hr = IDXGISwapChain1_ResizeBuffers(state->swapchain, 0, (UINT)width, (UINT)height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
  SOKOL_ASSERT(SUCCEEDED(hr));

  ID3D11Texture2D* backbuffer;
  hr = IDXGISwapChain1_GetBuffer(state->swapchain, 0, &IID_ID3D11Texture2D, (void**)&backbuffer);
  SOKOL_ASSERT(SUCCEEDED(hr));

  hr = ID3D11Device_CreateRenderTargetView(state->device, (ID3D11Resource*)backbuffer, NULL, &state->render_target_view);
  SOKOL_ASSERT(SUCCEEDED(hr));

  ID3D11Texture2D_Release(backbuffer);
}

static void sgg__platform_swapchain(sgg__state* state, sg_swapchain* swapchain) {
  swapchain->d3d11.render_view = state->render_target_view;
}

static void sgg__platform_present(sgg__state* state) {
  HRESULT hr = IDXGISwapChain1_Present(state->swapchain, state->desc.vsync_disabled ? 0 : 1, 0);
  SOKOL_ASSERT(SUCCEEDED(hr));
  _SOKOL_UNUSED(hr);
}

static void sgg__platform_shutdown(sgg__state* state) {
  ID3D11Device_Release(state->base_device);
  ID3D11DeviceContext_Release(state->base_device_context);
  ID3D11Device1_Release(state->device);
  ID3D11DeviceContext1_Release(state->device_context);
  IDXGISwapChain1_Release(state->swapchain);
  ID3D11RenderTargetView_Release(state->render_target_view);

#  ifdef SOKOL_DEBUG
  IDXGIDebug* debug;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, &IID_IDXGIDebug, (void**)&debug))) {
    debug->lpVtbl->ReportLiveObjects(debug, DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    debug->lpVtbl->Release(debug);
  }
#  endif // SOKOL_DEBUG
}
#elif defined(SOKOL_METAL)
static void sgg__platform_init(sgg__state* state) {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();

  CAMetalLayer* layer      = [CAMetalLayer layer];
  layer.opaque             = YES;
  layer.device             = device;
  layer.pixelFormat        = MTLPixelFormatBGRA8Unorm;
  layer.displaySyncEnabled = !state->desc.vsync_disabled;

  NSWindow* ns_window              = glfwGetCocoaWindow(state->desc.window);
  ns_window.contentView.layer      = layer;
  ns_window.contentView.wantsLayer = YES;

  state->layer    = layer;
  state->device   = device;
  state->drawable = nil;
}

static void sgg__platform_environment(const sgg__state* state, sg_environment* env) {
  env->metal.device = (__bridge const void*)state->device;
}

static void sgg__platform_swapchain_backbuffer_size(sgg__state* state, double* width, double* height) {
  *width  = state->layer.drawableSize.width;
  *height = state->layer.drawableSize.height;
}

static void sgg__platform_resize_swapchain_backbuffer(sgg__state* state, int width, int height) {
  state->layer.drawableSize = CGSizeMake(width, height);
}

static void sgg__platform_swapchain(sgg__state* state, sg_swapchain* swapchain) {
  state->drawable = [state->layer nextDrawable];

  swapchain->metal.current_drawable = (__bridge const void*)state->drawable;
}

static void sgg__platform_present(sgg__state* state) {
  _SOKOL_UNUSED(state);
}

static void sgg__platform_shutdown(sgg__state* state) {
  _SOKOL_UNUSED(state);
}
#endif // SOKOL_* backend

static double sgg__resolve_size(double current_size, double requested_size, double min_size, bool never_downsize) {
  if (min_size > requested_size) {
    requested_size = min_size;
  }

  if (requested_size < current_size && never_downsize) {
    return current_size;
  }

  return requested_size;
}

sg_environment sgg_environment(const sgg_environment_desc* desc) {
  SOKOL_ASSERT(desc);
  SOKOL_ASSERT(desc->window);
  SOKOL_ASSERT(desc->backbuffer_min_width >= 0);
  SOKOL_ASSERT(desc->backbuffer_min_height >= 0);

  if (!g_sgg_state.desc.window) {
    g_sgg_state.desc = *desc;
    sgg__platform_init(&g_sgg_state);
  }

  SOKOL_ASSERT(g_sgg_state.desc.window == desc->window);

  sg_environment env = {
    .defaults = {
      .color_format = SG_PIXELFORMAT_BGRA8,
      .depth_format = SG_PIXELFORMAT_NONE,
      .sample_count = 1,
    },
  };
  sgg__platform_environment(&g_sgg_state, &env);

  return env;
}

sg_swapchain sgg_swapchain(void) {
  if (!g_sgg_state.desc.window) {
    SOKOL_ASSERT(false && "sgg was not initialized");
    return (sg_swapchain){0};
  }

  int width, height;
  glfwGetFramebufferSize(g_sgg_state.desc.window, &width, &height);

  double curr_width, curr_height;
  sgg__platform_swapchain_backbuffer_size(&g_sgg_state, &curr_width, &curr_height);

  // clang-format off
  double new_width  = sgg__resolve_size(curr_width , width , g_sgg_state.desc.backbuffer_min_width , g_sgg_state.desc.backbuffer_never_downsize);
  double new_height = sgg__resolve_size(curr_height, height, g_sgg_state.desc.backbuffer_min_height, g_sgg_state.desc.backbuffer_never_downsize);
  // clang-format on

  if (new_width != curr_width || new_height != curr_height) {
#ifdef SOKOL_DEBUG
    printf("Drawable resized: %4.0f x %4.0f px\n", new_width, new_height);
#endif
    SOKOL_ASSERT(new_width == (int)new_width);
    SOKOL_ASSERT(new_height == (int)new_height);
    sgg__platform_resize_swapchain_backbuffer(&g_sgg_state, (int)new_width, (int)new_height);
  }

  sg_swapchain swapchain = {
    .width        = width,
    .height       = height,
    .sample_count = 1,
    .color_format = SG_PIXELFORMAT_BGRA8,
    .depth_format = SG_PIXELFORMAT_NONE,
  };
  sgg__platform_swapchain(&g_sgg_state, &swapchain);

  return swapchain;
}

void sgg_present(void) {
  if (!g_sgg_state.desc.window) {
    SOKOL_ASSERT(false && "sgg was not initialized");
    return;
  }

  sgg__platform_present(&g_sgg_state);
}

void sgg_shutdown(void) {
  if (!g_sgg_state.desc.window) {
    SOKOL_ASSERT(false && "sgg was not initialized");
    return;
  }

  sgg__platform_shutdown(&g_sgg_state);

  g_sgg_state = (sgg__state){0};
}

void sgg_toggle_vsync(void) {
  if (!g_sgg_state.desc.window) {
    SOKOL_ASSERT(false && "sgg was not initialized");
    return;
  }

  g_sgg_state.desc.vsync_disabled = !g_sgg_state.desc.vsync_disabled;
}

void sgg_max_monitor_size(int* width, int* height) {
  int           count    = 0;
  GLFWmonitor** monitors = glfwGetMonitors(&count);

  int max_width  = 0;
  int max_height = 0;

  for (int i = 0; i < count; i++) {
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

    float x_scale, y_scale;
    glfwGetMonitorContentScale(monitors[i], &x_scale, &y_scale);

    int mode_width  = (int)(x_scale * mode->width);
    int mode_height = (int)(y_scale * mode->height);

    if (mode_width > max_width) {
      max_width = mode_width;
    }
    if (mode_height > max_height) {
      max_height = mode_height;
    }
  }

  if (width != NULL) {
    *width = max_width;
  }
  if (height != NULL) {
    *height = max_height;
  }
}

#endif // SOKOL_GLFW_GLUE_IMPL || SOKOL_IMPL
