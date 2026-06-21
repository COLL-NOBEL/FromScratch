# OpenPrj Detailed Explanation

This document explains the OpenGL project under `OpenPrj/` in detail: architecture, important code sections, libraries used, and build/run instructions from scratch.

---

## 1) What this OpenGL project does

`OpenPrj` is a compact OpenGL 4.6 Core demo scaffold that:

- creates a cross-platform window via Nkentseu,
- creates an OpenGL context,
- loads OpenGL function pointers with GLAD,
- compiles shaders from files,
- renders a rotating colored triangle,
- and keeps rendering inside a robust update loop.

It is intentionally minimal so it can be used as a learning base.

---

## 2) File map

Core files:

- `OpenPrj/src/main.cpp`
- `OpenPrj/src/engine/Renderer.h`
- `OpenPrj/src/engine/Renderer.cpp`
- `OpenPrj/src/engine/ShaderProgram.h`
- `OpenPrj/src/engine/ShaderProgram.cpp`
- `OpenPrj/src/engine/SimpleMesh.h`
- `OpenPrj/src/engine/SimpleMesh.cpp`
- `OpenPrj/shaders/basic.vert`
- `OpenPrj/shaders/basic.frag`
- `OpenPrj/open.prj.jenga`

---

## 3) Libraries and modules used

## Nkentseu framework pieces

- `NKWindow`: window creation and event handling.
- `NKContext`: graphics-context abstraction and lifecycle.
- `NKLogger`: runtime logs.
- `NKMath`: vectors, matrices, transforms.

## OpenGL stack

- `GLAD` provides runtime function loading (`gladLoadGL`).
- OpenGL Core profile 4.6 is requested in context setup.

## OS/platform layer

- Windows links system graphics/runtime libs (e.g., `opengl32`, `user32`, etc.).
- Linux/macOS paths are handled by Jenga filters.

---

## 4) Step-by-step execution flow

1. Configure and create `NkWindow`.
2. Build `NkContextDesc` for OpenGL:
   - version 4.6,
   - core profile,
   - debug + forward-compatible flags,
   - vsync options, sRGB framebuffer, MSAA options.
3. Create graphics context with `NkContextFactory::Create`.
4. Call `LoadGL(...)` to initialize GLAD function pointers.
5. Initialize `graphics::Renderer`.
6. Enter event/render loop:
   - poll close events,
   - `BeginFrame` / render / `EndFrame` / `Present`.
7. Shutdown renderer and context.

---

## 5) Detailed code explanations

## `OpenPrj/src/main.cpp`

### Window + context creation

- Creates a 1280x720 resizable window.
- Requests OpenGL 4.6 Core using NkContext configuration.
- Enables optional runtime features:
  - debug callback,
  - adaptive vsync,
  - sRGB framebuffer,
  - automatic entry point loading checks.

### `LoadGL(...)`

- Retrieves OpenGL function loader from native context data (`NkOpenGLContextData`).
- Calls `gladLoadGL(...)`.
- Logs OpenGL/GLSL versions.
- Fails fast if loader/context invalid.

### Main render loop

- Polls events and exits cleanly on `NkWindowCloseEvent`.
- Calls renderer every frame with elapsed seconds from app start.
- Keeps window/context lifecycle explicit and deterministic.

## `OpenPrj/src/engine/Renderer.*`

`Renderer` owns high-level draw operations.

### `Init(...)`

- Stores viewport dimensions.
- Loads shaders from:
  - `OpenPrj/shaders/basic.vert`
  - `OpenPrj/shaders/basic.frag`
- Creates triangle mesh data.
- Enables depth testing.
- Sets clear color.

### `Render(float elapsedSeconds)`

- Sets viewport and clears color/depth buffers.
- Binds shader program.
- Builds transformation matrices:
  - model: rotating around Y,
  - view: camera using `LookAt`,
  - projection: perspective matrix.
- Calculates `MVP = projection * view * model`.
- Sends `uMVP` uniform.
- Draws the mesh.

### `Resize(...)`

- clamps invalid sizes to at least 1 to avoid divide-by-zero.

## `OpenPrj/src/engine/ShaderProgram.*`

This module wraps shader and program lifecycle.

### Major behavior

- reads shader text from file with fallback search prefixes,
- compiles vertex/fragment shader separately,
- logs compile/link errors,
- links a program,
- exposes `Bind()` and `FindUniform(...)`.

### Why this wrapper matters

- isolates raw OpenGL shader calls,
- centralizes error diagnostics,
- keeps `Renderer` focused on scene logic.

## `OpenPrj/src/engine/SimpleMesh.*`

This module builds and draws one triangle.

### `CreateTriangle()`

- creates VAO and VBO,
- uploads interleaved vertex data (position + color),
- configures vertex attributes:
  - location 0: position (vec3)
  - location 1: color (vec3)

### `Draw()`

- binds VAO and issues `glDrawArrays(GL_TRIANGLES, 0, 3)`.

### `Shutdown()`

- deletes GL buffer/array objects.

---

## 6) Shader pipeline summary

### Vertex shader (`basic.vert`)

- reads position/color attributes,
- multiplies position by `uMVP`,
- forwards color to fragment stage.

### Fragment shader (`basic.frag`)

- outputs interpolated color.

This is the minimum GPU pipeline for colored geometry.

---

## 7) Build from scratch

## Prerequisites

- Python 3.x
- Jenga build system
- C++ toolchain
- OpenGL-capable GPU/driver

Install Jenga:

```bash
python -m pip install --upgrade jenga-build
```

## Clone and enter repo

```bash
git clone https://github.com/COLL-NOBEL/FromScratch.git
cd FromScratch
```

## Generate build files

```bash
jenga generate --config=Debug
```

## Build OpenGL target

```bash
jenga build --project=MyOpenGLApp --config=Debug
```

(If your project name differs in your workspace file, use that project name.)

## Run binary

Find executable in:

- `Build/Bin/Debug-<platform>/MyOpenGLApp/`

---

## 8) How to extend this project safely

Recommended small next steps:

1. Add index buffers and draw multiple meshes.
2. Add uniform blocks for camera/material values.
3. Add texture sampling and UV attributes.
4. Add keyboard/mouse camera controls.
5. Split scene update/render for cleaner scaling.

---

## 9) Quick conceptual recap

- `main.cpp` handles app lifecycle.
- `Renderer` handles scene draw logic.
- `ShaderProgram` handles GPU program compile/link.
- `SimpleMesh` handles geometry buffer setup.
- shaders define GPU transformation + color output.

This separation is a good starter architecture for an OpenGL engine.
