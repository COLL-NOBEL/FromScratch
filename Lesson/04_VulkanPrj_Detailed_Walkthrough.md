# FromScratch Lesson 04 — VulkanPrj Detailed Walkthrough

This lesson covers every requested VulkanPrj file with tight line-range commentary and rebuild guidance.

---

## Files covered

- `VulkanPrj/src/main.cpp`
- `VulkanPrj/src/engine/VulkanCommon.h`
- `VulkanPrj/src/engine/VulkanContext.h`
- `VulkanPrj/src/engine/VulkanContext.cpp`
- `VulkanPrj/src/engine/VulkanEngine.h`
- `VulkanPrj/src/engine/VulkanEngine.cpp`
- `VulkanPrj/src/engine/VulkanInstance.h`
- `VulkanPrj/src/engine/VulkanInstance.cpp`
- `VulkanPrj/src/engine/VulkanRenderer.h`
- `VulkanPrj/src/engine/VulkanRenderer.cpp`
- `VulkanPrj/src/engine/VulkanShaderData.h`

---

## 1) `VulkanPrj/src/main.cpp`

## Role

Application bootstrap for Vulkan track:

- create window,
- initialize `VulkanEngine`,
- run event + frame loop,
- call `Tick(deltaSeconds)` each frame,
- shutdown cleanly.

## Line-range walkthrough

- **L1–7**: include Nkentseu window/main/logger, engine header, and `NkMathUtils`.
- **L9**: `using namespace nkentseu`.
- **L11**: entrypoint `nkmain`.
- **L12–19**: configures titled resizable window.
- **L20–25**: creates window and fails early if not open.
- **L27–32**: constructs and initializes `VulkanEngine`.
- **L34–37**: prepares event source, frame timer, and running flag.
- **L38–40**: per-frame elapsed time and clamped `deltaSeconds`.
- **L42–47**: polls events; close event stops loop.
- **L49–51**: exits loop if close requested.
- **L53**: runs `engine.Tick(deltaSeconds)`.
- **L55–60**: frame pacing to roughly 60 FPS (sleep or yield).
- **L63–64**: shutdown engine and close window.
- **L66**: return success.

---

## 2) `VulkanPrj/src/engine/VulkanCommon.h`

## Role

Defines Vulkan platform macros before including `<vulkan/vulkan.h>`.

## Line-range walkthrough

- **L1–4**: include guard + platform detection include.
- **L5–25**: based on `NKENTSEU_*` platform/windowing macros, defines the matching `VK_USE_PLATFORM_*` macro.
- **L27**: includes Vulkan C API header.

Why this matters: Vulkan surface creation APIs are exposed only when corresponding platform macros are set before Vulkan header inclusion.

---

## 3) `VulkanPrj/src/engine/VulkanContext.h`

## Role

Declares high-level runtime context that owns:

- Vulkan instance wrapper,
- Vulkan renderer,
- camera state and view/projection rebuild helpers.

## Line-range walkthrough

- **L1–7**: includes instance/renderer headers and nkMath matrix/vector types.
- **L9–11**: forward declares `NkWindow`.
- **L15–29** (`CameraState`):
  - position/forward/up vectors,
  - yaw/pitch,
  - projection params (FOV, near/far),
  - cached view/projection matrices.
- **L31–36** (`VulkanContextCreateInfo`): app/engine names + validation/debug toggles.
- **L38–43**: context API (`Initialize`, `Update`, `RenderFrame`, `Shutdown`).
- **L45–50**: convenience queries (`IsInitialized`, `GetInstance`, `GetCamera`).
- **L52–54**: private matrix rebuild helpers.
- **L56–60**: owned members (`NkWindow*`, `VulkanInstance`, `VulkanRenderer`, `CameraState`).

---

## 4) `VulkanPrj/src/engine/VulkanContext.cpp`

## Role

Coordinates camera state + Vulkan renderer calls each frame.

## Line-range walkthrough

- **L1–7**: includes context, math utils, window, and `<cmath>`.
- **L11–13**: stores window pointer on initialize.
- **L14–18**: maps context create info into `VulkanInstanceConfig`.
- **L20–22**: creates Vulkan instance, aborting on failure.
- **L24–28**: initializes camera defaults and yaw/pitch in radians.
- **L29**: rebuilds view matrix.
- **L31–37**: computes aspect ratio from window size (fallback to 16:9).
- **L37**: rebuilds projection matrix.
- **L39–44**: initializes renderer with `VkInstance`, `NkSurfaceDesc`, and window size.
- **L45–47**: cleanup path if renderer init fails.
- **L50**: init success.

