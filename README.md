# 3D Tetris with OpenGL

This project implements a **3D Tetris**-style game using **OpenGL** and **GLSL** shaders in C/C++. It is designed to illustrate core real-time rendering concepts such as shader-based graphics, transformations, and basic game mechanics.

## Features

- **3×3 Falling Blocks:** A single cube-shaped block composed of smaller cubes (3×3 grid) drops onto a **9×9 board**.
- **Grid Structure:** Both the falling blocks and the board have visible border lines.
- **Camera & View Controls:** Smoothly rotate the view around the game area while maintaining consistent movement directions for the falling block.
- **Lighting & Shading:**
  - Ambient, diffuse, and Blinn-Phong shading models.
  - At least one point light source plus ambient lighting for an appealing 3D effect.
- **Game Mechanics:**
  - **Step-by-Step Falling:** The block descends in discrete steps, rather than continuously.
  - **Movement & Speed Control:** 
    - Move left/right relative to the current view.
    - Speed up or slow down the fall, or even stop it completely.
  - **Line Clearing:** Completing a row fully collapses that row, and any blocks above it fall down.
  - **Scoring & Game Over:** Keep track of points based on cleared rows. The game ends when new blocks cannot spawn without collision.
  - **Text Overlay:** Displays the current camera view, score, and “Game Over” message when the game ends.

## Key Controls

- **A / D:** Move the block left/right (relative to the current view).
- **W / S:** Slow down / speed up the falling block.
- **H / K:** Rotate the camera/view smoothly around the scene.
  
*(You can adapt these controls to your preference, but they should match the assignment’s specifications.)*

## File Structure

- **Source Files:**
  - OpenGL/GLSL source code for rendering, shading, and game logic.
  - Optional utility classes/functions for collision detection, transformations, and bounding boxes.
- **Makefile:**
  - Compiles the project on a system with OpenGL, GLFW, GLEW, and GLM installed.

## Dependencies

- **OpenGL 3.3+** (or a compatible version supporting GLSL 330)
- **GLFW** for window/context management
- **GLEW** for OpenGL extension handling
- **GLM** for math operations (matrices, vectors, etc.)
- **C/C++** compiler (e.g., `g++` or `clang++`)

*(Ensure these libraries are installed on your system or environment before building.)*

## Build & Run

1. **Extract or clone** the project into a local directory.
2. **Compile** using the provided Makefile:
   ```bash
   make
