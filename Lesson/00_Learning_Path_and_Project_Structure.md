# FromScratch Lesson 00 — Learning Path and Project Structure

This lesson gives you a practical order for studying the repository and explains how the three projects connect.

---

## 1) What this repository is teaching

You have three rendering tracks:

1. **`2DEngine`**: software-style 2D rendering into a CPU framebuffer and blitting to a window.
2. **`OpenPrj`**: modern OpenGL pipeline (context, shaders, mesh, MVP matrix).
3. **`VulkanPrj`**: explicit Vulkan setup (instance/device/swapchain/pipeline/sync/command recording).

The framework around all three is **Nkentseu** (`NKWindow`, `NKEvent`, `NKContext`, `NKLogger`, `NKTime`, `NKMath`, etc.).

---

## 2) Recommended study order (author-friendly)

### Phase A — Foundation (read once)

1. `Lesson/01_Nkentseu_and_nkMath_Usage_Guide.md`
2. This file (`Lesson/00_Learning_Path_and_Project_Structure.md`)

Goal: understand *who owns what* (window/event/context/time/math vs rendering logic).

### Phase B — Easiest rendering path first

1. `Lesson/02_2DEngine_Detailed_Walkthrough.md`

Goal: learn the game-loop and draw-loop pattern without GPU complexity.

### Phase C — Programmable GPU path

1. `Lesson/03_OpenPrj_Detailed_Walkthrough.md`

Goal: learn shader-based rendering with a smaller API surface than Vulkan.

### Phase D — Explicit graphics API path

1. `Lesson/04_VulkanPrj_Detailed_Walkthrough.md`

Goal: map your OpenGL mental model to explicit Vulkan objects and frame synchronization.

### Phase E — Rebuild challenge

Rebuild in this order:

1. `2DEngine/src/apps/Apps.cpp`
2. `OpenPrj/src/engine/*` + `OpenPrj/src/main.cpp` + `OpenPrj/shaders/*`
3. `VulkanPrj/src/engine/*` + `VulkanPrj/src/main.cpp`

---

## 3) High-level architecture map

## Runtime layers

- **Application layer**: `nkmain(...)` in each project (`2DEngine`, `OpenPrj`, `VulkanPrj`).
- **Framework layer (Nkentseu)**: window creation, event polling, timing, logging, graphics context abstraction.
- **Rendering layer**:
  - CPU framebuffer drawing (`2DEngine`)
  - OpenGL renderer (`OpenPrj`)
  - Vulkan engine/context/renderer (`VulkanPrj`)

## Build/dependency layout

- Workspace root script: `trial.wks.jenga`
  - Includes external libs (`NKGlad`, `NKMath`)
  - Includes project scripts for each app (`open.prj.jenga`, `vulkan.prj.jenga`, `apps.prj.jenga`)
- External framework headers/libs:
  - `Externals/libs/nkentseu/include`
  - `Externals/libs/nkentseu/lib`

---

## 4) Project folder structure (learning-focused)

```text
FromScratch/
├── 2DEngine/
│   ├── apps.prj.jenga
│   └── src/apps/Apps.cpp
├── OpenPrj/
│   ├── open.prj.jenga
│   ├── src/main.cpp
│   ├── src/compat/msvcrt_compat.cpp
│   ├── src/engine/{Renderer,ShaderProgram,SimpleMesh}.{h,cpp}
│   ├── shaders/{basic.vert,basic.frag}
│   └── nkMath/*
├── VulkanPrj/
│   ├── vulkan.prj.jenga
│   ├── src/main.cpp
│   ├── src/engine/{VulkanCommon,VulkanContext,VulkanEngine,VulkanInstance,VulkanRenderer,VulkanShaderData}.{h,cpp}
│   └── nkMath/*
├── Externals/libs/
│   ├── nkentseu/
│   ├── NKGlad/
│   └── NKMath/
└── Lesson/
    ├── 00_Learning_Path_and_Project_Structure.md
    ├── 01_Nkentseu_and_nkMath_Usage_Guide.md
    ├── 02_2DEngine_Detailed_Walkthrough.md
    ├── 03_OpenPrj_Detailed_Walkthrough.md
    └── 04_VulkanPrj_Detailed_Walkthrough.md
```

---

## 5) Mental model before you dive into line-by-line files

Use this common frame loop model:

1. **Create window / rendering context (or Vulkan instance/device stack).**
2. **Poll events** (close, resize, input).
3. **Update simulation state** (time, camera, positions).
4. **Render frame** (CPU pixels or GPU draw calls).
5. **Present / swap**.
6. **Shutdown in reverse-order ownership**.

If you keep this model in mind, all three projects become variants of the same lifecycle.

---

## 6) How to use the detailed lessons effectively

For each source file lesson:

1. Keep the source file open side-by-side.
2. Read one line-range block.
3. Rewrite that block from memory.
4. Compare and correct.
5. Move to the next block.

This is the fastest path to rebuilding the code from scratch rather than only recognizing it.
