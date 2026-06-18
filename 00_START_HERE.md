# Complete Learning Guide: Nkentseu + OpenGL Graphics Programming

## Overview

This guide teaches you how to use **Nkentseu** and **Jenga** to build OpenGL graphics applications. Your project creates an **empty window with an OpenGL 4.6 Core context**, ready for graphics programming.

### What You Have

✅ Window creation (cross-platform via Nkentseu)  
✅ OpenGL 4.6 Core context with debug mode  
✅ GLAD function loader automatically loading OpenGL  
✅ Event system for handling window/input events  
✅ Main loop with frame timing  
✅ Clean architecture ready to extend  

### What You're Missing (Yet)

- Drawing geometry (triangles, meshes)
- Shaders (vertex/fragment programs)
- Textures and images
- Lighting models
- Advanced rendering techniques

**This guide teaches all of it.**

---

## Document Organization

### 📘 01_PROJECT_OVERVIEW.md
**Start here to understand the big picture**

Covers:
- Project folder structure
- Role of each library (Nkentseu, GLAD, Jenga)
- Technology stack
- How everything connects

**Read time**: 5 minutes  
**Best for**: Understanding what you have and why

---

### 📘 02_MAIN_CPP_LINE_BY_LINE.md
**The most detailed explanation of your code**

Explains every single line in `main.cpp`:
- Include statements and why each is needed
- `LoadGL()` function - how OpenGL function loading works
- Window creation
- OpenGL context configuration
- Main loop structure
- Event handling
- Rendering and presenting

**Read time**: 30 minutes  
**Best for**: Deep understanding of current code  
**Reading order**: After 01_PROJECT_OVERVIEW.md

**Key takeaway**: Your window DOES have an OpenGL context and it works!

---

### 📘 03_HOW_TO_EXTEND.md
**The practical guide to adding graphics**

Teaches:
- Architecture for growing a graphics engine
- How to create shader classes
- How to create mesh classes
- Complete example: drawing your first triangle
- The graphics pipeline explained
- The three critical transformation matrices
- File organization for a real graphics engine

**Read time**: 30 minutes  
**Best for**: Learning what to code next  
**Reading order**: After 02_MAIN_CPP_LINE_BY_LINE.md

**Key takeaway**: You're ready to build on top of this foundation!

---

### 📘 04_NKENTSEU_API_REFERENCE.md
**API documentation for Nkentseu classes**

Reference for:
- `NkWindow` - Window creation and management
- `NkContextDesc` - OpenGL configuration
- `NkContextFactory` - Create graphics contexts
- `NkIGraphicsContext` - Control rendering
- `NkNativeContext` - Platform-specific access
- `NkEvents` - Event system
- `GLAD` - OpenGL function loader
- `NkLogger` - Logging system

**Read time**: 20 minutes (reference material, read as needed)  
**Best for**: Looking up specific class/function usage  
**Reading order**: Refer to during development

**Key takeaway**: Know what APIs are available when you need them

---

### 📘 05_OPENGL_FUNDAMENTALS.md
**Graphics programming concepts explained**

Covers:
- What OpenGL is and why use it
- The GPU rendering pipeline
- Buffers (color, depth, framebuffer)
- Shaders (vertex, fragment, GLSL language)
- Vertex attributes and buffers
- Matrix transformations (model, view, projection)
- Textures and UV coordinates
- Lighting (Phong, diffuse, specular)
- Depth testing
- Common gotchas and solutions

**Read time**: 45 minutes  
**Best for**: Understanding graphics concepts  
**Reading order**: Before starting graphics implementation

**Key takeaway**: Graphics programming has underlying concepts that repeat everywhere

---

### 📘 06_BUILD_AND_RUN.md
**How to compile and run your project**

Covers:
- Jenga build system overview
- Requirements (Python, Clang, MinGW)
- Building from command line
- Troubleshooting compilation issues
- Customizing build configuration
- IDE integration (Visual Studio, Xcode, etc.)
- Performance tuning

**Read time**: 15 minutes  
**Best for**: Getting your code to compile  
**Reading order**: When you're ready to build

**Key takeaway**: Jenga handles all the build complexity for you

---

## Recommended Reading Order

