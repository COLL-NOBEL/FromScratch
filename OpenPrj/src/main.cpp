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

// #include "engine/Renderer.h"

#include <chrono>


const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);\n"
"}\n\0";

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
        -0.5f, -0.5f, 0.0f, // Bottom-left vertex
         0.5f, -0.5f, 0.0f, // Bottom-right vertex
         0.0f,  0.5f, 0.0f  // Top vertex
    };

    // Step 5: Initialize graphics-engine scaffold
    glViewport(0, 0, window.GetSize().width, window.GetSize().height);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);  // setting color to clear the window or screen with
    glClear(GL_COLOR_BUFFER_BIT);   // Clearing the color buffer with the color specified for clearing


    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


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
            
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3); // Draw the triangle using the vertex data


            ctx->EndFrame();
            ctx->Present();  // Present the rendered (or cleared color) frame to the screen or window
        }
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    
    ctx->Shutdown();
    window.Close();

    return 0;
}
