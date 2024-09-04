#pragma once

#include <Vulkan.h>
#include <Exceptions.h>

#include <vector>
#include <memory>

class VulkanInstance {
public:
	VulkanInstance()
	{
		VkApplicationInfo applicationInfo{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = "Image Size Test",
			.applicationVersion = 1,
			.pEngineName = "Image Size Test",
			.engineVersion = 1,
			.apiVersion = VK_API_VERSION_1_0,
		};

		std::vector<const char*> instance_layers{ "VK_LAYER_KHRONOS_validation" };
		std::vector<const char*> instance_extensions{ VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };

		VkInstanceCreateInfo instanceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &applicationInfo,
			.enabledLayerCount = static_cast<uint32_t>(instance_layers.size()),
			.ppEnabledLayerNames = instance_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size()),
			.ppEnabledExtensionNames = instance_extensions.data()
		};

		THROW_ON_VULKAN_ERROR(vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance));
	}

	~VulkanInstance()
	{
		vkDestroyInstance(this->instance, nullptr);
	}

	std::vector<VkPhysicalDevice> enumeratePhysicalDevices()
	{
		uint32_t physicalDeviceCount;
		vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, physicalDevices.data());

		return physicalDevices;
	}

	VkInstance instance{ nullptr };
};

class VulkanPhysicalDevice {
public:
	VulkanPhysicalDevice(VkPhysicalDevice physicalDevice) :
		physicalDevice(physicalDevice)
	{
		vkGetPhysicalDeviceProperties(this->physicalDevice, &this->physicalDeviceProperties);

		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &count, nullptr);
		this->physicalDeviceQueueFamilyProperties.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &count, this->physicalDeviceQueueFamilyProperties.data());

		this->queuePriorities.resize(this->physicalDeviceQueueFamilyProperties.size());
	}

	std::string deviceName() const
	{
		return std::string(this->physicalDeviceProperties.deviceName);
	}
	VkDeviceSize getMaxMemoryAllocationSize() const
	{
		VkPhysicalDeviceProperties2 properties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR };
		VkPhysicalDeviceMaintenance3Properties maintenance3_properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES };
		properties2.pNext = &maintenance3_properties;
		vkGetPhysicalDeviceProperties2(this->physicalDevice, &properties2);
		return maintenance3_properties.maxMemoryAllocationSize;
	}

	uint32_t getMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags required_flags) const
	{
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			if (((memoryTypeBits >> i) & 1) == 1 && (memory_properties.memoryTypes[i].propertyFlags & required_flags) == required_flags) {
				return i;
			}
		}
		throw Exception("VulkanPhysicalDevice::getMemoryTypeIndex: could not find suitable memory type");
	}

	uint32_t getQueueFamilyIndex(VkQueueFlags required_flags, const std::vector<VkBool32>& filter) const
	{
		// check for exact match of required flags
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < this->physicalDeviceQueueFamilyProperties.size(); queueFamilyIndex++) {
			if (filter[queueFamilyIndex] && this->physicalDeviceQueueFamilyProperties[queueFamilyIndex].queueFlags == required_flags) {
				return queueFamilyIndex;
			}
		}
		// check for queue with all required flags set
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < this->physicalDeviceQueueFamilyProperties.size(); queueFamilyIndex++) {
			if (filter[queueFamilyIndex] && (this->physicalDeviceQueueFamilyProperties[queueFamilyIndex].queueFlags & required_flags) == required_flags) {
				return queueFamilyIndex;
			}
		}
		throw Exception("VulkanDevice::getQueueFamilyIndex(): could not find queue with required properties");
	}

	uint32_t getQueueFamilyIndex(VkQueueFlags required_flags, VkSurfaceKHR surface = VK_NULL_HANDLE) const
	{
		std::vector<VkBool32> filter(this->physicalDeviceQueueFamilyProperties.size(), VK_TRUE);
		if (surface != VK_NULL_HANDLE) {
			for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < this->physicalDeviceQueueFamilyProperties.size(); queueFamilyIndex++) {
				THROW_ON_VULKAN_ERROR(vkGetPhysicalDeviceSurfaceSupportKHR(this->physicalDevice, queueFamilyIndex, surface, &filter[queueFamilyIndex]));
			}
		}
		return this->getQueueFamilyIndex(required_flags, filter);
	}

	uint32_t addQueue(uint32_t queueFamilyIndex, float priority = 1.0f, VkDeviceQueueCreateFlags flags = 0)
	{
		if (this->physicalDeviceQueueFamilyProperties[queueFamilyIndex].queueCount <= this->queuePriorities[queueFamilyIndex].size()) {
			throw Exception("VulkanDevice::addQueue(): cannot create more queues of queueFamilyIndex " + std::to_string(queueFamilyIndex));
		}

		if (this->queuePriorities[queueFamilyIndex].empty()) {
			this->deviceQueueCreateInfos.emplace_back(
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
				nullptr,									// pNext
				flags,										// flags
				queueFamilyIndex);							// queueFamilyIndex
		}

		this->queuePriorities[queueFamilyIndex].push_back(priority);
		for (auto& queueCreateInfo : this->deviceQueueCreateInfos) {
			if (queueCreateInfo.queueFamilyIndex == queueFamilyIndex) {
				queueCreateInfo.queueCount = static_cast<uint32_t>(this->queuePriorities[queueCreateInfo.queueFamilyIndex].size());
				queueCreateInfo.pQueuePriorities = this->queuePriorities[queueCreateInfo.queueFamilyIndex].data();
			}
		}
		return static_cast<uint32_t>(this->queuePriorities[queueFamilyIndex].size() - 1);
	}

	VkPhysicalDevice physicalDevice{ nullptr };
	VkPhysicalDeviceProperties physicalDeviceProperties;
	std::shared_ptr<VulkanInstance> instance{ nullptr };
	std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties;
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
	std::vector<std::vector<float>> queuePriorities;
};