### Phase 1: Understanding Your Current Code (60 minutes)
```
1. 01_PROJECT_OVERVIEW.md (5 min)
   ↓ [Understand structure]
2. 02_MAIN_CPP_LINE_BY_LINE.md (30 min)
   ↓ [Detailed explanation]
3. 06_BUILD_AND_RUN.md (15 min)
   ↓ [Build and verify it works]
→ You can now compile and see the empty window!
```

### Phase 2: Graphics Foundations (90 minutes)
```
1. 05_OPENGL_FUNDAMENTALS.md (45 min)
   ↓ [Learn concepts]
2. 04_NKENTSEU_API_REFERENCE.md (20 min, skim)
   ↓ [Know what's available]
3. 03_HOW_TO_EXTEND.md (25 min)
   ↓ [See examples]
→ Ready to start graphics programming!
```

### Phase 3: Implementation (Your PDF Tutorial)
```
Use your "Mastering OpenGL" PDF as your primary guide:
- Adapt GLFW examples to Nkentseu (usually 1:1 mapping)
- Use this guide when you're stuck or confused
- Refer to 05_OPENGL_FUNDAMENTALS.md for concept explanations
```

---

## Quick Answer: Does Your Window Have OpenGL?

**YES. ABSOLUTELY.**

**Evidence**:
1. Line 55 of main.cpp: `desc.api = NkGraphicsApi::NK_API_OPENGL`
2. Lines 56-57: Requesting OpenGL 4.6
3. Line 68: `NkContextFactory::Create(window, desc)` creates the context
4. Lines 27-34: `gladLoadGL()` loads OpenGL functions
5. Line 83: `glEnable(GL_DEPTH_TEST)` - you're calling OpenGL!

If there were no context, calling `glEnable()` would crash. It doesn't crash, so it works!

**What you see**: Dark blue window
**Why**: `glClearColor(0.1f, 0.1f, 0.15f, 1.0f)` sets background to dark blue, `glClear()` fills the screen with it every frame.

**No geometry is drawn** because the rendering section is empty. That's by design—you'll add geometry next!

---

## Key Concepts Map

### The Setup (What You Have)
```
Window (Native OS)
    ↓
OpenGL Context (GPU Connection)
    ↓
GLAD (Function Loader)
    ↓
OpenGL API (Ready to use!)
```

### The Pipeline (What Happens When You Render)
```
Your Vertex Data
    ↓ [Vertex Shader]
Screen Positions
    ↓ [Rasterization]
Pixels (Fragments)
    ↓ [Fragment Shader]
Colors
    ↓ [Output Merger]
Screen Display
```

### The Rendering Loop (Every Frame)
```
1. Poll Events (user input, window close)
2. BeginFrame() (GPU sync point)
3. Clear Screen (erase old frame)
4. Your Drawing Code (render geometry)
5. EndFrame() (GPU cleanup)
6. Present() (show frame on screen)
→ Repeat
```

### The Three Matrices (Core of 3D Graphics)
```
Model Matrix     → Where is object? (position, rotation, scale)
View Matrix      → Where is camera? (camera position, look direction)
Projection Matrix → How do we see? (perspective, aspect ratio)

gl_Position = Projection × View × Model × VertexPosition
```

---

## Glossary

| Term | Meaning |
|------|---------|
| **VAO** | Vertex Array Object - stores vertex format |
| **VBO** | Vertex Buffer Object - GPU memory for vertex data |
| **EBO** | Element Buffer Object - GPU memory for indices |
| **Shader** | Program that runs on GPU (vertex, fragment, etc.) |
| **Uniform** | Constant value passed to all shader invocations |
| **Attribute** | Per-vertex data (position, color, texture coordinates) |
| **Framebuffer** | Where rendering outputs to (color + depth buffers) |
| **GLAD** | OpenGL Loader - loads OpenGL function pointers |
| **Nkentseu** | Graphics/game framework with multi-API support |
| **Jenga** | Python-based build system (like CMake or Premake) |
| **Pipeline** | Stages GPU goes through to render (vertex → fragment → output) |
| **Rasterization** | Converting triangles to pixels |
| **Depth Testing** | Drawing closer objects on top |
| **MSAA** | Multi-Sample Anti-Aliasing - smooths jagged edges |
| **sRGB** | Color space standard - correct gamma handling |
| **VSync** | Vertical Sync - sync rendering to monitor refresh rate |

---

## Troubleshooting Matrix