### Per-frame update/render

- **L53–56**: `Update` exits if context not initialized.
- **L58–63**: if window dimensions valid, recomputes projection matrix from current aspect.
- **L67–70**: `RenderFrame` guard for initialization and valid window pointer.
- **L72–80**: forwards scene time + camera matrices + dimensions to renderer.

### Shutdown

- **L83–87**: renderer shutdown then instance destroy then null window pointer.

### View matrix reconstruction

- **L89–96**: derives forward vector from yaw/pitch trigonometry.
- **L98**: normalizes forward vector.
- **L100–103**: chooses stable world-up fallback if near parallel.
- **L105–106**: computes right and corrected up vectors using cross products.
- **L108–110**: computes translation terms with dot products.
- **L112–117**: writes final view matrix explicitly.

### Projection matrix reconstruction

- **L120–126**: validates aspect, computes tan-half-fov and axis scales.
- **L127–128**: aliases near/far planes.
- **L130–135**: writes Vulkan-style perspective matrix.
  - Includes Y-flip (`-yScale`) at **L132**.

### nkMath usage in this file

- `NkVec3<float>`, `NkMat4x4<float>`, `Normalize`, `Dot`, `CrossProduct`, `NkMathUtils::deg2rad`.
- API usage only.

---

## 5) `VulkanPrj/src/engine/VulkanEngine.h`

## Role

Thin façade around `VulkanContext`.

## Line-range walkthrough

- **L1–7**: include context and forward declare `NkWindow`.
- **L11–15**: engine lifecycle API (`Initialize`, `Tick`, `Shutdown`).
- **L17–18**: read-only queries (`IsInitialized`, `GetCameraState`).
- **L20–22**: internal state (`VulkanContext`, accumulated uptime seconds).

---

## 6) `VulkanPrj/src/engine/VulkanEngine.cpp`

## Role

Minimal orchestration class.

## Line-range walkthrough

- **L1–3**: includes own header and logger.
- **L7–13**: creates default context info and initializes context.
- **L15**: resets uptime.
- **L17–20**: logs initialized Vulkan instance pointer.
- **L22**: init success.
- **L25–28**: tick guard if context not initialized.
- **L30**: accumulates uptime.
- **L31–32**: updates context and renders frame using uptime as scene time.
- **L35–38**: shutdown guard.
- **L40–41**: context shutdown and uptime log.

---

## 7) `VulkanPrj/src/engine/VulkanInstance.h`

## Role

Declares Vulkan instance wrapper: create/destroy + extension/layer capability checks.

## Line-range walkthrough

- **L1–6**: include Vulkan common and STL support headers.
- **L10–18** (`VulkanInstanceConfig`): app/engine names, version values, API version, validation/debug toggles.
- **L20–29**: class lifecycle (`Create`, `Destroy`, destructor).
- **L30–31**: validity/handle accessors.
- **L33–36**: static helper declarations for layer/extension checks.
- **L38–40**: internal state: handle + enabled extension/layer lists.

---

## 8) `VulkanPrj/src/engine/VulkanInstance.cpp`

## Role

Implements instance creation with platform/validation extensions.

## Line-range walkthrough

- **L1–7**: includes own header, logger, and STL helpers.
- **L11–13**: validation layer constant.
- **L15–31**: `VkResultToString` helper for readable logging.
- **L35–37**: destructor delegates to `Destroy`.

### Create flow

- **L39–40**: reset previous state by calling `Destroy`.
- **L42–43**: clears enabled extension/layer vectors.
- **L45–48**: requires `VK_KHR_surface`.
- **L50–74**: platform-specific surface extension handling:
  - Win32 required,
  - Metal required (+ portability helpers optional),
  - XCB/Xlib/Wayland optional with log.
- **L76–87**: optional validation layer + debug utils extension if available.
- **L89–95**: fills `VkApplicationInfo`.
- **L97–103**: fills `VkInstanceCreateInfo` with extension/layer arrays.
- **L105–109**: portability enumeration flag path for Metal.
- **L111–117**: creates Vulkan instance; logs failure details.
- **L119–124**: logs API version on success.
- **L126**: returns success.

### Destroy flow

- **L129–134**: destroys instance if valid and logs.
- **L136–137**: clears extension/layer vectors.

### Capability checks

- **L140–154**: `IsInstanceLayerSupported` via `vkEnumerateInstanceLayerProperties`.
- **L156–170**: `IsInstanceExtensionSupported` via `vkEnumerateInstanceExtensionProperties`.
- **L172–184**: `TryAddExtension` helper validates availability and appends extension name.

