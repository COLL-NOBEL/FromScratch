# FromScratch Lesson 02 — 2DEngine Detailed Walkthrough

This lesson documents `2DEngine/src/apps/Apps.cpp` in enough detail to recreate it from scratch.

---

## Target file

- `2DEngine/src/apps/Apps.cpp`

---

## 1) What this file builds

`Apps.cpp` builds a complete mini game loop:

- creates a resizable window,
- allocates a software framebuffer,
- handles keyboard and window events,
- simulates player/bullet/enemy logic,
- rasterizes simple primitives (rectangles/circles) on the CPU,
- presents the pixel buffer to the window (Win32 backend).

It is a **CPU-rendered 2D shooter sample**, not a GPU pipeline.

---

## 2) Detailed line-range walkthrough (`Apps.cpp`)

## Includes and compile-time setup

- **L1–5**: brings in Nkentseu runtime pieces:
  - `NkMain` for `nkmain` entry,
  - `NkWindow` for window/surface,
  - `NkLog` for diagnostics,
  - keyboard events,
  - `NkColor` for draw colors.
- **L7–9**: conditionally includes `<windows.h>` when on Windows.
- **L11–15**: standard library utilities (`algorithm`, `cmath`, `cstdint`, `cstring`, `vector`).
- **L17**: `using namespace nkentseu;` shortens framework type names.

## Anonymous-namespace rendering helper and gameplay PODs

- **L19**: opens anonymous namespace for internal symbols.
- **L21–251**: defines `Framebuffer` helper class.
- **L253–263**: defines lightweight gameplay structs (`Bullet`, `Enemy`).
- **L265**: closes anonymous namespace.

### `Framebuffer` lifecycle and memory ownership

- **L23–26** (`Framebuffer` constructor): immediately queries window size and calls `Resize`.
- **L28–30** (destructor): always runs `CloseBackend` to release native objects.

### Resize/allocation path

- **L32–33**: on resize, always tear down backend first (safe re-init pattern).
- **L35–37**: stores width/height and computes stride (`width * 4` bytes per pixel RGBA/BGRA).
- **L39–42**: if window dimension is zero, clear pixel vector and return success.
- **L44–46**: computes total buffer bytes (`stride * height`) using `size_t` casts.
- **L47**: allocates and zero-fills `mPixels`.
- **L49**: initializes platform backend (DIB/DC on Windows).

### Validity checks

- **L52–58**:
  - on Windows, requires dimensions + pixel buffer + DIB/DC/window DC handles;
  - on non-Windows, only checks dimensions + pixel data.

### Basic accessors

- **L60–61**: `GetWidth`, `GetHeight` return framebuffer dimensions.

### Pixel write path

- **L63–66**: bounds-check in `SetPixel`; silently ignore out-of-range coordinates.
- **L68–70**: computes byte index of pixel in linear buffer.
- **L72–83**: writes channels in platform-correct order:
  - Windows DIB memory is BGRA,
  - fallback path writes RGBA.

### Clear path

- **L86–97**: precomputes four bytes in platform-specific channel order.
- **L99–104**: loops over every pixel (step 4 bytes) and fills color.

### Filled rectangle raster

- **L107–110**: reject non-positive dimensions.
- **L112–115**: clamps rectangle bounds to framebuffer extents.
- **L117–121**: nested loops call `SetPixel` for each covered point.

### Filled circle raster

- **L124–127**: reject non-positive radius.
- **L129**: precomputes `radius^2` for circle equation.
- **L131–137**: loops over square around center and fills points satisfying `x^2 + y^2 <= r^2`.

### Present path

- **L140–159**:
  - Windows-only path checks native handles,
  - copies CPU buffer into DIB bits (`memcpy`),
  - blits DIB onto window (`BitBlt`, `SRCCOPY`).

### Platform backend initialization

- **L162–168**:
  - fetches `HWND` from `window.GetSurfaceDesc().hwnd`,
  - validates handle.
- **L170–174**: acquires window device context via `GetDC`.
- **L176–183**: configures `BITMAPINFO` (32-bit top-down DIB).
- **L185–193**: creates DIB section, captures writable pixel pointer.
- **L194–198**: failure path logs and cleans up.
- **L200–206**: creates compatible memory DC and validates.
- **L208–209**: selects DIB into memory DC; backend ready.
- **L210–213** (non-Windows): returns `false` by design.

### Platform backend teardown

- **L216–236**:
  - delete memory DC,
  - delete DIB bitmap,
  - release window DC,
  - null all native pointers.

### Framebuffer member state

- **L239–243**: dimensions + stride + CPU pixel vector.
- **L244–250**: Win32 handle storage.

