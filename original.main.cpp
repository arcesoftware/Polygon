#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>

// Portable way to ensure PI is defined
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

const int WIDTH = 800;
const int HEIGHT = 800;
const int DIM = 17;

struct Point8D {
    float coords[DIM];
};

struct Particle {
    float x, y;
    SDL_Color color;
};

std::vector<Particle> particles;
float rotationAngle = 0.0f;

void generatePetrie8Cube() {
    particles.clear();
    int numVertices = 1 << DIM; // 256 vertices

    for (int i = 0; i < numVertices; ++i) {
        Point8D p;
        int onesCount = 0;
        for (int j = 0; j < DIM; ++j) {
            bool bit = (i & (1 << j));
            p.coords[j] = bit ? 1.0f : -1.0f;
            if (bit) onesCount++;
        }

        float projX = 0, projY = 0;
        for (int j = 0; j < DIM; ++j) {
            // Using SDL_PI_F is another safe alternative if M_PI fails
            float angle = 2.0f * (float)M_PI * j / DIM + rotationAngle;
            projX += p.coords[j] * cosf(angle);
            projY += p.coords[j] * sinf(angle);
        }

        float scale = 70.0f;

        // Color based on "Hamming weight" (number of 1s in the 8D coordinate)
        // This helps visualize the layers of the hypercube
        Uint8 r = (Uint8)(onesCount * 31);
        Uint8 g = (Uint8)(255 - onesCount * 20);
        Uint8 b = (Uint8)(150 + onesCount * 10);

        particles.push_back({
            projX * scale + (WIDTH / 2),
            projY * scale + (HEIGHT / 2),
            { r, g, b, 255 }
            });
    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("8-Cube Petrie Projection", WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        rotationAngle += 0.005f; // Slow rotation to appreciate the geometry
        generatePetrie8Cube();

        SDL_SetRenderDrawColor(renderer, 5, 5, 15, 255); // Dark blue background
        SDL_RenderClear(renderer);

        for (auto& p : particles) {
            SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, 255);

            // Draw vertices
            SDL_FRect rect = { p.x - 2.5f, p.y - 2.5f, 5.0f, 5.0f };
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
