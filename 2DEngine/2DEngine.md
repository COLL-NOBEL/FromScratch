# 2DEngine Shooter — Detailed Explanation

This document explains the `2DEngine` implementation, how each code block works, which libraries are used, and how to build/run it from scratch.

---

## 1) Gameplay goals implemented

The game scene now provides:

- black background,
- player as a blue square on the far left,
- player movement with **Up / Down** arrow keys,
- shooting with **Space**,
- bullets as small yellow circles moving right,
- enemies as red circles on the right side,
- bullet-enemy collision where enemy disappears.

The color-channel issue (RGBA/BGRA confusion on Win32 DIB) is also fixed.

---

## 2) Source file

Implementation is in:

- `2DEngine/src/apps/Apps.cpp`

Project config is:

- `2DEngine/apps.prj.jenga`

---

## 3) Libraries and framework pieces used

From Nkentseu:

- `NKWindow` for windowing and event polling.
- `NKEvent` keyboard/window events.
- `NKMath/NkColor` for color constants.
- `NKLogger` for runtime error logs.
- `NKTime` through `NkChrono` for frame timing.

Platform API:

- Win32 GDI (`CreateDIBSection`, `BitBlt`) for software framebuffer presentation.

Standard C++:

- `std::vector` for bullets/enemies/pixel storage.
- `std::clamp` for safe movement bounds.
- `std::remove_if` for deleting dead bullets.

---

## 4) Code sections explained

## 4.1 Framebuffer class

`Framebuffer` is a tiny software renderer:

- owns a CPU pixel buffer (`mPixels`),
- handles window resize and reallocation,
- draws primitive shapes using pixel writes,
- presents to screen via Win32 GDI.

Main methods:

- `Resize(...)`: reallocates pixel buffer and rebuilds backend surface.
- `SetPixel(...)`: writes one pixel safely with bounds checks.
- `Clear(...)`: fills entire frame with one color.
- `DrawFilledRect(...)`: used for player square.
- `DrawFilledCircle(...)`: used for enemies and bullets.
- `Present()`: copies CPU buffer into DIB and blits to window.

## 4.2 Color-order fix (RGBA/BGRA)

The previous issue happened because Win32 DIB expects BGRA byte order, while game logic uses RGBA colors.

Fix implemented:

- `SetPixel(...)` converts logical RGBA to BGRA when writing DIB memory.
- `Clear(...)` also writes in BGRA order on Windows.

Result:

- `NkColor::Red` now renders red,
- `NkColor::Blue` now renders blue,
- black background remains correct.

## 4.3 Game state

Two lightweight structs are used:

- `Bullet { x, y, alive }`
- `Enemy { x, y, alive }`

Runtime state stores:

- player position and speed,
- key-held booleans (up/down/space),
- fire cooldown,
- `std::vector<Bullet>`,
- `std::vector<Enemy>`.

## 4.4 Input handling

Events are polled every frame via `NkEvents().PollEvent()`.

Handled event types:

- `NkWindowCloseEvent`
- `NkWindowResizeEvent`
- `NkWindowMaximizeEvent`
- `NkWindowMinimizeEvent`
- `NkKeyPressEvent`
- `NkKeyRepeatEvent`
- `NkKeyReleaseEvent`

Movement logic:

- `NK_UP` pressed/repeated => move-up held.
- `NK_DOWN` pressed/repeated => move-down held.
- release events clear held state.

Shooting logic:

- `NK_SPACE` sets `shootHeld`.
- bullets spawn at fire interval while held.

## 4.5 Player update

Per frame:

- apply vertical velocity based on held keys,
- clamp Y position so player remains on screen.

Player is intentionally fixed near the left edge:

- `playerX = 40 + halfSize`.

## 4.6 Bullet update

Each bullet:

- moves right by `kBulletSpeed * deltaSeconds`,
- gets marked dead when outside viewport,
- is later removed with erase/remove-if.

## 4.7 Enemy layout

Enemies are created by `resetEnemies()`:

- placed on right side in two lanes,
- spread vertically across usable height,
- remain stationary.

## 4.8 Collision and destruction

For each alive bullet and alive enemy:

- compute squared distance,
- compare with squared sum of radii,
- on hit: `bullet.alive = false; enemy.alive = false;`

Enemy disappears because render step skips non-alive enemies.

## 4.9 Render order

Each frame renders in this order:

1. `Clear(Black)` background.
2. Blue player square.
3. Yellow bullet circles.
4. Red enemy circles (alive only).
5. `Present()`.

This ensures stable layering and clear visuals.

---

## 5) Timing and frame pacing

`NkChrono` is used for:

- per-frame delta time (`frameChrono.Reset()`),
- simple 60 FPS cap (sleep/yield based on elapsed milliseconds).

`deltaSeconds` is clamped to avoid unstable behavior on long frame stalls.

---

## 6) Build from scratch

## Prerequisites

- Python 3.x
- Jenga build system
- C++ toolchain

Install Jenga:

```bash
python -m pip install --upgrade jenga-build
```

## Clone repository

```bash
git clone https://github.com/COLL-NOBEL/FromScratch.git
cd FromScratch
```

## Generate build files

```bash
jenga generate --config=Debug
```

## Build the 2DEngine project

If your workspace already includes `2DEngine/apps.prj.jenga`:

```bash
jenga build --project=Apps --config=Debug
```

If `Apps` is not included in the workspace yet, add this line under workspace includes in `trial.wks.jenga`:

```python
with include("2DEngine/apps.prj.jenga"):
    pass
```

Then regenerate and build again.

## Run

Executable output path:

- `Build/Bin/Debug-<platform>/Apps/`

---

## 7) Suggested next improvements

- add player/enemy health and score,
- add enemy movement patterns,
- add restart logic when all enemies are destroyed,
- add text/UI overlay,
- separate renderer/input/gameplay into different files once scope grows.

---

## 8) Summary

The current `2DEngine` implementation is intentionally small and readable while still being fully playable:

- movement,
- shooting,
- collisions,
- enemy destruction,
- fixed color-channel correctness,
- and straightforward code structure for learning and extension.