### Gameplay data structs

- **L253–257**: `Bullet` with position and alive flag.
- **L259–263**: `Enemy` with position and alive flag.

---

## `nkmain` game/app flow

### Window and framebuffer startup

- **L267**: framework entrypoint `nkmain`.
- **L268–275**: configures titled, centered, resizable window.
- **L276–280**: creates window; returns `-2` on failure.
- **L282–287**: constructs framebuffer and validates; returns `-3` if init fails.
- **L289–290**: initializes viewport dimensions from framebuffer.

### Gameplay tuning constants and state

- **L292–298**: compile-time gameplay constants:
  - player size/speed,
  - bullet size/speed,
  - enemy size,
  - fire interval.
- **L299–301**: initial player location (left side, vertically centered).
- **L302–305**: input state flags (`moveUpHeld`, `moveDownHeld`, `shootHeld`).
- **L306**: fire cooldown accumulator.
- **L308–309**: bullet and enemy vectors.

### Enemy reset lambda

- **L311–313**: clears existing enemies.
- **L314–319**: layout constants and vertical span.
- **L320–330**: creates 8 enemies across two x-lanes and interpolated y positions.
- **L333**: initial call to populate enemy wave.

### Main loop setup

- **L335**: `NkChrono frameChrono` for per-frame timing.
- **L336**: gets event bus reference via `NkEvents()`.
- **L337**: `running` sentinel.

### Per-frame timing and event pump

- **L339–342**:
  - reset frame chrono,
  - compute clamped `deltaSeconds` (`0..0.05`) to limit instability.
- **L343**: marks resize flag false.
- **L345–402**: consumes all pending events each frame.

#### Event handling blocks

- **L346–351**: close event for current window stops loop.
- **L353–357**: resize event updates viewport dimensions.
- **L359–365**: maximize event refreshes current size from window.
- **L367–372**: minimize event zeroes viewport dimensions.
- **L374–382**: local lambda for key-down/repeat mapping.
- **L384–390**: key press/repeat set movement/fire booleans.
- **L392–400**: key release unsets corresponding booleans.

### Early-out and resize handling

- **L404–406**: break frame loop if close requested.
- **L408–421**:
  - if resized and dimensions valid, resize framebuffer,
  - clamp player Y into new bounds,
  - reset enemies for new viewport.
- **L423–426**: if minimized or invalid framebuffer, sleep briefly and skip render/update.

### Simulation update

- **L428–434**: apply held movement input to player Y.
- **L436–438**: clamp player Y within viewport.
- **L440**: decrement cooldown with floor at zero.
- **L441–448**:
  - when shooting and cooldown elapsed, spawn bullet,
  - reset cooldown.
- **L450–460**:
  - move live bullets right,
  - mark off-screen bullets dead.
- **L462–464**: computes collision radius and squared threshold.
- **L465–485**: bullet/enemy collision check using squared distance.
- **L487–494**: compacts bullet vector by removing dead bullets.

### Draw pass

- **L496**: clear screen to black.
- **L498–504**: draw player as filled blue rectangle.
- **L506–513**: draw bullets as yellow circles.
- **L515–526**: draw alive enemies as red circles.
- **L528**: present framebuffer to window.

### Frame pacing and shutdown

- **L530–535**:
  - measure elapsed frame duration,
  - sleep to target ~16ms/frame (~60 FPS), else yield thread.
- **L538**: close window.
- **L540**: return success.

---

## 3) Rebuild-from-scratch checklist (`Apps.cpp`)

Use this exact order while recreating:

1. Add includes and `using namespace nkentseu`.
2. Implement `Framebuffer` with:
   - CPU buffer storage,
   - resize/allocation,
   - set/clear/draw primitives,
   - present,
   - Win32 backend init/close.
3. Add `Bullet` and `Enemy` POD structs.
4. Implement `nkmain`:
   - window create,
   - framebuffer create/validate,
   - constants/state,
   - enemy reset lambda,
   - frame loop + event handling,
   - movement/shoot/collision,
   - draw + present,
   - frame pacing + cleanup.

---

## 4) nkMath usage in this file (no internals)

- `math::NkColor` is used as a value type for pixel and primitive coloring.
- Predefined colors are used directly in render calls:
  - `math::NkColor::Black()`
  - `math::NkColor::Blue`
  - `math::NkColor::Yellow`
  - `math::NkColor::Red`

No deeper math-library internals are required to understand this file.

---

## 5) Important gotcha

- On non-Windows platforms, `InitBackend` returns false (`L210–213`), so this sample is effectively Windows-focused in its current form.
