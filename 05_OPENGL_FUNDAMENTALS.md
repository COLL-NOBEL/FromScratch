# OpenGL Fundamentals for Beginners

This guide explains the core concepts of OpenGL graphics programming. If your "Mastering OpenGL" PDF uses different terminology, this will help you bridge the gap.

---

## What is OpenGL?

**OpenGL** = **Open Graphics Library**

Think of it as an **instruction manual for the GPU**:
- You tell OpenGL what to draw (geometry)
- You tell OpenGL how to draw it (shaders, textures, lighting)
- OpenGL tells the GPU to render it
- The GPU produces pixels
- Those pixels appear on your screen

### Layers of Graphics Programming

```
Your Code (main.cpp)
    ↓
Nkentseu Wrapper (easier API)
    ↓
OpenGL (GPU instruction set)
    ↓
GPU Driver (translates to GPU ISA)
    ↓
GPU Hardware (executes, produces pixels)
    ↓
Monitor (displays pixels)
```

### Why OpenGL?

- **Cross-platform**: Works on Windows, Linux, macOS
- **Portable**: Code works across different GPUs
- **Standardized**: Defined by Khronos Group
- **Powerful**: Access to GPU parallelism

---

## The GPU Rendering Pipeline

When you render something, the GPU executes this pipeline:

```
1. VERTEX PROCESSING
   - Your vertex positions → Vertex Shader
   - Shader transforms positions (model/view/projection)
   - Result: Screen-space positions

2. RASTERIZATION
   - Triangles → Scan converter
   - Determines which pixels are inside triangle
   - Interpolates vertex data across triangle
   - Result: Fragments (pixel candidates)

3. FRAGMENT PROCESSING
   - Each fragment → Fragment Shader
   - Shader determines fragment color
   - Can sample textures, calculate lighting, etc.
   - Result: Color values

4. OUTPUT MERGING
   - Fragments → Framebuffer
   - Depth test (is this closer than what's already there?)
   - Blending (if enabled)
   - Result: Final pixel color on screen
```

### Real World Analogy

Imagine drawing with paint:

1. **Vertex Processing** = Planning (where should I draw?)
2. **Rasterization** = Sketching (draw the outline)
3. **Fragment Processing** = Painting (fill in with color/texture)
4. **Output Merging** = Finishing (blend, add shadows, etc.)

---

## Buffers: Where GPU Data Lives

All data on the GPU lives in **buffers** - chunks of GPU memory.

### The Framebuffer

```
Framebuffer = Ultimate render target
├── Color Buffer     - RGB values for each pixel
├── Depth Buffer     - Z value for each pixel (for depth testing)
├── Stencil Buffer   - Mask values for each pixel
└── Accumulation Buffer - For advanced effects
```

**Your rendered image** = pixels in the Color Buffer

### Double Buffering

```
Frame 1:
- Render to BACK buffer (user doesn't see)
- Present() → Swap with FRONT buffer
- User sees Frame 1
- Render to BACK buffer again (now shows new stuff)

Result: Smooth animation, no flickering
```

```cpp
// In your main loop:
if (ctx->BeginFrame()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // You're rendering to the BACK buffer
    
    ctx->EndFrame();
    ctx->Present();  // Swap back/front buffers
}
```

---

## Shaders: GPU Programs

### What Shaders Are

**Shaders** = Small programs that run on the GPU

Each shader runs for every vertex/fragment, in parallel.

### Vertex Shader

Runs **once per vertex**. Transforms vertex data.

```glsl
#version 460 core

// Input vertex data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

// Output to fragment shader
out vec3 vertexColor;

// Uniforms (constant for all vertices in a draw call)
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    // Transform vertex position
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    
    // Pass color to fragment shader
    vertexColor = aColor;
}
```

**Key Points**:
- `in vec3 aPosition` - Input: vertex position
- `out vec3 vertexColor` - Output: data to fragment shader
- `uniform mat4 model` - Constant for all vertices this frame
- `gl_Position` - Special variable: screen position

### Fragment Shader

Runs **once per pixel**. Determines pixel color.

```glsl
#version 460 core

// Input from vertex shader (interpolated across triangle)
in vec3 vertexColor;

// Output color
out vec4 FragColor;

void main() {
    // Set pixel color
    FragColor = vec4(vertexColor, 1.0);
}
```

**Key Points**:
- `in vec3 vertexColor` - Comes from vertex shader, automatically interpolated
- `out vec4 FragColor` - Final pixel color (RGBA)

### Interpolation Between Vertices

```
Vertex A: Color = Red (1, 0, 0)
Vertex B: Color = Green (0, 1, 0)
Vertex C: Color = Blue (0, 0, 1)

Pixel inside triangle = Mix of all three colors!
Fragment shader gets interpolated color
```

