# main.cpp Line-by-Line Detailed Explanation

This document explains **every single line** of your `main.cpp`, how it works, what it does, and how it relates to graphics programming and OpenGL.

---

## Lines 1-6: Include Headers

```cpp
#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"

#include "NKGlad/include/glad/wgl.h"
#include "NKGlad/include/glad/gl.h"
```

### What This Does

You're **including header files** from the Nkentseu library. Headers declare interfaces you'll use.

| Header | What It Provides | Why You Need It |
|--------|-----------------|-----------------|
| `NkWindow.h` | Window creation/management | Defines `NkWindow` class to create your application window |
| `NkMain.h` | Entry point handling | Defines `nkmain()` function (Nkentseu's main entry, like GLFW's glfwMain) |
| `NkLog.h` | Logging system | Defines `logger` object to print debug messages |
| `glad/wgl.h` | Windows OpenGL loader | Loads **WGL** (Windows Graphics Library) - Windows-specific OpenGL extensions for context creation |
| `glad/gl.h` | OpenGL function loader | Loads **core OpenGL** functions like `glClear()`, `glDrawArrays()`, etc. |

### Why WGL and GL Separately?

On **Windows**, OpenGL has two parts:

1. **WGL (Windows Graphics Library)**: Platform-specific code to:
   - Create an OpenGL context on Windows
   - Make the context current
   - Swap buffers (flip front/back buffers)
   
2. **GL (Core OpenGL)**: Portable graphics functions that work on any OS:
   - `glClear()` - clear framebuffer
   - `glDrawArrays()` - draw geometry
   - `glBindTexture()` - bind textures
   - etc.

On **Linux**, you'd include `X11/glx.h` instead of `wgl.h`. On **macOS**, you'd use `Cocoa/gl.h`. Nkentseu handles this abstraction for you!

---

## Lines 8-10: Undefine Bool Macro

```cpp
#if defined(Bool)
    #undef Bool
#endif
```

### What This Does

**Checks** if a macro named `Bool` is defined, and if so, **removes/undefines it**.

### Why This Matters

The Windows API (`#include <windows.h>`, indirectly included) defines:

```cpp
#define Bool int  // Windows typedef
```

But **GLAD** also uses `Bool`. These collide! This code **removes the Windows version** so GLAD's definition works.

### Real-World Analogy

Like if two libraries both define a function named `Draw()` — you need to tell the compiler which one to use. This undefines Windows's version to avoid conflict.

---

## Lines 12-15: Graphics Context Headers

```cpp
#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkNativeContextAccess.h"
#include "NKContext/Core/NkOpenGLDesc.h"
```

### What Each Header Does

| Header | Purpose |
|--------|---------|
| `NkContextFactory.h` | **Factory pattern**: Creates graphics contexts (OpenGL, DirectX, Metal). Provides `NkContextFactory::Create()` |
| `NkContextDesc.h` | **Configuration struct**: Describes what graphics context you want (API type, version, settings) |
| `NkNativeContextAccess.h` | **Platform access**: Gets platform-specific data (like OpenGL function loader) from the context |
| `NkOpenGLDesc.h` | **OpenGL-specific settings**: Major/minor version, profile (Core vs Compat), MSAA samples, debug mode, etc. |

### Why Separate Headers?

**Separation of concerns**: Each header is responsible for one thing. This makes code modular and easier to understand.

---

## Line 17: Using Namespace

```cpp
using namespace nkentseu;
```

### What This Does

**Imports the `nkentseu` namespace**, so you can write:

```cpp
NkWindow window;          // Instead of nkentseu::NkWindow window;
NkContextFactory::Create  // Instead of nkentseu::NkContextFactory::Create
```

---

## Lines 19-36: LoadGL Function

```cpp
// Load OpenGL function pointers using GLAD
static bool LoadGL(NkIGraphicsContext* ctx) {
    auto loader = NkNativeContext::GetOpenGLProcAddressLoader(ctx);
    if (!loader) { 
        logger.Info("[OpenGL] Function loader not found\n"); 
        return false; 
    }

    int ver = gladLoadGL((GLADloadfunc)loader);
    
    if (!ver) { 
        logger.Error("[OpenGL] gladLoadGL failed\n"); 
        return false; 
    }
    logger.Infof("[OpenGL] OpenGL %s  GLSL %s\n",
        glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    return true;
}
```

