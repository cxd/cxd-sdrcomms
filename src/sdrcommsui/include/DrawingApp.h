//
// Created by Chris Davey on 10/8/2026.
//

#ifndef DRAWSKETCH_DRAWINGAPP_H
#define DRAWSKETCH_DRAWINGAPP_H
#include <memory>
#include <string>
#include <chrono>
#include <iostream>
#include<SDL.h>
#include "sketch.h"

namespace drawing {
    enum class DisplayServer {
        Wayland,
        X11,
        macOS,
        Windows,
        Other
    };

    inline DisplayServer getDisplayServer() {
#if defined(__APPLE__)
        return DisplayServer::macOS;
#elif defined(_WIN32)
        return DisplayServer::Windows;
#elif defined(__linux__) || defined(__unix__)
        const char *driver = SDL_GetCurrentVideoDriver();
        std::cout << "Current video driver: [" << std::string_view(driver) << "]" << std::endl;

        if (driver != nullptr) {

            if (std::string_view(driver) == "wayland") {
                return DisplayServer::Wayland;
            } else if (std::string_view(driver) == "x11") {
                return DisplayServer::X11;
            }
        }

        // fallback to environment variables.
        if (std::getenv("WAYLAND_DISPLAY") != nullptr) {
            return DisplayServer::Wayland;
        }
        return DisplayServer::Other;
#else
        return DisplayServer::Other;
#endif
    }

    /**
     * Drawing app is the root of the application
     * responsible for initialization SDL and running the event loop.
     */
    class DrawingApp {
    public:
        DrawingApp(Uint32 initFlags, std::string title, bool fullscreen, int width, int height,
                   const std::shared_ptr<Sketch> &sketch,
                   const std::shared_ptr<Sketch> &lineSketch,
                   const std::shared_ptr<Sketch> &controlView);

        /**
         * Execute the event loop
         * @return
         */
        virtual int Run();

    protected:
        /**
         * execute the render loop.
         * @param renderer
         */
        void render(SDL_Renderer *renderer);
        const float stepInterval = 100;
        int Frame = 0;
        int Fps = 0;
        int TimeStep = 0;

        Uint32 initFlags;
        std::string title;
        std::shared_ptr<Sketch> sketch;
        std::shared_ptr<Sketch> lineSketch;
        std::shared_ptr<Sketch> controlView;
        int width;
        int height;
        bool fullscreen;
        DisplayServer displayServer;



        std::chrono::steady_clock::time_point lastSecond;
        std::chrono::steady_clock::time_point lastTimeStep;
    };
} // drawing

#endif //DRAWSKETCH_DRAWINGAPP_H
