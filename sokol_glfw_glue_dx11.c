#include "sokol_glfw_glue.h"

// The rest of the D3D SDK is linked by sokol_gfx.
#pragma comment(lib, "dxguid")

#include <assert.h> // assert
#include <stddef.h> // NULL
#include <string.h> // memset
#ifndef NDEBUG
#  include <stdio.h> // printf
#endif

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <d3d11_1.h> // DXGI*, ID3D11*, IDXGI*
#ifndef NDEBUG
#  include <dxgidebug.h>
#endif

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>       // glfw*
#include <GLFW/glfw3native.h> // glfwGetWin32Window

#include <sokol_gfx.h> // sg_environment, sg_swapchain

#include "sokol_glfw_glue_common.h"

static struct {
  sgg_environment_desc    desc;
  ID3D11Device*           base_device;
  ID3D11DeviceContext*    base_device_context;
  ID3D11Device1*          device;
  ID3D11DeviceContext1*   device_context;
  IDXGISwapChain1*        swap_chain;
  DXGI_SWAP_CHAIN_DESC1   swap_chain_desc;
  ID3D11RenderTargetView* render_target_view;

} g_state = {0};

static const IID s_IID_Texture2D = {0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48, 0x95, 0x35, 0xd3, 0x4f, 0x9c};

static double sgg__resolve_size(double current_size, double requested_size, double min_size, bool never_downsize) {
  if (min_size > requested_size) {
    requested_size = min_size;
  }

  if (requested_size < current_size && never_downsize) {
    return current_size;
  }

  return requested_size;
}

static bool sgg__init2(const sgg_environment_desc* desc) {
  D3D_FEATURE_LEVEL    feature_levels[] = {D3D_FEATURE_LEVEL_11_1};
  ID3D11Device*        base_device;
  ID3D11DeviceContext* base_device_context;

  UINT device_flags = 0;
  device_flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
  device_flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG // TODO : Runtime configurable.
  device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
  assert(SUCCEEDED(hr));

  ID3D11Device1* device;
  hr = ID3D11Device_QueryInterface(base_device, &IID_ID3D11Device1, (void**)(&device));
  assert(SUCCEEDED(hr));

  ID3D11DeviceContext1* device_context;
  hr = ID3D11DeviceContext_QueryInterface(base_device_context, &IID_ID3D11DeviceContext1, (void**)&device_context);
  assert(SUCCEEDED(hr));

  IDXGIDevice1* dxgi_device;
  hr = ID3D11Device_QueryInterface(device, &IID_IDXGIDevice1, (void**)&dxgi_device);
  assert(SUCCEEDED(hr));

  IDXGIAdapter1* adapter;
  hr = IDXGIDevice_GetParent(dxgi_device, &IID_IDXGIAdapter1, (void**)&adapter);
  assert(SUCCEEDED(hr));

  IDXGIFactory2* factory;
  hr = IDXGIAdapter1_GetParent(adapter, &IID_IDXGIFactory2, (void**)&factory);
  assert(SUCCEEDED(hr));

  UINT swap_chain_flags = 0;
  // swap_chain_flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
    .Format           = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc.Count = 1,
    .BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount      = 2,
    .Scaling          = DXGI_SCALING_NONE,
    .Flags            = swap_chain_flags,
    .SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD,
  };

  IDXGISwapChain1* swap_chain;
  hr = IDXGIFactory2_CreateSwapChainForHwnd(
    factory,
    (IUnknown*)device,
    glfwGetWin32Window(desc->window),
    &swap_chain_desc,
    NULL,
    NULL,
    &swap_chain);
  assert(SUCCEEDED(hr));

  IDXGIFactory2_Release(factory);
  IDXGIAdapter1_Release(adapter);
  IDXGIDevice1_Release(dxgi_device);

  g_state.base_device         = base_device;
  g_state.base_device_context = base_device_context;
  g_state.device              = device;
  g_state.device_context      = device_context;
  g_state.swap_chain          = swap_chain;
  g_state.swap_chain_desc     = swap_chain_desc;

  return true;
}

