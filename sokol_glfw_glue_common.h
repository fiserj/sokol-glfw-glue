void sgg_max_monitor_size(int* width, int* height) {
  int           count    = 0;
  GLFWmonitor** monitors = glfwGetMonitors(&count);

  int max_width  = 0;
  int max_height = 0;

  for (int i = 0; i < count; i++) {
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

    float x_scale, y_scale;
    glfwGetMonitorContentScale(monitors[i], &x_scale, &y_scale);

    int width  = (int)(x_scale * mode->width);
    int height = (int)(y_scale * mode->height);

    if (width > max_width) {
      max_width = width;
    }
    if (height > max_height) {
      max_height = height;
    }
  }

  if (width != NULL) {
    *width = max_width;
  }
  if (height != NULL) {
    *height = max_height;
  }
}
