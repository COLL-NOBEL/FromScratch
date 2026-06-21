# VulkanPrj: Architecture, Render Flow, and Build/Run Guide

This document describes the Vulkan project under `VulkanPrj/` as currently implemented.

Current runtime behavior:
- opens a window,
- initializes a Vulkan renderer,
- draws one cube,
- keeps the cube **green**,
- animates the cube with continuous rotation and a **2D circular orbit** in the XY plane (equal X/Y radius).

---

## 1) Project architecture at a glance

`VulkanPrj` is organized as a small layered runtime:

1. `src/main.cpp`
   - Creates the window and drives the application loop.
2. `src/engine/VulkanEngine.*`
   - Thin lifecycle wrapper (initialize / tick / shutdown).
3. `src/engine/VulkanContext.*`
   - Owns camera state and coordinates instance + renderer.
4. `src/engine/VulkanInstance.*`
   - Creates and destroys `VkInstance` plus optional validation setup.
5. `src/engine/VulkanRenderer.*`
   - Owns all device/swapchain/pipeline/resources and performs frame rendering.
6. `src/engine/VulkanShaderData.h`
   - Embedded SPIR-V bytecode used by pipeline creation.
7. `nkMath/*`
   - Local math types/utilities used by camera/model transforms.

The renderer intentionally uses a compact design: one mesh, one pipeline, push constants for MVP, and no descriptor sets.

---

## 2) All VulkanPrj code files and responsibilities

### Build definition

| File | Responsibility |
| --- | --- |
| `VulkanPrj/vulkan.prj.jenga` | Jenga project definition for target `VKGraphicsEngine` (sources, include dirs, link libs, platform-specific link settings). |

### Application + engine files

| File | Responsibility |
| --- | --- |
| `VulkanPrj/src/main.cpp` | Entry point (`nkmain`): creates window, polls events, computes frame delta, calls `engine.Tick(deltaSeconds)`, throttles frame loop. |
| `VulkanPrj/src/engine/VulkanEngine.h` | Declares high-level engine API and stores `VulkanContext` + uptime. |
| `VulkanPrj/src/engine/VulkanEngine.cpp` | Implements initialize/tick/shutdown flow and forwards update/render to context. |
| `VulkanPrj/src/engine/VulkanContext.h` | Declares camera state (`viewMatrix`, `projectionMatrix`) and context API. |
| `VulkanPrj/src/engine/VulkanContext.cpp` | Builds camera matrices, creates instance + renderer, updates projection on resize, dispatches frame render calls. |
| `VulkanPrj/src/engine/VulkanCommon.h` | Sets platform Vulkan surface macros (`VK_USE_PLATFORM_*`) and includes Vulkan headers. |
| `VulkanPrj/src/engine/VulkanInstance.h` | Declares `VulkanInstance` wrapper and config. |
| `VulkanPrj/src/engine/VulkanInstance.cpp` | Creates/destroys `VkInstance`, checks instance layers/extensions, enables validation/debug utils when available. |
| `VulkanPrj/src/engine/VulkanRenderer.h` | Declares renderer state, Vulkan resource creation methods, render path, and cube model-matrix builder. |
| `VulkanPrj/src/engine/VulkanRenderer.cpp` | Full Vulkan rendering implementation: surface/device/swapchain/pipeline/depth/mesh/commands/synchronization, per-frame record+submit+present, cube animation transform. |
| `VulkanPrj/src/engine/VulkanShaderData.h` | Embedded vertex/fragment SPIR-V arrays consumed by `CreateGraphicsPipeline()`. |

### Math files in `VulkanPrj/nkMath`

| File | Responsibility |
| --- | --- |
| `VulkanPrj/nkMath/NkColor.h` | Color struct (`RGBA` bytes) and conversion helpers. |
| `VulkanPrj/nkMath/NkMat3x3.h` | Template 3×3 matrix type with arithmetic, determinant/inverse, transforms. |
| `VulkanPrj/nkMath/NkMat4x4.h` | Template 4×4 matrix type with arithmetic and 3D transform helpers (translation/rotation/scale/projection helpers). Core matrix type used by Vulkan camera/model math. |
| `VulkanPrj/nkMath/NkMath.cpp` | Aggregates/compiles local math headers into one translation unit. |
| `VulkanPrj/nkMath/NkMathUtils.h` | Scalar utility functions/constants (`clamp`, `deg2rad`, etc.). |
| `VulkanPrj/nkMath/NkQuat.h` | Legacy quaternion implementation snapshot; content is currently commented out. |
| `VulkanPrj/nkMath/NkRandom.h` | Legacy random helper snapshot; content is currently commented out. |
| `VulkanPrj/nkMath/NkRectangle.h` | Rectangle type declarations and generic rect template aliases. |
| `VulkanPrj/nkMath/NkTransform3D.h` | Alternative/static 3D transform helper template content; not part of active Vulkan render path. |
| `VulkanPrj/nkMath/NkVec2.h` | Template 2D vector type + interpolation helpers. |
| `VulkanPrj/nkMath/NkVec3.h` | Template 3D vector type + dot/cross/interpolation helpers. |
| `VulkanPrj/nkMath/NkVec4.h` | Template 4D vector type + interpolation helpers. |
| `VulkanPrj/nkMath/Quaternion.h` | Minimal quaternion POD-style container (x/y/z/w). |

