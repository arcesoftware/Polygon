#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 1.618033988749894f
#endif

const int DIM = 14;
int windowWidth = 1280;
int windowHeight = 800;

struct Point8D {
    float coords[DIM];
    int hammingWeight;
};

std::vector<Point8D> hypercubeVertices;
float rotationAngle = 0.0f;
float colorShift = 0.0f; // For dynamic color cycling
float zoom = 1.0f;
bool isPaused = false;

int countSetBits(int n) {
    int count = 0;
    while (n > 0) { n &= (n - 1); count++; }
    return count;
}

void initHypercube() {
    hypercubeVertices.clear();
    for (int i = 0; i < (1 << DIM); ++i) {
        Point8D p;
        p.hammingWeight = countSetBits(i);
        for (int j = 0; j < DIM; ++j) {
            p.coords[j] = (i & (1 << j)) ? 1.0f : -1.0f;
        }
        hypercubeVertices.push_back(p);
    }
}

// MATHEMATICAL COLORING: Cosine Gradient
// This function creates a smooth spectrum based on the layer
SDL_Color getMathematicalColor(int weight, float shift) {
    // Normalize weight to 0.0 - 1.0 range
    float t = (float)weight / (float)DIM;

    // Using a cosine gradient formula: color = a + b * cos(2*PI * (c*t + d))
    // This creates a high-end "scientific visualization" look
    Uint8 r = (Uint8)(127.5f * (1.0f + cosf(2.0f * M_PI * (1.0f * t + 0.0f + shift))));
    Uint8 g = (Uint8)(127.5f * (1.0f + cosf(2.0f * M_PI * (1.0f * t + 0.33f + shift))));
    Uint8 b = (Uint8)(127.5f * (1.0f + cosf(2.0f * M_PI * (1.0f * t + 0.66f + shift))));

    return { r, g, b, 255 };
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("8-Cube Mathematical Projection", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    initHypercube();

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
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_SPACE) isPaused = !isPaused;
        }

        if (!isPaused) {
            rotationAngle += 0.005f;
            colorShift += 0.002f; // Slowly cycles the colors through the layers
        }

        SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
        SDL_RenderClear(renderer);

        float centerX = windowWidth / 2.0f;
        float centerY = windowHeight / 2.0f;
        float baseScale = (std::min(windowWidth, windowHeight) / 10.0f) * zoom;

        for (const auto& v : hypercubeVertices) {
            float projX = 0, projY = 0;
            for (int j = 0; j < DIM; ++j) {
                float theta = 2.0f * (float)M_PI * j / DIM + rotationAngle;
                projX += v.coords[j] * cosf(theta);
                projY += v.coords[j] * sinf(theta);
            }

            SDL_Color c = getMathematicalColor(v.hammingWeight, colorShift);
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);

            float drawX = projX * baseScale + centerX;
            float drawY = projY * baseScale + centerY;

            // Use a slightly larger point for the "nodes"
            SDL_FRect dot = { drawX - 3.5f, drawY - 3.5f, 7.0f, 7.0f };
            SDL_RenderFillRect(renderer, &dot);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_Quit();
    return 0;
}
