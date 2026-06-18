# Nkentseu API Reference

This document explains the key Nkentseu classes and functions used in `main.cpp`. Think of this as a reference manual for the APIs you're using.

---

## NkWindow - Window Management

### What It Is
Platform-independent window creation and management. Nkentseu creates native windows (HWND on Windows, X11 on Linux, NSWindow on macOS).

### Key Members

#### `NkWindowConfig` - Configuration Structure

```cpp
struct NkWindowConfig {
    const char* title;           // Window title (shown in title bar)
    uint32_t width;              // Width in pixels
    uint32_t height;             // Height in pixels
    bool centered;               // Center on screen?
    bool resizable;              // Allow user to resize?
    bool decorated;              // Show window decorations (title bar)?
    bool fullscreen;             // Fullscreen mode?
    uint32_t monitor;            // Monitor index (for multi-monitor)
    void* userData;              // Custom data pointer
};
```

#### `NkWindow::Create(const NkWindowConfig& config)` - Constructor

```cpp
NkWindow window;
NkWindowConfig cfg;
cfg.title = "My App";
cfg.width = 1280;
cfg.height = 720;
cfg.centered = true;
cfg.resizable = true;

bool success = window.Create(cfg);
if (!success) {
    // Window creation failed
}
```

**Returns**: `true` if window created successfully, `false` otherwise

#### `NkWindow::Close()` - Destructor

```cpp
window.Close();  // Close the window and free resources
```

#### `NkWindow::GetSize()` - Get Window Dimensions

```cpp
auto size = window.GetSize();
logger.Infof("Window: %d x %d\n", size.width, size.height);
```

**Returns**: Structure with `width` and `height` members

#### `NkWindow::GetTitle()` - Get Window Title

```cpp
const char* title = window.GetTitle();
```

#### `NkWindow::SetTitle()` - Change Window Title

```cpp
window.SetTitle("New Title");
```

#### `NkWindow::IsAlive()` - Check if Window is Open

```cpp
if (window.IsAlive()) {
    // Window is still open
}
```

#### `NkWindow::Maximize() / Minimize() / Restore()`

```cpp
window.Maximize();   // Maximize window
window.Minimize();   // Minimize to taskbar
window.Restore();    // Restore from minimized
```

#### `NkWindow::SetResizable(bool)`

```cpp
window.SetResizable(true);   // Allow resizing
window.SetResizable(false);  // Lock size
```

---

## NkContextDesc & NkContextFactory - Graphics Context Setup

### What It Is
Configuration and creation of graphics contexts. This is where you specify OpenGL version, settings, and create the actual context.

### NkContextDesc - Configuration Structure

```cpp
struct NkContextDesc {
    NkGraphicsApi api;              // API type (OpenGL, DirectX, Metal, Software)
    
    // OpenGL-specific settings
    struct {
        uint32_t majorVersion;      // Major version (4 for GL 4.6)
        uint32_t minorVersion;      // Minor version (6 for GL 4.6)
        NkGLProfile profile;        // Core or Compatibility profile
        uint32_t contextFlags;      // Flags (ForwardCompat, Debug)
        uint32_t msaaSamples;       // 0=off, 2/4/8/16 samples
        bool srgbFramebuffer;       // sRGB color space?
        NkGLSwapInterval swapInterval;  // VSync mode
        
        // Runtime settings
        struct {
            bool autoLoadEntryPoints;      // Auto-load entry points?
            bool installDebugCallback;     // Enable debug output?
            bool validateVersion;          // Validate version exists?
        } runtime;
    } opengl;
};
```

### NkGraphicsApi - API Selection

```cpp
enum class NkGraphicsApi {
    NK_API_OPENGL,      // OpenGL (your choice)
    NK_API_DIRECTX_11,  // DirectX 11 (Windows only)
    NK_API_DIRECTX_12,  // DirectX 12 (Windows only)
    NK_API_METAL,       // Metal (macOS only)
    NK_API_SOFTWARE,    // Software rendering (CPU)
};
```

### NkGLProfile - OpenGL Profile

```cpp
enum class NkGLProfile {
    Compatibility,  // Legacy profile (supports old glBegin/glEnd)
    Core,          // Modern profile (only shader-based, GL 3.2+)
};
```

For modern graphics programming, always use **Core**. Compatibility is deprecated.

### NkGLContextFlags - Context Flags

```cpp
enum class NkGLContextFlags {
    ForwardCompat = 0x0001,  // Remove deprecated features
    Debug         = 0x0002,  // Enable error checking
};
```

**ForwardCompat**: Tells OpenGL driver "don't support old features like `glBegin()`"

**Debug**: Enables GPU error reporting to console (slower but helps debugging)

### NkGLSwapInterval - VSync Mode

```cpp
enum class NkGLSwapInterval {
    NoSync,          // No VSync (unlimited FPS, screen tearing)
    VSync,           // Sync to monitor (60 FPS on 60Hz monitor)
    AdaptiveVSync,   // VSync unless FPS < 60Hz (smooth but responsive)
};
```

### Example Configuration - Full Breakdown