---

## 9) `VulkanPrj/src/engine/VulkanRenderer.h`

## Role

Declares the full Vulkan renderer object and all helper workflows.

## Line-range walkthrough

- **L1–12**: includes Vulkan common, nkMath matrix type, `NkSurfaceDesc`, and STL helpers.
- **L16–37**: public API (`Initialize`, `Render`, `Shutdown`, `IsInitialized`).
- **L40–46**: vertex struct + static binding/attribute descriptor builders.
- **L49**: in-flight frame constant (`kMaxFramesInFlight = 2`).
- **L51–57**: `MeshBuffers` struct (vertex/index buffer+memory+index count).
- **L59–66**: queue-family indices structure with completeness check.
- **L68–72**: swapchain support detail struct.
- **L74–76**: push constants struct (16-float MVP).
- **L78–143**: private method declarations for every Vulkan resource creation and helper selection.
- **L144–179**: all owned Vulkan handles/resources and synchronization state.

---

## 10) `VulkanPrj/src/engine/VulkanRenderer.cpp`

## Role

Implements the complete Vulkan rendering pipeline:

- surface/device/swapchain creation,
- render pass + graphics pipeline,
- mesh/depth buffers,
- per-frame command recording/submission/presentation,
- swapchain recreation and cleanup.

## Top-level helpers and static data

- **L1–7**: includes renderer header, embedded shader data, math utils, logger.
- **L9–15**: STL helpers (`algorithm`, `array`, `cmath`, `cstring`, `limits`, `set`).
- **L18–22**: required device extension list (`VK_KHR_swapchain`).
- **L24–49**: `VkResultToString` diagnostic mapping.
- **L51–65**: checks if device supports required extensions.
- **L67–79**: utility to create shader module from SPIR-V bytes.
- **L81–90**: static cube vertex data (position + color).
- **L92–99**: static cube index data (36 indices).

## Vertex layout helpers

- **L103–109**: binding description (`binding=0`, stride `sizeof(Vertex)`, per-vertex rate).
- **L111–125**: two attribute descriptions:
  - location 0 position (`R32G32B32_SFLOAT`, offset `position`),
  - location 1 color (`R32G32B32_SFLOAT`, offset `color`).

## Initialization pipeline

- **L127–133**: stores instance and validates not null.
- **L140–179**: sequentially builds renderer; each failure calls `Shutdown`:
  1. `CreateSurface`
  2. `PickPhysicalDevice`
  3. `CreateLogicalDevice`
  4. `CreateCommandPool`
  5. `CreatePipelineLayout`
  6. `CreateMeshBuffers`
  7. swapchain-dependent resources (`CreateSwapchain`, `CreateImageViews`, `CreateRenderPass`, `CreateGraphicsPipeline`, `CreateDepthResources`, `CreateFramebuffers`, `CreateCommandBuffers`, `CreateSyncObjects`).
- **L182–183**: logs successful initialization.

## Per-frame render flow

- **L186–199**: guard on initialization and non-zero window size.
- **L201**: waits on in-flight fence for current frame slot.
- **L203–211**: acquires next swapchain image.
- **L213–216**: recreates swapchain on `VK_ERROR_OUT_OF_DATE_KHR`.
- **L218–225**: logs and aborts frame on acquire failures.
- **L227–230**: safety check for render-finished semaphore vector size.
- **L232**: resets in-flight fence.
- **L234–241**: resets command buffer and records draw commands.
- **L243–255**: prepares submit synchronization arrays and `VkSubmitInfo`.
- **L257–271**: submits command buffer to graphics queue.
- **L273–282**: prepares presentation info.
- **L283–293**: presents and handles out-of-date/suboptimal cases.
- **L295**: advances frame index modulo `kMaxFramesInFlight`.

## Shutdown path

- **L298–301**: device idle wait before destroying resources.
- **L303–313**: destroys per-frame semaphores/fences.
- **L315–324**: frees command buffers.
- **L325**: cleans swapchain-owned resources.
- **L327–329**: destroys cube buffers and resets index count.
- **L331–339**: destroys pipeline layout and command pool.
- **L341–344**: destroys logical device.
- **L346–349**: destroys Vulkan surface.
- **L351–356**: nulls remaining state handles/counters.

## Surface creation (`NkSurfaceDesc` bridge)

- **L358–433**: platform-specific `CreateSurface` implementation:
  - Win32 (`vkCreateWin32SurfaceKHR`)
  - XCB (`vkCreateXcbSurfaceKHR`)
  - Xlib (`vkCreateXlibSurfaceKHR`)
  - Wayland (`vkCreateWaylandSurfaceKHR`)
  - macOS Metal (`vkCreateMetalSurfaceEXT`)
  - unsupported platform fallback.
