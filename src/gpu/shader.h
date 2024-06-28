#ifndef VILLA_GPU_SHADER_H_
#define VILLA_GPU_SHADER_H_

#include "vulkan/vulkan_core.h"

namespace villa {

class Shader {
public:
   Shader() : device_(VK_NULL_HANDLE), module_(VK_NULL_HANDLE) {}

   void init(VkDevice device, const char *file_path);
   void deinit();

private:
   VkDevice device_;
   VkShaderModule module_;
};

} // namespace villa

#endif // !VILLA_GPU_SHADER_H_
