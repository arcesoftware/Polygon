#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Configuration & State ---
int currentDim = 8;
int currentIdx = 0;
int windowWidth = 1280;
int windowHeight = 800;
float rotationAngle = 0.0f;
float colorShift = 0.0f;
float zoom = 1.0f;
std::string inputBuffer = "";

struct PhysicalConstant {
    std::string name;
    float value;
    std::string rawDisplay;
};

std::vector<PhysicalConstant> physicsTable = {
    {"Standard Pi", (float)M_PI, "3.14159"},
    {"Muon g-factor", 2.0023318f, "-2.00233184123"},
    {"Electron g-factor", 2.0023193f, "-2.00231930436"},
    {"Cosmological Constant", 1.089f, "1.089e-52"},
    {"Reduced Planck", 1.0545718f, "1.054571817e-34"},
    {"Planck Constant", 6.6260701f, "6.62607015e-34"},
    {"Electron Mass", 9.1093837f, "9.1093837139e-31"},
    {"Thomson Cross Section", 6.6524587f, "6.6524587051e-29"},
    {"Muon Mass", 1.8835316f, "1.883531627e-28"},
    {"Atomic Mass Constant", 1.6605391f, "1.66053906892e-27"},
    {"Proton Mass", 1.6726219f, "1.67262192595e-27"},
    {"Neutron Mass", 1.6749275f, "1.67492750056e-27"},
    {"Tau Mass", 3.16754f, "3.16754e-27"},
    {"Nuclear Magneton", 5.0507837f, "5.0507837393e-27"},
    {"Bohr Magneton", 9.2740101f, "9.2740100657e-24"},
    {"Boltzmann Constant", 1.380649f, "1.380649e-23"},
    {"Elementary Charge", 1.6021766f, "1.602176634e-19"},
    {"Rydberg Energy", 2.1798724f, "2.1798723611e-18"},
    {"Hartree Energy", 4.3597447f, "4.3597447222e-18"},
    {"1st Radiation (Radiance)", 1.1910429f, "1.191042972e-16"},
    {"1st Radiation Constant", 3.7417718f, "3.741771852e-16"},
    {"Magnetic Flux Quantum", 2.0678338f, "2.067833848e-15"},
    {"Classical Electron Radius", 2.8179403f, "2.8179403205e-15"},
    {"Vacuum Permittivity", 8.8541878f, "8.8541878188e-12"},
    {"Bohr Radius", 5.2917721f, "5.29177210544e-11"},
    {"Gravitational Constant", 6.67430f, "6.67430e-11"},
    {"Molar Planck Constant", 3.9903127f, "3.990312712e-10"},
    {"Stefan-Boltzmann", 5.6703744f, "5.670374419e-8"},
    {"Vacuum Permeability", 1.2566371f, "1.256637061e-6"},
    {"Fermi Coupling", 1.1663787f, "1.1663787e-5"},
    {"Molar Vol Silicon", 1.2058832f, "1.205883199e-5"},
    {"Conductance Quantum", 7.7480917f, "7.748091729e-5"},
    {"Quantum Circulation", 3.6369475f, "3.6369475467e-4"},
    {"Molar Mass Constant", 1.0000000f, "1.00000000105e-3"},
    {"Wien Wavelength", 2.8977719f, "2.897771955e-3"},
    {"Wien Entropy", 3.0029161f, "3.002916077e-3"},
    {"Fine-Structure Constant", 7.2973525f, "7.2973525643e-3"},
    {"Molar Mass C-12", 1.2000000f, "1.20000000126e-2"},
    {"2nd Radiation Constant", 1.4387768f, "1.438776877e-2"},
    {"Weak Mixing Angle", 0.23153f, "0.23153"},
    {"W-to-Z Mass Ratio", 0.88145f, "0.88145"},
    {"Proton g-factor", 5.5856947f, "5.5856946893"},
    {"Molar Gas Constant", 8.3144626f, "8.314462618"},
    {"Inv Fine-Structure", 1.3703599f, "137.035999"},
    {"Vacuum Impedance", 3.7673031f, "376.730313412"},
    {"Proton-Electron Ratio", 1.8361527f, "1836.152673"},
    {"Inv Conductance Quantum", 1.2906403f, "12906.40372"},
    {"Von Klitzing Constant", 2.5812807f, "25812.80745"},
    {"Faraday Constant", 9.6485332f, "96485.3321"},
    {"Rydberg Constant", 1.0973731f, "10973731.568"},
    {"Speed of Light", 2.9979245f, "299792458"},
    {"Cesium Frequency", 9.1926317f, "9192631770"},
    {"Wien Frequency", 5.8789257f, "5.878925757e10"},
    {"Josephson Constant", 4.8359784f, "4.835978484e14"},
    {"Avogadro Constant", 6.0221407f, "6.02214076e23"},
    {"Fibonacci Constant", 1.618033f, "1.618033"},
    {"Feigenbaum Alpha", 2.502907f, "2.502907875"},
    {"Feigenbaum Delta", 4.669201f, "4.669201609"},
    {"Euler-Mascheroni", 0.577215f, "0.577215664"},
    {"Apéry's Constant", 1.202056f, "1.202056903"},
    {"Gelfond's Constant", 23.14069f, "23.14069263"},
    {"Khinchin's Constant", 2.685452f, "2.685452001"},
    {"Lévy's Constant", 3.275822f, "3.275822918"},
    {"Twin Prime Constant", 0.660161f, "0.660161815"},
    {"Viswanath's Constant", 1.131988f, "1.13198824"},
    {"Plastic Number", 1.324718f, "1.324717957"},
    {"Silver Ratio", 2.414213f, "2.414213562"},
    {"Super-Golden Ratio", 1.465571f, "1.465571231"}
};

