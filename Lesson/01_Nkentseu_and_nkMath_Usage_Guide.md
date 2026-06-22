# FromScratch Lesson 01 — Nkentseu and nkMath Usage Guide

This lesson explains **how Nkentseu is structured** in this repository and **how each project uses it**.

> Scope note: this is intentionally usage-focused. It does **not** dive into nkMath internals.

---

## 1) Nkentseu structure in this repository

## Physical layout

- Headers: `Externals/libs/nkentseu/include`
- Prebuilt libraries: `Externals/libs/nkentseu/lib`

### Main modules visible in `include/`

- `NKWindow` — window creation and platform surface descriptors
- `NKEvent` — event polling and typed events
- `NKContext` — graphics context abstraction (used by OpenGL path)
- `NKLogger` — logging helpers (`logger.Info`, `logger.Error`, `logger.Infof`)
- `NKTime` (`NkChrono`, `NkElapsedTime`) — frame timing and sleep/yield
- `NKMath` — vectors/matrices/colors/math utilities
- `NKPlatform` — platform detection macros

## How projects link Nkentseu

### `2DEngine/apps.prj.jenga` line map

- **L19–22**: include `src` + Nkentseu headers.
- **L24–28**: link foundational and runtime modules (`NKPlatform`, `NKCore`, `NKMemory`, `NKContainers`, `NKMath`, `NKTime`, `NKLogger`, `NKThreading`, `NKEvent`, `NKWindow`).
- **L30–32**: library directory is `Externals/libs/nkentseu/lib`.

### `OpenPrj/open.prj.jenga` line map

- **L22–26**: include `src`, Nkentseu headers, and `NKGlad` headers.
- **L28–33**: links same foundational runtime stack, plus `NKContext` and `NKGlad`.
- **L35–37**: uses Nkentseu lib directory.

### `VulkanPrj/vulkan.prj.jenga` line map

- **L18–22**: include `src`, Nkentseu headers, Vulkan SDK headers.
- **L24–28**: links foundational/runtime stack including `NKContext`.
- **L30–33**: lib directories include Nkentseu and Vulkan SDK.

### Workspace-level inclusion (`trial.wks.jenga`)

- **L108–112**: includes external `NKGlad` and `NKMath` libraries.
- **L114–121**: includes each application project script (`OpenPrj`, `VulkanPrj`, `2DEngine`).

---

## 2) Nkentseu usage map by project

## Common pattern in all apps

Every app follows:

1. `nkmain(...)` entrypoint (framework-managed startup)
2. Create `NkWindow`
3. Poll events using `NkEvents().PollEvent()`
4. Use timing helpers (`NkChrono`, `NkElapsedTime` or `std::chrono`)
5. Shutdown cleanly (`window.Close()`, renderer/context shutdown)

---

## `2DEngine` usage (`2DEngine/src/apps/Apps.cpp`)

### Core framework usage

- **L1–5**: imports `NkMain`, `NkWindow`, logger, keyboard events, `NkColor`.
- **L267**: `nkmain(const NkEntryState&)` entrypoint.
- **L268–280**: `NkWindowConfig` + `NkWindow::Create`.
- **L335–402**: event polling with typed checks (`NkWindowCloseEvent`, resize/maximize/minimize, key press/repeat/release).
- **L335, L340, L530–535**: `NkChrono` and `NkElapsedTime` for frame delta and pacing.

### Window surface use

- **L164**: `window.GetSurfaceDesc().hwnd` gives native handle used by Win32 blit backend.

### Logging

- Error/info logs through `logger.Error(...)` for window/framebuffer failures.

---

## `OpenPrj` usage (`OpenPrj/src/main.cpp` + engine files)

### Core framework usage in `main.cpp`

- **L1–3**: `NkWindow`, `NkMain`, logger.
- **L14–18**: `NkContext` interfaces and OpenGL context metadata types.
- **L52**: `nkmain` entrypoint.
- **L54–63**: window config and creation.
- **L68–79**: configure `NkContextDesc` for OpenGL API/version/profile/debug/msaa/swap interval.
- **L82**: create context with `NkContextFactory::Create(window, desc)`.
- **L109–118**: event loop (close event handling).
- **L124–134**: per-frame context calls (`BeginFrame`, `EndFrame`, `Present`).

