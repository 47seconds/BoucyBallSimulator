# BouncyBallSimulator

A simple graphics application built in C that simulates the motion of a ball within a closed environment. This simulation incorporates basic physics principles such as gravity, friction, and wall damping.

**Check the [Releases](https://github.com/47seconds/BouncyBallSimulator.git) page to test it out yourself!**

## Features

- **Realistic Physics**: Simulates gravity, friction, and wall damping to mimic real-world behavior.
- **Customizable**: Adjust parameters such as gravity, friction, and initial velocity to test different scenarios.
- **Lightweight**: Minimal dependencies with efficient C code.

## Getting Started

### Prerequisites

Ensure you have the following installed on your system:

- GCC (GNU Compiler Collection)
- SDL2 library (for graphics rendering)

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/47seconds/BouncyBallSimulator.git
   cd BouncyBallSimulator
   ```
2. Install the SDL2 library:

   - **Ubuntu/Debian:**
   ```bash
   sudo apt-get update
   sudo apt-get install libsdl2-dev
   ```

   - **Arch Linux:**
   ```bash
   sudo pacman -S sdl2
   ```

   - **Windows:**
     - Refer to [this](https://www.youtube.com/watch?v=H08t6gD1Y1E&t=98s&ab_channel=HerbGlitch) video. Note that this is not the best way of doing things.
   1. Select the **SDL version 2** from [releases](https://github.com/libsdl-org/SDL/releases/) and downlaod **SDL2-devel-2.xx.xx-mingw.zip**.
   2. Extract the files in a folder.
   3. Go to: **SDL2-2.xx.xx > i686-w64-mingw32** and copy **include** and **lib** folders and paste these folders in **BouncyBallSimulator > src** (make a src directory yourself).
   4. Also copy **SDL2-2.xx.xx > i686-w64-mingw32 > bin > SDL2.dll** into **BouncyBallSimulator**.

3. Compile the source code:
   - **Linux:**
   ```bash
   gcc -o main main.c -lm `SDL2-config --cflags --libs`
   ```
   - **Windows:**
   ```bash
   gcc -Isrc/Include -Lsrc/lib -o main main.c -Imingw32 -ISDL2main -SDL2
   ```

   **Running the Alternate Version:**
      - **Linux:**
      ```bash
      gcc -o main2 main2.c -lm `SDL2-config --cflags --libs`
      ```
      - **Windows:**
      ```bash
      gcc -Isrc/Include -Lsrc/lib -o main2 main2.c -Imingw32 -ISDL2main -SDL2
      ```

### Project Structure
```bash
.
├── main.c         # Simulator Version-1 source file
├── main2.c        # Somulator Version-2 source file
├── README.md      # Project documentation
├── LICENCE        # MIT LICENSE
```

### How It Works
This simulation employs:

   - Gravity: Adds downward acceleration.
   - Friction: Slows down the ball's motion over time.
   - Wall Damping: Reduces energy upon collisions with boundaries.
   - Multiple ball simulation with collision detection.
   - Enhanced graphics and smoother animations.

Adjust the parameters in the source files to explore different behaviors.

### License
This project is licensed under the **MIT License**.