static void sgg__reset_render_target(UINT new_width, UINT new_height) {
  if (g_state.render_target_view) {
    ID3D11DeviceContext_ClearState(g_state.device_context);
    ID3D11RenderTargetView_Release(g_state.render_target_view);
  }

  if (new_width == 0 || new_height == 0) {
    return;
  }

  g_state.swap_chain_desc.Width  = new_width;
  g_state.swap_chain_desc.Height = new_height;

  HRESULT hr = IDXGISwapChain1_ResizeBuffers(g_state.swap_chain, 0, new_width, new_height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
  assert(SUCCEEDED(hr));

  ID3D11Texture2D* backbuffer;
  hr = IDXGISwapChain1_GetBuffer(g_state.swap_chain, 0, &s_IID_Texture2D, (void**)&backbuffer);
  assert(SUCCEEDED(hr));

  hr = ID3D11Device_CreateRenderTargetView(g_state.device, (ID3D11Resource*)backbuffer, NULL, &g_state.render_target_view);
  assert(SUCCEEDED(hr));

  ID3D11Texture2D_Release(backbuffer);
}

sg_environment sgg_environment(const sgg_environment_desc* desc) {
  assert(desc != NULL);
  assert(desc->window != NULL);
  assert(desc->backbuffer_min_width >= 0);
  assert(desc->backbuffer_min_height >= 0);

  if (g_state.desc.window == NULL) {
    if (sgg__init2(desc)) {
      g_state.desc = *desc;
    } else {
      assert(false && "Failed to initialize sgg_environment.");
      return (sg_environment){0};
    }
  }

  assert(g_state.desc.window == desc->window);

  return (sg_environment){
    .defaults = {
      .color_format = SG_PIXELFORMAT_BGRA8,
      .depth_format = SG_PIXELFORMAT_NONE,
      .sample_count = 1,
    },
    .d3d11 = {
      .device         = g_state.base_device,         // TODO : Use base or the
      .device_context = g_state.base_device_context, //        newer version?
    }};
}

sg_swapchain sgg_swapchain(void) {
  assert(g_state.desc.window != NULL);

  int width, height;
  glfwGetFramebufferSize(g_state.desc.window, &width, &height);

  // clang-format off
  UINT drawable_width  = (UINT)sgg__resolve_size(g_state.swap_chain_desc.Width , width , g_state.desc.backbuffer_min_width , g_state.desc.backbuffer_never_downsize);
  UINT drawable_height = (UINT)sgg__resolve_size(g_state.swap_chain_desc.Height, height, g_state.desc.backbuffer_min_height, g_state.desc.backbuffer_never_downsize);
  // clang-format on

  if (drawable_width != g_state.swap_chain_desc.Width || drawable_height != g_state.swap_chain_desc.Height) {
#ifndef NDEBUG
    printf("Drawable resized: %4u x %4u px\n", drawable_width, drawable_height);
#endif
    sgg__reset_render_target(drawable_width, drawable_width);
  }

  return (sg_swapchain){
    .width             = width,
    .height            = height,
    .sample_count      = 1,
    .color_format      = SG_PIXELFORMAT_BGRA8,
    .depth_format      = SG_PIXELFORMAT_NONE,
    .d3d11.render_view = g_state.render_target_view,
  };
}

void sgg_present(void) {
  assert(g_state.desc.window != NULL);

  HRESULT hr            = S_OK;
  bool    vsync         = true;
  bool    allow_tearing = false;
  if (vsync) {
    hr = IDXGISwapChain1_Present(g_state.swap_chain, 1, 0);
  } else {
    hr = IDXGISwapChain1_Present(g_state.swap_chain, 0, allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
  }
  assert(SUCCEEDED(hr));
}

void sgg_shutdown(void) {
  if (!g_state.desc.window) {
    return;
  }

  ID3D11Device_Release(g_state.base_device);
  ID3D11DeviceContext_Release(g_state.base_device_context);
  ID3D11Device1_Release(g_state.device);
  ID3D11DeviceContext1_Release(g_state.device_context);
  IDXGISwapChain1_Release(g_state.swap_chain);
  ID3D11RenderTargetView_Release(g_state.render_target_view);

  memset(&g_state, 0, sizeof(g_state));

#ifndef NDEBUG
  IDXGIDebug* debug;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, &IID_IDXGIDebug, (void**)&debug))) {
    debug->lpVtbl->ReportLiveObjects(debug, DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    debug->lpVtbl->Release(debug);
  }
#endif
}
