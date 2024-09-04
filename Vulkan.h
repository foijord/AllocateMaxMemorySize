#pragma once

#define VK_NO_PROTOTYPES

#ifdef _WIN32
#include <Volk/volk.h>
#else
#include <volk.h>
#endif

#include <vulkan/vk_enum_string_helper.h>
