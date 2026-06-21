# VulkanPrj Practical Guide

This document explains how the `VulkanPrj/` sample is wired today, with emphasis on the rotating blue cube path.

## 1) `VulkanPrj/src` file-by-file (practical)

### `src/main.cpp`
- Program entry (`nkmain`) for the sample app.
- Creates an `NkWindow` and runs the event/render loop.
- Owns frame pacing (simple ~60 FPS sleep/yield behavior).
- Calls `VulkanEngine::Initialize`, then per-frame `VulkanEngine::Tick(deltaSeconds)`, then shutdown.

### `src/engine/VulkanCommon.h`
- Platform bridge header.
- Sets Vulkan platform macros (`VK_USE_PLATFORM_*`) based on Nkentseu platform defines before including `<vulkan/vulkan.h>`.
- Keeps the rest of the engine source platform-agnostic.

### `src/engine/VulkanEngine.h`
- Thin top-level engine wrapper around `VulkanContext`.
- Exposes `Initialize`, `Tick`, and `Shutdown`.
- Tracks accumulated uptime (`mUptimeSeconds`) used as scene time.

### `src/engine/VulkanEngine.cpp`
- Bootstraps `VulkanContext`.
- In `Tick`, advances scene time and triggers both update + render paths.
- Adds basic Vulkan lifecycle logging.

### `src/engine/VulkanContext.h`
- Defines camera state (`CameraState`) and context create options (`VulkanContextCreateInfo`).
- Owns `VulkanInstance`, `VulkanRenderer`, and camera matrices.
- Declares helpers to rebuild view/projection matrices.

### `src/engine/VulkanContext.cpp`
- Configures and creates the Vulkan instance.
- Initializes camera values and computes view/projection matrices with `NkVec3` and `NkMat4x4`.
- For each frame:
  - `Update`: refresh projection matrix from current window aspect ratio.
  - `RenderFrame`: forwards scene time + matrices to `VulkanRenderer::Render`.

### `src/engine/VulkanInstance.h`
- Declares `VulkanInstanceConfig` (app name/version/API/layers/extensions toggles).
- Declares `VulkanInstance` wrapper that owns `VkInstance` and enabled extension/layer lists.

### `src/engine/VulkanInstance.cpp`
- Implements VkInstance creation/destruction.
- Checks requested extensions/layers, including platform surface extensions.
- Handles validation layer enabling when available.
- Creates raw `VkInstance` with configuration from context.

### `src/engine/VulkanRenderer.h`
- Declares the low-level renderer that owns swapchain, pipeline, depth resources, command buffers, sync objects, and cube mesh buffers.
- Declares `Vertex` format (position + color) and all Vulkan setup helpers.
- Declares `BuildCubeModelMatrix(sceneTimeSeconds)` for cube animation.

### `src/engine/VulkanRenderer.cpp`
- Implements nearly all Vulkan render-system work:
  - surface/device/swapchain creation,
  - render pass + pipeline creation,
  - vertex/index buffer setup,
  - command recording and draw submission,
  - resize/swapchain recreation,
  - cleanup.
- Contains cube geometry and per-vertex color source data (`kCubeVertices` / `kCubeIndices`).
- Builds the model matrix (now in-place rotation) and composes final `MVP`.

### `src/engine/VulkanShaderData.h`
- Stores precompiled SPIR-V shader bytecode arrays for this sample.
- Renderer loads these arrays directly into shader modules.
- Shader expects vertex position + color, and an MVP push-constant matrix.

## 2) `VulkanPrj/nkMath` file-by-file (practical)

### `nkMath/NkColor.h`
- Lightweight RGBA color struct utilities.
- Includes conversions between component values and packed integer forms.
- Includes named helpers (`Red`, `Green`, `Blue`, etc.).

### `nkMath/NkMat3x3.h`
- Generic `NkMat3x3<T>` math type.
- Supports matrix arithmetic, determinant/inverse/transpose, and common 2D transform helpers.

### `nkMath/NkMat4x4.h`
- Core `NkMat4x4<T>` type used by Vulkan camera/model math.
- Supports matrix multiplication + transform builders (`Translation`, `RotationX/Y/Z`, etc.).
- This is the main matrix type used in `VulkanContext` and `VulkanRenderer`.

