#ifndef VILLA_GPU_COMMAND_POOL_H_
#define VILLA_GPU_COMMAND_POOL_H_

#include <cstdint>

#include <vulkan/vulkan_core.h>

namespace villa {

class CommandPool {
public:
   CommandPool() : command_pool_(VK_NULL_HANDLE) {}

   void init(VkDevice device, uint32_t graphics_queue_family);

   void deinit();

   VkCommandBuffer allocate();

private:
   VkDevice device_;
   VkCommandPool command_pool_;
};

} // namespace villa

#endif // !VILLA_GPU_COMMAND_POOL_H_