```cpp
NkContextDesc desc;

// Select OpenGL
desc.api = NkGraphicsApi::NK_API_OPENGL;

// OpenGL 4.6
desc.opengl.majorVersion = 4;
desc.opengl.minorVersion = 6;

// Modern profile only
desc.opengl.profile = NkGLProfile::Core;

// Forward compatible (no legacy) + Debug mode
desc.opengl.contextFlags = NkGLContextFlags::ForwardCompat | NkGLContextFlags::Debug;

// Anti-aliasing
desc.opengl.msaaSamples = 4;  // 4x MSAA

// sRGB color space (correct color handling)
desc.opengl.srgbFramebuffer = true;

// VSync settings
desc.opengl.swapInterval = NkGLSwapInterval::AdaptiveVSync;

// Auto-setup
desc.opengl.runtime.autoLoadEntryPoints = true;
desc.opengl.runtime.installDebugCallback = true;
desc.opengl.runtime.validateVersion = true;
```

### NkContextFactory::Create() - Create Graphics Context

```cpp
auto ctx = NkContextFactory::Create(window, desc);

if (!ctx) {
    logger.Error("Context creation failed");
    // Handle error
} else {
    // ctx is a NkIGraphicsContext*
}
```

**Takes**: Window + Configuration
**Returns**: `NkIGraphicsContext*` (or nullptr if failed)

---

## NkIGraphicsContext - Graphics Context Interface

### What It Is
The actual graphics context object. Controls rendering to window.

### Key Methods

#### `BeginFrame()` - Prepare for Rendering

```cpp
if (ctx->BeginFrame()) {
    // Safe to render this frame
    glClear(...);
    // Draw code here
    ctx->EndFrame();
    ctx->Present();
} else {
    // Failed to begin frame (window minimized, etc.)
}
```

**Returns**: `true` if frame started successfully

**When it fails**: Window is minimized, being moved, or other OS interference

#### `EndFrame()` - Finish Rendering

```cpp
ctx->EndFrame();
```

Cleans up frame state, resets some settings, prepares for next frame.

#### `Present()` - Show Frame on Screen

```cpp
ctx->Present();  // Swap front/back buffers, show rendered frame
```

**This is what makes the window update!** Without it, users see nothing.

#### `Shutdown()` - Clean Up

```cpp
ctx->Shutdown();  // Free graphics resources
```

Must be called before closing window.

#### `MakeCurrent()` - Activate This Context (Advanced)

```cpp
ctx->MakeCurrent();  // Make this context the active OpenGL context
```

Only needed if using multiple contexts or threads.

#### `SwapBuffers()` - Manual Buffer Swap (Advanced)

```cpp
ctx->SwapBuffers();  // Manually swap buffers
```

Usually you use `Present()` instead.

---

## NkNativeContext - Platform-Specific Access

### What It Is
Provides access to platform-specific graphics data. Used to integrate with GLAD.

### NkNativeContext::GetOpenGLProcAddressLoader()

```cpp
auto loader = NkNativeContext::GetOpenGLProcAddressLoader(ctx);
```

**What it does**: Gets a function that loads OpenGL function pointers

**Platform differences (Nkentseu handles this):**
- **Windows**: Returns `wglGetProcAddress`
- **Linux**: Returns `glXGetProcAddress`
- **macOS**: Returns `dlsym`

**Why needed**: GLAD needs a way to load OpenGL functions. This provides that way.

---

## NkEvents - Event System

### What It Is
Event polling system for window and input events.

### NkEvents() - Get Global Event System

```cpp
auto& events = NkEvents();
```

Returns reference to global event system.

### Events::PollEvent() - Get Next Event

```cpp
while (NkEvent* ev = events.PollEvent()) {
    // Process event
    
    if (ev->Is<NkWindowCloseEvent>()) {
        // User clicked close button
    }
}
```

**Returns**: Next event in queue, or `nullptr` if no more events

**Important**: Returns `nullptr` when queue is empty (not when end reached)

### Common Events

#### NkWindowCloseEvent

```cpp
if (ev->Is<NkWindowCloseEvent>()) {
    // Window close button clicked
    running = false;
}
```

#### NkWindowResizeEvent

```cpp
if (ev->Is<NkWindowResizeEvent>()) {
    auto& resize = ev->As<NkWindowResizeEvent>();
    uint32_t newWidth = resize.newWidth;
    uint32_t newHeight = resize.newHeight;
    
    // Update your projection matrix
}
```

#### NkKeyEvent

```cpp
if (ev->Is<NkKeyEvent>()) {
    auto& key = ev->As<NkKeyEvent>();
    
    if (key.key == NK_KEY_ESCAPE) {
        running = false;
    }
    
    if (key.action == NK_KEY_DOWN) {
        // Key pressed
    } else if (key.action == NK_KEY_UP) {
        // Key released
    }
}
```

#### NkMouseEvent

```cpp
if (ev->Is<NkMouseEvent>()) {
    auto& mouse = ev->As<NkMouseEvent>();
    
    float x = mouse.x;
    float y = mouse.y;
    
    if (mouse.button == NK_MOUSE_BUTTON_LEFT) {
        // Left click
    }
}
```