struct PointND {
    std::vector<float> coords;
    int hammingWeight;
    uint32_t id;
};

std::vector<PointND> vertices;

void logStatus() {
    SDL_Log("--- Navigation Sync ---");
    SDL_Log("DIM: %d | Vertices: %llu", currentDim, (unsigned long long)vertices.size());
    SDL_Log("Physics Table Index: [%d]", currentIdx);
    SDL_Log("Constant: %s", physicsTable[currentIdx].name.c_str());
    SDL_Log("Value = %f", physicsTable[currentIdx].value);
    SDL_Log("-----------------------");
}

void initHypercube(int dim) {
    vertices.clear();
    if (dim <= 0) return;
    uint32_t numVertices = 1 << dim;
    for (uint32_t i = 0; i < numVertices; ++i) {
        PointND p; p.id = i; p.coords.resize(dim);
        uint32_t n = i; int count = 0;
        while (n > 0) { n &= (n - 1); count++; }
        p.hammingWeight = count;
        for (int j = 0; j < dim; ++j) p.coords[j] = (i & (1 << j)) ? 1.0f : -1.0f;
        vertices.push_back(p);
    }
}

int main(int argc, char* argv[]) {
    // Fill remaining to 255 with procedural variations
    while (physicsTable.size() < 256) {
        float seed = 1.0f + (physicsTable.size() * 0.12345f);
        physicsTable.push_back({ "Procedural Warp", seed, std::to_string(seed) });
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("BioDigitalSim: Hyper-Explorer", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    initHypercube(currentDim);
    logStatus();

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN) {
                // Number Entry Logic
                if (e.key.key >= SDLK_0 && e.key.key <= SDLK_9) {
                    inputBuffer += (char)('0' + (e.key.key - SDLK_0));
                }
                if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER) {
                    if (!inputBuffer.empty()) {
                        currentIdx = std::clamp(std::stoi(inputBuffer), 0, (int)physicsTable.size() - 1);
                        inputBuffer = ""; logStatus();
                    }
                }
                if (e.key.key == SDLK_BACKSPACE && !inputBuffer.empty()) inputBuffer.pop_back();

                // Navigation D/S (DIM) and C/X (Constant)
                if (e.key.key == SDLK_C) { currentIdx = (currentIdx + 1) % physicsTable.size(); logStatus(); }
                if (e.key.key == SDLK_X) { currentIdx = (currentIdx - 1 + physicsTable.size()) % physicsTable.size(); logStatus(); }
                if (e.key.key == SDLK_D) { currentDim = std::min(16, currentDim + 1); initHypercube(currentDim); logStatus(); }
                if (e.key.key == SDLK_S) { currentDim = std::max(1, currentDim - 1); initHypercube(currentDim); logStatus(); }
            }
            if (e.type == SDL_EVENT_WINDOW_RESIZED) SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            if (e.type == SDL_EVENT_MOUSE_WHEEL) zoom *= (e.wheel.y > 0) ? 1.1f : 0.9f;
        }

        rotationAngle += 0.005f; colorShift += 0.003f;
        SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255); SDL_RenderClear(renderer);

        float cx = windowWidth / 2.0f, cy = windowHeight / 2.0f;
        float scale = (std::min(windowWidth, windowHeight) / (currentDim + 2.8f)) * 1.4f * zoom;
        float warp = physicsTable[currentIdx].value;

        std::vector<SDL_FPoint> proj(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            float px = 0, py = 0;
            for (int j = 0; j < currentDim; ++j) {
                float a = (2.0f * warp * j / (float)currentDim) + rotationAngle;
                px += vertices[i].coords[j] * cosf(a); py += vertices[i].coords[j] * sinf(a);
            }
            proj[i] = { px * scale + cx, py * scale + cy };
        }

        if (currentDim <= 10) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 80, 80, 120, 50);
            for (const auto& v : vertices) {
                for (int j = 0; j < currentDim; ++j) {
                    uint32_t nIdx = v.id ^ (1 << j);
                    if (nIdx > v.id) SDL_RenderLine(renderer, proj[v.id].x, proj[v.id].y, proj[nIdx].x, proj[nIdx].y);
                }
            }
        }

        for (size_t i = 0; i < vertices.size(); ++i) {
            float t = (float)vertices[i].hammingWeight / (float)currentDim;
            Uint8 r = (Uint8)(127 * (1 + cosf(2 * M_PI * (t + colorShift))));
            Uint8 g = (Uint8)(127 * (1 + cosf(2 * M_PI * (t + 0.33 + colorShift))));
            Uint8 b = (Uint8)(127 * (1 + cosf(2 * M_PI * (t + 0.66 + colorShift))));
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            float dS = (currentDim > 11) ? 2.5f : 5.0f;
            SDL_FRect dot = { proj[i].x - dS / 2, proj[i].y - dS / 2, dS, dS };
            SDL_RenderFillRect(renderer, &dot);
        }
        SDL_RenderPresent(renderer); SDL_Delay(10);
    }
    SDL_Quit(); return 0;
}