- Validates platform data fields before Vulkan call; logs errors with readable result strings.

## Device and queue setup

### Physical device selection

- **L435–446**: enumerate physical devices.
- **L447–460**: choose first device with complete queue families + required extensions + adequate swapchain support.
- **L463–466**: fail if none suitable.
- **L468–470**: log selected GPU name.

### Logical device creation

- **L475–481**: resolves graphics/present queue families and deduplicates set.
- **L483–494**: builds queue create info entries.
- **L496**: uses default device features.
- **L498–505**: fills `VkDeviceCreateInfo` with queue infos and extensions.
- **L506–514**: creates logical device and logs failures.
- **L516–517**: retrieves graphics and present queue handles.

### Command pool

- **L522–528**: chooses graphics queue family and sets resettable command buffers.
- **L530–538**: creates command pool.

## Swapchain and image views

### Swapchain

- **L543–549**: query support details and validate formats/present modes.
- **L551–553**: pick format/present mode/extent.
- **L555–558**: choose image count (`min+1`, clamped to max).
- **L560–564**: gather queue family indices.
- **L566–589**: fills swapchain create info (sharing mode concurrent/exclusive based on queue family equality).
- **L590–598**: creates swapchain.
- **L600–605**: retrieves swapchain images and stores format/extent.

### Image views

- **L610–637**: creates one color image view per swapchain image.

## Render pass and pipeline

### Render pass

- **L639–644**: finds depth format and validates.
- **L646–655**: color attachment config.
- **L656–665**: depth attachment config.
- **L666–673**: attachment references.
- **L674–679**: graphics subpass setup.
- **L680–687**: subpass dependency for color/depth write hazards.
- **L688–700**: render pass create info.
- **L702–710**: creates render pass.

### Pipeline layout

- **L715–719**: push-constant range for vertex stage MVP.
- **L721–727**: pipeline layout create info with no descriptor sets.
- **L728–736**: create pipeline layout.

### Graphics pipeline

- **L741–752**: creates shader modules from embedded SPIR-V arrays.
- **L754–763**: handles shader module creation failure.
- **L765–780**: shader stage setup for vertex/fragment entry `main`.
- **L782–790**: vertex input from `Vertex` descriptors.
- **L792–795**: triangle-list input assembly.
- **L797–807**: viewport and scissor defaults from swapchain extent.
- **L809–815**: viewport state.
- **L816–825**: rasterizer state (fill, no cull, CCW front).
- **L826–830**: multisample state (1 sample).
- **L831–837**: depth/stencil test state (depth test/write enabled).
- **L839–852**: color blend attachment/state (no blending).
- **L853–862**: dynamic states enabled (`VIEWPORT`, `SCISSOR`).
- **L863–878**: fills graphics pipeline create info.
- **L879**: creates graphics pipeline.
- **L881–882**: destroys temporary shader modules.
- **L884–891**: logs pipeline create failure.

## Depth, framebuffers, mesh, commands, sync

### Depth resources

- **L896–908**: creates depth image + memory.
- **L910–913**: computes depth/stencil aspect mask.
- **L915–925**: depth image view create info.
- **L926–934**: creates depth image view.

### Framebuffers

- **L939–969**: one framebuffer per swapchain image view, each with color + shared depth view.

### Mesh buffers

- **L971–974**: computes vertex/index buffer sizes from cube arrays.
- **L975–983**: creates host-visible/coherent vertex buffer.
- **L985–988**: maps and copies cube vertex data.
- **L990–998**: creates host-visible/coherent index buffer.
- **L1000–1002**: maps and copies index data.
- **L1004–1005**: stores index count.

### Command buffers

- **L1008–1015**: allocate `kMaxFramesInFlight` primary command buffers.
- **L1017–1025**: allocate via Vulkan and check result.

### Sync objects

- **L1030–1034**: validates swapchain image availability.
- **L1036–1041**: creates semaphore/fence create infos; fences start signaled.
- **L1043–1051**: creates per-frame image-available semaphores and fences.
- **L1053–1057**: destroys stale render-finished semaphores if recreating.
- **L1059–1071**: allocates one render-finished semaphore per swapchain image with cleanup on failure.

## Swapchain recreation and cleanup

### Recreation

- **L1078–1081**: reject zero dimensions.
- **L1083–1084**: device idle and old swapchain cleanup.
- **L1086–1092**: rebuilds swapchain-dependent resources.
- **L1093–1094**: logs recreation failure.

