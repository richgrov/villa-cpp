#include "gpu.h"

#include <format>
#include <optional>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan_core.h>

using namespace villa;

namespace {

#ifdef VILLA_DEBUG

const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};

void ensure_validation_layers_supported() {
   uint32_t total_layers;
   vkEnumerateInstanceLayerProperties(&total_layers, nullptr);

   std::vector<VkLayerProperties> all_layers(total_layers);
   vkEnumerateInstanceLayerProperties(&total_layers, all_layers.data());

   for (const char *const layer_name : validation_layers) {
      bool match = false;

      for (const auto &layer : all_layers) {
         if (strcmp(layer_name, layer.layerName) == 0) {
            match = true;
            break;
         }
      }

      if (!match) {
         throw std::runtime_error(std::format("validation layer {} not supported", layer_name));
      }
   }
}
#endif

std::optional<uint32_t> get_graphics_queue_family(VkPhysicalDevice device) {
   uint32_t num_queue_families;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, nullptr);

   std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, queue_families.data());

   for (int i = 0; i < queue_families.size(); ++i) {
      if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1) {
         return std::make_optional(i);
      }
   }

   return std::nullopt;
}

struct PhysicalDeviceInfo {
   VkPhysicalDevice device;
   uint32_t graphics_queue_family;
};

std::optional<PhysicalDeviceInfo> find_best_physical_device(VkInstance instance) {
   uint32_t num_devices;
   vkEnumeratePhysicalDevices(instance, &num_devices, nullptr);
   if (num_devices == 0) {
      throw std::runtime_error("no vulkan devices");
   }

   std::vector<VkPhysicalDevice> devices(num_devices);
   vkEnumeratePhysicalDevices(instance, &num_devices, devices.data());

   for (const auto &device : devices) {
      std::optional<uint32_t> graphics_queue_family = get_graphics_queue_family(device);
      if (graphics_queue_family.has_value()) {
         return PhysicalDeviceInfo{
             .device = device,
             .graphics_queue_family = graphics_queue_family.value(),
         };
      }
   }

   return std::nullopt;
}

VkDevice create_logical_device(const PhysicalDeviceInfo &device_info) {
   float queue_priority = 1.0f;

   VkDeviceQueueCreateInfo queue_create_info = {
       .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
       .queueFamilyIndex = device_info.graphics_queue_family,
       .queueCount = 1,
       .pQueuePriorities = &queue_priority,
   };

   VkPhysicalDeviceFeatures physical_device_features = {};

   VkDeviceCreateInfo create_info = {
       .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
       .queueCreateInfoCount = 1,
       .pQueueCreateInfos = &queue_create_info,
#ifdef VILLA_DEBUG
       .enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
       .ppEnabledLayerNames = validation_layers.data(),
#endif
       .pEnabledFeatures = &physical_device_features,
   };

   VkDevice device;
   if (vkCreateDevice(device_info.device, &create_info, nullptr, &device) != VK_SUCCESS) {
      throw std::runtime_error("failed to create logical device");
   }

   return device;
}

} // namespace

Gpu::Gpu()
    : vk_instance_(VK_NULL_HANDLE), physical_device_(VK_NULL_HANDLE), device_(VK_NULL_HANDLE),
      surface_(VK_NULL_HANDLE) {}

void Gpu::init(const std::vector<const char *> &extensions) {
   VkApplicationInfo app_info = {
       .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
       .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
       .engineVersion = VK_MAKE_VERSION(1, 0, 0),
       .apiVersion = VK_API_VERSION_1_0,
   };

   VkInstanceCreateInfo create_info = {
       .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
       .pApplicationInfo = &app_info,
       .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
       .ppEnabledExtensionNames = extensions.data(),
   };

#ifdef VILLA_DEBUG
   ensure_validation_layers_supported();
   create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
   create_info.ppEnabledLayerNames = validation_layers.data();
#endif

   if (vkCreateInstance(&create_info, nullptr, &vk_instance_) != VK_SUCCESS) {
      throw std::runtime_error("failed to create vulkan instance");
   }

   auto physical_device = find_best_physical_device(vk_instance_);
   if (!physical_device.has_value()) {
      throw std::runtime_error("no suitable physical device");
   }

   device_ = create_logical_device(physical_device.value());

   VkQueue graphics_queue;
   vkGetDeviceQueue(device_, physical_device.value().graphics_queue_family, 0, &graphics_queue);
}

Gpu::~Gpu() {
   if (device_ != VK_NULL_HANDLE) {
      vkDestroyDevice(device_, nullptr);
   }

   if (surface_ != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);
   }

   if (vk_instance_ != VK_NULL_HANDLE) {
      vkDestroyInstance(vk_instance_, nullptr);
   }
}
