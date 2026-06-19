# 🖥️ Raylib Game — PC

A 2D game built with **Raylib** and **C++**. You control a circle with an orbiting satellite that acts as your attack hitbox — hold and drag the mouse to move toward your cursor and hit enemies.

>  **Looking for the Android version?** It lives in a separate repository and runs directly on your phone via Android Studio → https://github.com/kevinnach0/Orbit-Defender-Android-.git

---

##  How to play

- **Hold left click** and drag toward where you want to move.
- While holding, a **red satellite** orbits around you — that's your attack hitbox.
- **Red enemies** chase you. If they touch your body, you lose health.
- If your **satellite** hits an enemy while you're attacking, it **pushes them away**.
- If your health reaches zero, the game resets.

---

##  Project structure

```
project/
├── main.cpp      # Main game loop, input, physics, render
└── Game.hpp              # AttackPlayer class declaration
└── Game.cpp              # Attack logic, hitbox collision, push effect

---

##  How to run

This version uses plain **Raylib** — no Android toolchain, no extra setup. Just Raylib and a C++ compiler.

### 1. Prerequisites

- [Raylib](https://www.raylib.com/) installed on your system.
- A C++ compiler that supports **C++17** (GCC, Clang, or MSVC).

### 2. Clone the repo

```bash
git clone <your-repo-url>
cd <project-folder>
```

### 3. Compile and run

**Linux / macOS**
```bash
g++ main.cpp Game.cpp -o game -lraylib -lm -lpthread -std=c++17
./game
```

**Windows (MinGW)**
```bash
g++ main.cpp Game.cpp -o game.exe -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++17
game.exe
```

> If Raylib is not in your system path, add `-I/path/to/raylib/include` and `-L/path/to/raylib/lib` to the compile command.

---

##  Dependencies

| Library | Description |
|---|---|
| [Raylib](https://www.raylib.com/) | Simple and powerful 2D/3D graphics library |

That's it. No extra wrappers, no build system required.

---

##  Android version

This project has a companion repository built for **Android**, using [raymob](https://github.com/Bigfoot71/raymob.git) as a Raylib compatibility layer. The core gameplay is identical — the only differences are the input system (touch drag instead of mouse) and the build environment (Android Studio instead of a terminal).

If you have an Android phone, that version is even simpler to run: open Android Studio, plug in your phone via USB, and hit **▶ Run**. No compile commands needed.

👉 **https://github.com/kevinnach0/Orbit-Defender-Android-.git**

---

##  Technical notes

- Written in **C++17**.
- Window opens at **1280 × 720** — change `InitWindow(1280, 720, ...)` in `main.cpp` to resize.
- Body collision uses overlap separation with a player radius of `38px`.
- The orbital satellite hitbox has a radius of `20px` and pushes enemies at `688 px/s`.
- Contact damage has a **0.4 second cooldown** to prevent health draining every frame on contact.
- Enemy separation is symmetric — both enemies push apart equally when overlapping.
