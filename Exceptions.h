#pragma once

#include <Vulkan.h>
#include <stdexcept>

class Exception : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
	explicit Exception(VkResult error) : runtime_error(string_VkResult(error)) {}
};

#define THROW_ON_VULKAN_ERROR(__function__) {	\
	VkResult result = (__function__);			\
	if (result != VK_SUCCESS) {					\
		throw Exception(result);				\
	}											\
}												\
static_assert(true, "")

