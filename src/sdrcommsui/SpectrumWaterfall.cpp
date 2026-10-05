//
// Created by Chris Davey on 5/10/2026.
//

#include "include/SpectrumWaterfall.h"

#include "constants.h"

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
        if (this->spectrum.size() < this->MAX_QUEUE_SIZE) {
            return;
        }
        SDL_RenderClear(renderer);
        //std::shared_lock<std::shared_mutex> lock(this->updateMutex);
        std::vector<std::vector<float>> averages{this->spectrum.begin(), this->spectrum.end()};
        this->draw_blocks(renderer, averages);
        //lock.unlock();
    }

    /**
     * pass spectrum copy
     * @param renderer
     * @param averages
     */
    void SpectrumWaterfall::draw_blocks(SDL_Renderer *renderer, std::vector<std::vector<float>> averages) {

        int y_block_size = height / averages.size();
        int y_pos = 0;

        auto [min, max] = this->get_min_max(averages);
        size_t num_rows = std::min(averages.size(), this->MAX_QUEUE_SIZE);

        for (size_t i = 0; i < num_rows; i++) {
            int x_pos = 0;
            if (i >= num_rows) {
                break;
            }
            size_t num_blocks = averages[i].size();
            for (size_t j = 0; j < num_blocks; j++) {

                float avg = this->normalizePower(averages[i][j],
                    min,
                    max);
                SDL_Color col = this->getSpectrumColor(avg);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
                SDL_Rect rect = SDL_Rect{x_pos, y_pos, this->block_width, y_block_size};
                SDL_RenderFillRect(renderer, &rect);
                x_pos += this->block_width;
            }
            y_pos += y_block_size;
        }

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
                std::vector<std::complex<float> > fftOut = this->fft->fft(block.samples);
                // write lock block readers
                std::unique_lock<std::shared_mutex> lock(this->updateMutex);

                // we will only draw if the spectrum is full of data.
                // if we have more values than we can render we need to take a sliding average
                // of the spectrum values in each rectangle size and then render the rectangle
                // for the spectrum. Each point in the spectrum will be displayed as a small rectangle of N pixels wide.
                // the last value of the spectrum is drawn at the bottom of the rectangle.
                // we should modify the spectrum to be a sliding average of the last N values.
                std::vector<std::complex<float> > fftshifted = this->fftShift(fftOut);
                std::vector<float> block_averages = this->slidingAverage(fftshifted);

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
         * fft shift the spectrum
         * @param spectrum
         * @return
         */
    std::vector<std::complex<float> > SpectrumWaterfall::fftShift(const std::vector<std::complex<float> > &source) {
        size_t half_size = source.size() / 2;
        size_t N = source.size();
        std::vector<std::complex<float> > output(source.size());
        std::memcpy(output.data(), source.data() + half_size, (N - half_size) * sizeof(std::complex<float>));
        std::memcpy(output.data() + half_size, source.data(), half_size * sizeof(std::complex<float>));
        return output;
    }

    /**
     * average the spectrum for block sizes within our drawable dimensions
     * @param spectrum
     * @return
     */
    std::vector<float> SpectrumWaterfall::slidingAverage(
        const std::vector<std::complex<float> > &spectrum) {
        size_t blocks_size = spectrum.size() / this->block_width;
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
} // sdrcommsui
