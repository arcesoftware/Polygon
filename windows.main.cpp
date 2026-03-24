#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Configuration ---
const int DIM = 8;
int windowWidth = 1280;
int windowHeight = 800;

struct Point8D {
    float coords[DIM];
    int hammingWeight; // Number of '1' bits
};

struct Particle {
    float x, y;
    SDL_Color color;
};

// --- State ---
std::vector<Point8D> hypercubeVertices;
float rotationAngle = 0.0f;
float zoom = 1.0f;
bool isPaused = false;

// Helper to count set bits (Hamming Weight)
int countSetBits(int n) {
    int count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
}

void initHypercube() {
    hypercubeVertices.clear();
    int numVertices = 1 << DIM; // 256
    for (int i = 0; i < numVertices; ++i) {
        Point8D p;
        p.hammingWeight = countSetBits(i);
        for (int j = 0; j < DIM; ++j) {
            p.coords[j] = (i & (1 << j)) ? 1.0f : -1.0f;
        }
        hypercubeVertices.push_back(p);
    }
}

SDL_Color getPetrieColor(int weight) {
    // Colors mapped to the 9 possible Hamming weights (0 through 8)
    // This creates the "ring" aesthetic from your image
    switch (weight) {
    case 0: case 8: return { 255, 50, 50, 255 };   // Red (Outer/Inner)
    case 1: case 7: return { 255, 165, 0, 255 };  // Orange
    case 2: case 6: return { 255, 255, 0, 255 };  // Yellow
    case 3: case 5: return { 50, 255, 50, 255 };   // Green
    case 4:         return { 0, 255, 255, 255 };  // Cyan (Center)
    default:        return { 255, 255, 255, 255 };
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("8-Cube Petrie Engine", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    initHypercube();

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;

            // Handle Window Resize
            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                windowWidth = e.window.data1;
                windowHeight = e.window.data2;
            }

            // Camera: Zoom with Mouse Wheel
            if (e.type == SDL_EVENT_MOUSE_WHEEL) {
                if (e.wheel.y > 0) zoom *= 1.1f;
                else if (e.wheel.y < 0) zoom /= 1.1f;
            }

            // Controls: Space to pause rotation
            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_SPACE) isPaused = !isPaused;
            }
        }

        if (!isPaused) rotationAngle += 0.005f;

        // Render Background
        SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255);
        SDL_RenderClear(renderer);

        float centerX = windowWidth / 2.0f;
        float centerY = windowHeight / 2.0f;
        float baseScale = (std::min(windowWidth, windowHeight) / 10.0f) * zoom;

        // Project and Draw
        for (const auto& v : hypercubeVertices) {
            float projX = 0, projY = 0;

            for (int j = 0; j < DIM; ++j) {
                float theta = 2.0f * (float)M_PI * j / DIM + rotationAngle;
                projX += v.coords[j] * cosf(theta);
                projY += v.coords[j] * sinf(theta);
            }

            float drawX = projX * baseScale + centerX;
            float drawY = projY * baseScale + centerY;

            SDL_Color c = getPetrieColor(v.hammingWeight);
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            // Draw Point with size relative to screen
            SDL_FRect dot = { drawX - 3, drawY - 3, 6, 6 };
            SDL_RenderFillRect(renderer, &dot);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_Quit();
    return 0;
}
