//
// Created by Chris Davey on 10/8/2026.
//

#include "include/DrawingApp.h"
#include <iostream>

#include "constants.h"

namespace drawing {
    DrawingApp::DrawingApp(Uint32 initFlags, std::string title, bool fullscreen, int width, int height,
                           const std::shared_ptr<Sketch> &sketch) {
        this->initFlags = initFlags;
        this->title = title;
        this->sketch = sketch;
        this->width = width;
        this->height = height;
        this->fullscreen = fullscreen;
        this->lastSecond = std::chrono::steady_clock::now();
        this->lastTimeStep = std::chrono::steady_clock::now();
    }

    int DrawingApp::Run() {

        if (SDL_Init(this->initFlags) < 0) {
            return -1;
        }
        displayServer = getDisplayServer();
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        bool running = false;

        if (!fullscreen) {
            Uint32 windowFlags = SDL_WINDOW_SHOWN;
#if defined(__linux__) || defined(__unix__) || defined(WIN32)
            windowFlags |= SDL_WINDOW_OPENGL;
#endif
            window = SDL_CreateWindow(title.c_str(),
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      width,
                                      height,
                                      windowFlags);
        } else {
            Uint32 windowFlags = SDL_WINDOW_FULLSCREEN;

#if defined(__linux__) || defined(__unix__) || defined(WIN32)
            windowFlags |= SDL_WINDOW_OPENGL;
#endif
            if (displayServer == DisplayServer::Wayland) {
                windowFlags |= SDL_WINDOW_ALLOW_HIGHDPI;
            }
            window = SDL_CreateWindow(title.c_str(),
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      0,
                                      0,
                                      windowFlags);
        }
        if (window == nullptr) {
            return -1;
        }
        if (displayServer == DisplayServer::Wayland) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        } else {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        }
        if (renderer == nullptr) {
            // fallback for software

            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
            if (renderer == nullptr) {
                std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
                return -1;
            }
            std::cout << "Using software renderer" << std::endl;
        } else {
            std::cout << "Using hardware renderer" << std::endl;
        }

        SDL_GetWindowSize(window, &width, &height);
        SDL_SetWindowSize(window, width, height);

        running = true;
        SDL_Texture* canvas = nullptr;
        if (displayServer != DisplayServer::Wayland) {
            SDL_RenderSetViewport(renderer, NULL);
            canvas = SDL_CreateTexture(renderer,
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET,
                                            width, height);
        } else if (displayServer == DisplayServer::Wayland) {
            SDL_PumpEvents();
            SDL_Delay(50);
            SDL_PumpEvents();
        }
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    return 0;
                }
                if (event.type == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    width = event.window.data1;
                    height = event.window.data2;
                }
            }
            if (displayServer == DisplayServer::Wayland) {
                render(renderer);
            } else {
                SDL_SetRenderTarget(renderer, canvas);
                render(renderer);
                SDL_SetRenderTarget(renderer, NULL);
            }

            auto now = std::chrono::steady_clock::now();
            long delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastSecond).count();
            if (delta > 1000) {
                this->lastSecond = now;
                this->Fps = this->Frame;
                this->Frame = 0;
                std::cout << "FPS: " << this->Fps << std::endl;
            }
            this->Frame++;
            long stepDelta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastTimeStep).count();
            if (stepDelta > stepInterval) {
                this->lastTimeStep = now;
                this->TimeStep++;
                if (this->TimeStep >= std::numeric_limits<int>::max()) {
                    this->TimeStep = 1;
                }
            }
            if (displayServer != DisplayServer::Wayland) {
                // 4. Copy your clean, unghosted canvas snapshot straight to the window
                SDL_RenderCopy(renderer, canvas, NULL, NULL);
            }
            SDL_RenderPresent(renderer);

        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    void DrawingApp::render(SDL_Renderer *renderer) {
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);
        sketch->DrawSketch(renderer, width, height, TimeStep, Frame, Fps);
    }
} // drawing
