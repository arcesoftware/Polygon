#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// -------- CONFIG --------
const int DIM = 8;                 // Can go HIGH now
const int MAX_PARTICLES = 15000;    // Constant workload

int windowWidth = 1280;
int windowHeight = 800;

float rotationAngle = 0.0f;
float zoom = 1.0f;
bool isPaused = false;
uint32_t frameSeed = 0;

// -------- FAST HASH --------
inline uint32_t fastHash(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// -------- BIT → COORD --------
inline float getCoord(uint32_t index, int dim) {
    return (index >> dim) & 1 ? 1.0f : -1.0f;
}

// -------- APPROX HAMMING --------
inline int pseudoWeight(uint32_t x) {
    return __popcnt(x);
}

// -------- COLOR --------
SDL_Color getPetrieColor(int weight) {
    switch (weight % 6) {
    case 0: return { 255, 80, 80, 255 };
    case 1: return { 255, 165, 0, 255 };
    case 2: return { 255, 255, 0, 255 };
    case 3: return { 50, 255, 50, 255 };
    case 4: return { 0, 255, 255, 255 };
    default:return { 180, 120, 255, 255 };
    }
}

// -------- MAIN --------
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Procedural Infinite Hypercube",
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                windowWidth = e.window.data1;
                windowHeight = e.window.data2;
            }

            if (e.type == SDL_EVENT_MOUSE_WHEEL) {
                zoom *= (e.wheel.y > 0) ? 1.1f : 0.9f;
            }

            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_SPACE) isPaused = !isPaused;
            }
        }

        if (!isPaused) {
            rotationAngle += 0.004f;
            frameSeed += 1; // slowly explore space
        }

        SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255);
        SDL_RenderClear(renderer);

        float centerX = windowWidth * 0.5f;
        float centerY = windowHeight * 0.5f;
        float baseScale = (std::min(windowWidth, windowHeight) / 12.0f) * zoom;

        // -------- PROJECT PROCEDURAL POINTS --------
        for (int i = 0; i < MAX_PARTICLES; ++i) {

            // Deterministic “vertex”
            uint32_t index = fastHash(i + frameSeed * 1315423911u);

            float px = 0.0f;
            float py = 0.0f;

            for (int d = 0; d < DIM; ++d) {
                float val = getCoord(index, d);

                float theta = 2.0f * M_PI * d / DIM + rotationAngle;

                px += val * cosf(theta);
                py += val * sinf(theta);
            }

            float x = px * baseScale + centerX;
            float y = py * baseScale + centerY;

            SDL_Color c = getPetrieColor(pseudoWeight(index));

            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 200);

            SDL_FRect dot = { x - 2.0f, y - 2.0f, 4.0f, 4.0f };
            SDL_RenderFillRect(renderer, &dot);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}
