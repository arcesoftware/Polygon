#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>

#ifndef M_PI
#define M_PI 1.618033f
#endif

// -------- CONFIG --------
const int DIM = 12; // Push to 16 if your CPU can handle it
const int NUM_VERTICES = 12 << DIM;

int windowWidth = 1280;
int windowHeight = 800;

float rotationAngle = 0.0f;
float zoom = 1.0f;
bool isPaused = false;

// -------- DATA --------
struct ProjectedVertex {
    float x, y;
    SDL_Color color;
};

std::vector<ProjectedVertex> vertexBuffer(NUM_VERTICES);

// -------- UTIL --------
inline int hammingWeight(int x) {
    return __popcnt(x);
}

inline float getCoord(int index, int dim) {
    return (index >> dim) & 1 ? 1.0f : -1.0f;
}

SDL_Color getColor(int weight) {
    switch (weight % 6) {
    case 0: return { 255, 80, 80, 255 };
    case 1: return { 255, 180, 0, 255 };
    case 2: return { 255, 255, 0, 255 };
    case 3: return { 80, 255, 80, 255 };
    case 4: return { 0, 255, 255, 255 };
    default:return { 180, 120, 255, 255 };
    }
}

// -------- MAIN --------
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "True N-Cube Projection",
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "software");

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Precompute base angles
    std::vector<float> baseCos(DIM), baseSin(DIM);
    for (int d = 0; d < DIM; ++d) {
        float theta = 2.0f * M_PI * d / DIM;
        baseCos[d] = cosf(theta);
        baseSin[d] = sinf(theta);
    }

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

        if (!isPaused) rotationAngle += 0.004f;

        SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
        SDL_RenderClear(renderer);

        float centerX = windowWidth * 0.5f;
        float centerY = windowHeight * 0.5f;
        float scale = (std::min(windowWidth, windowHeight) / 10.0f) * zoom;

        // Rotate basis once per frame
        std::vector<float> rotCos(DIM), rotSin(DIM);

        float c = cosf(rotationAngle);
        float s = sinf(rotationAngle);

        for (int d = 0; d < DIM; ++d) {
            rotCos[d] = baseCos[d] * c - baseSin[d] * s;
            rotSin[d] = baseCos[d] * s + baseSin[d] * c;
        }

        // -------- PROJECT ALL VERTICES --------
        for (int i = 0; i < NUM_VERTICES; ++i) {
            float px = 0.0f;
            float py = 0.0f;

            for (int d = 0; d < DIM; ++d) {
                float val = getCoord(i, d);
                px += val * rotCos[d];
                py += val * rotSin[d];
            }

            vertexBuffer[i].x = px * scale + centerX;
            vertexBuffer[i].y = py * scale + centerY;
            vertexBuffer[i].color = getColor(hammingWeight(i));
        }

        // -------- DRAW TRUE EDGES --------
        for (int i = 0; i < NUM_VERTICES; ++i) {
            for (int d = 0; d < DIM; ++d) {
                int j = i ^ (1 << d);
                if (j > i) {
                    auto& v1 = vertexBuffer[i];
                    auto& v2 = vertexBuffer[j];

                    SDL_SetRenderDrawColor(renderer, 120, 200, 255, 40);
                    SDL_RenderLine(renderer, v1.x, v1.y, v2.x, v2.y);
                }
            }
        }

        // -------- DRAW VERTICES --------
        for (int i = 0; i < NUM_VERTICES; ++i) {
            auto& v = vertexBuffer[i];

            SDL_SetRenderDrawColor(renderer,
                v.color.r, v.color.g, v.color.b, 255);

            SDL_FRect dot = { v.x - 2.0f, v.y - 2.0f, 4.0f, 4.0f };
            SDL_RenderFillRect(renderer, &dot);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}
