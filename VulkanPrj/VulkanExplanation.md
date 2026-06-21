# VulkanPrj Explanation (Green Rotating Orbiting Cube)

This document explains the current Vulkan implementation under `VulkanPrj/`.

Current runtime result:

- one **green cube** is rendered,
- it **rotates continuously**, and
- it **moves in a circular 2D path** on screen.

---

## 1) Project structure

- `VulkanPrj/src/main.cpp`
  - App entry point.
  - Creates the window, handles close events, computes `deltaSeconds`, and ticks the Vulkan engine.

- `VulkanPrj/src/engine/VulkanEngine.h/.cpp`
  - Thin coordinator around the Vulkan context.
  - Tracks uptime and runs per-frame `Update` + `RenderFrame`.

- `VulkanPrj/src/engine/VulkanContext.h/.cpp`
  - Bridges app/window state and renderer state.
  - Owns `VulkanInstance`, `VulkanRenderer`, and camera matrices.
  - Rebuilds projection when window size changes.

- `VulkanPrj/src/engine/VulkanInstance.h/.cpp`
  - Creates/destroys `VkInstance`.
  - Resolves required instance extensions and optional validation layer.

- `VulkanPrj/src/engine/VulkanRenderer.h/.cpp`
  - Core Vulkan rendering path.
  - Owns surface/device/swapchain/render pass/pipeline/depth image/buffers/command buffers/sync objects.
  - Records and submits the indexed cube draw each frame.

- `VulkanPrj/src/engine/VulkanShaderData.h`
  - Embedded SPIR-V shader bytecode arrays used to create Vulkan shader modules.
  - No runtime shader-file loading is required.

- `VulkanPrj/src/engine/VulkanCommon.h`
  - Vulkan include and platform-specific `VK_USE_PLATFORM_*` setup.

---

## 2) Render flow (per frame)

`VulkanRenderer::Render(...)` executes this sequence:

1. Wait for the current frame fence.
2. Acquire next swapchain image.
3. Reset current-frame fence.
4. Reset and record the command buffer for the acquired image.
5. Submit graphics work to the graphics queue.
6. Present acquired image on the present queue.
7. Advance frame slot (`kMaxFramesInFlight`).

Swapchain recreation is triggered on `VK_ERROR_OUT_OF_DATE_KHR` / `VK_SUBOPTIMAL_KHR` and rebuilds dependent resources.

---

## 3) nkMath usage in the transform path

Local math files from `VulkanPrj/nkMath` are used directly in camera/model transform logic:

- `NkMat4x4.h`
- `NkVec3.h`
- `NkMathUtils.h`

### Camera matrices

In `VulkanContext`:

- View matrix is built from camera yaw/pitch and position using `NkMat4x4` rotations/translations.
- Projection matrix is rebuilt on resize (Vulkan clip-space conventions included).

### Animated cube model matrix

In `VulkanRenderer::BuildCubeModelMatrix(float sceneTimeSeconds)`:

- circular motion in XY plane:
  - `x = cos(t * 0.9) * 0.95`
  - `y = sin(t * 0.9) * 0.55`
  - `z = -0.35`
- continuous spin:
  - `rotationY = t * deg2rad(90)`
  - `rotationX = t * deg2rad(55)`
- composition:
  - `Model = Translation * RotationY * RotationX`

Final transform per draw:

- `MVP = Projection * View * Model`

This matrix is uploaded through push constants before `vkCmdDrawIndexed`.

---

## 4) Why the cube is green and visible

- Cube vertex data (`kCubeVertices`) uses green color `(0, 1, 0)` for all vertices.
- The renderer now sets up a complete Vulkan graphics path:
  - surface
  - physical/logical device
  - swapchain + image views
  - render pass + graphics pipeline
  - depth resources
  - framebuffers
  - vertex/index buffers
  - command recording/submission/presentation

This closes the previous gap where the app only created a `VkInstance` and never issued draw calls.

---

## 5) Build and run from scratch

## Prerequisites

- Git
- Python 3.8+
- Jenga (`jenga-build`)
- C++ toolchain
- Vulkan SDK (or Vulkan development headers/libs)

## Commands

```bash
git clone https://github.com/COLL-NOBEL/FromScratch.git
cd FromScratch
python -m pip install --upgrade jenga-build
jenga --version
jenga generate --config=Debug
jenga build --project=VKGraphicsEngine --config=Debug
```

Run (example paths):

- Windows:

```bash
Build\\Bin\\Debug-Windows\\VKGraphicsEngine\\VKGraphicsEngine.exe
```

- Linux:

```bash
./Build/Bin/Debug-Linux/VKGraphicsEngine/VKGraphicsEngine
```

Notes:

- `VulkanPrj/vulkan.prj.jenga` currently includes a Windows Vulkan SDK path (`C:\\VulkanSDK\\1.4.341.0`).
- If your local SDK path differs, update include/lib dirs accordingly.

---

## 6) Quick troubleshooting

- Black/empty output:
  - ensure Vulkan loader + GPU driver are installed,
  - verify the app can create a surface for your platform,
  - check swapchain creation logs.

- Build errors about Vulkan headers/libs:
  - install Vulkan dev packages/SDK,
  - verify include/lib paths in `vulkan.prj.jenga`.

- Window appears but no animation:
  - ensure event loop is running,
  - ensure `VulkanEngine::Tick` is called each frame.
