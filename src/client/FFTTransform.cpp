//
// Created by Chris Davey on 1/10/2026.
//

#include "FFTTransform.h"

#include <iostream>

#include "../../../../../../../opt/homebrew/include/vulkan/vulkan_core.h"
#include "../../cmake-build-debug/_deps/vkfft-src/benchmark_scripts/vkFFT_scripts/include/utils_VkFFT.h"

namespace sdrcomms {
    /**
     * select the physical GPU with the highest amount of ram otherwise just return the first CPU device.
     * @param devices
     * @return
     */
    VkPhysicalDevice FFTTransform::selectDevice(const std::vector<VkPhysicalDevice> &devices) {
        VkPhysicalDevice selectedDevice = devices[0];
        VkDeviceSize maxVRAM = 0;
        for (size_t deviceIdx = 0; deviceIdx < devices.size(); deviceIdx++) {
            auto device = devices[deviceIdx];
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            std::cout << "Device Name: " << deviceProperties.deviceName << std::endl;
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                VkPhysicalDeviceMemoryProperties properties;
                vkGetPhysicalDeviceMemoryProperties(device, &properties);
                for (size_t i = 0; i < properties.memoryTypeCount; i++) {
                    if (properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                        VkDeviceSize totalMem = 0;
                        for (size_t j = 0; j < properties.memoryHeaps[i].size; j++) {
                            totalMem += properties.memoryHeaps[i].size;
                        }
                        if (totalMem > maxVRAM) {
                            maxVRAM = totalMem;
                            selectedDevice = device;
                        }
                    }
                }
            }
        }
        return selectedDevice;
    }

    bool FFTTransform::makeQueue() {
        /**
        * select the queue family index.
        **/
        uint32_t queueCount = 0;
        queueFamilyIndex = -1;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues{};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queues.data());
        for (uint32_t i = 0; i < queueCount; i++) {
            if (queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queueFamilyIndex = i;
                break;
            }
        }
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queueFamilyIndex, 1, &queuePriority
        };
        VkDeviceCreateInfo deviceInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            nullptr,
            0,
            1,
            &queueInfo,
            0,
            nullptr,
            0,
            nullptr,
            nullptr
        };
        if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS) {
            return false;
        }
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
        return true;
    }


    bool FFTTransform::createCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            queueFamilyIndex
        };;
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &devicePool) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    bool FFTTransform::createDeviceBuffer() {
        VkDeviceSize buffer_size = bufferSize;
        VkBufferCreateInfo bufferInfo = {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, buffer_size,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr
        };
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &deviceBuffer) != VK_SUCCESS) {
            return false;
        }
        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, deviceBuffer, &memReqs);
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
        uint32_t memTypeIndex = -1;
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if ((memReqs.memoryTypeBits & (1 << i)) && (
                    memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
                memTypeIndex = i;
                break;
            }
        }
        if (memTypeIndex < 0) {
            return false;
        }
        VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, memReqs.size, memTypeIndex};
        if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
            return false;
        }
        vkBindBufferMemory(device, deviceBuffer, deviceMemory, 0);

        return true;
    }

    bool FFTTransform::createFFTApplication() {
        fftConfig.physicalDevice = &physicalDevice;
        fftConfig.device = &device;
        fftConfig.queue = &queue;
        fftConfig.commandPool = &devicePool;
        fftConfig.FFTdim = 1;
        fftConfig.size[0] = fftSize;
        VkBuffer *inputOutputBuffers = {&deviceBuffer};
        fftConfig.buffer = inputOutputBuffers;
        if (initializeVkFFT(&fftApp, fftConfig) != VKFFT_SUCCESS) {
            return false;
        }
        return true;
    }

    bool FFTTransform::createCommandBuffer() {
        VkCommandBufferAllocateInfo cmdAllocInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            nullptr,
            devicePool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1
        };
        if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &commandBuffer) != VK_SUCCESS) {
            return false;
        }

        return true;
    }


    /**
         * initialise the transform operation for reuse by the application.
         * This initialises the underlying structures from the Vulkan api and the vkFFT api.
         * @return
         */
    bool FFTTransform::init(size_t fftSize) {
        this->fftSize = fftSize;
        this->bufferSize = fftSize * sizeof(std::complex<float>);

        VkResult res = VK_SUCCESS;
        appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "SDRCOMMS", 1, "No Engine", 1, VK_API_VERSION_1_2};
        instance_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &appInfo, 0, nullptr, 0, nullptr};
        res = vkCreateInstance(&instance_info, nullptr, &instance);
        if (res != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan instance. Error code [" << res << "]" << std::endl;
            return false;
        }
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            std::cerr << "Failed to find Devices with Vulkan support" << std::endl;
            return false;
        }
        std::vector<VkPhysicalDevice> devices{deviceCount};
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        if (devices.size() == 0) {
            std::cerr << "Failed to find Devices with Vulkan support" << std::endl;
            return false;
        }
        physicalDevice = selectDevice(devices);
        if (!makeQueue()) {
            std::cerr << "Could not create device and queue" << std::endl;
            return false;
        }
        if (!createCommandPool()) {
            std::cerr << "Could not create VK command pool" << std::endl;
            return false;
        }
        if (!createDeviceBuffer()) {
            std::cerr << "Could not allocate VK device buffer" << std::endl;
            return false;
        }
        if (!createFFTApplication()) {
            std::cerr << "Could not initialise the vkFFT library" << std::endl;
            return false;
        }
        isInit = true;
        return true;
    }


    /**
     * cleanup the transform operation
     */
    void FFTTransform::cleanup() {
        deleteVkFFT(&fftApp);
        vkFreeCommandBuffers(device, devicePool, 1, &commandBuffer);
        vkDestroyBuffer(device, deviceBuffer, nullptr);
        vkFreeMemory(device, deviceMemory, nullptr);
        vkDestroyCommandPool(device, devicePool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    /**
     * perform foward FFT using the supplied input
     * @param input
     * @return
     */
    std::vector<std::complex<float> > FFTTransform::fft(const std::vector<std::complex<float> >& input) {
        if (!isInit) {
            return {};
        }
        void* data;
        uint32_t buffer_size = bufferSize;
        vkMapMemory(device, deviceMemory, 0, buffer_size, 0, &data);
        std::memcpy(data, input.data(), buffer_size);
        vkUnmapMemory(device, deviceMemory);

        commandBufferBeginInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            nullptr
        };

        vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
        // Append the VkFFT transform instructions onto our command stream
        // (-1 tells VkFFT to compute a forward FFT transform)
        VkFFTLaunchParams launchParams = {};
        launchParams.commandBuffer = &commandBuffer;
        if (VkFFTAppend(&fftApp, -1, &launchParams) != VKFFT_SUCCESS) {
            return {};
        }
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            nullptr,
            0,
            nullptr,
            nullptr,
            1,
            &commandBuffer,
            0,
            nullptr
        };

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        // copy data back from device into output
        std::vector<std::complex<float> > output(bufferSize);
        vkMapMemory(device, deviceMemory, 0, buffer_size, 0, &data);
        std::memcpy(output.data(), data, buffer_size);
        vkUnmapMemory(device, deviceMemory);
        return output;
    }
} // sdrcomms
