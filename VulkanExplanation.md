# VulkanPrj Explanation (Rotating Green Orbiting Cube)

This document explains the Vulkan project in `VulkanPrj/` as it exists now.

The current renderer draws **one green cube** that:

- rotates continuously, and
- moves in a circular path on screen.

It also fixes the swapchain semaphore reuse problem that triggered:

- `VUID-vkQueueSubmit-pSignalSemaphores-00067`

---

## 1) Project structure and file responsibilities

## Root build/workspace files used by VulkanPrj

- `trial.wks.jenga`
  - Workspace definition that includes `VulkanPrj/vulkan.prj.jenga`.
- `VulkanPrj/vulkan.prj.jenga`
  - Vulkan project definition (`VKGraphicsEngine` target), source file glob, include dirs, link libs, platform filters.

## Vulkan runtime files

- `VulkanPrj/src/main.cpp`
  - Application entry point.
  - Creates the window, runs the event loop, computes `deltaSeconds`, and calls engine tick.

- `VulkanPrj/src/engine/VulkanEngine.h/.cpp`
  - Thin high-level orchestrator.
  - Owns `VulkanContext`, tracks uptime, and drives per-frame `Update` + `RenderFrame`.

- `VulkanPrj/src/engine/VulkanContext.h/.cpp`
  - Bridges app/window state and renderer state.
  - Owns:
    - `VulkanInstance`
    - `VulkanRenderer`
    - camera matrices (view/projection)
  - Keeps camera framing stable and updates projection when window size changes.

- `VulkanPrj/src/engine/VulkanInstance.h/.cpp`
  - Creates/destroys `VkInstance`.
  - Handles validation layer and debug utils setup.

- `VulkanPrj/src/engine/VulkanRenderer.h/.cpp`
  - Core Vulkan renderer implementation.
  - Owns device/swapchain/render pass/pipeline/depth buffers/mesh buffers/command buffers/sync objects.
  - Records draw commands and submits/presents every frame.

- `VulkanPrj/src/engine/VulkanShaderData.h`
  - Embedded SPIR-V shader bytecode arrays used by pipeline creation.
  - No runtime shader file loading is required.

- `VulkanPrj/src/engine/VulkanCommon.h`
  - Shared Vulkan include + platform surface macro wiring (`VK_USE_PLATFORM_*`).

## Math files used by this Vulkan project

- `VulkanPrj/nkMath/NkMat4x4.h`
- `VulkanPrj/nkMath/NkVec3.h`
- `VulkanPrj/nkMath/NkMathUtils.h`

These provide matrix math, vector types, and utility math functions used in camera and model transforms.

---

## 2) Rendering and synchronization flow

This is the frame flow in `VulkanRenderer::Render(...)`:

1. Wait for the current frame fence:
   - `mInFlightFences[mCurrentFrame]`
2. Acquire next swapchain image:
   - signals `mImageAvailableSemaphores[mCurrentFrame]`
   - returns `imageIndex`
3. Reset current frame fence.
4. Record command buffer for that acquired `imageIndex`.
5. Submit graphics work:
   - waits on `mImageAvailableSemaphores[mCurrentFrame]`
   - signals `mRenderFinishedSemaphores[imageIndex]`
   - uses `mInFlightFences[mCurrentFrame]`
6. Present:
   - waits on `mRenderFinishedSemaphores[imageIndex]`
7. Advance `mCurrentFrame` (modulo `kMaxFramesInFlight`).

## Why the semaphore bug happened and how it is fixed

The validation error was caused by signaling render-finished semaphores indexed by frame slot (`mCurrentFrame`) instead of by acquired swapchain image (`imageIndex`).

Now:

- `mImageAvailableSemaphores`: still per-frame (2 slots)
- `mInFlightFences`: still per-frame (2 slots)
- `mRenderFinishedSemaphores`: **per swapchain image**

This ensures a render-finished semaphore is reused only with the corresponding swapchain image lifecycle, which removes the `VUID-vkQueueSubmit-pSignalSemaphores-00067` misuse pattern.

On swapchain recreation, per-image render-finished semaphores are recreated to match the new image count.

---

## 3) Cube animation (rotation + circular translation)

The cube transform is built in:

- `VulkanRenderer::BuildCubeModelMatrix(float sceneTimeSeconds)`

Animation math:

- `orbitRadians = sceneTimeSeconds * 0.9`
- `x = cos(orbitRadians) * 0.95`
- `y = sin(orbitRadians) * 0.55`
- `z = -0.35`

So translation follows a circular path in the XY plane (with fixed depth), which appears as circular motion on screen.

Continuous rotation:

- `rotationY = sceneTimeSeconds * deg2rad(90)`
- `rotationX = sceneTimeSeconds * deg2rad(55)`

Model matrix composition:

- `Model = Translation * RotationY * RotationX`

Per-frame final transform:

- `MVP = Projection * View * Model`
- Uploaded through push constants before `vkCmdDrawIndexed`.

Color:

- All cube vertices are set to green (`0, 1, 0`) in `kCubeVertices`, so the rendered cube is clearly green.

---

## 4) Minimal rendering design choices

To keep the Vulkan path minimal and predictable:

- One cube is drawn (single indexed draw).
- No descriptor sets/uniform buffers are used for transforms.
- Push constants are used for the per-frame MVP matrix.
- Shader code is embedded (`VulkanShaderData.h`) to avoid shader file I/O complexity.

---

## 5) Build and run from scratch

## Prerequisites

- Git
- Python 3.8+
- Jenga (`jenga-build`)
- C++ toolchain for your platform
- Vulkan SDK / Vulkan development headers and loader

### Windows-specific note

`VulkanPrj/vulkan.prj.jenga` currently references this SDK path:

- `C:\VulkanSDK\1.4.341.0\Include`
- `C:\VulkanSDK\1.4.341.0\Lib`

If your Vulkan SDK is installed elsewhere, update those paths before building.

## Step-by-step commands

1. Clone and enter repository:

```bash
git clone https://github.com/COLL-NOBEL/FromScratch.git
cd FromScratch
```

2. Install Jenga:

```bash
python -m pip install --upgrade jenga-build
jenga --version
```

3. Generate build files:

```bash
jenga generate --config=Debug
```

4. Build Vulkan target:

```bash
jenga build --project=VKGraphicsEngine --config=Debug
```

5. Run executable:

- Windows (example):

```bash
Build\\Bin\\Debug-Windows\\VKGraphicsEngine\\VKGraphicsEngine.exe
```

- Linux (example):

```bash
./Build/Bin/Debug-Linux/VKGraphicsEngine/VKGraphicsEngine
```

(Exact output folder naming can vary by selected config/platform/toolchain.)

## Linux package hint (if Vulkan headers/libs are missing)

Example Ubuntu/Debian packages:

```bash
sudo apt-get update
sudo apt-get install -y build-essential clang libvulkan-dev libx11-dev
```

---

## 6) Runtime summary

At runtime the app initializes Vulkan, records one indexed cube draw each frame, applies animated model transforms, and presents through a swapchain-safe synchronization strategy.

Result: a stable, continuously rotating green cube orbiting in a circular path, with corrected semaphore usage for presentation synchronization.
