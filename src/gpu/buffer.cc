#include "buffer.h"
#include "vulkan/vulkan_core.h"

#include <format>
#include <stdexcept>
#include <vulkan/vulkan.h>

using namespace villa;

namespace {

uint32_t find_memory_type_index(
    VkPhysicalDevice physical_device, uint32_t supported_bits, VkMemoryPropertyFlagBits extra
) {
   VkPhysicalDeviceMemoryProperties properties;
   vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

   VkMemoryType mem_type;
   for (int i = 0; i < properties.memoryTypeCount; ++i) {
      bool supports_mem_type = (supported_bits & (1 << i)) != 0;
      bool supports_extra = (properties.memoryTypes[i].propertyFlags & extra) == extra;
      if (supports_mem_type && supports_extra) {
         return i;
      }
   }

   throw std::runtime_error(std::format(
       "no suitable memory type for bits {} and extra flags {}", supported_bits, (int)extra
   ));
}

} // namespace

Buffer::Buffer(
    size_t size, VkBufferUsageFlags usage, VkDevice device, VkPhysicalDevice physical_device
)
    : buffer_(VK_NULL_HANDLE), device_(device) {
   VkBufferCreateInfo create_info = {
       .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
       .size = size,
       .usage = usage,
       .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
   };

   if (vkCreateBuffer(device, &create_info, nullptr, &buffer_) != VK_SUCCESS) {
      throw std::runtime_error("failed to create buffer");
   }

   VkMemoryRequirements requirements;
   vkGetBufferMemoryRequirements(device, buffer_, &requirements);

   VkMemoryAllocateInfo alloc_info = {
       .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
       .allocationSize = requirements.size,
       .memoryTypeIndex = find_memory_type_index(
           physical_device, requirements.memoryTypeBits,
           static_cast<VkMemoryPropertyFlagBits>(
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
           )
       ),
   };

   if (vkAllocateMemory(device, &alloc_info, nullptr, &allocation_) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate memory");
   }

   if (vkBindBufferMemory(device, buffer_, allocation_, 0) != VK_SUCCESS) {
      throw std::runtime_error("failed to bind memory buffer");
   }
}

void Buffer::upload_memory(void *data, size_t size) {
   void *cpy_dst;
   vkMapMemory(device_, allocation_, 0, size, 0, &cpy_dst);
   memcpy(cpy_dst, data, size);
   vkUnmapMemory(device_, allocation_);
}