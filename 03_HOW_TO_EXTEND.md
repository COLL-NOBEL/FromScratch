# How to Extend: Building a Graphics Engine on This Foundation

Now that you understand how `main.cpp` creates an empty window with an OpenGL context, let's learn how to add graphics to it. This guide builds a bridge between your setup and your "Mastering OpenGL" PDF tutorial.

---

## The Empty Window Architecture

Your current code structure looks like:

```
main.cpp
├── Initialize Window
├── Initialize OpenGL Context
├── Load OpenGL Functions (GLAD)
├── Main Loop
│   ├── Poll Events
│   ├── BeginFrame()
│   ├── Clear Screen
│   ├── [EMPTY - RENDERING CODE GOES HERE]
│   ├── EndFrame()
│   └── Present()
└── Cleanup
```

The `// Your rendering code here` comment (line 104) is where you'll add graphics.

---

## Step-by-Step: Adding Your First Triangle

### Stage 1: Create a Shader Class

First, you need **shaders** - programs that run on the GPU to transform and color your geometry.

Create file: `OpenPrj/src/Shader.h`

```cpp
#pragma once
#include <string>
#include <glm/glm.hpp>  // Note: You'll need glm for math

class Shader {
public:
    unsigned int ID;
    
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();
    
    void Use();
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec3(const std::string& name, const glm::vec3& vec) const;
    void SetFloat(const std::string& name, float value) const;
};
```

**What shaders do:**
- **Vertex Shader**: Processes each vertex (position, rotation, scaling)
- **Fragment Shader**: Determines color of each pixel

### Stage 2: Create Simple Shaders

Create: `OpenPrj/shaders/vertex.glsl`

```glsl
#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    vertexColor = aColor;
}
```

Create: `OpenPrj/shaders/fragment.glsl`

```glsl
#version 460 core
in vec3 vertexColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
```

**What's happening:**
- Vertex shader: Takes vertex position, applies transformations, passes color to fragment shader
- Fragment shader: Takes color from vertex shader, outputs final pixel color

### Stage 3: Create Mesh Class

Create: `OpenPrj/src/Mesh.h`

```cpp
#pragma once
#include <glad/gl.h>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Color;
};

class Mesh {
public:
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds);
    ~Mesh();
    
    void Draw();
    
private:
    void SetupMesh();
};
```

**What this does:**
- **VAO** (Vertex Array Object): Stores vertex format
- **VBO** (Vertex Buffer Object): Stores vertex data on GPU
- **EBO** (Element Buffer Object): Stores index data (which vertices form triangles)

### Stage 4: Modify main.cpp

Replace the empty rendering section with:

```cpp
// At the top of nkmain(), after LoadGL:
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Mesh.h"

// Create shader
Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

// Create a simple triangle
std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left, red
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Bottom-right, green
    {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Top, blue
};

std::vector<unsigned int> indices = {0, 1, 2};
Mesh triangle(vertices, indices);

// In main loop, replace "// Your rendering code here" with:
shader.Use();

// Setup matrices
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
    1280.0f / 720.0f, 0.1f, 100.0f);
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
glm::mat4 model = glm::mat4(1.0f);

shader.SetMat4("projection", projection);
shader.SetMat4("view", view);
shader.SetMat4("model", model);

triangle.Draw();
```

---

## Key Concepts: Understanding the Graphics Pipeline

### The Rendering Pipeline (What Happens When You Draw)

```
Your Triangle Data
    ↓
[Vertex Shader] - Processes each vertex
    ↓               - Transforms position (model, view, projection matrices)
                    - Passes data to fragment shader
[Rasterization] - Converts triangles to pixels
    ↓               - Determines which pixels are inside the triangle
[Fragment Shader] - Runs for each pixel
    ↓               - Determines pixel color
[Output Merger] - Writes to framebuffer
    ↓
Screen Display
```

### The Three Critical Matrices

All graphics programming relies on **three transformation matrices**:

