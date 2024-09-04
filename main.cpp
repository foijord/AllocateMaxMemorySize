
#include <VulkanObjects.h>
#include <iostream>

int main(int argc, char* argv[])
{
	try {
		volkInitialize();
		auto instance = std::make_shared<VulkanInstance>();
		volkLoadInstance(instance->instance);

		for (auto physicalDevice : instance->enumeratePhysicalDevices()) {
			auto vulkanPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(physicalDevice);
			std::cout << vulkanPhysicalDevice->deviceName() << std::endl;
			std::cout << "Driver Version: " << vulkanPhysicalDevice->driverVersion() << std::endl;

			// need at least one queue
			auto computeQueueFamilyIndex = vulkanPhysicalDevice->getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			auto computeQueueIndex = vulkanPhysicalDevice->addQueue(computeQueueFamilyIndex, 1);

			auto device = std::make_shared<VulkanDevice>(instance, vulkanPhysicalDevice);

			// create image just to get some memoryTypeBits
			auto image = std::make_shared<VulkanImage>(device, VkExtent3D{ 512, 512, 512 });
			auto memory_requirements = image->getMemoryRequirements();
			auto memory_type_index = vulkanPhysicalDevice->getMemoryTypeIndex(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			auto max_allocation_size_bytes = vulkanPhysicalDevice->getMaxMemoryAllocationSize(memory_type_index);
			std::cout << "max allocation size: " << max_allocation_size_bytes << " bytes." << std::endl;

			VkDeviceSize allocation_size_bytes = std::numeric_limits<VkDeviceSize>::max();

			// use some other size if given as argument
			if (argc == 2) {
				VkDeviceSize allocation_size_gib = std::stoi(argv[1]);
				allocation_size_bytes = allocation_size_gib << 30;
			}

			if (allocation_size_bytes > max_allocation_size_bytes) {
				std::cout << "allocation size larger than maximum allowed, clamping..." << std::endl;
				allocation_size_bytes = max_allocation_size_bytes;
			}

			try {
				std::cout << "allocating " << allocation_size_bytes << " bytes." << std::endl;
				auto memory = std::make_shared<VulkanMemory>(device, allocation_size_bytes, memory_type_index);
				std::cout << "successfully allocated " << allocation_size_bytes << " bytes." << std::endl;
			}
			catch (Exception& e) {
				std::cerr << e.what() << std::endl;
			}
			std::cout << std::endl;
		}
	}
	catch (Exception& e) {
		std::cerr << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
