# FromScratch Lesson 03 — OpenPrj Detailed Walkthrough

This lesson covers every requested OpenPrj source/shader file with tight line-range commentary so you can rebuild the project from scratch.

---

## Files covered

- `OpenPrj/src/main.cpp`
- `OpenPrj/src/compat/msvcrt_compat.cpp`
- `OpenPrj/src/engine/Renderer.h`
- `OpenPrj/src/engine/Renderer.cpp`
- `OpenPrj/src/engine/ShaderProgram.h`
- `OpenPrj/src/engine/ShaderProgram.cpp`
- `OpenPrj/src/engine/SimpleMesh.h`
- `OpenPrj/src/engine/SimpleMesh.cpp`
- `OpenPrj/shaders/basic.vert`
- `OpenPrj/shaders/basic.frag`

---

## 1) `OpenPrj/src/main.cpp`

## Role

Application bootstrap for the OpenGL path:

- window creation,
- OpenGL context creation via Nkentseu,
- GL entrypoint loading through GLAD,
- event/render loop,
- shutdown.

## Line-range walkthrough

- **L1–3**: includes core Nkentseu window/main/logging headers.
- **L5–8**: includes GLAD OpenGL/WGL headers (WGL only on Windows).
- **L10–12**: undefines `Bool` macro collision on Win32.
- **L14–18**: includes NkContext abstractions and OpenGL-native context data bridge.
- **L20**: includes project renderer class.
- **L22**: includes `<chrono>` for elapsed time.
- **L24**: brings `nkentseu` names into scope.

### GL loader helper (`LoadGL`)

- **L27–31**: validates context pointer and confirms API is OpenGL.
- **L33–35**: accesses native OpenGL loader callback from `NkOpenGLContextData`.
- **L35–38**: guards against missing function loader.
- **L40**: calls `gladLoadGL` using framework-provided loader.
- **L42–45**: failure path if GLAD cannot load function pointers.
- **L47–49**: logs runtime OpenGL/GLSL versions.

### App entrypoint (`nkmain`)

- **L52**: framework entrypoint.
- **L53–60**: sets window config (title/size/centered/resizable).
- **L61–65**: creates window with failure return.

### OpenGL context description

- **L68–79**: fills `NkContextDesc`:
  - API = OpenGL,
  - version 4.6,
  - core profile,
  - forward-compatible + debug flags,
  - runtime debug callback,
  - MSAA and sRGB options,
  - adaptive vsync,
  - runtime validation/auto-load toggles.

### Context and renderer initialization

- **L82–87**: creates graphics context from factory.
- **L90–94**: loads GL functions; shutdown/close on failure.
- **L97–99**: creates renderer + reads initial window size.
- **L100–105**: initializes renderer, with cleanup on failure.

### Main loop

- **L108–110**: loop state + event source + app start timestamp.
- **L112–118**: event polling; close-event exits loop.
- **L120–122**: early break when closed.
- **L124**: begins frame through graphics context.
- **L125–126**: updates renderer viewport from current window size.
- **L128–130**: computes elapsed seconds and calls `renderer.Render`.
- **L132–133**: ends frame and presents swapchain/backbuffer.

### Shutdown

- **L137–140**: renderer shutdown, context shutdown, window close.
- **L142**: returns success.

---

## 2) `OpenPrj/src/compat/msvcrt_compat.cpp`

## Role

Windows-only compatibility shim: defines `stat64i32` and forwards to `_stat64`.

## Line-range walkthrough

- **L1–2**: includes C runtime stat and memory headers.
- **L4**: file is compiled only under `_WIN32`.
- **L5**: C linkage export for `stat64i32`.
- **L6–8**: null output check.
- **L10–12**: performs `_stat64` call.
- **L12–15**: failure path zeroes output struct and returns `-1`.
- **L17–27**: maps fields from `_stat64` to `_stat64i32` output.
- **L29**: returns original `_stat64` result.

---

## 3) `OpenPrj/src/engine/Renderer.h`

## Role

Declares the OpenGL renderer façade owning shader+mesh and viewport state.

## Line-range walkthrough

- **L1–4**: includes dependent engine components (`ShaderProgram`, `SimpleMesh`).
- **L6**: `graphics` namespace start.
- **L8–13**: public renderer API (`Init`, `Resize`, `Render`, `Shutdown`).
- **L15–17**: cached viewport dimensions with nonzero defaults.
- **L19–20**: owned resources:
  - `mShader` program wrapper,
  - `mMesh` geometry wrapper.
