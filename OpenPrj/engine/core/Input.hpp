#pragma once
// engine/core/Input.hpp
// Keyboard and mouse state snapshot, polled once per frame.
// Uses nkentseu NkEvent system — call Input::Update() in your event loop.

#include "NKEvent/NkEventState.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKEvent/NkKeycodeMap.h"
#include "../core/Math.hpp"
#include <unordered_set>

namespace engine {

class Input {
public:
    // ---- Keyboard ---------------------------------------------------

    // True every frame the key is held
    static bool IsKeyDown(nkentseu::NkKeycode key);

    // True only on the frame the key was first pressed
    static bool IsKeyPressed(nkentseu::NkKeycode key);

    // True only on the frame the key was released
    static bool IsKeyReleased(nkentseu::NkKeycode key);

    // ---- Mouse ------------------------------------------------------

    // True while button is held (0=left, 1=right, 2=middle)
    static bool IsMouseButtonDown(int button);
    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonReleased(int button);

    // Current cursor position in window pixels
    static Vec2f GetMousePosition();

    // Mouse movement since last frame
    static Vec2f GetMouseDelta();

    // Scroll wheel delta (Y axis)
    static float GetScrollDelta();

    // ---- Call these from your main loop -----------------------------

    // Call before polling events each frame
    static void BeginFrame();

    // Call for each polled nkentseu event
    static void ProcessEvent(nkentseu::NkEvent* ev);

private:
    static std::unordered_set<int> keysDown;
    static std::unordered_set<int> keysPressed;
    static std::unordered_set<int> keysReleased;

    static bool mouseDown[3];
    static bool mousePressed[3];
    static bool mouseReleased[3];

    static Vec2f mousePos;
    static Vec2f mouseDelta;
    static Vec2f lastMousePos;
    static float scrollDelta;
    static bool  firstMouse;
};

} // namespace engine
