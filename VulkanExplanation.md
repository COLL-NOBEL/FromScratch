# VulkanPrj Detailed Explanation

This document explains how the Vulkan project is structured, what each major source file does, which libraries are involved, and how to build/run it from scratch.

---

## 1) What this project is

`VulkanPrj` is a small Vulkan rendering engine scaffold that:

- opens a native window through the Nkentseu framework,
- initializes a Vulkan instance + swapchain,
- builds a graphics pipeline for indexed cube rendering,
- renders two animated cubes every frame,
- applies a camera view/projection matrix,
- and keeps cube animation bounded so the cubes remain visible in camera view.

This is intentionally lightweight so it stays easy to extend.

---

## 2) Project layout

Main project files:

- `VulkanPrj/src/main.cpp`
- `VulkanPrj/src/engine/VulkanEngine.h`
- `VulkanPrj/src/engine/VulkanEngine.cpp`
- `VulkanPrj/src/engine/VulkanContext.h`
- `VulkanPrj/src/engine/VulkanContext.cpp`
- `VulkanPrj/src/engine/VulkanRenderer.h`
- `VulkanPrj/src/engine/VulkanRenderer.cpp`
- `VulkanPrj/src/engine/VulkanInstance.h`
- `VulkanPrj/src/engine/VulkanInstance.cpp`
- `VulkanPrj/src/engine/VulkanShaderData.h`
- `VulkanPrj/vulkan.prj.jenga`

Math helpers used by this project are in `VulkanPrj/nkMath/`.

---

## 3) Libraries and frameworks used

### Nkentseu runtime modules

Linked modules (from the `.jenga` project config):

- `NKWindow` (window creation + surface abstraction)
- `NKEvent` (event polling)
- `NKContext` (graphics runtime plumbing)
- `NKLogger` (logging)
- `NKTime` (timing)
- `NKMath` (math types/utilities)
- plus core/foundation modules (`NKCore`, `NKMemory`, `NKContainers`, etc.)

### Vulkan

- Vulkan instance/device/swapchain management is done with the Vulkan C API.
- Project links `vulkan-1` on Windows and `vulkan` on Linux/macOS in `vulkan.prj.jenga`.

---

## 4) Runtime flow (high-level)

1. `main.cpp` creates a window.
2. `VulkanEngine::Initialize` builds a `VulkanContext`.
3. `VulkanContext`:
   - creates Vulkan instance,
   - configures camera,
   - initializes renderer resources.
4. Main loop:
   - polls events,
   - computes frame delta,
   - calls `engine.Tick(deltaSeconds)`.
5. `Tick`:
   - updates camera motion,
   - renders one frame with current scene time.
6. On exit, all Vulkan resources are destroyed in reverse order.

---

## 5) Source-by-source explanation

## `VulkanPrj/src/main.cpp`

Responsibilities:

- configure and open the window,
- initialize engine,
- run frame loop with ~60 FPS pacing,
- forward per-frame `deltaSeconds` into the engine,
- clean shutdown.

Important points:

- `NkMathUtils::clamp` limits `deltaSeconds` to avoid very large dt spikes.
- event loop handles close events.
- sleeping/yielding caps frame pacing.

## `VulkanPrj/src/engine/VulkanEngine.*`

This is a slim orchestrator around `VulkanContext`:

- `Initialize` sets up context.
- `Tick` accumulates uptime and calls:
  - `mContext.Update(deltaSeconds)`
  - `mContext.RenderFrame(mUptimeSeconds)`
- `Shutdown` tears down context.

The engine keeps higher-level app timing separate from low-level renderer setup.

## `VulkanPrj/src/engine/VulkanContext.*`

`VulkanContext` owns:

- platform-agnostic Vulkan instance setup (`VulkanInstance`),
- camera state (position, forward/up vectors, view/projection matrices),
- renderer object (`VulkanRenderer`).

### Camera behavior

Current behavior keeps camera motion bounded:

- base yaw/pitch remains pointed at scene center,
- small sinusoidal sway is applied on yaw/pitch,
- sway is clamped to safe pitch limits,
- this prevents camera drift away from the cubes.

This keeps motion alive without losing the scene.

## `VulkanPrj/src/engine/VulkanRenderer.*`

This file contains most Vulkan work:

- surface creation from `NkSurfaceDesc`,
- physical device selection,
- logical device + queues,
- swapchain + image views,
- render pass + pipeline layout + graphics pipeline,
- depth image,
- framebuffers,
- cube vertex/index buffers,
- command buffers + semaphores + fences,
- per-frame draw/submit/present path.

### Draw path

`RecordCommandBuffer(...)`:

- begins render pass,
- binds viewport/scissor/pipeline,
- binds cube VBO/IBO,
- loops over 2 cube instances,
- computes each cube model matrix,
- computes `MVP = projection * view * model`,
- pushes MVP as push constants,
- issues indexed draw call.

### Bounded cube motion

`BuildCubeModelMatrix(sceneTimeSeconds, instanceIndex)` now combines:

- sinusoidal translation in X/Y/Z (bounded amplitudes),
- rotation on Y and X,
- per-instance phase/speed differences,
- optional per-instance scale.

Because translations are bounded around the center volume, cubes keep moving but stay inside camera-visible range.

## `VulkanPrj/src/engine/VulkanShaderData.h`

Contains embedded SPIR-V arrays for the cube shaders.

Pros:

- no runtime shader file lookup required,
- easy single-binary deployment.

Tradeoff:

- shader sources are less editable at runtime unless regeneration tooling is added.

---

## 6) Mathematical pipeline used for rendering

For each cube instance:

1. Build model matrix from translation/rotation/scale.
2. Multiply with camera view matrix.
3. Multiply with projection matrix.
4. Upload final matrix through push constants.
5. Vertex shader transforms object-space vertices into clip space.

Formula:

`clipPos = Projection * View * Model * localVertex`

---

## 7) Build from scratch

## Prerequisites

- Python 3.x
- Jenga build tool
- C++ toolchain (Clang/MSVC/GCC depending on platform)
- Vulkan SDK installed and available

On Windows, the project expects Vulkan SDK paths consistent with:

- include: `C:\VulkanSDK\1.4.341.0\Include`
- libs: `C:\VulkanSDK\1.4.341.0\Lib`

If your SDK is elsewhere, update `VulkanPrj/vulkan.prj.jenga`.

## Setup

```bash
git clone https://github.com/COLL-NOBEL/FromScratch.git
cd FromScratch
python -m pip install --upgrade jenga-build
```

## Generate build files

```bash
jenga generate --config=Debug
```

## Build Vulkan target

```bash
jenga build --project=VKGraphicsEngine --config=Debug
```

## Run

The executable is generated under:

- `Build/Bin/Debug-<platform>/VKGraphicsEngine/`

Run the binary for your platform from that folder.

---

## 8) Extension ideas

Good next incremental upgrades:

- add camera input controls (WASD + mouse look),
- move per-object transform data into uniform/storage buffers,
- add more meshes and materials,
- add descriptor sets and textures,
- move embedded SPIR-V to shader build pipeline if desired.

---

## 9) Summary

This Vulkan project is a clean baseline:

- window + Vulkan setup is already complete,
- renderer can draw indexed 3D geometry with depth,
- cube animation is bounded and remains visible,
- architecture is split into clear layers (`Engine` / `Context` / `Renderer`).

Use this as a safe base to grow toward a fuller engine.