- **L23**: namespace close.

---

## 4) `OpenPrj/src/engine/Renderer.cpp`

## Role

Implements render path:

- set viewport and clear,
- compute MVP matrix,
- upload uniform,
- draw triangle mesh.

## Line-range walkthrough

- **L1**: includes own header.
- **L3**: imports nkMath convenience header.
- **L5**: namespace start.

### Initialization

- **L7–8**: `Init` calls `Resize` first to sanitize dimensions.
- **L10–12**: loads shader files (`basic.vert`, `basic.frag`).
- **L14–17**: creates mesh; if fail, releases shader and returns false.
- **L19**: enables depth testing.
- **L20**: sets dark clear color.
- **L22**: success.

### Resize handling

- **L25–28**: stores viewport dimensions but clamps each to at least 1.

### Render pass

- **L30–32**: sets viewport and clears color+depth buffers.
- **L34**: binds shader program.
- **L36**: imports `nkentseu::math` symbols for compact calls.
- **L38**: computes aspect ratio from viewport dimensions.
- **L40–42**: defines camera vectors `eye/center/up`.
- **L44**: animated model rotation around Y axis (degrees via `NkAngle`).
- **L45**: view matrix with `LookAt`.
- **L46**: projection matrix with perspective FOV/near/far.
- **L47**: MVP composition (`projection * view * model`).
- **L49**: queries uniform location `uMVP`.
- **L50–52**: uploads matrix if uniform exists.
- **L54**: draws mesh.

### Shutdown

- **L57–60**: shutdown order mesh then shader.
- **L62**: namespace close.

### nkMath usage in this file

- `NkVec3f`, `NkMat4f`, `NkAngle`, `RotationY`, `LookAt`, `Perspective`, matrix multiplication.
- Only API usage; no math internals are required.

---

## 5) `OpenPrj/src/engine/ShaderProgram.h`

## Role

Declares file-loading, shader compile/link, program bind, uniform lookup, and cleanup.

## Line-range walkthrough

- **L1–5**: includes `string` and OpenGL types/functions (`glad/gl.h`).
- **L7**: namespace start.
- **L9–16**: public API:
  - `LoadFromFiles(...)`,
  - `Bind()`,
  - `FindUniform(...)`,
  - `Shutdown()`.
- **L19–27**: private helpers:
  - `ReadTextWithFallbacks(...)`,
  - `Compile(...)`,
  - `Link(...)`.
- **L30**: `mProgram` OpenGL object handle.
- **L33**: namespace close.

---

## 6) `OpenPrj/src/engine/ShaderProgram.cpp`

## Role

Owns shader source loading, compile diagnostics, program linking diagnostics, and runtime bind/uniform operations.

## Line-range walkthrough

- **L1**: include own header.
- **L3–6**: standard C++ file/string/stream/array helpers.
- **L8**: namespace start.

### Text-loading helper

- **L10–12**: signature outputs shader text and resolved path.
- **L13–22**: defines fallback prefixes from current directory up to many parent levels.
- **L24–29**: attempts each prefixed path; skips missing files.
- **L31–36**: reads full file into string and returns resolved candidate path.
- **L39**: returns false if no candidate exists.

### Shader compilation helper

- **L42–44**: compile function signature (type, source, debug label).
- **L45–49**: creates shader object; logs and fails on zero handle.
- **L51–53**: sets shader source and compiles.
- **L55–57**: fetches compile status.
- **L58–60**: success returns shader handle.
- **L62–68**: reads compile log if available.
- **L70–73**: prints failure diagnostics.
- **L75–76**: deletes failed shader object and returns 0.

### Program link helper

- **L79–84**: creates program object and validates handle.
- **L86–88**: attaches vertex+fragment and links.
- **L90–92**: fetches link status.
- **L93–95**: success path returns true.
- **L97–103**: reads link log if available.
- **L105–108**: prints link diagnostics.
- **L110–112**: deletes failed program and resets handle.

### High-level load pipeline

- **L115–117**: `LoadFromFiles` begins by `Shutdown()` to avoid leaks.
- **L119–123**: source/resolved-path storage.
- **L124–127**: load vertex source or fail.
- **L129–132**: load fragment source or fail.
- **L134–137**: compile vertex shader.
- **L139–143**: compile fragment shader; cleanup vertex on failure.
- **L145**: link both shaders.
- **L147–148**: delete temporary shader objects after linking attempt.
- **L150**: return link status.