### Graphics context bridge

- **L27–49 (`LoadGL`)**: checks `NkIGraphicsContext` API type, casts native data to `NkOpenGLContextData`, retrieves function loader, and initializes GLAD.

### Logging

- `logger.Info/Error/Infof` around context/loader startup.

---

## `VulkanPrj` usage (`VulkanPrj/src/main.cpp` + engine files)

### Core framework usage in `main.cpp`

- **L1–3**: window/main/logger headers.
- **L11**: `nkmain` entrypoint.
- **L12–21**: window setup and open check.
- **L34–47**: event loop using `NkEvents` and `NkWindowCloseEvent`.
- **L35–60**: frame timing and pacing via `NkChrono` + `NkElapsedTime`.

### Surface abstraction for Vulkan

- `VulkanContext::Initialize` passes `mWindow->GetSurfaceDesc()` to `VulkanRenderer::Initialize` (**VulkanContext.cpp L39–44**).
- `VulkanRenderer::CreateSurface` consumes `NkSurfaceDesc` and creates a platform Vulkan surface (**VulkanRenderer.cpp L358–433**).

### Logging

- Vulkan startup/shutdown and failure points consistently report through `logger`.

---

## 3) nkMath usage (API-level only)

## 2DEngine

- `NKMath/NkColor.h` (**Apps.cpp L5**).
- Color values passed into software draw methods (`SetPixel`, `Clear`, `DrawFilledRect`, `DrawFilledCircle`).
- Uses predefined color constructors/constants (`Black`, `Blue`, `Yellow`, `Red`) at draw time.

## OpenPrj

- `NKMath/NKMath.h` included in renderer (**Renderer.cpp L3**).
- Uses math API only for transforms:
  - `NkVec3f` camera vectors (**L40–42**)
  - `NkMat4f::RotationY`, `NkMat4f::LookAt`, `NkMat4f::Perspective` (**L44–46**)
  - `NkAngle(...)` wrapper for degrees (**L44, L46**)
  - `mvp = projection * view * model` (**L47**)

## VulkanPrj

- `NkMathUtils::clamp` for frame delta in main loop (**main.cpp L40**).
- Camera math in `VulkanContext`:
  - `NkVec3<float>` and `NkMat4x4<float>` state (**VulkanContext.h L16–18, L27–28**)
  - `NkMathUtils::deg2rad` for yaw/pitch defaults (**VulkanContext.cpp L26–27**)
  - vector ops `Normalize`, `Dot`, `CrossProduct` when rebuilding view matrix (**L98–106, L108–110**)
  - explicit matrix construction for view/projection (**L112–117, L130–135**)
- Model animation in renderer:
  - `NkMathUtils::deg2rad` rotation speeds (**VulkanRenderer.cpp L1485–1486**)
  - `NkMat4x4<float>::RotationY/X` to build rotating cube matrix (**L1488–1489**)

---

## 4) Practical comparison: Nkentseu role in each project

| Concern | 2DEngine | OpenPrj | VulkanPrj |
|---|---|---|---|
| Entrypoint | `nkmain` | `nkmain` | `nkmain` |
| Window | `NkWindow` | `NkWindow` | `NkWindow` |
| Event loop | `NkEvents` | `NkEvents` | `NkEvents` |
| Rendering backend | CPU framebuffer + Win32 blit | OpenGL (`NkContext` + GLAD) | Vulkan (manual instance/device/swapchain) |
| nkMath role | colors + small scalar math | MVP transform assembly | camera/projection/model math |
| Presentation | `Framebuffer::Present` | `ctx->Present()` | `vkQueuePresentKHR` |

---

## 5) Rebuild guidance for author

When recreating the projects from scratch, reuse this dependency principle:

1. Start with **Nkentseu lifecycle plumbing** (entrypoint/window/events/time/logger).
2. Add **renderer-specific layer** only after lifecycle is stable.
3. Add **nkMath usage where transforms/colors are needed**, but keep math usage local and explicit.
4. Keep ownership clear: app loop owns high-level flow, renderer owns API resources.