| Problem | Likely Cause | Solution |
|---------|--------------|----------|
| Empty window | Working as intended | You haven't added geometry yet—read 03_HOW_TO_EXTEND.md |
| Window won't open | NkWindow::Create() failed | Check window config, verify no OS window conflicts |
| "Cannot find OpenGL functions" | LoadGL() failed | Check OpenGL context created, verify graphics drivers |
| "glClear is undefined" | GLAD didn't load | Ensure gladLoadGL() returns non-zero |
| Project won't compile | Missing headers/libraries | Run `jenga generate` to regenerate build files |
| Triangle doesn't appear | Many possible causes | Check: VAO bound, shader compiled, matrices correct, viewport set |
| Wrong colors | sRGB mismatch | Enable `desc.opengl.srgbFramebuffer = true` |
| Flickering | Missing double buffering | You have it! Using Present() correctly |

---

## Next Steps After Reading

### Immediate (Next 1-2 Hours)
- [ ] Read 01_PROJECT_OVERVIEW.md
- [ ] Read 02_MAIN_CPP_LINE_BY_LINE.md
- [ ] Build your project (06_BUILD_AND_RUN.md)
- [ ] Verify the empty window works

### Short Term (Next 1-2 Days)
- [ ] Read 05_OPENGL_FUNDAMENTALS.md
- [ ] Read 03_HOW_TO_EXTEND.md
- [ ] Draw your first triangle
- [ ] Render a simple mesh

### Medium Term (Next 1-2 Weeks)
- [ ] Implement shader system
- [ ] Add camera controls
- [ ] Load 3D models
- [ ] Apply textures

### Long Term (Ongoing)
- [ ] Follow your Mastering OpenGL PDF tutorial
- [ ] Implement lighting
- [ ] Build a rendering engine
- [ ] Create a game or graphics demo

---

## Connecting to Your PDF Tutorial

Your "Mastering OpenGL" PDF probably covers the same topics, potentially with different tools (GLFW, SFML). Here's how to adapt:

| PDF Concept | Your Implementation |
|-------------|-------------------|
| glfwCreateWindow() | NkWindow::Create() |
| glfwMakeContextCurrent() | NkContextFactory::Create() |
| glfwGetProcAddress() | NkNativeContext::GetOpenGLProcAddressLoader() |
| gladLoadGL() | Same! (you have GLAD loaded) |
| Event loop | NkEvents::PollEvent() |
| Window close | NkWindowCloseEvent |
| glfwSwapBuffers() | ctx->Present() |

**Key difference**: Nkentseu abstracts platform details, so you write less platform-specific code.

---

## Resources

### Official Documentation
- OpenGL Specs: https://www.khronos.org/opengl/wiki/
- GLAD Generator: https://glad.dav1d.de/
- Nkentseu: (included in your Externals/)
- Jenga: `jenga --help`

### Learning
- "Mastering OpenGL" PDF (your current resource)
- LearnOpenGL.com (excellent tutorials)
- Real-Time Rendering (advanced reference)

### Tools
- RenderDoc - GPU debugger (capture frames, inspect)
- NVIDIA Nsight - Performance profiler
- VS Code - Code editor with Clang support
- Visual Studio - Full IDE (generate project with Jenga)

---

## Final Thoughts

You have a **solid, modern graphics foundation** set up with Nkentseu. Rather than being limited compared to GLFW/SFML, you're actually ahead because:

1. **Cross-platform abstraction** - easier to port
2. **Built-in logging** - easier to debug
3. **Multi-API support** - can switch to DirectX/Metal later
4. **Data structures** - container library included
5. **Event system** - more feature-complete than GLFW

Your path forward is clear:
- Understand what you have (read this guide)
- Add graphics on top (follow 03_HOW_TO_EXTEND.md)
- Follow your PDF tutorial, adapting as needed
- Build something cool!

**You're ready. Go build graphics!**

---

## Questions?

- **"How do I draw X?"** → Read 03_HOW_TO_EXTEND.md then 05_OPENGL_FUNDAMENTALS.md
- **"What does this function do?"** → Search 04_NKENTSEU_API_REFERENCE.md
- **"Why doesn't it compile?"** → Check 06_BUILD_AND_RUN.md Troubleshooting
- **"How does OpenGL work?"** → Read 05_OPENGL_FUNDAMENTALS.md
- **"What's this line of code?"** → Find it in 02_MAIN_CPP_LINE_BY_LINE.md

**Good luck! 🚀**