### Function Purpose

**Loads all OpenGL function pointers** so your code can call OpenGL functions. This is crucial and often misunderstood!

### Why Load Function Pointers?

Unlike traditional C libraries, OpenGL functions are **dynamic**:
- You don't call `glClear()` directly
- Instead, the GPU driver provides a **pointer to the function** at runtime
- GLAD's job is to find these pointers and populate function pointers

Think of it like this:

```cpp
// Without GLAD (won't work):
glClear(GL_COLOR_BUFFER_BIT);  // glClear is undefined!

// With GLAD:
// 1. Get function pointer from GPU driver
// 2. Now glClear is a valid function pointer
// 3. Now you can call it
glClear(GL_COLOR_BUFFER_BIT);  // Works!
```

### Line-by-Line Breakdown

#### Line 21: Get Function Loader
```cpp
auto loader = NkNativeContext::GetOpenGLProcAddressLoader(ctx);
```
- **`NkNativeContext::`**: Static class from Nkentseu
- **`GetOpenGLProcAddressLoader(ctx)`**: Asks Nkentseu to provide a function that loads OpenGL function pointers
  - On **Windows**, this internally uses `wglGetProcAddress()`
  - On **Linux**, this uses `glXGetProcAddress()`
  - On **macOS**, this uses `dlsym()`
- **`loader`**: A function pointer (a pointer to a function that loads OpenGL function pointers!)

#### Lines 22-25: Error Check
```cpp
if (!loader) { 
    logger.Info("[OpenGL] Function loader not found\n"); 
    return false; 
}
```
- If `loader` is `nullptr` (null), getting the loader **failed**
- Log an info message (less severe than error)
- Return `false` to indicate failure

#### Line 27: Load OpenGL Functions
```cpp
int ver = gladLoadGL((GLADloadfunc)loader);
```
- **`gladLoadGL()`**: GLAD's main function. Does the actual loading.
- **`(GLADloadfunc)loader`**: Casts `loader` to GLAD's expected function pointer type
- **`ver`**: Return value is an integer:
  - **0**: Loading failed
  - **Non-zero**: OpenGL version number (e.g., 46 for GL 4.6)

#### Lines 29-32: Check Load Success
```cpp
if (!ver) { 
    logger.Error("[OpenGL] gladLoadGL failed\n"); 
    return false; 
}
```
- If `ver` is 0, loading **failed**
- Log an error message (more severe than info)
- Return `false`

#### Lines 33-34: Query OpenGL Version
```cpp
logger.Infof("[OpenGL] OpenGL %s  GLSL %s\n",
    glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
```
- Now that OpenGL is loaded, you **can call OpenGL functions**!
- **`glGetString(GL_VERSION)`**: OpenGL function that returns OpenGL version as string (e.g., "4.6.0")
- **`glGetString(GL_SHADING_LANGUAGE_VERSION)`**: Returns GLSL version (e.g., "4.60")
- **`logger.Infof()`**: Prints formatted message like: `[OpenGL] OpenGL 4.6.0  GLSL 4.60`

#### Line 35: Success
```cpp
return true;
```
- Everything loaded successfully

---

## Lines 38-116: nkmain Function

```cpp
int nkmain(const nkentseu::NkEntryState& /*state*/) {
```

### Entry Point

This is **your application's main function**. Nkentseu provides `nkmain()` instead of standard `main()` so it can:
1. Initialize Nkentseu systems first
2. Call your `nkmain()` next
3. Shutdown cleanly

The `/*state*/` parameter:
- Type: `NkEntryState` (initialization information)
- Marked as unused with `/*...*/` comment (prevents "unused parameter" warnings)

---

## Lines 39-51: Step 1 - Create Window

```cpp
    // Step 1: Create window
    NkWindowConfig cfg;
    cfg.title = "My OpenGL App";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;

    NkWindow window;
    if (!window.Create(cfg)) {
        logger.Error("Failed to create window");
        return -2;
    }
```

### What This Does

