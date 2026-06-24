#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include "NKLogger/NkLog.h"

#if defined(_WIN32)
#include "NKGlad/include/glad/wgl.h"
#endif
#include "NKGlad/include/glad/gl.h"

#if defined(_WIN32) && defined(Bool)
    #undef Bool
#endif

#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkIGraphicsContext.h"
#include "NKContext/Core/NkOpenGLDesc.h"
#include "NKContext/Graphics/OpenGL/NkOpenGLContextData.h"
#include "stb_image.h"

// #include "engine/Renderer.h"

#include <chrono>

#include "EBO.h"
#include "VBO.h"
#include "VAO.h"
#include "ShaderClass.h"
#include "Texture.h"



using namespace nkentseu;

// Load OpenGL function pointers using GLAD
static bool LoadGL(NkIGraphicsContext* ctx) {
    if (!ctx || ctx->GetApi() != NkGraphicsApi::NK_API_OPENGL) {
        logger.Info("[OpenGL] Invalid graphics context API\n");
        return false;
    }

    auto* nativeData = static_cast<NkOpenGLContextData*>(ctx->GetNativeContextData());
    auto loader = (nativeData != nullptr) ? nativeData->getProcAddress : nullptr;
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

int nkmain(const nkentseu::NkEntryState& /*state*/) {
    // Step 1: Create window
    NkWindowConfig cfg;
    cfg.title = "My OpenGL App";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;


    // We create a window called "window" and log the message "Failed to create window" if window creating fails.
    NkWindow window;
    if (!window.Create(cfg)) {
        logger.Error("Failed to create window");
        return -2;
    }

    // Step 2: Configure OpenGL context
    NkContextDesc desc;
    desc.api = NkGraphicsApi::NK_API_OPENGL;
    desc.opengl.majorVersion = 3;
    desc.opengl.minorVersion = 3;
    desc.opengl.profile = NkGLProfile::Core;
    desc.opengl.contextFlags = NkGLContextFlags::ForwardCompat | NkGLContextFlags::Debug;
    desc.opengl.runtime.installDebugCallback = true;
    desc.opengl.msaaSamples = 4;
    desc.opengl.srgbFramebuffer = true;
    desc.opengl.swapInterval = NkGLSwapInterval::AdaptiveVSync;
    desc.opengl.runtime.autoLoadEntryPoints = true;
    desc.opengl.runtime.validateVersion = true;

    // Step 3: Create graphics context
    auto ctx = NkContextFactory::Create(window, desc);
    if (!ctx) {
        logger.Error("[OpenGL] Context creation failed");
        window.Close();
        return -3;
    }

    // Step 4: Load OpenGL functions
    if (!LoadGL(ctx)) {
        ctx->Shutdown();
        window.Close();
        return -4;
    }


    GLfloat vertices[] = 
    {
        // Bottom row (3 vertices)
        -0.5f, 0.5f, 0.0f,       0.2f, 0.1f, 0.3f,         2.0f, 0.0f,// Vertex 0: top-left
        0.5f, 0.5f, 0.0f,        0.4f, 0.5f, 0.6f,         2.0f, 2.0f,// Vertex 1: top-right
        -0.5f, -0.5f, 0.0f,      0.7f, 0.8f, 0.9f,         0.0f, 0.0f,// Vertex 2: bottom-left
        
        // Middle row (2 vertices)
        0.5f,  -0.5f,   0.0f,    0.1f, 0.2f, 0.3f,         0.0f, 2.0f,// Vertex 3: bottom-right
    };

    GLuint indices[] = 
    {
        0, 1, 2, 
        1, 2, 3, 
    };

    // Creating a shader program from the vertex and fragment shader files
    Shader shaderProgram("OpenPrj/shaders/pbr.vert", "OpenPrj/shaders/pbr.frag");

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices, sizeof(vertices));
    EBO EBO1(indices, sizeof(indices));

    VAO1.LinkAttributes(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    VAO1.LinkAttributes(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO1.LinkAttributes(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    // Initializing texture, generating ID and binding to texture slot or target.
    Texture squareImg("OpenPrj/assets/square_image.JPG", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
    squareImg.texUnit(shaderProgram, "actualTexture", 0);
    
    
    GLuint scaleUniform = glGetUniformLocation(shaderProgram.ID, "scale"); // Declaring uniform for scaling

    shaderProgram.Activate();  // Activating the shader program for rendering
    glUniform1f(scaleUniform, 0.5f);  // Setting the value of the uniform variable
    squareImg.Unbind(); 

    // Step 5: Initialize graphics-engine scaffold
    glViewport(0, 0, window.GetSize().width, window.GetSize().height);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);  // setting color to clear the window or screen with
    glClear(GL_COLOR_BUFFER_BIT);   // Clearing the color buffer with the color specified for clearing

    
    // Step 6: Main loop
    bool running = true;
    auto& events = NkEvents();
    const auto appStartTime = std::chrono::steady_clock::now();


    // Loop that continues and keeps window and app live
    while (running) {
        while (NkEvent* ev = events.PollEvent()) { // Polling events from the event queue
            if (ev->Is<NkWindowCloseEvent>()) {
                running = false;
                break;
            }
        }

        if (!running) { // creating condition for breaking loop
            break;
        }

        if (ctx->BeginFrame()) {
            const auto currentSize = window.GetSize();
            glViewport(0, 0, currentSize.width, currentSize.height); // resizing the screen
            glClear(GL_COLOR_BUFFER_BIT); // clear the color buffer with the color specified for clearing
            
            
            VAO1.Bind();           // Binding the VAO to use the vertex attribute configuration for rendering

            squareImg.Bind(); // Binding texture to square (two triangles)

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Drawing the triangle using the index data in the EBO

            ctx->EndFrame();
            ctx->Present();  // Present the rendered (or cleared color) frame to the screen or window
        }
    }

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    squareImg.Delete();
    shaderProgram.Delete();

    
    ctx->Shutdown();
    window.Close();

    return 0;
}
