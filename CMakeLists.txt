cmake_minimum_required (VERSION 3.28)
project(AllocateMaxMemorySize LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(TARGET "AllocateMaxMemorySize")

find_package(Vulkan REQUIRED)

add_executable(${TARGET})

target_sources(${TARGET}
	PUBLIC
		Vulkan.cpp
		main.cpp
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS
		${Vulkan_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		Vulkan.h
		Exceptions.h
		VulkanObjects.h)