**Creates a native OS window** (on Windows: HWND, on Linux: X11 window, on macOS: NSWindow).

### Line-by-Line

#### Lines 40-45: Configure Window
```cpp
NkWindowConfig cfg;           // Create configuration structure
cfg.title = "My OpenGL App";   // Window title (shown in title bar)
cfg.width = 1280;              // Window width in pixels
cfg.height = 720;              // Window height in pixels
cfg.centered = true;           // Center window on screen
cfg.resizable = true;          // Allow user to resize window
```

#### Lines 47-51: Create and Check
```cpp
NkWindow window;               // Declare window object
if (!window.Create(cfg)) {    // Create window with config
    logger.Error("Failed to create window");  // If failed, log error
    return -2;                 // Return error code (-2)
}
```

### Important: What Does "Create Window" Mean?

**Does NOT create an OpenGL context yet!** Only creates:
- Native OS window (HWND, X11 window, NSWindow)
- Window title, size, position
- Event handling system

**OpenGL context comes next!**

---

## Lines 53-73: Step 2-3 - Configure & Create Graphics Context

```cpp
    // Step 2: Configure OpenGL context
    NkContextDesc desc;
    desc.api    = NkGraphicsApi::NK_API_OPENGL;
    desc.opengl.majorVersion = 4;
    desc.opengl.minorVersion = 6;
    desc.opengl.profile = NkGLProfile::Core;
    desc.opengl.contextFlags = NkGLContextFlags::ForwardCompat | NkGLContextFlags::Debug;
    desc.opengl.runtime.installDebugCallback = true;
    desc.opengl.msaaSamples        = 4;
    desc.opengl.srgbFramebuffer    = true;
    desc.opengl.swapInterval       = NkGLSwapInterval::AdaptiveVSync;
    desc.opengl.runtime.autoLoadEntryPoints  = true;
    desc.opengl.runtime.validateVersion      = true;

    // Step 3: Create graphics context
    auto ctx = NkContextFactory::Create(window, desc);
    if (!ctx) { 
        logger.Error("[OpenGL] Context creation failed"); 
        window.Close(); 
        return -3; 
    }
```

### What This Does

**Specifies exactly what kind of OpenGL context you want, then creates it.**

This is where the **magic happens** for graphics initialization!

### Line-by-Line Breakdown

#### Line 54: Create Configuration Structure
```cpp
NkContextDesc desc;
```
- Create a structure that describes your graphics context

#### Lines 55-65: Configure OpenGL Settings

| Line | What It Does | Explanation |
|------|--------------|-------------|
| `desc.api = NK_API_OPENGL` | Select graphics API | Telling Nkentseu "I want OpenGL, not DirectX or Metal" |
| `majorVersion = 4` | OpenGL major version | OpenGL 4.x (4.0, 4.1, 4.6, etc.) |
| `minorVersion = 6` | OpenGL minor version | OpenGL 4.6 (latest stable as of 2024) |
| `profile = Core` | OpenGL profile | **Core Profile**: Modern OpenGL (required for OpenGL 4.6). No legacy `glBegin()`/`glEnd()` |
| `contextFlags = ForwardCompat \| Debug` | Context flags | **ForwardCompat**: Remove deprecated features. **Debug**: Enable error checking |
| `installDebugCallback = true` | Debug output | Enable GPU error messages in your console |
| `msaaSamples = 4` | Anti-aliasing | 4x MSAA smooths jagged edges (more = slower) |
| `srgbFramebuffer = true` | Color space | sRGB framebuffer for correct color handling |
| `swapInterval = AdaptiveVSync` | Vsync mode | Adaptive VSync: sync to monitor unless FPS drops below 60Hz |
| `autoLoadEntryPoints = true` | Auto-loading | Automatically load entry points (prepare for GLAD) |
| `validateVersion = true` | Version validation | Check that requested OpenGL version is available |

**In Plain English**: "Create me an OpenGL 4.6 Core context with debug mode and 4x anti-aliasing"

