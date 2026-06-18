// engine/core/Input.cpp

#include "Input.hpp"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKEvent/NkMouseEvent.h"

namespace engine {

using namespace nkentseu;

// ---- Static storage -----------------------------------------------------

std::unordered_set<int> Input::keysDown;
std::unordered_set<int> Input::keysPressed;
std::unordered_set<int> Input::keysReleased;

bool  Input::mouseDown[3]     = {false, false, false};
bool  Input::mousePressed[3]  = {false, false, false};
bool  Input::mouseReleased[3] = {false, false, false};

Vec2f Input::mousePos      = {0, 0};
Vec2f Input::mouseDelta    = {0, 0};
Vec2f Input::lastMousePos  = {0, 0};
float Input::scrollDelta   = 0.0f;
bool  Input::firstMouse    = true;

// ---- BeginFrame ---------------------------------------------------------

void Input::BeginFrame() {
    keysPressed.clear();
    keysReleased.clear();
    for (int i = 0; i < 3; i++) {
        mousePressed[i]  = false;
        mouseReleased[i] = false;
    }
    mouseDelta  = {0, 0};
    scrollDelta = 0.0f;
}

// ---- ProcessEvent -------------------------------------------------------

void Input::ProcessEvent(NkEvent* ev) {
    if (!ev) return;

    // Keyboard pressed
    if (auto* ke = ev->Cast<NkKeyboardPressEvent>()) {
        int code = (int)ke->GetKeycode();
        if (keysDown.find(code) == keysDown.end()) {
            keysPressed.insert(code);
        }
        keysDown.insert(code);
    }
    // Keyboard released
    else if (auto* ke = ev->Cast<NkKeyboardReleaseEvent>()) {
        int code = (int)ke->GetKeycode();
        keysDown.erase(code);
        keysReleased.insert(code);
    }
    // Mouse button pressed
    else if (auto* me = ev->Cast<NkMouseButtonPressEvent>()) {
        int btn = (int)me->GetButton();
        if (btn >= 0 && btn < 3) {
            mouseDown[btn]    = true;
            mousePressed[btn] = true;
        }
    }
    // Mouse button released
    else if (auto* me = ev->Cast<NkMouseButtonReleaseEvent>()) {
        int btn = (int)me->GetButton();
        if (btn >= 0 && btn < 3) {
            mouseDown[btn]     = false;
            mouseReleased[btn] = true;
        }
    }
    // Mouse moved
    else if (auto* me = ev->Cast<NkMouseMoveEvent>()) {
        float x = (float)me->GetX();
        float y = (float)me->GetY();
        mousePos = {x, y};
        if (firstMouse) { lastMousePos = mousePos; firstMouse = false; }
        mouseDelta = mousePos - lastMousePos;
        lastMousePos = mousePos;
    }
    // Mouse scroll
    else if (auto* me = ev->Cast<NkMouseScrollEvent>()) {
        scrollDelta += (float)me->GetDeltaY();
    }
}

// ---- Keyboard queries ---------------------------------------------------

bool Input::IsKeyDown(NkKeycode key) {
    return keysDown.count((int)key) > 0;
}
bool Input::IsKeyPressed(NkKeycode key) {
    return keysPressed.count((int)key) > 0;
}
bool Input::IsKeyReleased(NkKeycode key) {
    return keysReleased.count((int)key) > 0;
}

// ---- Mouse queries ------------------------------------------------------

bool Input::IsMouseButtonDown(int b)     { return (b>=0&&b<3) && mouseDown[b]; }
bool Input::IsMouseButtonPressed(int b)  { return (b>=0&&b<3) && mousePressed[b]; }
bool Input::IsMouseButtonReleased(int b) { return (b>=0&&b<3) && mouseReleased[b]; }

Vec2f Input::GetMousePosition() { return mousePos; }
Vec2f Input::GetMouseDelta()    { return mouseDelta; }
float Input::GetScrollDelta()   { return scrollDelta; }

} // namespace engine
