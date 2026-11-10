//
// Created by Chris Davey on 1/10/2026.
//

#ifndef SDRCOMMS_FFTTRANSFORM_H
#define SDRCOMMS_FFTTRANSFORM_H

#include <vulkan/vulkan.h>
#include "vkFFT.h"
#include <vector>
#include <complex>

namespace sdrcomms {
    class FFTTransform {
    public:
        /**
         * default ctor
         */
        FFTTransform() = default;

        /**
         * initialise the transform operation for reuse by the application.
         * This initialises the underlying structures from the Vulkan api and the vkFFT api.
         * @return
         */
        bool init(size_t fftSize);

        /**
         * cleanup the transform operation
         */
        void cleanup();

        /**
         * perform foward FFT using the supplied input
         * @param input
         * @return
         */
        std::vector<std::complex<float>> fft(const std::vector<std::complex<float> >& input);

        /**
         * fft shift the spectrum
         * @param spectrum
         * @return
         */
        std::vector<std::complex<float>> fftShift(const std::vector<std::complex<float>>& source);

    private:
        size_t fftSize;
        // calculated as fftSize * sizeof(std::complex<float>)
        size_t bufferSize;
        // optional GPU mask which can be supplied to choose the PCI bus id.
        int gpuPciBus = -1;
        bool isInit;
        VkApplicationInfo appInfo ;
        VkInstanceCreateInfo instance_info ;
        VkInstance instance;

        VkPhysicalDevice physicalDevice;
        uint32_t queueFamilyIndex;
        VkQueue queue;
        VkDevice device;
        VkCommandPool devicePool;
        VkBuffer deviceBuffer;
        VkDeviceMemory deviceMemory;
        VkCommandBuffer commandBuffer;
        VkFence fence;

        // storage buffer for fft input output
        VkBuffer *inputOutputBuffers;
        VkDeviceSize buffer_size;

        VkFFTApplication fftApp {};
        VkFFTConfiguration fftConfig {};

        VkCommandBufferBeginInfo commandBufferBeginInfo;
        VkSubmitInfo submitInfo;

        VkPhysicalDevice selectDevice(const std::vector<VkPhysicalDevice>& devices);

        bool makeQueue();

        bool createCommandPool();

        bool createDeviceBuffer();

        bool createCommandBuffer();

        bool createFence();

        bool createFFTApplication();



    };
} // sdrcomms

#endif //SDRCOMMS_FFTTRANSFORM_H
