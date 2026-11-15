//
// Created by Chris Davey on 5/10/2026.
//

#include "include/SpectrumWaterfall.h"

#include <format>

#include "constants.h"
#include "SDL2_gfxPrimitives.h"

namespace sdrcommsui {
    SpectrumWaterfall::SpectrumWaterfall(int width, int height, const size_t fftSize,
                                         const std::shared_ptr<sdrcomms::ReceiveQueue> &receiveQueue) {
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
    bool SpectrumWaterfall::init() {
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

    void SpectrumWaterfall::stop() {
        isRunning.store(false, std::memory_order_relaxed);
        isInit.store(false, std::memory_order_relaxed);
        if (this->receive_thread.joinable()) {
            this->receive_thread.join();
        }
        this->fft->cleanup();
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
    void SpectrumWaterfall::DrawSketch(SDL_Renderer *renderer, int width, int height,
                                       int timestep, int frame, int fps) {
        this->width = width;
        this->height = height;
        std::vector<std::vector<float>> averages;
        std::vector<float> frequencies;
        {
            std::shared_lock<std::shared_mutex> lock(this->updateMutex);


            averages = this->spectrum;
            frequencies = this->avgFrequenciesHz;
        }
        if (averages.size() >= this->MAX_QUEUE_SIZE) {
            this->draw_blocks(renderer, averages);
        }
        if (frequencies.size() > 0) {
            this->draw_frequencies(renderer, frequencies);
        }

    }

    /**
     * pass spectrum copy
     * @param renderer
     * @param averages
     */
    void SpectrumWaterfall::draw_blocks(SDL_Renderer *renderer, const std::vector<std::vector<float>>& averages) {
        if (averages.size() == 0) {
            return;
        }

        auto [min, max] = this->get_min_max(averages);
        size_t num_rows = std::min(averages.size(), this->MAX_QUEUE_SIZE);

        size_t block_count = averages[0].size() * averages.size();

        std::vector<SDL_Vertex> vertices(block_count*4);
        std::vector<int> indices(block_count*6);


        for (size_t i = 0; i < num_rows; i++) {
            int x_pos = 0;
            if (i >= num_rows) {
                break;
            }
            size_t num_blocks = averages[i].size();
            // allow 50 pixels at the bottom for labels.
            float waterfallHeight = static_cast<float>(height - TICK_MARGIN);
            const float top = static_cast<float>(i)*waterfallHeight/num_rows;
            const float bottom = static_cast<float>(i+1)*waterfallHeight/num_rows;

            for (size_t j = 0; j < num_blocks; j++) {

                float avg = this->normalizePower(averages[i][j],
                    min,
                    max);
                SDL_Color col = this->getSpectrumColor(avg);

                const float left = static_cast<float>(j)*width/num_blocks;
                const float right = static_cast<float>(j+1)*width/num_blocks;

                int base_index = static_cast<int>(vertices.size());
                vertices.push_back({ {left, top}, col, {0, 0} });
                vertices.push_back({ {right, top}, col, {0, 0} });
                vertices.push_back({ {right, bottom}, col, {0, 0}});
                vertices.push_back( { {left, bottom}, col, {0, 0} });

                indices.insert(indices.end(), {
                    base_index, base_index + 1, base_index + 2,
                    base_index, base_index + 2, base_index + 3
                });


            }

        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_RenderGeometry(renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()),
            indices.data(), static_cast<int>(indices.size()));

    }

    /**
 * draw the tickmarks for the average frequencies
 * @param renderer
 */
    void SpectrumWaterfall::draw_frequencies(SDL_Renderer *renderer, const std::vector<float>& frequencies) {

        if (frequencies.size() == 0) {
            return;
        }
        float midFreq = frequencies[frequencies.size()/2];
        float minFreq = frequencies[0];
        float maxFreq = frequencies[frequencies.size()-1];

        // 1 GHz = 1e9 Hz
        // 1 MHz = 1e6 Hz
        // 1 kHz = 1e3 Hz
        float divider = 1e9;
        std::string unit = "GHz";
        if (maxFreq > 1e9/2.0) {
            divider = 1e9;
        } else {
            divider = 1e6;
            unit = "MHz";
        }

        int top = (height - TICK_MARGIN);
        int tick_step = TICK_STEP;
        // draw every fourth step.
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        size_t num_blocks = this->width / this->block_width;

        for (size_t i=0;i<frequencies.size();i+=tick_step) {

            const float left = static_cast<float>(i)*width/num_blocks;
            const float right = static_cast<float>(i+1)*width/num_blocks;

            float x = (left + right)/2;
            SDL_RenderDrawLine(renderer, x, top, x, top + 5);

            if (i % LABEL_TICK_STEP == 0) {
                // draw text.
                std::string value = std::format("{:.5f}", frequencies[i]/divider);
                stringRGBA(renderer, x, top + LABEL_TICK_OFFSET, value.c_str(), 255, 255, 255, 255);
            }

        }
        int n = frequencies.size()/2 - 30;
        const float left = static_cast<float>(n)*width/num_blocks;
        const float right = static_cast<float>(n+1)*width/num_blocks;

        float x = (left + right)/2;
        std::string title = "Frequencies (" + unit + ")";
        stringRGBA(renderer, x, top + LABEL_TICK_TITLE_OFFSET, title.c_str(), 255, 255, 255, 255);
    }


    /**
        * get min and max power from spectrum
        * @return
        */
    std::tuple<float,float> SpectrumWaterfall::get_min_max(std::vector<std::vector<float>> averages) {
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();
        size_t num_blocks = std::min(averages.size(), this->MAX_QUEUE_SIZE);
        for (size_t i=0;i<num_blocks;i++) {
            if (averages[i].empty()) {
                continue;
            }
            auto miniter = std::min_element(averages[i].begin(), averages[i].end());
            auto maxiter = std::max_element(averages[i].begin(), averages[i].end());
            if (miniter == averages[i].end() || maxiter == averages[i].end()) {
                continue;
            }
            float cMin = *miniter;
            float cMax = *maxiter;
            if (cMin < min) {
                min = cMin;
            }
            if (cMax > max) {
                max = cMax;
            }
        }
        /*
        if (min > min_spectrum_db) {
            min = min_spectrum_db;
        }
        if (max < max_spectrum_db) {
            max = max_spectrum_db;
        }*/
        return std::make_tuple(min,max);
    }

    float SpectrumWaterfall::normalizePower(float db_val, float min_db, float max_db) {
        // Clamp the incoming value to your expected minimum and maximum thresholds
        if (db_val < min_db) db_val = min_db;
        if (db_val > max_db) db_val = max_db;

        // Scale lineally between 0.0 and 1.0
        return (db_val - min_db) / (max_db - min_db);
    }

    SDL_Color SpectrumWaterfall::getSpectrumColor(float normalized_val) {
        // 1. Clamp input to 0.0 - 1.0 just to be safe
        normalized_val = std::max(0.0f, std::min(normalized_val, 1.0f));

        // 2. Define your 6 anchor colors using SDL_Color {R, G, B, A}
        const int NUM_COLORS = 6;
        SDL_Color palette[NUM_COLORS] = {
            {0, 0, 0, 255}, // 0.0 : Black (Noise floor)
            {0, 0, 255, 255}, // 0.2 : Blue
            {0, 255, 0, 255}, // 0.4 : Green
            {255, 255, 0, 255}, // 0.6 : Yellow
            {255, 128, 0, 255}, // 0.8 : Orange
            {255, 0, 0, 255} // 1.0 : Red (Peak power)
        };

        // 3. Find which two colors we are blending between
        float position = normalized_val * (NUM_COLORS - 1);
        int index = static_cast<int>(position);

        if (index >= NUM_COLORS - 1) {
            return palette[NUM_COLORS - 1];
        }

        // 4. Calculate the interpolation fractional delta (0.0 to 1.0)
        float t = position - index;

        // 5. Linearly interpolate the channels natively into an SDL_Color struct
        SDL_Color color;
        color.r = static_cast<uint8_t>(palette[index].r + t * (palette[index + 1].r - palette[index].r));
        color.g = static_cast<uint8_t>(palette[index].g + t * (palette[index + 1].g - palette[index].g));
        color.b = static_cast<uint8_t>(palette[index].b + t * (palette[index + 1].b - palette[index].b));
        color.a = 255; // Keep fully opaque

        return color;
    }


    /**
         * receiver thread updates blocks
         */
    void SpectrumWaterfall::receive_blocks() {
        while (this->isRunning.load(std::memory_order_relaxed)) {
            sdrcomms::IQBlock block{sdrcomms::REQUIRED_SAMPLES};
            if (receiveQueue->pop(block)) {
                if (block.samples.size() == 0) {
                    continue;
                }
                std::vector<std::complex<float> > fftOut;
                try {
                    fftOut = this->fft->fft(block.samples);
                } catch (std::exception) {
                    continue;
                }
                // write lock block readers
                std::unique_lock<std::shared_mutex> lock(this->updateMutex);

                // we will only draw if the spectrum is full of data.
                // if we have more values than we can render we need to take a sliding average
                // of the spectrum values in each rectangle size and then render the rectangle
                // for the spectrum. Each point in the spectrum will be displayed as a small rectangle of N pixels wide.
                // the last value of the spectrum is drawn at the bottom of the rectangle.
                // we should modify the spectrum to be a sliding average of the last N values.
                std::vector<std::complex<float> > fftshifted = this->fft->fftShift(fftOut);
                std::vector<float> frequencies = this->calculateFrequencies(block.centerFreq, sdrcomms::DEFAULT_SAMPLE_RATE_HZ, fftshifted.size());
                std::vector<float> block_averages = this->slidingAverage(fftshifted);
                this->avgFrequenciesHz = this->averageFrequencies(frequencies);

                this->centerFreq = block.centerFreq;

                // queue the average abs power for blocks in the spectrum
                this->spectrum.push_back(block_averages);
                this->iqBlocks.push_back(block);
                if (this->spectrum.size() > this->MAX_QUEUE_SIZE) {
                    this->spectrum.erase(this->spectrum.begin());
                    this->iqBlocks.erase(this->iqBlocks.begin());
                }
                lock.unlock();
            }
        }
    }



    /**
     * average the spectrum for block sizes within our drawable dimensions
     * @param spectrum
     * @return
     */
    std::vector<float> SpectrumWaterfall::slidingAverage(
        const std::vector<std::complex<float> > &spectrum) {
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
         * Generate a vector of the frequencies for the spectrum.
         * @param centerFreqHz
         * @param sampleRateHz
         * @param num_samples
         * @return
         */
    std::vector<float> SpectrumWaterfall::calculateFrequencies(const double centerFreqHz, const double sampleRateHz, const size_t num_samples) {
        std::vector<float> frequencies {};
        double binSpacingHz = sampleRateHz / num_samples;
        for (int i=0;i<num_samples;i++) {
            double freqHz = centerFreqHz + (i - num_samples / 2.0) * binSpacingHz;
            frequencies.push_back(freqHz);
        }
        return frequencies;
    }

    /**
         * Average the frequencies.
         * @return
         */
    std::vector<float> SpectrumWaterfall::averageFrequencies(const std::vector<float> &frequencies) {
        size_t num_blocks = this->width / this->block_width;

        std::vector<float> avgFreq(num_blocks);
        if (frequencies.size() == 0) {
            return avgFreq;
        }
        size_t sample_size = frequencies.size();
        for (size_t i = 0; i < num_blocks; i++) {
            size_t start_index = i * this->block_width;
            size_t end_index = std::min(start_index + this->block_width, sample_size);
            for (size_t j = start_index; j < end_index; j++) {
                avgFreq[i] += frequencies[j];
            }
            avgFreq[i] /= this->block_width;
        }
        return avgFreq;
    }
} // sdrcommsui
