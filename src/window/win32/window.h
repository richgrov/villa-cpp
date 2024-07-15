#ifndef VKAD_WINDOW_WIN32_WINDOW_H_
#define VKAD_WINDOW_WIN32_WINDOW_H_

#include <vector>

#include <Windows.h>
#include <vulkan/vulkan_core.h>

namespace vkad {

LRESULT CALLBACK window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param);

class Window {
public:
   explicit Window(const char *title);

   bool poll();

   inline std::vector<const char *> vulkan_extensions() const {
      return {"VK_KHR_surface", "VK_KHR_win32_surface"};
   }

   VkSurfaceKHR create_surface(VkInstance instance);

   void set_capture_mouse(bool capture);

   void request_close();

   int width() const {
      return width_;
   }

   int height() const {
      return height_;
   }

   int mouse_x() const {
      return mouse_x_;
   }

   int mouse_y() const {
      return mouse_y_;
   }

   int delta_mouse_x() const {
      return delta_mouse_x_;
   }

   int delta_mouse_y() const {
      return delta_mouse_y_;
   }

   bool left_clicking() const {
      return left_clicking_;
   }

   inline bool is_key_down(uint8_t key_code) const {
      return pressed_keys_[key_code];
   }

private:
   HWND window_;
   bool open_;
   bool closing_;
   bool cursor_captured_;
   WORD window_x_;
   WORD window_y_;
   WORD width_;
   WORD height_;

   int mouse_x_;
   int mouse_y_;
   int delta_mouse_x_;
   int delta_mouse_y_;
   bool left_clicking_;

   bool pressed_keys_[256];

   friend LRESULT CALLBACK window_proc(HWND window, UINT msg, WPARAM w_param, LPARAM l_param);
};

}; // namespace vkad

#endif // !VKAD_WINDOW_WIN32_WINDOW_H_
