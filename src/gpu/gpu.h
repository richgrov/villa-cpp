#ifndef VILLA_GPU_VK_GPU_H_
#define VILLA_GPU_VK_GPU_H_

#include <vector>

#include <vulkan/vulkan_core.h>

namespace villa {

struct QueueFamilies;

class Gpu {
public:
   explicit Gpu();
   ~Gpu();

   void init(const std::vector<const char *> &extensions);

   inline VkInstance instance() const {
      return vk_instance_;
   }

   void connect_to_surface(VkSurfaceKHR surface);

private:
   bool init_physical_device(QueueFamilies *families);

   VkInstance vk_instance_;
   VkPhysicalDevice physical_device_;
   VkDevice device_;
   VkSurfaceKHR surface_;
};

}; // namespace villa

#endif // !VILLA_GPU_VK_GPU_H_
