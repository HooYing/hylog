# CMakeList.txt: tools 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("test_hylog")

file(GLOB_RECURSE CPP_FILES hylog/*.cpp test_hylog.cpp)
message(${CPP_FILES})

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Zi /Od")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} /MTd")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
	add_compile_options(-std=c++11)
endif()


# 将源代码添加到此项目的可执行文件。
add_executable (test_hylog ${CPP_FILES})

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(test_hylog pthread)
endif()

# TODO: 如有需要，请添加测试并安装目标。
