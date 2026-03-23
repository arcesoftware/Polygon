#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Config ---
const int DIM = 7;                    // Can go VERY high now
const int MAX_VERTICES = 20000;        // Constant workload
const int THREAD_COUNT = std::thread::hardware_concurrency();

int windowWidth = 1280;
int windowHeight = 800;

float rotationAngle = 0.0f;
float zoom = 1.0f;
bool isPaused = false;

// --- Projection buffers ---
struct ProjectedVertex {
    float x, y;
    uint8_t r, g, b;
};

std::vector<ProjectedVertex> vertexBuffer(MAX_VERTICES);

// --- Precomputed trig ---
std::vector<float> baseCos(DIM);
std::vector<float> baseSin(DIM);

// --- Fast hash (deterministic pseudo-random) ---
uint32_t hash(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// Generate coordinate without memory
inline float getCoord(uint32_t seed, int dim) {
    return (hash(seed + dim * 73856093u) & 1) ? 1.0f : -1.0f;
}

// Color from pseudo hamming weight
SDL_Color getColor(uint32_t seed) {
    int w = __popcnt(seed);

    switch (w % 6) {
    case 0: return { 255, 80, 80, 255 };
    case 1: return { 255, 180, 0, 255 };
    case 2: return { 255, 255, 0, 255 };
    case 3: return { 80, 255, 80, 255 };
    case 4: return { 0, 255, 255, 255 };
    default:return { 180, 120, 255, 255 };
    }
}

// --- Multithreaded projection ---
void projectRange(int start, int end,
    const std::vector<float>& rotCos,
    const std::vector<float>& rotSin,
    float baseScale,
    float centerX,
    float centerY,
    uint32_t frameSeed)
{
    for (int i = start; i < end; ++i) {
        uint32_t seed = hash(i + frameSeed);

        float px = 0.0f;
        float py = 0.0f;

        for (int d = 0; d < DIM; ++d) {
            float val = getCoord(seed, d);
            px += val * rotCos[d];
            py += val * rotSin[d];
        }

        auto& out = vertexBuffer[i];
        out.x = px * baseScale + centerX;
        out.y = py * baseScale + centerY;

        SDL_Color c = getColor(seed);
        out.r = c.r;
        out.g = c.g;
        out.b = c.b;
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Infinite-Dimension Hypercube",
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "software");

    // Precompute base trig
    for (int i = 0; i < DIM; ++i) {
        float theta = 2.0f * (float)M_PI * i / DIM;
        baseCos[i] = cosf(theta);
        baseSin[i] = sinf(theta);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool running = true;
    SDL_Event e;
    uint32_t frame = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                windowWidth = e.window.data1;
                windowHeight = e.window.data2;
            }

            if (e.type == SDL_EVENT_MOUSE_WHEEL) {
                if (e.wheel.y > 0) zoom *= 1.1f;
                else zoom /= 1.1f;
            }

            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_SPACE) isPaused = !isPaused;
            }
        }

        if (!isPaused) rotationAngle += 0.003f;

        SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
        SDL_RenderClear(renderer);

        float centerX = windowWidth * 0.5f;
        float centerY = windowHeight * 0.5f;
        float baseScale = (std::min(windowWidth, windowHeight) / 12.0f) * zoom;

        // Rotate trig once per frame
        std::vector<float> rotCos(DIM), rotSin(DIM);

        float c = cosf(rotationAngle);
        float s = sinf(rotationAngle);

        for (int i = 0; i < DIM; ++i) {
            rotCos[i] = baseCos[i] * c - baseSin[i] * s;
            rotSin[i] = baseCos[i] * s + baseSin[i] * c;
        }

        // --- Multithreading ---
        std::vector<std::thread> threads;
        int chunk = MAX_VERTICES / THREAD_COUNT;

        for (int t = 0; t < THREAD_COUNT; ++t) {
            int start = t * chunk;
            int end = (t == THREAD_COUNT - 1) ? MAX_VERTICES : start + chunk;

            threads.emplace_back(projectRange,
                start, end,
                std::cref(rotCos),
                std::cref(rotSin),
                baseScale,
                centerX,
                centerY,
                frame
            );
        }

        for (auto& th : threads) th.join();

        // --- Draw edges (cheap approximation) ---
        for (int i = 0; i < MAX_VERTICES - 1; i += 2) {
            auto& v1 = vertexBuffer[i];
            auto& v2 = vertexBuffer[i + 1];

            SDL_SetRenderDrawColor(renderer, 100, 200, 255, 30);
            SDL_RenderLine(renderer, v1.x, v1.y, v2.x, v2.y);
        }

        // --- Draw vertices ---
        for (int i = 0; i < MAX_VERTICES; ++i) {
            auto& v = vertexBuffer[i];

            SDL_SetRenderDrawColor(renderer, v.r, v.g, v.b, 255);

            SDL_FRect dot = { v.x - 2.0f, v.y - 2.0f, 4.0f, 4.0f };
            SDL_RenderFillRect(renderer, &dot);
        }

        SDL_RenderPresent(renderer);

        frame++;
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}