### Cleanup

- **L1100–1107**: destroy render-finished semaphores.
- **L1108–1114**: destroy framebuffers.
- **L1115**: destroy depth image resources.
- **L1117–1125**: destroy graphics pipeline and render pass.
- **L1127–1132**: destroy swapchain image views.
- **L1134–1138**: destroy swapchain and clear image list.

## Command recording and draw call

- **L1141–1151**: begins command buffer.
- **L1153–1155**: sets clear values (color + depth).
- **L1157–1165**: render-pass begin info.
- **L1166**: begins render pass.
- **L1168–1179**: sets viewport and scissor structs.
- **L1180–1181**: emits dynamic viewport/scissor commands.
- **L1183**: binds graphics pipeline.
- **L1185–1187**: binds vertex and index buffers.
- **L1189–1190**: builds model matrix and full MVP (`projection * view * model`).
- **L1192–1194**: packs matrix into push constants.
- **L1195–1202**: pushes constants to vertex stage.
- **L1204**: issues indexed draw.
- **L1206–1207**: ends render pass and command buffer.

## Selection/helper utilities

- **L1210–1236**: queue family search (graphics + present support).
- **L1238–1263**: swapchain capabilities query.
- **L1265–1274**: preferred swap surface format selection.
- **L1276–1284**: present mode selection (mailbox preferred, FIFO fallback).
- **L1286–1299**: swap extent selection (current extent or clamped window size).
- **L1301–1318**: depth format selection.
- **L1320–1322**: stencil-component helper.

## Generic resource allocation helpers

### Buffers

- **L1324–1336**: populate `VkBufferCreateInfo`.
- **L1337–1345**: create buffer with error logging.
- **L1347–1349**: query memory requirements.
- **L1350–1354**: fill memory allocation info.
- **L1355–1365**: allocate memory and cleanup on failure.
- **L1367**: bind buffer memory.

### Images

- **L1371–1395**: populate `VkImageCreateInfo`.
- **L1396–1404**: create image with logging.
- **L1406–1408**: query image memory requirements.
- **L1409–1413**: fill memory allocation info.
- **L1414–1424**: allocate image memory with cleanup on failure.
- **L1426**: bind image memory.

### Memory type and destruction helpers

- **L1430–1445**: finds compatible memory type index.
- **L1447–1457**: `DestroyBuffer` utility.
- **L1459–1474**: `DestroyImage` utility.

## Matrix helpers

- **L1476–1482**: converts `NkMat4x4<float>` to column-major float array for shader push constants.
- **L1484–1490**: builds animated cube model matrix from time-driven X/Y rotations.

### nkMath usage in renderer

- `NkMat4x4<float>` matrix math,
- `NkMathUtils::deg2rad`,
- `NkMat4x4<float>::RotationY/X`,
- matrix multiplication to produce MVP.

---

## 11) `VulkanPrj/src/engine/VulkanShaderData.h`

## Role

Stores embedded SPIR-V bytecode arrays used at runtime to create shader modules.

## Line-range walkthrough

- **L1–5**: include guard, `<cstdint>`, namespace.
- **L7–317**: `kCubeVertexShaderSpv[]` bytes.
- **L319–444**: `kCubeFragmentShaderSpv[]` bytes.
- **L446**: namespace close.

## Embedded SPIR-V approach explained

- Shaders are **not loaded from `.vert/.frag` files** at runtime.
- Instead, precompiled SPIR-V words are compiled into the executable.
- `VulkanRenderer::CreateGraphicsPipeline` consumes these arrays via `CreateShaderModule`.
- Benefits:
  - no runtime shader file path dependency,
  - deterministic shader payload.
- Tradeoff:
  - editing shader logic requires regenerating header arrays.

---

## 12) Rebuild-from-scratch order for VulkanPrj

1. `VulkanCommon.h` (platform macro setup).
2. `VulkanInstance.h/.cpp` (instance creation + extension/layer checks).
3. `VulkanRenderer.h` declarations.
4. `VulkanShaderData.h` embedded SPIR-V arrays.
5. `VulkanRenderer.cpp` in layers:
   - helpers/static data,
   - init + teardown,
   - device/swapchain/pipeline,
   - command record/render,
   - helpers/allocators.
6. `VulkanContext.h/.cpp` (camera + renderer orchestration).
7. `VulkanEngine.h/.cpp` (thin façade).
8. `main.cpp` (window/event/tick loop).

If you keep ownership boundaries clear (`main` → `engine` → `context` → `renderer`/`instance`), the full stack remains understandable and maintainable.
