# Building and Running Your Project

This guide explains how to compile and run your Nkentseu + Jenga project.

---

## Build System Overview

Your project uses **Jenga**, a Python-based build system similar to CMake or Premake.

### Jenga Files

```
trial.wks.jenga         # Workspace file (root, ties everything together)
OpenPrj/open.prj.jenga  # Project file (your app)
Externals/libs/NKGlad/NKGlad.lib.jenga   # GLAD library
Externals/libs/NKMath/NKMath.lib.jenga   # Math library
```

### Build Flow

```
trial.wks.jenga
    ↓
Jenga reads it
    ↓
Generates build files (Makefile, Visual Studio, etc.)
    ↓
Compiler runs
    ↓
Links with Nkentseu libraries
    ↓
Executable created
    ↓
You run it!
```

---

## Requirements

Before building, ensure you have:

### 1. Python 3.8+
```bash
python --version  # Should show 3.8 or higher
```

### 2. Jenga
```bash
jenga --version
```

If not installed, install with:
```bash
pip install jenga-build
```

### 3. Compiler (Clang/MinGW)
Your `trial.wks.jenga` uses Clang with MinGW on Windows.

```bash
clang --version
clang++ --version
llvm-ar --version  # or ar.exe
```

If not installed on Windows with MSYS2:
```bash
# Install MSYS2 from https://www.msys2.org/
# Then in MSYS2 terminal:
pacman -S mingw-w64-ucrt64-clang
pacman -S mingw-w64-ucrt64-tools
```

### 4. OpenGL Development Libraries (Windows)
Usually pre-installed with Windows, but ensure:
- `opengl32.lib` available (Windows SDK)
- `windows.h` available (Windows SDK)

---

## Building From Command Line

### Step 1: Navigate to Project

```bash
cd C:\Users\colli\Desktop\FromSratch
```

### Step 2: Generate Build Files

```bash
jenga generate
```

This reads `trial.wks.jenga` and creates build files.

**Output**: Build files appear in `Build/` directory

### Step 3: Compile

**Option A: Using Make** (if Make is installed)
```bash
cd Build
make -j4  # Compile with 4 parallel jobs
```

**Option B: Using Ninja** (faster, if installed)
```bash
cd Build
ninja
```

**Option C: Manual Clang** (if other tools not available)
```bash
clang++ -std=c++20 \
  -I./Externals/libs/nkentseu/include \
  -I./Externals/libs/NKGlad/include \
  -I./OpenPrj/src \
  -L./Externals/libs/nkentseu/lib \
  -o MyOpenGLApp.exe \
  OpenPrj/src/main.cpp \
  -lNKPlatform -lNKCore -lNKMemory -lNKContainers -lNKMath \
  -lNKTime -lNKLogger -lNKThreading -lNKEvent -lNKWindow -lNKContext \
  -lNKGlad \
  -lopengl32 -luser32 -lgdi32
```

### Step 4: Run Executable

Executable is in: `Build/Bin/Debug-Windows/MyOpenGLApp/MyOpenGLApp.exe`

```bash
./Build/Bin/Debug-Windows/MyOpenGLApp/MyOpenGLApp.exe
```

Or from Windows Explorer, double-click it.

---

## Troubleshooting Build Issues

### Issue: "jenga command not found"

**Solution**: Install Jenga
```bash
pip install --upgrade jenga-build
```

Then check if it's in PATH:
```bash
jenga --version
```

### Issue: "clang++ not found"

**Solution**: Install Clang via MSYS2

1. Install MSYS2: https://www.msys2.org/
2. Open MSYS2 terminal
3. Run:
```bash
pacman -Syu                              # Update package list
pacman -S mingw-w64-ucrt64-clang         # Install Clang
pacman -S mingw-w64-ucrt64-make          # Install Make
```

4. Add to PATH:
```bash
# Add C:\msys64\ucrt64\bin to Windows PATH
# Or run MSYS2 terminal and work from there
```

### Issue: "Cannot find NKContext/Factory/NkContextFactory.h"

**Cause**: Include path wrong or library not found

**Solution**: Verify file exists
```bash
ls Externals/libs/nkentseu/include/NKContext/Factory/NkContextFactory.h
```

If not found, Nkentseu installation incomplete.

### Issue: "Linker error: undefined reference to NkWindowCreate"

**Cause**: Not linking with Nkentseu libraries

**Check** `open.prj.jenga`:
```python
links([
    "NKPlatform", "NKCore", "NKMemory", "NKContainers", "NKMath",
    "NKTime", "NKLogger", "NKThreading",
    "NKEvent", "NKWindow", "NKContext",
    "NKGlad"
])
```

And library path:
```python
libdirs([
    "%{wks.location}/Externals/libs/nkentseu/lib",
])
```

### Issue: "OpenGL function not loaded (glClear returns null)"

**Cause**: `LoadGL()` didn't complete successfully

