# Issue #1 — Basic OpenGL Graphics-Engine Scaffold

## What was added and why

I added a **minimal, educational graphics-engine scaffold** around the existing Nkentseu window/context startup.  
The goal is to separate the app into clear responsibilities so future features (camera controls, more meshes, materials, scene graph, etc.) can be added incrementally.

The scaffold is intentionally simple:

- **Shader program management** is isolated in one class.
- **Geometry/mesh ownership** is isolated in one class.
- **Renderer lifecycle** (`Init`, `Render`, `Shutdown`) is isolated in one class.
- `main.cpp` remains responsible for **platform lifecycle and event loop**, then delegates drawing to the renderer.

This gives a clean “first engine shape” without introducing heavy abstractions.

---

## File-by-file walkthrough

### `OpenPrj/src/engine/ShaderProgram.h`
Declares a tiny shader-program utility:

- loads vertex + fragment shader source from files,
- compiles and links the OpenGL program,
- binds the program,
- resolves uniform locations,
- cleans up GPU program state.

### `OpenPrj/src/engine/ShaderProgram.cpp`
Implements shader loading/compilation/linking.

Notable behavior:

- uses fallback path prefixes (`./`, `../`, `../../`, …) so shader loading still works from different launch folders,
- prints compile/link diagnostics to stderr,
- safely tears down program objects on failure.

### `OpenPrj/src/engine/SimpleMesh.h`
Declares a minimal mesh class for one colored triangle.

### `OpenPrj/src/engine/SimpleMesh.cpp`
Implements VAO/VBO lifecycle for the triangle:

- uploads interleaved position/color vertex data,
- configures vertex attributes,
- draws with `glDrawArrays`,
- releases GL buffers/arrays on shutdown.

### `OpenPrj/src/engine/Renderer.h`
Declares renderer lifecycle:

- `Init(width, height)`
- `Resize(width, height)`
- `Render(elapsedSeconds)`
- `Shutdown()`

### `OpenPrj/src/engine/Renderer.cpp`
Implements the render pipeline setup and per-frame draw.

Important math details:

- Uses **NKMath** from Nkentseu includes (`#include "NKMath/NKMath.h"`, namespace `nkentseu::math`).
- Builds matrices with NKMath types/functions:
  - `NkMat4f::RotationY(...)` for model animation,
  - `NkMat4f::LookAt(...)` for camera view,
  - `NkMat4f::Perspective(...)` for projection.
- Sends combined `uMVP` matrix to the shader.

### `OpenPrj/src/compat/msvcrt_compat.cpp`
Adds a tiny Windows CRT compatibility shim (`stat64i32`) needed to satisfy
one symbol expected by the prebuilt `nkentseu` binary libs during MinGW/LLVM
cross-linking in this environment.

### `OpenPrj/shaders/basic.vert`
Basic vertex shader:

- receives position + color,
- applies `uMVP`,
- passes color to fragment shader.

### `OpenPrj/shaders/basic.frag`
Basic fragment shader:

- outputs interpolated color.

### `OpenPrj/src/main.cpp`
Entry point remains responsible for:

- creating window,
- creating OpenGL context,
- loading GL function pointers,
- polling Nkentseu events.

It now also:

- creates and initializes `graphics::Renderer`,
- forwards window size each frame,
- forwards elapsed time to animate rotation,
- calls renderer for frame rendering,
- shuts renderer down before context shutdown.

### `OpenPrj/open.prj.jenga`
Project build config updates:

- binds the app to the workspace `mingw` toolchain on Windows,
- adds explicit Windows link options (`--target=x86_64-w64-windows-gnu`, `-fuse-ld=lld`) so linking uses the correct target mode.

### `trial.wks.jenga`
Workspace build config updates:

- enables `RegisterJengaGlobalToolchains()`,
- defines a Windows `mingw` toolchain using clang cross-target settings,
- keeps the original include/link setup for app and external libs.

### `Explanation.md`
This document, added for issue #1, to explain architecture and next steps.

---

## Step-by-step: understand, run, and extend

## 1) Understand the flow

1. Start from `OpenPrj/src/main.cpp` and locate the `graphics::Renderer` lifecycle calls.
2. Open `Renderer.cpp` to see the high-level render sequence.
3. Open `ShaderProgram.cpp` to understand shader file loading + GL program creation.
4. Open `SimpleMesh.cpp` to understand GPU mesh setup and draw call.
5. Open `OpenPrj/shaders/basic.vert` / `basic.frag` to connect CPU-side uniforms/attributes to shader logic.

## 2) Build and run

From repo root:

```bash
JENGA_DISABLE_CCACHE=1 jenga build --config Debug --platform Windows --no-cache
```

Then run the produced executable from the build output folder used by your platform/config.

Expected result: a rotating colored triangle on a dark background.

## 3) Extend the scaffold safely

Recommended incremental steps:

1. **Add another mesh class** (e.g., indexed quad) under `src/engine/`.
2. **Add another shader pair** in `OpenPrj/shaders/` and switch between programs.
3. **Add camera controls** by changing `eye/center/up` and/or adding a camera struct.
4. **Split scene data from renderer** (renderer consumes immutable draw data).
5. **Add a tiny material struct** for color/texture uniforms.

Keep each step small and compilable; this scaffold is intended as a learning-first base.
