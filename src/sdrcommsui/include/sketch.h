//
// Created by Chris Davey on 9/8/2026.
//

#ifndef DRAWSKETCH_SKETCH_H
#define DRAWSKETCH_SKETCH_H
#include <SDL.h>
namespace drawing {
    class Sketch {
    public:

        virtual void DrawSketch(SDL_Renderer *renderer, int width, int height, int timestep, int frame, int fps) = 0;


    protected:
        int TimeStep;
        int Frame;
        int Fps;

    };
}

#endif //DRAWSKETCH_SKETCH_H
