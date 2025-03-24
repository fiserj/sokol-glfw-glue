#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define SOKOL_IMPL
#define SOKOL_METAL
#include <sokol_gfx.h>
#include <sokol_log.h>

#include <sokol_glfw_glue.h>

#include "shaders.h"

static void render_frame(GLFWwindow* window, int width, int height) {
  sg_pass pass = {
    .action.colors[0] = {
      .load_action = SG_LOADACTION_CLEAR,
      .clear_value = {0.2f, 0.2f, 0.2f, 1.0f},
    },
    .swapchain = sgg_swapchain_with_full_monitor_size(),
  };
  sg_begin_pass(&pass);

  sg_pipeline* pipeline = (sg_pipeline*)glfwGetWindowUserPointer(window);
  sg_apply_pipeline(*pipeline);

  vs_params_t vs_params = {.inv_aspect = (float)height / (float)width};
  sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));

  sg_draw(0, 3, 1);

  sg_end_pass();
  sg_commit();
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  glfwInit();

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

  GLFWwindow* window = glfwCreateWindow(320, 240, "Sokol-GLFW Glue Test", 0, 0);
  sgg_init_for_window(window);

  glfwSetFramebufferSizeCallback(window, render_frame);

  sg_setup(&(sg_desc){
    .environment = sgg_environment(),
    .logger.func = slog_func,
  });

  sg_pipeline pipeline = sg_make_pipeline(&(sg_pipeline_desc){
    .shader = sg_make_shader(triangle_shader_desc(sg_query_backend())),
  });
  glfwSetWindowUserPointer(window, &pipeline);

  while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    render_frame(window, width, height);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