#### 1. Model Matrix
**What it does**: Positions/rotates/scales your object in world space

```cpp
glm::mat4 model = glm::mat4(1.0f);
model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));  // Rotate around Y axis
model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));      // Move 2 units right
```

**Example**: Your triangle at position (2, 0, 0), rotated 45 degrees

#### 2. View Matrix
**What it does**: Positions the camera

```cpp
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 3.0f),   // Camera position
    glm::vec3(0.0f, 0.0f, 0.0f),   // Look at point
    glm::vec3(0.0f, 1.0f, 0.0f)    // Up direction
);
```

**Example**: Camera at z=3 looking at origin, y-axis points up

#### 3. Projection Matrix
**What it does**: Defines the viewing frustum (what the camera can see)

```cpp
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),    // Field of view (45 degrees)
    1280.0f / 720.0f,       // Aspect ratio
    0.1f,                   // Near plane
    100.0f                  // Far plane
);
```

**Example**: 45-degree field of view, can see from 0.1 to 100 units away

### How Shaders Use These Matrices

In your **vertex shader**:

```glsl
gl_Position = projection * view * model * vec4(aPosition, 1.0);
```

**This multiplies your vertex position by all three matrices in order:**
1. **model**: Transform from object space to world space
2. **view**: Transform from world space to camera space
3. **projection**: Transform from camera space to clip space (for screen)

**Result**: A 3D point becomes a 2D screen coordinate

---

## The State Machine: Understanding OpenGL

OpenGL is a **state machine**. Think of it like a light switch:

```cpp
glEnable(GL_DEPTH_TEST);    // Turn on depth testing
// All subsequent drawing uses depth testing
glDisable(GL_DEPTH_TEST);   // Turn off depth testing
// All subsequent drawing ignores depth
```

### Important OpenGL States for Rendering

```cpp
// Depth testing - draw closer objects on top
glEnable(GL_DEPTH_TEST);

// Back-face culling - don't draw triangle backs
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);

// Smooth shading - blend colors across triangle
glShadeModel(GL_SMOOTH);

// Anti-aliasing (if MSAA enabled)
glEnable(GL_MULTISAMPLE);

// Transparency blending
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

---

## Common OpenGL Objects You'll Work With

| Object | Purpose | Example |
|--------|---------|---------|
| **VAO (Vertex Array Object)** | Stores vertex format | Stores "vertices have position + color" |
| **VBO (Vertex Buffer Object)** | GPU memory for vertices | Stores actual vertex positions/colors |
| **EBO (Element Buffer Object)** | GPU memory for indices | Stores triangle vertex indices (0,1,2), (2,3,0), etc. |
| **Shader Program** | GPU code | Vertex + Fragment shaders compiled together |
| **Texture** | Image on GPU | Brick texture for a wall |
| **Framebuffer** | Render target | Off-screen rendering, post-processing |
| **Uniform Buffer** | Constant GPU memory | Matrix data, lighting info |

### Typical Usage Pattern

```cpp
// 1. Create and bind VAO
GLuint VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// 2. Create and bind VBO
GLuint VBO;
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 3. Specify vertex attributes
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
glEnableVertexAttribArray(0);

