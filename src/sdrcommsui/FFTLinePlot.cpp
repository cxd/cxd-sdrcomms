//
// Created by Chris Davey on 7/11/2026.
//

#include "include/FFTLinePlot.h"

#include <algorithm>
#include <ranges>

namespace sdrcommsui {

    FFTLinePlot::FFTLinePlot(int width, int height, const size_t fftSize, const std::shared_ptr<sdrcomms::ReceiveQueue>& receiveQueue) {
        this->width = width;
        this->height = height;
        this->fftSize = fftSize;
        this->receiveQueue = receiveQueue;
        this->fft = std::make_shared<sdrcomms::FFTTransform>();
    }

    /**
     * initialise the receiver thread and fft transform block.
     * @return
     */
    bool FFTLinePlot::init() {
        if (this->fft->init(fftSize)) {
            this->isInit.store(true, std::memory_order_relaxed);
        } else {
            return false;
        }
        this->isRunning.store(true, std::memory_order_relaxed);
        this->receive_thread = std::thread(
            [this]() {
                this->receive_blocks();
            }
        );
        return true;
    }

    /**
         * stop processing
         */
    void FFTLinePlot::stop() {
        isRunning.store(false, std::memory_order_relaxed);
        isInit.store(false, std::memory_order_relaxed);
        this->fft->cleanup();
        if (this->receive_thread.joinable()) {
            this->receive_thread.join();
        }
    }


    /**
     * Draw the spectrum waterfall.
     * @param renderer
     * @param width
     * @param height
     * @param timestep
     * @param frame
     * @param fps
     */
    void FFTLinePlot::DrawSketch(SDL_Renderer *renderer, int width, int height, int timestep, int frame, int fps) {
        std::shared_lock<std::shared_mutex> lock(this->updateMutex);

        if (this->spectrum.size() == 0) {
            return;
        }
        SDL_RenderClear(renderer);
        // draw fft points.

        // we need to downsample by skipping the stride length
        std::vector<SDL_Point> points (width);
        float stride = static_cast<float>(this->spectrum.size()) / static_cast<float>(width);
        for (int x=0; x<width; x++) {

            size_t idx = static_cast<size_t>(x*stride);
            if (idx > this->spectrum.size()) {
                break;
            }
            // a value between 0.0 and 1.0
            float fft_val = spectrum[idx];

            if (!std::isfinite(fft_val)) {
                fft_val = 0.0f;
            }

            // Clamp the raw FFT signal value strictly between 0.0f and 1.0f
            if (fft_val < 0.0f) fft_val = 0.0f;
            if (fft_val > 1.0f) fft_val = 1.0f;

            int y =  static_cast<int>(fft_val * (height-10));
            points[x] = {x, y};
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLines(renderer, points.data(), points.size());
        lock.unlock();
    }

    /**
         * get min and max power from spectrum
         * @return
         */
    std::tuple<float,float> FFTLinePlot::get_min_max(std::vector<float> averages) {
        auto miniter = std::min_element(averages.begin(), averages.end());
        auto maxiter = std::max_element(averages.begin(), averages.end());
        return std::make_tuple(*miniter, *maxiter);
    }

    /**
     * normalise the db
     * @param db_val
     * @param min_db
     * @param max_db
     * @return
     */
    float FFTLinePlot::normalizePower(float db_val, float min_db, float max_db) {
        // Clamp the incoming value to your expected minimum and maximum thresholds
        if (db_val < min_db) db_val = min_db;
        if (db_val > max_db) db_val = max_db;

        // Scale lineally between 0.0 and 1.0
        return (db_val - min_db) / (max_db - min_db);
    }

    /**
         * average the spectrum for block sizes within our drawable dimensions
         * @param spectrum
         * @return
         */
    std::vector<float> FFTLinePlot::slidingAverage(const std::vector<std::complex<float>>& spectrum) {
        size_t num_blocks = this->width / this->block_width;
        std::vector<float> block_averages(num_blocks);
        for (size_t i = 0; i < num_blocks; i++) {
            size_t start_index = i * this->block_width;
            size_t end_index = std::min(start_index + this->block_width, spectrum.size());
            for (size_t j = start_index; j < end_index; j++) {
                block_averages[i] += std::abs(spectrum[j]);
            }
            block_averages[i] /= this->block_width;
            float avg_power = block_averages[i];
            // convert to dB
            block_averages[i] = 10.0f * std::log10(avg_power + 1e-20f);
        }
        return block_averages;
    }

    /**
     * receiver thread updates blocks
     */
    void FFTLinePlot::receive_blocks() {
        while (isRunning.load(std::memory_order_relaxed)) {
            sdrcomms::IQBlock block {};
            if (receiveQueue->pop(block)) {
                // this is the latest sample update the sample details.
                if (block.samples.size() == 0) {
                    continue;
                }
                std::vector<std::complex<float> > fftOut = this->fft->fft(block.samples);
                // write lock block readers
                std::unique_lock<std::shared_mutex> lock(this->updateMutex);

                std::vector<float> localspectrum = slidingAverage(this->fft->fftShift(fftOut));
                this->centerFreq = block.centerFreq;

                auto [min_pow, max_pow] = this->get_min_max(localspectrum);
                std::ranges::generate(localspectrum, [this, localspectrum, min_pow, max_pow, i=0]() mutable {
                    float item = localspectrum[i];
                    float power = normalizePower(item, min_pow, max_pow);
                    i++;
                    return power;
                });
                // copy to the current spectrum
                this->spectrum = localspectrum;

                lock.unlock();
            }

        }
    }

} // sdrcommsui