
#include <VulkanObjects.h>
#include <iostream>

int main(int, char* [])
{
	try {
		volkInitialize();
		auto instance = std::make_shared<VulkanInstance>();
		volkLoadInstance(instance->instance);

		for (auto physicalDevice : instance->enumeratePhysicalDevices()) {
			auto vulkanPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(physicalDevice);
			std::cout << "device name: " << vulkanPhysicalDevice->deviceName() << std::endl;

			// need at least one queue
			auto computeQueueFamilyIndex = vulkanPhysicalDevice->getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			auto computeQueueIndex = vulkanPhysicalDevice->addQueue(computeQueueFamilyIndex, 1);

			auto device = std::make_shared<VulkanDevice>(instance, vulkanPhysicalDevice);

			// create image just to get some memoryTypeBits
			auto image = std::make_shared<VulkanImage>(device, VkExtent3D{ 512, 512, 512 });
			auto memory_requirements = image->getMemoryRequirements();
			auto memory_type_index = vulkanPhysicalDevice->getMemoryTypeIndex(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			auto max_allocation_size = vulkanPhysicalDevice->getMaxMemoryAllocationSize();
			std::cout << "max allocation size: " << max_allocation_size << " bytes." << std::endl;

			auto memory = std::make_shared<VulkanMemory>(device, max_allocation_size, memory_type_index);
			std::cout << "successfully allocated " << max_allocation_size << " bytes." << std::endl;
		}
	}
	catch (Exception& e) {
		std::cerr << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