### Runtime and cleanup

- **L153–155**: `Bind` -> `glUseProgram(mProgram)`.
- **L157–162**: `FindUniform`; guards invalid program/null name.
- **L164–168**: `Shutdown` deletes GL program and zeros handle.
- **L171**: namespace close.

---

## 7) `OpenPrj/src/engine/SimpleMesh.h`

## Role

Declares minimal mesh object for one triangle (VAO + VBO).

## Line-range walkthrough

- **L1–3**: include guard + OpenGL header.
- **L5**: namespace start.
- **L7–11**: public API (`CreateTriangle`, `Draw`, `Shutdown`).
- **L14–15**: GPU object handles (`mVao`, `mVbo`).
- **L18**: namespace close.

---

## 8) `OpenPrj/src/engine/SimpleMesh.cpp`

## Role

Creates and renders an interleaved position+color triangle.

## Line-range walkthrough

- **L1**: include own header.
- **L3**: namespace start.

### Triangle creation

- **L5–6**: `CreateTriangle` starts by `Shutdown` to avoid stale handles.
- **L8–13**: hard-coded interleaved vertex array (xyz + rgb for 3 vertices).
- **L15–16**: allocates VAO and VBO.
- **L18–21**: fail-safe if allocation did not succeed.
- **L23**: binds VAO.
- **L25–26**: binds VBO and uploads static vertex data.
- **L28–36**: enables attribute 0 (position), 3 floats, stride=6 floats, offset=0.
- **L38–46**: enables attribute 1 (color), 3 floats, same stride, offset=3 floats.
- **L48–49**: unbinds VBO then VAO.
- **L51**: success.

### Draw and cleanup

- **L54–57**: `Draw` early-returns if VAO is invalid.
- **L59–61**: binds VAO, issues `glDrawArrays(GL_TRIANGLES, 0, 3)`, unbinds.
- **L64–68**: deletes VBO if present.
- **L70–73**: deletes VAO if present.
- **L76**: namespace close.

---

## 9) `OpenPrj/shaders/basic.vert`

## Role

Vertex shader that forwards per-vertex color and applies model-view-projection transform.

## Line-range walkthrough

- **L1**: GLSL version directive (`330 core`).
- **L3**: input location 0 -> `aPosition` (vec3).
- **L4**: input location 1 -> `aColor` (vec3).
- **L6**: uniform matrix `uMVP`.
- **L8**: output varying `vColor` to fragment stage.
- **L10–12**:
  - writes `vColor = aColor`,
  - computes clip-space position: `gl_Position = uMVP * vec4(aPosition, 1.0)`.

### Why this matters

It separates vertex transform logic (vertex stage) from color output (fragment stage).

---

## 10) `OpenPrj/shaders/basic.frag`

## Role

Fragment shader that emits interpolated RGB color with full opacity.

## Line-range walkthrough

- **L1**: GLSL version directive (`330 core`).
- **L3**: receives interpolated `vColor` from vertex stage.
- **L4**: fragment output variable `FragColor`.
- **L6–7**: outputs `vec4(vColor, 1.0)`.

### Why this matters

The GPU interpolates vertex colors per pixel; this fragment shader simply writes that result.

---

## 11) Rebuild-from-scratch order for OpenPrj

1. Implement `ShaderProgram` (`.h/.cpp`) first (file IO + compile/link + bind/uniform).
2. Implement `SimpleMesh` (`.h/.cpp`) second (VAO/VBO + draw).
3. Implement `Renderer` (`.h/.cpp`) third (init/resize/render/shutdown).
4. Add GLSL files (`basic.vert`, `basic.frag`).
5. Implement `main.cpp` last (window/context/event loop + renderer usage).
6. Keep `msvcrt_compat.cpp` for Windows compatibility.

---

## 12) OpenPrj shader + runtime integration summary

- CPU side computes `mvp` matrix in `Renderer.cpp`.
- CPU uploads to uniform `uMVP`.
- Vertex shader transforms positions and forwards colors.
- Fragment shader outputs final color.
- `SimpleMesh` supplies the vertex attributes matching shader locations:
  - location 0 = position,
  - location 1 = color.