---

## 3) Render flow

## 3.1 Startup flow

1. `main.cpp`
   - Creates `NkWindow` and `VulkanEngine`.
2. `VulkanEngine::Initialize`
   - Builds `VulkanContextCreateInfo`, calls `VulkanContext::Initialize`.
3. `VulkanContext::Initialize`
   - Creates `VkInstance` via `VulkanInstance`.
   - Initializes camera pose and computes initial `viewMatrix`/`projectionMatrix`.
   - Initializes `VulkanRenderer` with instance, surface description, and window size.
4. `VulkanRenderer::Initialize`
   - Creates Vulkan surface.
   - Picks physical device and queue families.
   - Creates logical device and command pool.
   - Creates pipeline layout + cube vertex/index buffers.
   - Creates swapchain/image views/render pass/graphics pipeline.
   - Creates depth resources, framebuffers, command buffers, sync objects.

## 3.2 Per-frame flow

1. `main.cpp`
   - Computes `deltaSeconds`, polls events, calls `engine.Tick(deltaSeconds)`.
2. `VulkanEngine::Tick`
   - Accumulates `mUptimeSeconds`, calls context `Update` then `RenderFrame`.
3. `VulkanContext::Update`
   - Recomputes projection matrix when window size changes.
4. `VulkanContext::RenderFrame`
   - Calls `VulkanRenderer::Render(sceneTimeSeconds, view, projection, width, height)`.
5. `VulkanRenderer::Render`
   - Waits for current-frame fence.
   - Acquires swapchain image.
   - Handles out-of-date/suboptimal swapchain cases.
   - Records command buffer for acquired image.
   - Submits graphics queue work:
     - wait semaphore: `mImageAvailableSemaphores[mCurrentFrame]`
     - signal semaphore: `mRenderFinishedSemaphores[imageIndex]`
   - Presents swapchain image waiting on the corresponding render-finished semaphore.
   - Advances `mCurrentFrame` modulo `kMaxFramesInFlight`.

## 3.3 Command buffer contents

`VulkanRenderer::RecordCommandBuffer(...)` performs:
- begin command buffer,
- begin render pass (color + depth clear),
- set viewport/scissor,
- bind graphics pipeline,
- bind cube vertex/index buffers,
- compute `MVP = Projection * View * Model`,
- upload MVP through push constants,
- `vkCmdDrawIndexed` for cube indices,
- end render pass and command buffer.

## 3.4 Cube transform and color

Cube model transform is generated in `VulkanRenderer::BuildCubeModelMatrix(sceneTimeSeconds)`:

- Orbit:
  - `orbitRadians = sceneTimeSeconds * 0.9`
  - `x = cos(orbitRadians) * 0.75`
  - `y = sin(orbitRadians) * 0.75`
  - `z = -0.35`
  - Equal X/Y radii produce an exact 2D circular orbit in the XY plane.
- Rotation:
  - continuous Y-axis rotation (`90°/s`),
  - continuous X-axis rotation (`55°/s`).
- Composition:
  - `Model = Translation * RotationY * RotationX`

Cube color comes from per-vertex color in `kCubeVertices` (all vertices set to `{0, 1, 0}`), so the cube renders green.

---

## 4) Build and run VulkanPrj from scratch

## 4.1 Prerequisites

- Git
- Python 3.8+ (for Jenga)
- C++ toolchain supported by your platform
- Vulkan SDK/runtime (headers + loader + libs)
- Dependencies required by workspace projects (`Externals/libs/nkentseu` is expected by the workspace/project setup)

Install Jenga:

```bash
python3 -m pip install --upgrade jenga-build
jenga --version
```

## 4.2 Clone and enter repository

```bash
git clone https://github.com/COLL-NOBEL/FromScratch.git
cd FromScratch
```

## 4.3 Generate build files

```bash
jenga generate --config=Debug
```

## 4.4 Build Vulkan target

```bash
jenga build --project=VKGraphicsEngine --config=Debug
```

## 4.5 Run the executable

Typical output locations:

- Linux:

```bash
./Build/Bin/Debug-Linux/VKGraphicsEngine/VKGraphicsEngine
```

- Windows:

```bash
Build\\Bin\\Debug-Windows\\VKGraphicsEngine\\VKGraphicsEngine.exe
```

(Exact folder names can vary with toolchain/config/platform.)

## 4.6 Platform notes

- On Linux, if Vulkan dev packages are missing, install at least Vulkan loader headers/libs and X11 development packages.
- `VulkanPrj/vulkan.prj.jenga` currently includes Windows Vulkan SDK include/lib paths under `C:\\VulkanSDK\\1.4.341.0\\...`; if your SDK is elsewhere, update those paths before Windows builds.

---

## 5) Quick troubleshooting

- `vkCreateInstance` failure:
  - verify Vulkan runtime/loader installation.
- Surface creation failure:
  - verify window backend/platform macros and runtime support.
- Linker errors for Vulkan symbols:
  - verify Vulkan SDK include/lib paths (especially on Windows).
- Swapchain recreation loops on resize:
  - ensure valid non-zero window dimensions (renderer already guards against zero-sized windows).