### `nkMath/NkMath.cpp`
- Aggregation translation unit that includes nkMath headers.
- Effectively acts as a compilation anchor for the math module in this project layout.

### `nkMath/NkMathUtils.h`
- Scalar utility helpers and constants (`Pi`, `deg2rad`, `clamp`, interpolation/easing).
- Used directly in this sample for frame-time clamping and angle conversion.

### `nkMath/NkQuat.h`
- Currently archived/commented-out quaternion implementation.
- Present for reference, but not active in this build.

### `nkMath/NkRandom.h`
- Currently archived/commented-out random helper implementation.
- Present for reference, but not active in this build.

### `nkMath/NkRectangle.h`
- Rectangle and typed-rectangle (`NkRectT`) helper definitions.
- Useful for 2D bounds/containment style logic.
- Not part of the hot path for this cube rendering sample.

### `nkMath/NkTransform3D.h`
- Additional transform helper content in the repo.
- Not used by current Vulkan sample flow (core matrix usage comes from `NkMat4x4.h`).

### `nkMath/NkVec2.h`
- Generic 2D vector type with arithmetic and interpolation utilities.

### `nkMath/NkVec3.h`
- Generic 3D vector type with dot/cross/normalize and interpolation helpers.
- Used by `VulkanContext` for camera vectors.

### `nkMath/NkVec4.h`
- Generic 4D vector type with arithmetic/interpolation helpers.

### `nkMath/Quaternion.h`
- Minimal quaternion container type.
- Not used directly in the current Vulkan sample path.

## 3) Key rendering/data-flow blocks for the cube

### A. Frame loop drives scene time
- `src/main.cpp`
  - Calculates `deltaSeconds` each loop.
  - Calls `engine.Tick(deltaSeconds)`.

### B. Engine accumulates scene time and calls render path
- `src/engine/VulkanEngine.cpp`
  - `mUptimeSeconds += deltaSeconds;`
  - `mContext.RenderFrame(mUptimeSeconds);`

### C. Context forwards camera matrices + time to renderer
- `src/engine/VulkanContext.cpp`
  - `mRenderer.Render(sceneTimeSeconds, mCamera.viewMatrix, mCamera.projectionMatrix, width, height);`

### D. Renderer records and submits cube draw
- `src/engine/VulkanRenderer.cpp`, `VulkanRenderer::Render`
  - Acquire swapchain image.
  - Record command buffer.
  - Submit graphics queue.
  - Present image.

### E. Cube geometry + color source data
- `src/engine/VulkanRenderer.cpp`
  - `kCubeVertices`: 8 cube vertices with per-vertex color.
  - `kCubeIndices`: indexed triangle list for 12 cube triangles.
  - Current color is blue (`{0.0f, 0.0f, 1.0f}`).

### F. Model transform (animation)
- `src/engine/VulkanRenderer.cpp`, `BuildCubeModelMatrix`
  - Uses `NkMathUtils::deg2rad` and `NkMat4x4::RotationY/RotationX`.
  - Produces in-place rotation (no orbit translation).

### G. MVP push constants + draw call
- `src/engine/VulkanRenderer.cpp`, `RecordCommandBuffer`
  - `mvp = projection * view * model`
  - `MatrixToColumnMajorArray(mvp, push.mvp)`
  - `vkCmdPushConstants(...)`
  - `vkCmdDrawIndexed(...)`

## 4) Build and run (current repo setup)

## Prerequisites
- `jenga` installed and available on PATH.
- Vulkan SDK + platform graphics prerequisites available on your machine.

## Build steps (from repo root)
1. Generate build files from workspace:
   ```bash
   jenga generate --config=Debug
   ```
2. Build Vulkan project target:
   ```bash
   jenga build --project=VKGraphicsEngine --config=Debug
   ```

## Run
- Windows:
  ```bash
  Build/Bin/Debug-Windows/VKGraphicsEngine/VKGraphicsEngine.exe
  ```
- Linux (path pattern used by workspace):
  ```bash
  ./Build/Bin/Debug-Linux/VKGraphicsEngine/VKGraphicsEngine
  ```

## Notes
- Workspace root is `trial.wks.jenga`, which includes `VulkanPrj/vulkan.prj.jenga`.
- The Vulkan sample target name is `VKGraphicsEngine`.
- If your environment lacks Vulkan loader/SDK or GUI/surface support, generation/build may succeed but running can still fail at runtime.