class VulkanDevice {
public:
	VulkanDevice(
		std::shared_ptr<VulkanInstance> instance,
		std::shared_ptr<VulkanPhysicalDevice> physicalDevice) :
		instance(std::move(instance)),
		physicalDevice(std::move(physicalDevice))
	{
		std::vector<const char*> device_layers{ "VK_LAYER_KHRONOS_validation" };
		std::vector<const char*> device_extensions{};
		const std::vector<VkDeviceQueueCreateInfo>& queue_create_infos = this->physicalDevice->deviceQueueCreateInfos;

		VkDeviceCreateInfo device_create_info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
			.pQueueCreateInfos = queue_create_infos.data(),
			.enabledLayerCount = static_cast<uint32_t>(device_layers.size()),
			.ppEnabledLayerNames = device_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
			.ppEnabledExtensionNames = device_extensions.data(),
			.pEnabledFeatures = nullptr,
		};

		THROW_ON_VULKAN_ERROR(vkCreateDevice(this->physicalDevice->physicalDevice, &device_create_info, nullptr, &this->device));
	}

	~VulkanDevice()
	{
		vkDestroyDevice(this->device, nullptr);
	}

	std::shared_ptr<VulkanInstance> instance{ nullptr };
	std::shared_ptr<VulkanPhysicalDevice> physicalDevice;
	VkDevice device{ nullptr };
};

class VulkanImage {
public:
	VulkanImage(
		std::shared_ptr<VulkanDevice> device,
		VkExtent3D extent) :
		device(std::move(device))
	{
		VkImageCreateInfo imageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_3D,
			.format = VK_FORMAT_R8_UNORM,
			.extent = extent,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_SAMPLED_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		THROW_ON_VULKAN_ERROR(vkCreateImage(this->device->device, &imageCreateInfo, nullptr, &this->image));
	}

	~VulkanImage()
	{
		vkDestroyImage(this->device->device, this->image, nullptr);
	}

	VkMemoryRequirements getMemoryRequirements()
	{
		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(
			this->device->device,
			this->image,
			&memory_requirements);

		return memory_requirements;
	}

	std::shared_ptr<VulkanDevice> device{ nullptr };
	VkImage image{ nullptr };
};

class VulkanMemory {
public:
	VulkanMemory(
		std::shared_ptr<VulkanDevice> device,
		VkDeviceSize allocationSize,
		uint32_t memoryTypeIndex) :
		device(std::move(device))
	{
		VkMemoryAllocateInfo allocate_info{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = nullptr,
			.allocationSize = allocationSize,
			.memoryTypeIndex = memoryTypeIndex,
		};

		THROW_ON_VULKAN_ERROR(vkAllocateMemory(this->device->device, &allocate_info, nullptr, &this->memory));
	}

	~VulkanMemory()
	{
		vkFreeMemory(this->device->device, this->memory, nullptr);
	}

	std::shared_ptr<VulkanDevice> device{ nullptr };
	VkDeviceMemory memory{ nullptr };
};

class VulkanQueue {
public:
	VulkanQueue(
		std::shared_ptr<VulkanDevice> device,
		uint32_t queueFamilyIndex,
		uint32_t queueIndex) :
		device(std::move(device)),
		queueIndex(queueIndex),
		queueFamilyIndex(queueFamilyIndex)
	{
		vkGetDeviceQueue(this->device->device, this->queueFamilyIndex, this->queueIndex, &this->queue);
	}

	std::shared_ptr<VulkanDevice> device{ nullptr };
	VkQueue queue{ nullptr };
	uint32_t queueIndex{ 0 };
	uint32_t queueFamilyIndex{ 0 };
};