// 4. When rendering, bind and draw
glBindVertexArray(VAO);
glDrawArrays(GL_TRIANGLES, 0, vertexCount);
```

---

## Building Towards Your PDF Tutorial

Your "Mastering OpenGL" PDF probably covers:

1. **Hello Triangle** ← You are here
2. **Shaders & Transformations** ← Next step
3. **Texturing** - Load images onto geometry
4. **Lighting** - Phong/PBR models
5. **Advanced Techniques** - Normal mapping, parallax mapping, etc.

### This Setup vs GLFW/SFML

Your Nkentseu setup is **actually more comprehensive** than GLFW/SFML:

| Feature | GLFW | SFML | Nkentseu |
|---------|------|------|----------|
| Window creation | ✅ | ✅ | ✅ |
| OpenGL loading | 🔴 (need GLAD) | 🔴 (need GLAD) | ✅ Built-in |
| Event handling | ✅ | ✅ | ✅ |
| Graphics abstraction | 🔴 | 🔴 | ✅ (OpenGL/DirectX/Metal) |
| Built-in logger | 🔴 | 🔴 | ✅ |
| Data structures | 🔴 | 🔴 | ✅ |

**Advantage**: Your setup abstracts away platform differences more cleanly.

**Disadvantage**: Less tutorials available online (but you can adapt GLFW tutorials).

---

## File Organization for Growing Graphics Engine

As you add features, organize your code:

```
OpenPrj/
├── src/
│   ├── main.cpp                 # Entry point
│   ├── Shader.h / Shader.cpp    # Shader compilation & uniforms
│   ├── Mesh.h / Mesh.cpp        # VAO/VBO/EBO management
│   ├── Texture.h / Texture.cpp  # Image loading & GPU texture
│   ├── Camera.h / Camera.cpp    # View/Projection matrices
│   ├── Model.h / Model.cpp      # Load 3D models (GLTF, OBJ)
│   ├── Light.h / Light.cpp      # Lighting calculations
│   └── Renderer.h / Renderer.cpp # High-level rendering system
├── shaders/
│   ├── vertex.glsl              # Vertex shader source
│   ├── fragment.glsl            # Fragment shader source
│   ├── toonify.glsl             # Toon shading variant
│   └── ...
├── assets/
│   ├── models/                  # 3D models
│   ├── textures/                # Images
│   └── fonts/                   # Text rendering
└── nkMath/                      # Math helpers
```

---

## Next Steps for Learning

### Phase 1: Geometry (1-2 weeks)
- [x] Empty window with OpenGL context
- [ ] Draw your first triangle
- [ ] Draw multiple shapes (cube, sphere, etc.)
- [ ] Implement basic transformations (rotation, scaling)

### Phase 2: Shading (2-3 weeks)
- [ ] Understand vertex/fragment shaders
- [ ] Implement basic lighting (Phong model)
- [ ] Load and apply textures
- [ ] Normal mapping for surface details

### Phase 3: Advanced Graphics (ongoing)
- [ ] Particle systems
- [ ] Skeletal animation
- [ ] Deferred rendering
- [ ] Post-processing effects

---

## Debugging Tips

When things don't work, check these in order:

1. **OpenGL Errors**
```cpp
GLenum err = glGetError();
if (err != GL_NO_ERROR) {
    logger.Errorf("OpenGL Error: %d\n", err);
}
```

2. **Debug Context Messages** (enabled in your main.cpp)
```
Enable debug callback → GPU drivers print errors to console
```

3. **Shader Compilation Errors**
```cpp
// Check if shader compiled successfully
GLint success;
glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    logger.Error(infoLog);
}
```

4. **Validate Your Data**
```cpp
// Print vertex data before uploading
for (const auto& v : vertices) {
    logger.Infof("Vertex: (%f, %f, %f)\n", v.Position.x, v.Position.y, v.Position.z);
}
```

---

## Key Differences from PDF Tutorial

Your PDF might mention things differently:

| PDF Term | Your Nkentseu Equivalent | Notes |
|----------|-------------------------|-------|
| `glfwCreateWindow()` | `NkWindow::Create()` | Same concept, different API |
| `glfwMakeContextCurrent()` | `NkContextFactory::Create()` | Nkentseu does this internally |
| `glfwGetProcAddress()` | `NkNativeContext::GetOpenGLProcAddressLoader()` | Platform abstraction |
| `gladLoadGL(loader)` | Same `gladLoadGL()` | Identical |
| `glfw` event loop | `NkEvents()::PollEvent()` | Similar polling mechanism |

**Main difference**: Nkentseu abstracts platform details, so you write less platform-specific code.

---

**Next**: Read `04_NKENTSEU_API_REFERENCE.md` for detailed API documentation of key classes.