#### Lines 68-73: Create Context
```cpp
auto ctx = NkContextFactory::Create(window, desc);
if (!ctx) { 
    logger.Error("[OpenGL] Context creation failed"); 
    window.Close(); 
    return -3; 
}
```
- **`NkContextFactory::Create()`**: Uses the Factory pattern to create a graphics context
- **Takes**: window and configuration description
- **Returns**: Graphics context object (or nullptr if failed)
- **Stores in**: `ctx` variable
- **Error check**: If nullptr, log error, close window, return error code -3

### At This Point

✅ Native OS window exists  
✅ OpenGL context created and made current  
❌ OpenGL functions not yet loaded (that's next!)

---

## Lines 75-80: Step 4 - Load OpenGL Functions

```cpp
    // Step 4: Load OpenGL functions
    if (!LoadGL(ctx)) { 
        ctx->Shutdown(); 
        window.Close(); 
        return -4; 
    }
```

### What This Does

**Calls the `LoadGL()` function** we defined earlier (lines 19-36).

- **If LoadGL returns true**: Continue
- **If LoadGL returns false**: 
  - `ctx->Shutdown()`: Cleanup graphics context
  - `window.Close()`: Close window
  - `return -4`: Exit with error code

### Why Shutdown Before Closing Window?

**Order matters!** Must shutdown graphics context **before** closing window because the graphics context owns resources on the window.

---

## Lines 82-84: Step 5 - Initialize OpenGL State

```cpp
    // Step 5: Initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
```

### What This Does

**Sets initial OpenGL state** before rendering.

#### `glEnable(GL_DEPTH_TEST)`

Enables **depth testing** (Z-buffering).

**What is depth testing?**

Without it: Objects drawn last always appear on top (wrong!)

```
Draw cube (z=5)    → Appears on top
Draw sphere (z=1)  → Appears on top (overwrites cube!)
```

**With depth testing**: Objects closer to camera appear on top (correct!)

```
Draw cube (z=5)    → Behind sphere (drawn first)
Draw sphere (z=1)  → In front of cube (closer to camera, wins depth test)
```

**Technical Details**: When you render a pixel, OpenGL checks: "Is this pixel's Z value closer than what's already drawn?" If yes, draw it. If no, skip it.

#### `glClearColor(0.1f, 0.1f, 0.15f, 1.0f)`

Sets the **background color** (what screen looks like when cleared).

- **Parameters**: (Red, Green, Blue, Alpha)
- **Values**: 0.0 to 1.0 (0.1 = ~25 out of 255)
- **Result**: Dark blue-ish color `(25, 25, 38)` in RGB 0-255

Visual: Your window will have a dark blue background.

---

## Lines 86-109: Step 6 - Main Game Loop

```cpp
    // Step 6: Main loop
    bool running = true;
    auto& events = NkEvents();

    while (running) {
        while (NkEvent* ev = events.PollEvent()) {
            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) break;

        if (ctx->BeginFrame()) {
            glViewport(0, 0, window.GetSize().width, window.GetSize().height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Your rendering code here
            
            ctx->EndFrame();
            ctx->Present();
        }
    }
```

### What This Does

**The main game loop** - runs every frame until window closes.

### Line-by-Line Breakdown

#### Line 87: Running Flag
```cpp
bool running = true;
```
- Flag to control loop: loop continues while `running == true`
- Set to `false` when user closes window

#### Line 88: Get Event System
```cpp
auto& events = NkEvents();
```
- **`NkEvents()`**: Returns reference to global event system
- **Events**: Window close, mouse click, keyboard press, etc.
- **`&`**: Reference (direct access, not a copy)

#### Lines 90-96: Event Polling Loop
```cpp
while (running) {
    while (NkEvent* ev = events.PollEvent()) {
        if (ev->Is<NkWindowCloseEvent>()) {
            running = false;
            break;
        }
    }
```

**Inner loop**: Process all pending events this frame

- **`events.PollEvent()`**: Gets next event from queue (returns nullptr if no more events)
- **`while (NkEvent* ev = ...)`**: Continues until `PollEvent()` returns nullptr
- **`ev->Is<NkWindowCloseEvent>()`**: Is this event a "window close" event?
- **If yes**: Set `running = false` and break from event loop

**In Plain English**: "Check all events this frame. If user closed window, stop running."

#### Lines 98: Double Exit Check
```cpp
if (!running) break;
```
- If we set `running = false`, break from outer loop too
- (Prevents trying to render after deciding to quit)

#### Lines 100-108: Render Frame
```cpp
if (ctx->BeginFrame()) {
    glViewport(0, 0, window.GetSize().width, window.GetSize().height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Your rendering code here
    
    ctx->EndFrame();
    ctx->Present();
}
```

**This is where rendering happens!**

##### `ctx->BeginFrame()`
- **Prepares** graphics context for this frame
- Returns `true` if successful, `false` if not

##### `glViewport(0, 0, window.GetSize().width, window.GetSize().height)`
- Sets the **rendering viewport** (which part of window to render to)
- Parameters: `(x, y, width, height)`
- `(0, 0)`: Start at top-left
- `(window.GetSize().width, height)`: Use full window size
- **Effect**: Render fills entire window

##### `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)`
- **Clears** (erases) the screen
- **`GL_COLOR_BUFFER_BIT`**: Clear color (erase pixels to `glClearColor`)
- **`GL_DEPTH_BUFFER_BIT`**: Clear depth (reset Z-buffer)
- **`|`**: Bitwise OR (clear both)
- **Effect**: Screen becomes the dark blue color we set earlier

##### Comment: `// Your rendering code here`
- This is where **you draw your graphics**
- Currently empty (just shows empty window)

##### `ctx->EndFrame()`
- **Finishes** rendering for this frame
- Does cleanup/setup for next frame

##### `ctx->Present()`
- **Swaps buffers**: Shows the frame you just rendered
- **How buffers work**:
  - You render to **back buffer** (hidden)
  - `Present()` **swaps** front and back
  - Front buffer now shows on screen
  - This prevents flickering (user never sees partially-rendered frame)

---

## Lines 111-113: Step 7 - Cleanup

```cpp
    // Step 7: Cleanup
    ctx->Shutdown();
    window.Close();

    return 0;
}
```

### What This Does

**Cleans up resources** before exiting.

#### `ctx->Shutdown()`
- Frees graphics context
- Deletes GPU objects
- Closes OpenGL connection

#### `window.Close()`
- Closes the OS window
- Frees window resources

#### `return 0`
- Returns 0 (success code)

### Importance of Cleanup Order

```
Creation:        Cleanup:
1. Window        1. Graphics Context (depends on window)
2. Context       2. Window
3. Load GL       (GL unloads automatically with context)
```

**Rule**: Destroy in **reverse order** of creation!

---

## Summary: The Full Flow

```
1. Include necessary headers (OpenGL, window, graphics libs)
2. Create native OS window
   ↓ (window exists, no OpenGL yet)
3. Describe OpenGL context settings
4. Create OpenGL context
   ↓ (context exists, but OpenGL functions not loaded)
5. Load OpenGL functions with GLAD
   ↓ (now you can call glClear(), glDrawArrays(), etc.)
6. Set initial OpenGL state (depth test, clear color)
7. Enter main loop:
   a. Poll events (check if user closed window)
   b. BeginFrame()
   c. Clear screen
   d. [Your rendering code goes here]
   e. EndFrame()
   f. Present() (show frame on screen)
8. Shutdown context and window
9. Return success
```

---

## Answer to Your Question: Does This Window Have OpenGL Context?

**YES!** Absolutely.

**Proof:**

1. **Line 55**: `desc.api = NK_API_OPENGL` — Explicitly request OpenGL
2. **Lines 56-57**: Specify OpenGL 4.6 Core Profile
3. **Line 68**: `NkContextFactory::Create(window, desc)` — Creates the context
4. **Lines 27-34**: `gladLoadGL()` — Loads OpenGL functions
5. **Line 83**: `glEnable(GL_DEPTH_TEST)` — You're calling OpenGL functions!

If there were no context, `glEnable()` would crash.

**The window displays a dark blue background** because:
- OpenGL context exists and is rendering
- Each frame: `glClear()` fills the window with the color set by `glClearColor()`
- No geometry is drawn (empty scene), so just the background appears

---

**Next**: Read `03_HOW_TO_EXTEND.md` to learn how to add graphics to this window!
