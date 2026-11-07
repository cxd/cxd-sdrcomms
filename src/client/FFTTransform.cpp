//
// Created by Chris Davey on 1/10/2026.
//

#include "FFTTransform.h"

#include <iostream>
#include <cstring>
#include "vulkan/vulkan_core.h"

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
            VkPhysicalDevicePCIBusInfoPropertiesEXT pciBusInfo{};
            pciBusInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
            VkPhysicalDeviceProperties2 deviceProperties;
            deviceProperties.pNext = &pciBusInfo;
            deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            vkGetPhysicalDeviceProperties2(device, &deviceProperties);

            // avoid selecting software renderers if possible.
            std::string deviceName = deviceProperties.properties.deviceName;
            if (deviceName.find("llvmpipe") != std::string::npos ||
                deviceName.find("softpipe") != std::string::npos) {
                continue;
            }

            std::cout << "Device Name: " << deviceProperties.properties.deviceName << std::endl;

            if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                // use the vkGetPhysicalProperties2 to obtain PCI information about device memory.
                // https://community.khronos.org/t/vkgetphysicaldeviceproperties2-not-returning-any-data/111831

                if (gpuPciBus >= 0 && pciBusInfo.pciBus == gpuPciBus) {
                    selectedDevice = device;
                    break;
                }

                VkPhysicalDeviceMemoryProperties memProps;
                vkGetPhysicalDeviceMemoryProperties(device, &memProps);

                VkDeviceSize totalLocalMemory = 0;
                for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
                    if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                        totalLocalMemory += memProps.memoryHeaps[i].size;
                    }
                }
                if (totalLocalMemory > maxVRAM) {
                    maxVRAM = totalLocalMemory;
                    selectedDevice = device;
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
        std::vector<VkQueueFamilyProperties> queues{queueCount};
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


    bool FFTTransform::createFence() {
        VkFenceCreateInfo fence_info {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = 0;
        if (vkCreateFence(device, &fence_info, nullptr, &fence) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    bool FFTTransform::createFFTApplication() {
        fftConfig.physicalDevice = &physicalDevice;
        fftConfig.device = &device;
        fftConfig.queue = &queue;
        fftConfig.commandPool = &devicePool;
        fftConfig.FFTdim = 1;
        fftConfig.size[0] = fftSize;
        fftConfig.size[1] = 1;
        fftConfig.size[2] = 1;
        fftConfig.fence = &fence;
        fftConfig.performR2C = 0;


        // assign buffer
        VkBuffer *inputOutputBuffers = {&deviceBuffer};
        fftConfig.buffer = inputOutputBuffers;
        VkDeviceSize buffer_size = fftSize * sizeof(float);
        fftConfig.bufferSize = &buffer_size;
        // init application
        VkFFTResult res = initializeVkFFT(&fftApp, fftConfig);
        if (res != VKFFT_SUCCESS) {
            std::cerr << "Could not initialise VkFFT Result = " << res << std::endl;
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
        appInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            "SDRCOMMS",
            1,
            "No Engine",
            1,
            VK_API_VERSION_1_2
        };
        instance_info = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &appInfo,
            0,
            nullptr,
            0,
            nullptr
        };
        // enable portability extensions.
        instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        std::vector<const char*> instance_extensions = {
            "VK_KHR_portability_enumeration" // Match the native literal macro
        };
        instance_info.enabledExtensionCount = instance_extensions.size();
        instance_info.ppEnabledExtensionNames = instance_extensions.data();
        // create instance.
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
        if (!createCommandBuffer()) {
            std::cerr << "Could not create command buffer" << std::endl;
            return false;
        }
        if (!createFence()) {
            std::cerr << "Failed to create Vulken Fence" << std::endl;
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
        vkDestroyFence(device, fence, nullptr);
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
    std::vector<std::complex<float> > FFTTransform::fft(const std::vector<std::complex<float> > &input) {
        if (!isInit) {
            return {};
        }
        if (input.size() < fftSize) {
            std::cerr << "FFTTransform::fft: input size does not match fftSize" << std::endl;
            throw std::runtime_error("FFTTransform::fft: input size does not match fftSize");
        }
        void *data;
        uint32_t buffer_size = bufferSize;
        vkMapMemory(device, deviceMemory, 0, buffer_size, 0, &data);
        std::memcpy(data, input.data(), bufferSize);
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
        std::vector<std::complex<float> > output(fftSize);
        vkMapMemory(device, deviceMemory, 0, buffer_size, 0, &data);
        std::memcpy(output.data(), data, buffer_size);
        vkUnmapMemory(device, deviceMemory);
        return output;
    }

    /**
         * fft shift the spectrum
         * @param spectrum
         * @return
         */
    std::vector<std::complex<float>> FFTTransform::fftShift(const std::vector<std::complex<float>>& source) {
        size_t half_size = source.size() / 2;
        size_t N = source.size();
        std::vector<std::complex<float> > output(source.size());
        std::memcpy(output.data(), source.data() + half_size, (N - half_size) * sizeof(std::complex<float>));
        std::memcpy(output.data() + half_size, source.data(), half_size * sizeof(std::complex<float>));
        return output;
    }
} // sdrcomms