### How GLSL Looks

**GLSL** = **GL Shading Language** (like C for GPU)

| C | GLSL | Purpose |
|---|------|---------|
| `float` | `float` | 32-bit floating point |
| `vec3 v = {1,2,3}` | `vec3 v = vec3(1, 2, 3)` | 3D vector |
| `mat4` | `mat4` | 4x4 matrix |
| `a * b` | `dot(a, b)` | Vector dot product |
| `normalize(v)` | `normalize(v)` | Unit vector |
| `reflect(d, n)` | `reflect(d, n)` | Reflect vector |
| `texture(tex, uv)` | `texture(tex, uv)` | Sample texture |

---

## Vertex Attributes & Buffers

### What Are Attributes?

**Vertex Attributes** = Data per vertex

```cpp
struct Vertex {
    glm::vec3 position;    // Vertex position
    glm::vec3 color;       // Vertex color
    glm::vec2 texCoord;    // Texture coordinate
    glm::vec3 normal;      // Surface normal (for lighting)
};
```

### GPU Buffer Objects

```cpp
// 1. Create buffer
GLuint VBO;
glGenBuffers(1, &VBO);

// 2. Bind buffer (make it active)
glBindBuffer(GL_ARRAY_BUFFER, VBO);

// 3. Upload data to GPU
Vertex vertices[3] = {
    {{-0.5, -0.5, 0}, {1, 0, 0}},
    {{ 0.5, -0.5, 0}, {0, 1, 0}},
    {{ 0.0,  0.5, 0}, {0, 0, 1}}
};
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 4. Later, use the buffer
glDrawArrays(GL_TRIANGLES, 0, 3);
```

### VAO (Vertex Array Object)

**VAO** = Stores "how to interpret buffer data"

```cpp
// Create VAO
GLuint VAO;
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// Tell VAO how to interpret VBO data
glBindBuffer(GL_ARRAY_BUFFER, VBO);

// Attribute 0: Position (3 floats)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
glEnableVertexAttribArray(0);

// Attribute 1: Color (3 floats, after position)
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// Now VAO remembers this layout!
```

---

## Drawing: The Actual Rendering

### Basic Draw Calls

#### glDrawArrays - Draw by Index

```cpp
glDrawArrays(GL_TRIANGLES, 0, 3);
```

**Parameters**:
- `GL_TRIANGLES`: Primitive type (connect every 3 vertices into a triangle)
- `0`: Start index
- `3`: Number of vertices to draw

**Result**: Draws 1 triangle (3 vertices)

```cpp
glDrawArrays(GL_TRIANGLES, 0, 9);  // Draws 3 triangles (9 vertices)
```

#### glDrawElements - Draw by Indices

```cpp
GLuint indices[] = {0, 1, 2, 2, 3, 0};  // Two triangles sharing vertices
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
```

More efficient when vertices are reused.

### Primitive Types

```cpp
GL_POINTS;          // Each vertex = point
GL_LINES;           // Each pair of vertices = line
GL_LINE_STRIP;      // Connected line segments
GL_TRIANGLES;       // Each triple of vertices = triangle
GL_TRIANGLE_STRIP;  // Continuous mesh
GL_TRIANGLE_FAN;    // Fan of triangles (efficient for circles)
```

### Draw Order & Winding

```
Clockwise (CW):          Counter-Clockwise (CCW):
    v0                       v0
   /  \                      /  \
  v2--v1                    v1--v2

Front-facing (visible)  Back-facing (culled)
```

**Front-face winding**: Counter-clockwise (by default)

---

## Matrices: The Core of 3D Graphics

**Matrices** transform positions. All 3D graphics = matrix math.

### Model Matrix

**What it does**: Positions object in world space

```glsl
gl_Position = model * vec4(vertexPosition, 1.0);
```

### View Matrix

**What it does**: Positions camera

```glsl
gl_Position = view * gl_Position;
```

### Projection Matrix

**What it does**: Perspective/orthographic projection

```glsl
gl_Position = projection * gl_Position;
```

### The Complete Transform

```glsl
// Combine all three
vec4 worldPos = model * vec4(aPosition, 1.0);
vec4 cameraPos = view * worldPos;
vec4 clipPos = projection * cameraPos;
gl_Position = clipPos;

// Usually written as:
gl_Position = projection * view * model * vec4(aPosition, 1.0);
```

### Creating Matrices (using GLM)

```cpp
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Model matrix: position and rotate object
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));  // Move right
model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));  // Rotate

// View matrix: position camera
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 3.0f),   // Camera position
    glm::vec3(0.0f, 0.0f, 0.0f),   // Look-at point
    glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
);

// Projection matrix: perspective
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),      // FOV (field of view)
    1280.0f / 720.0f,         // Aspect ratio
    0.1f, 100.0f              // Near/far planes
);

// Send to shader
shader.SetMat4("model", model);
shader.SetMat4("view", view);
shader.SetMat4("projection", projection);
```

