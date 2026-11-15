//
// Created by Chris Davey on 5/10/2026.
//

#ifndef SDRCOMMS_SPECTRUMWATERFALL_H
#define SDRCOMMS_SPECTRUMWATERFALL_H
#include <SDL.h>
#include <shared_mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>
#include "ReceiveQueue.h"
#include "FFTTransform.h"
#include "sketch.h"
namespace sdrcommsui {
    class SpectrumWaterfall : public drawing::Sketch {
    public:
        SpectrumWaterfall() = default;

        SpectrumWaterfall(int width, int height, const size_t fftSize, const std::shared_ptr<sdrcomms::ReceiveQueue>& receiveQueue);

        /**
         * initialise the receiver thread and fft transform block.
         * @return
         */
        bool init();

        /**
         * stop processing
         */
        void stop();


        /**
         * Draw the spectrum waterfall.
         * @param renderer
         * @param width
         * @param height
         * @param timestep
         * @param frame
         * @param fps
         */
        void DrawSketch(SDL_Renderer *renderer, int width, int height, int timestep, int frame, int fps) override;


    private:
        int width;
        int height;

        std::shared_ptr<sdrcomms::ReceiveQueue> receiveQueue;
        size_t fftSize;
        std::atomic<bool> isInit;
        std::atomic<bool> isRunning;
        std::shared_ptr<sdrcomms::FFTTransform> fft;
        std::vector<sdrcomms::IQBlock> iqBlocks;
        std::vector<std::vector<float>> spectrum;
        std::vector<float> avgFrequenciesHz {};
        std::atomic<float> centerFreq;
        std::shared_mutex updateMutex;
        std::thread receive_thread;

        SDL_Texture *texture = nullptr;

        const size_t MAX_QUEUE_SIZE = 256;

        const float TICK_MARGIN = 50;
        const int TICK_HEIGHT = 5;
        const int TICK_STEP = 4;
        const int LABEL_TICK_OFFSET = 10;
        const int LABEL_TICK_STEP = 40;
        const int LABEL_TICK_TITLE_OFFSET = 30;
        /**
         * display block width in pixels
         */
        const int block_width = 2;

        const float min_spectrum_db = -20.0f;

        const float max_spectrum_db = 20.0f;

        /**
         * receiver thread updates blocks
         */
        void receive_blocks();

        /**
         * draw blocks in the renderer
         * @param renderer
         */
        void draw_blocks(SDL_Renderer *renderer, const std::vector<std::vector<float>>& averages);

        /**
         * draw the tickmarks for the average frequencies
         * @param renderer
         */
        void draw_frequencies(SDL_Renderer *renderer, const std::vector<float> &frequencies);

        /**
         * get min and max power from spectrum
         * @return
         */
        std::tuple<float,float> get_min_max(std::vector<std::vector<float>> averages);

        /**
         * normalise the db
         * @param db_val
         * @param min_db
         * @param max_db
         * @return
         */
        float normalizePower(float db_val, float min_db = -90.0f, float max_db = 0.0f);

        /**
         * get spectrum color for normalised power.
         * @param normalized_val
         * @return
         */
        SDL_Color getSpectrumColor(float normalized_val);

        /**
         * fft shift the spectrum
         * @param spectrum
         * @return
         */
        std::vector<std::complex<float>> fftShift(const std::vector<std::complex<float>>& source);

        /**
         * Generate a vector of the frequencies for the spectrum.
         * @param centerFreqHz
         * @param sampleRateHz
         * @param num_samples
         * @return
         */
        std::vector<float> calculateFrequencies(const double centerFreqHz, const double sampleRateHz, const size_t num_samples);

        /**
         * Average the frequencies.
         * @return
         */
        std::vector<float> averageFrequencies(const std::vector<float> &frequencies);

        /**
         * average the spectrum for block sizes within our drawable dimensions
         * @param spectrum
         * @return
         */
        std::vector<float> slidingAverage(const std::vector<std::complex<float>>& spectrum);
    };
} // sdrcommsui

#endif //SDRCOMMS_SPECTRUMWATERFALL_H
