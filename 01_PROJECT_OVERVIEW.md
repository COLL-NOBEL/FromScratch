# Project Overview: Nkentseu + Jenga Build System

## Project Structure

Your project is a graphics programming setup using **Nkentseu** (a comprehensive game/graphics library) with the **Jenga** build system (a Python-based build tool similar to Premake).

```
FromSratch/
├── trial.wks.jenga          # Workspace configuration (root build file)
├── OpenPrj/
│   ├── open.prj.jenga       # Project configuration (MyOpenGLApp)
│   ├── src/
│   │   └── main.cpp         # Your application entry point
│   ├── nkMath/              # Math helper headers
│   ├── shaders/             # GPU shader files (GLSL)
│   └── assets/              # Game/graphics assets
├── Externals/libs/
│   ├── nkentseu/            # Main Nkentseu library headers
│   │   ├── NKContainers/    # Data structures
│   │   ├── NKContext/       # Graphics context abstraction
│   │   ├── NKWindow/        # Window/platform layer
│   │   ├── NKLogger/        # Logging system
│   │   └── ...
│   ├── NKGlad/              # OpenGL function loader
│   └── NKMath/              # Math library
└── Build/                   # Generated build artifacts
    ├── Bin/                 # Compiled executables
    ├── Lib/                 # Compiled libraries
    └── Obj/                 # Object files
```

## Key Technologies

| Component | Purpose | Details |
|-----------|---------|---------|
| **Nkentseu** | Core Library | Comprehensive graphics/game framework with multiple graphics APIs |
| **NKWindow** | Window Creation | Cross-platform window management (Windows, Linux, macOS) |
| **NKContext** | Graphics Context | Abstraction layer for OpenGL, DirectX, Metal, Software rendering |
| **NKGlad** | OpenGL Loader | Loads OpenGL function pointers dynamically |
| **Jenga** | Build System | Python-based build generator (similar to CMake/Premake) |
| **C++20** | Language | Modern C++ with latest features |
| **Clang/MinGW** | Compiler | Clang C++ compiler with MinGW on Windows |

## Build Flow

```
Your Source Code (main.cpp, etc.)
         ↓
    Jenga Build System
    (trial.wks.jenga)
         ↓
    C++ Compiler (Clang/MinGW)
         ↓
    Links with Nkentseu Libraries
         ↓
    Executable (MyOpenGLApp)
         ↓
    Runtime: Loads OpenGL with GLAD
         ↓
    Displays Window with OpenGL Context
```

## Nkentseu Library Structure

Nkentseu is a **comprehensive graphics framework** that abstracts graphics APIs. Your project specifically uses:

### Graphics Stack (NKContext)
- **NKContext**: Abstracts window + graphics context creation
- **NKWindow**: Platform-specific window management
- **NKGlad**: OpenGL function pointer loader
- **GL**: Direct OpenGL calls (once loaded)

### Supporting Systems
- **NKContainers**: STL-like data structures (Vector, Map, etc.)
- **NKLogger**: Logging/debugging output
- **NKEvent**: Event system (window close, input, etc.)
- **NKTime**: Time/delta time management
- **NKThreading**: Multi-threading utilities

## Graphics API Support in Nkentseu

Nkentseu supports **multiple graphics backends**:
- **OpenGL** (your current choice)
- **DirectX 11/12** (Windows only)
- **Metal** (macOS only)
- **Software Rendering** (CPU fallback)

Your `main.cpp` specifically configures **OpenGL 4.6 Core Profile** with:
- Debug callbacks enabled
- 4x MSAA (anti-aliasing)
- sRGB framebuffer support
- Adaptive VSync

## Workflow Summary

1. **Jenga** reads `trial.wks.jenga` and `open.prj.jenga`
2. **Clang/MinGW** compiles your code
3. **Linker** combines your object files with Nkentseu libraries
4. **Executable** runs your code
5. **NKWindow** creates a window
6. **NKContext** initializes an OpenGL 4.6 context
7. **GLAD** loads all OpenGL function pointers
8. **Your code** renders using OpenGL calls
9. **Present** swaps buffers to display on screen

---

**Next**: Read `02_MAIN_CPP_LINE_BY_LINE.md` for detailed explanation of every line in main.cpp.
