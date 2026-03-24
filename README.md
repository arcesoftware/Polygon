# BioDigitalSim: Hyper-Dimensional Physics Navigator

A real-time C++ simulation developed with **SDL3** that explores the intersection of high-dimensional geometry and fundamental physics. This engine projects $N$-dimensional hypercubes (from 0D to 16D) onto a 2D plane, using the "digit signature" of 55 universal physical constants to define the angular distribution of dimensions.

## 🌌 Overview

In a standard Petrie projection, dimensions are spread evenly by dividing $2\pi$ by the dimension count. This engine replaces that static symmetry with a **Physics Warp**: it uses the significand of physical constants (like the Planck Constant or Fine-Structure Constant) to determine the rotational offset of each dimension.



## 🛠 Features

* **Dynamic Dimensionality:** Seamlessly scale from a single point (**0-DIM**) to a complex hyper-web (**16-DIM**) containing $2^{16}$ ($65,536$) vertices.
* **Physical Constant Warping:** A built-in table of 55 constants (Speed of Light, Gravitational Constant, etc.) acts as the mathematical DNA for the projection.
* **Hamming Weight Coloring:** Vertices are colored based on their "layer" in the hypercube, creating a spectral radial gradient that reveals the shape's shell structure.
* **Optimized Edge Logic:** Renders topological connections for dimensions $\le 10$ using high-performance bitwise XOR neighbor detection.
* **Real-time Centering:** Adaptive window resizing and scaling to keep the geometry perfectly framed at any resolution.

## 🎮 Controls

| Key | Action |
| :--- | :--- |
| **D** | **Increase Dimension** (Up to 16) |
| **S** | **Decrease Dimension** (Down to 1) |
| **C** | **Next Physics Constant** (Cycle Forward) |
| **X** | **Previous Physics Constant** (Cycle Backward) |
| **Mouse Wheel** | **Zoom In / Out** |

## 🧪 Technical Logic

### The Projection Math
The 2D projection of any vertex $V$ in $N$ dimensions is calculated by summing the contributions of its coordinates $v_j$ (which are either $-1$ or $1$) across all dimensions:

$$x = \sum_{j=0}^{N-1} v_j \cdot \cos\left(\frac{2\pi \cdot \text{WarpValue} \cdot j}{N} + \text{Rotation}\right)$$
$$y = \sum_{j=0}^{N-1} v_j \cdot \sin\left(\frac{2\pi \cdot \text{WarpValue} \cdot j}{N} + \text{Rotation}\right)$$

When the **WarpValue** is $\pi$, the structure is perfectly symmetrical. When using constants like the **Fine-Structure Constant (7.297...)**, the symmetry breaks into complex, non-repeating interference patterns.



## 🚀 How to Run

### Prerequisites
* A C++17 compatible compiler (GCC, Clang, or MSVC).
* **SDL3** library installed on your system.

### Compilation
Using G++:
```bash
g++ main.cpp -o BioDigitalSim -lSDL3