#### NkMouseScrollEvent

```cpp
if (ev->Is<NkMouseScrollEvent>()) {
    auto& scroll = ev->As<NkMouseScrollEvent>();
    float delta = scroll.offsetY;  // + = scroll up, - = scroll down
}
```

### Event Type Checking

```cpp
// Method 1: Check before accessing
if (ev->Is<NkWindowCloseEvent>()) {
    auto& close_ev = ev->As<NkWindowCloseEvent>();
}

// Method 2: Get type directly
if (ev->GetTypeID() == NkWindowCloseEvent::GetStaticTypeID()) {
    // ...
}
```

---

## GLAD - OpenGL Function Loader

### What It Is
Loads OpenGL function pointers so you can call OpenGL functions.

### gladLoadGL()

```cpp
int ver = gladLoadGL((GLADloadfunc)loader);
```

**Parameters**:
- `loader`: Function that loads individual OpenGL functions (provided by platform)

**Returns**:
- `0`: Loading failed
- Non-zero: OpenGL version (46 for GL 4.6, 41 for GL 4.1, etc.)

**What it does**: Loads ALL OpenGL functions:
- `glClear`
- `glDrawArrays`
- `glBindTexture`
- ... (hundreds more)

### After gladLoadGL() Succeeds

All OpenGL functions become available:

```cpp
glClearColor(0.1f, 0.1f, 0.15f, 1.0f);  // Now valid
glClear(GL_COLOR_BUFFER_BIT);             // Now valid
glViewport(0, 0, width, height);          // Now valid
```

### OpenGL Function Categories

| Category | Examples | Purpose |
|----------|----------|---------|
| **Vertex** | `glVertexAttribPointer`, `glBindBuffer` | Set up geometry data |
| **Shader** | `glCreateShader`, `glCompileShader` | Compile GPU programs |
| **Texture** | `glBindTexture`, `glTexImage2D` | Load/manage images |
| **Draw** | `glDrawArrays`, `glDrawElements` | Render geometry |
| **State** | `glEnable`, `glDisable` | Enable/disable features |
| **Query** | `glGetString`, `glGetError` | Query GPU info |

---

## NkLogger - Logging System

### What It Is
Built-in logging system for debug output.

### logger.Info()

```cpp
logger.Info("[OpenGL] Function loader not found\n");
```

Info-level message (least severe).

### logger.Error()

```cpp
logger.Error("Failed to create window");
```

Error-level message (more severe).

### logger.Infof() / logger.Errorf()

```cpp
logger.Infof("[OpenGL] OpenGL %s  GLSL %s\n", 
    glGetString(GL_VERSION), 
    glGetString(GL_SHADING_LANGUAGE_VERSION));
```

Formatted messages (like `printf`).

### Log Levels (in order of severity)

```
1. Debug    - Very detailed, only during development
2. Info    - General information
3. Warning - Something unexpected but non-fatal
4. Error   - Something failed
5. Fatal   - Unrecoverable error
```

### Common Logging Patterns

```cpp
// Initialization success
logger.Infof("Created window: %dx%d\n", width, height);

// Error with context
logger.Error("OpenGL context creation failed");

// Detailed debug info
logger.Infof("Loaded %d vertices\n", vertexCount);

// Performance warning
logger.Warn("Rendering took %f ms\n", deltaTime * 1000);
```

---

## Complete Example: Minimal Setup

```cpp
#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"
#include "NKGlad/include/glad/gl.h"
#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkNativeContextAccess.h"

using namespace nkentseu;

static bool LoadGL(NkIGraphicsContext* ctx) {
    auto loader = NkNativeContext::GetOpenGLProcAddressLoader(ctx);
    if (!loader) return false;
    
    int ver = gladLoadGL((GLADloadfunc)loader);
    return ver != 0;
}

int nkmain(const NkEntryState& /*state*/) {
    // 1. Create window
    NkWindowConfig cfg;
    cfg.title = "Hello Graphics";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;
    
    NkWindow window;
    if (!window.Create(cfg)) return -1;
    
    // 2. Create OpenGL context
    NkContextDesc desc;
    desc.api = NkGraphicsApi::NK_API_OPENGL;
    desc.opengl.majorVersion = 4;
    desc.opengl.minorVersion = 6;
    desc.opengl.profile = NkGLProfile::Core;
    desc.opengl.runtime.autoLoadEntryPoints = true;
    
    auto ctx = NkContextFactory::Create(window, desc);
    if (!ctx) return -2;
    
    // 3. Load OpenGL functions
    if (!LoadGL(ctx)) return -3;
    
    // 4. Set OpenGL state
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // 5. Main loop
    bool running = true;
    while (running) {
        while (NkEvent* ev = NkEvents().PollEvent()) {
            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
            }
        }
        
        if (ctx->BeginFrame()) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Your drawing code here
            
            ctx->EndFrame();
            ctx->Present();
        }
    }
    
    // 6. Cleanup
    ctx->Shutdown();
    window.Close();
    
    return 0;
}
```

---

**Next**: Read `05_OPENGL_FUNDAMENTALS.md` to understand graphics concepts.
