#include "window.h"

#include <Windows.h>
#include <format>
#include <stdexcept>
#include <windowsx.h>

#include "vulkan/vulkan_core.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>

using namespace villa;

namespace {

const char *WIN32_CLASS_NAME = "villa";

static Window *get_window_class(HWND window) {
   LONG_PTR user_ptr = GetWindowLongPtr(window, GWLP_USERDATA);
   if (user_ptr == 0) {
      throw std::runtime_error(std::format("GetWindowLongPtr returned {}", GetLastError()));
   }

   return reinterpret_cast<Window *>(user_ptr);
}

} // namespace

LRESULT CALLBACK villa::window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param) {
   switch (msg) {
   case WM_DESTROY:
      get_window_class(window)->open_ = false;
      PostQuitMessage(0);
      return 0;

   case WM_SIZE: {
      WORD height = HIWORD(l_param);
      WORD width = LOWORD(l_param);
      auto window_class = get_window_class(window);
      window_class->width_ = width;
      window_class->height_ = height;
      return 0;
   }

   case WM_MOUSEMOVE: {
      int x = GET_X_LPARAM(l_param);
      int y = GET_Y_LPARAM(l_param);
      Window *window_class = get_window_class(window);
      window_class->mouse_x_ = x;
      window_class->mouse_y_ = y;
      return 0;
   }

   case WM_LBUTTONDOWN:
      get_window_class(window)->left_clicking_ = true;
      return 0;

   case WM_LBUTTONUP:
      get_window_class(window)->left_clicking_ = false;
      return 0;

   case WM_KEYDOWN:
      get_window_class(window)->pressed_keys_[static_cast<uint8_t>(w_param)] = true;
      return 0;

   case WM_KEYUP:
      get_window_class(window)->pressed_keys_[static_cast<uint8_t>(w_param)] = false;
      return 0;
   }

   return DefWindowProc(window, msg, w_param, l_param);
}

Window::Window(const char *title)
    : open_(false), mouse_x_(0), mouse_y_(0), left_clicking_(false), pressed_keys_{0} {
   HINSTANCE h_instance = GetModuleHandle(nullptr);

   WNDCLASS clazz = {
       .lpfnWndProc = window_proc,
       .hInstance = h_instance,
       .lpszClassName = WIN32_CLASS_NAME,
   };

   if (RegisterClass(&clazz) == 0) {
      throw std::runtime_error(std::format("RegisterClass returned {}", GetLastError()));
   }

   window_ = CreateWindowEx(
       0, WIN32_CLASS_NAME, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, nullptr, nullptr, h_instance, nullptr
   );

   if (window_ == nullptr) {
      throw std::runtime_error(std::format("CreateWindowEx returned {}", GetLastError()));
   }

   SetWindowLongPtr(window_, GWLP_USERDATA, (int64_t)this);

   ShowWindow(window_, SW_SHOW);
   open_ = true;
}

bool Window::poll() {
   MSG msg;
   if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == 0) {
      return open_;
   }

   TranslateMessage(&msg);
   DispatchMessage(&msg);
   return open_;
}

VkSurfaceKHR Window::create_surface(VkInstance instance) {
   VkWin32SurfaceCreateInfoKHR surface_create = {
       .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
       .hinstance = GetModuleHandle(nullptr),
       .hwnd = window_,
   };

   VkSurfaceKHR surface;
   if (vkCreateWin32SurfaceKHR(instance, &surface_create, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("couldn't create win32 surface");
   }

   return surface;
}