**Check**:
1. Is OpenGL context created? (check `NkContextFactory::Create()` succeeds)
2. Is `gladLoadGL()` returning non-zero?
3. Print debug info:

```cpp
if (!LoadGL(ctx)) {
    logger.Error("LoadGL failed!");
    logger.Errorf("GLAD returned: %d\n", ver);
    return -4;
}
```

---

## Build Configuration

Your project has multiple configurations:

### Debug Build
- Optimizations: Off (slower, but easier to debug)
- Debug symbols: On (can step through code)
- Size: Larger executable

```bash
jenga generate --config=Debug
```

### Release Build
- Optimizations: On (faster)
- Debug symbols: Off (harder to debug)
- Size: Smaller executable

```bash
jenga generate --config=Release
```

### Specify Both

```bash
jenga generate --config=Debug --config=Release
```

---

## Visual Studio / IDE Integration

### Generate Visual Studio Project

```bash
jenga generate --action=vs2022
```

This creates `.sln` file you can open in Visual Studio.

### Generate Xcode Project (macOS)

```bash
jenga generate --action=xcode
```

### Generate QtCreator Project

```bash
jenga generate --action=qtcreator
```

---

## Customizing the Build

### Change C++ Dialect

Edit `OpenPrj/open.prj.jenga`:

```python
cppdialect("C++20")  # Change to C++17, C++23, etc.
```

### Add New Source Files

```python
files([
    "src/**.cpp",       # All .cpp in src/
    "src/engine/*.cpp", # All .cpp in src/engine/
])
```

### Add Include Directories

```python
includedirs([
    "src",
    "%{wks.location}/Externals/libs/nkentseu/include",
    "%{wks.location}/Externals/libs/NKGlad/include",
    "include/mylib",    # New directory
])
```

### Add Preprocessor Defines

```python
with filter("system:Windows"):
    defines([
        "WIN32_LEAN_AND_MEAN",
        "MY_CUSTOM_DEFINE=1",
        "DEBUG_GRAPHICS",
    ])
```

### Link Additional Libraries

```python
links([
    "NKContext",
    "mylib",           # Custom library
    "external_lib",
])
```

---

## Common Build Commands

```bash
# Clean build artifacts
jenga clean

# Generate build files
jenga generate

# Build (if using generated Makefile)
cd Build && make -j4 && cd ..

# Generate and build together (some configurations)
jenga generate && jenga build

# Build specific project
jenga build --project=MyOpenGLApp

# Build in Release mode
jenga generate --config=Release
cd Build && make -j4 && cd ..

# Remove all build artifacts
rm -rf Build/
```

---

## Running Your Program

### From Command Line

```bash
./Build/Bin/Debug-Windows/MyOpenGLApp/MyOpenGLApp.exe
```

### From IDE (if using Visual Studio)

1. Open generated `.sln` file
2. Right-click "MyOpenGLApp" → Set as StartUp Project
3. Press F5 to build and run
4. Set breakpoints, debug normally

### With Debug Output

```bash
# On Windows, run with output visible
MyOpenGLApp.exe 2>&1 | tee output.log

# Check logs directory
cat logs/*.log
```

---

## What Should Happen When You Run

1. **Window appears**: 1280x720 titled "My OpenGL App"
2. **Background color**: Dark blue (from `glClearColor(0.1f, 0.1f, 0.15f, 1.0f)`)
3. **Console output** (if debug mode):
   ```
   [OpenGL] Function loader not found  (if fails)
   [OpenGL] gladLoadGL failed           (if GLAD fails)
   [OpenGL] OpenGL 4.6.0  GLSL 4.60     (if succeeds!)
   ```
4. **No geometry**: Just an empty window with dark background (no shapes rendered yet)
5. **Close button**: Click to exit program

---

## Performance Tuning

### Build Flags

Edit `trial.wks.jenga` to add optimization:

```python
with filter("config:Release"):
    cxxflags([
        "-O3",              # Maximum optimization
        "-march=native",    # Use CPU-specific features
    ])
```

### Runtime Optimization

In your code:

```cpp
// Disable debug callbacks in Release
#ifdef NDEBUG
    // Debug disabled
    desc.opengl.contextFlags &= ~NkGLContextFlags::Debug;
#endif
```

### Profiling

Use tools to identify bottlenecks:

```bash
# Windows with Clang
clang++ -pg ... # Generate profiling data

# Check frame time
logger.Infof("Frame time: %.2f ms\n", deltaTime * 1000.0f);
```

---

## Next Steps

1. **Build and run** your current project (should show empty window)
2. **Add a triangle** (follow 03_HOW_TO_EXTEND.md)
3. **Verify shaders** compile and render
4. **Follow your PDF tutorial**, adapting examples from GLFW to Nkentseu

---

Recommended reading:
- `02_MAIN_CPP_LINE_BY_LINE.md` - Understand the current code
- `03_HOW_TO_EXTEND.md` - Add graphics
- `05_OPENGL_FUNDAMENTALS.md` - Understand graphics concepts