---

## Textures: Images on Geometry

### What Textures Are

**Texture** = Image stored on GPU that's mapped onto geometry

```
Normal:                 With Texture:
+------+                +------+
|      |                |  🧱  |
|      |  → Apply      |  🧱  | (brick wall image)
|      |    texture    |  🧱  |
+------+                +------+
```

### Texture Coordinates

```glsl
// Vertex shader
layout(location = 2) in vec2 aTexCoord;  // 0.0 to 1.0
out vec2 texCoord;

void main() {
    texCoord = aTexCoord;
}

// Fragment shader
in vec2 texCoord;
uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, texCoord);
}
```

### Texture Coordinates Explained

```
(0, 1) -------- (1, 1)
  |               |
  | Texture       | Texture
  | Coordinates   | Image
  |               |
(0, 0) -------- (1, 0)
```

Each vertex has `texCoord (u, v)` that maps to texture.

---

## Lighting Basics

### Three Types of Light

#### Ambient Light
Light bouncing around, illuminating everything uniformly.

```glsl
vec3 ambient = ambientStrength * lightColor;
FragColor = vec4(ambient * objectColor, 1.0);
```

#### Diffuse Light
Light reflecting off surface, brightness depends on angle.

```glsl
vec3 norm = normalize(normal);
vec3 lightDir = normalize(lightPos - fragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
```

#### Specular Light
Shiny reflections on smooth surfaces.

```glsl
vec3 viewDir = normalize(viewPos - fragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 specular = spec * lightColor;
```

### Complete Phong Lighting

```glsl
#version 460 core

in vec3 fragPos;
in vec3 normal;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main() {
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Combine
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
```

---

## Depth Testing: Fixing Occlusion

### Without Depth Testing

```
You render:
1. Cube (z = 5)      → Appears
2. Sphere (z = 1)    → Overwrites cube! WRONG
```

### With Depth Testing

```cpp
glEnable(GL_DEPTH_TEST);
```

```
You render:
1. Cube (z = 5)      → Stored in depth buffer
2. Sphere (z = 1)    → Z is less (closer), so it's drawn OVER cube ✓
```

**How it works**: For each pixel, OpenGL checks: "Is this pixel's Z less than what's already there?" If yes, draw it. If no, skip it.

---

## Common Gotchas & Solutions

### Issue 1: Triangle Not Visible

**Check**:
- Did you load OpenGL functions? (`LoadGL()`)
- Did you enable attributes? (`glEnableVertexAttribArray()`)
- Did you bind VAO before drawing? (`glBindVertexArray(VAO)`)
- Is camera looking at the triangle?
- Is triangle in front of camera? (z should be closer than far plane)

```cpp
// Debugging: Check OpenGL errors
GLenum err = glGetError();
if (err != GL_NO_ERROR) {
    logger.Errorf("OpenGL Error: %d\n", err);
}
```

### Issue 2: Weird Colors

**Cause**: sRGB color space mismatch

**Solution**:
```cpp
glEnable(GL_FRAMEBUFFER_SRGB);  // Enable sRGB framebuffer
// Or in context config:
desc.opengl.srgbFramebuffer = true;
```

### Issue 3: Flickering/Tearing

**Cause**: Not using double buffering

**Solution**: Your Nkentseu setup already does this with `Present()`

### Issue 4: Shader Compilation Error

**Solution**: Check shader info log

```cpp
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    logger.Error(infoLog);
}
```

---

## Suggested Learning Path

### Week 1-2: Fundamentals
- [x] Window + OpenGL context (you have this!)
- [ ] Draw a triangle
- [ ] Understand vertex attributes
- [ ] Modify colors with shaders
- [ ] Rotate with matrices

### Week 3-4: Geometry
- [ ] Draw a cube
- [ ] Load 3D models (OBJ)
- [ ] Understand normals
- [ ] Camera controls (FPS-style)

### Week 5-6: Texturing
- [ ] Load images
- [ ] Apply textures to geometry
- [ ] Understand UV coordinates
- [ ] Multiple textures (diffuse, normal maps)

### Week 7-8: Lighting
- [ ] Implement Phong lighting
- [ ] Point lights, directional lights
- [ ] Shadow mapping
- [ ] Advanced lighting (PBR)

### Ongoing
- [ ] Performance optimization
- [ ] Post-processing effects
- [ ] Particle systems
- [ ] Advanced techniques

---

**Now You're Ready**: Your PDF tutorial will make much more sense with this foundation!

Next, read `06_BUILD_AND_RUN.md` to get your project compiling.
