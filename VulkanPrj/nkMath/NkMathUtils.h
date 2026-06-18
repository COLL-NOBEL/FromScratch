 
struct NkMathUtils { 
    static constexpr float Pi      = 3.14159265358979323846f; 
    static constexpr float TwoPi   = Pi * 2.0f; 
    static constexpr float HalfPi  = Pi / 2.0f; 
    static constexpr float Deg2Rad = Pi / 180.0f; 
    static constexpr float Rad2Deg = 180.0f / Pi; 
 
    static float lerp(float a, float b, float t) { return a + (b-a)*t; } 
    static float clamp(float v, float lo, float hi) { 
        return v < lo ? lo : (v > hi ? hi : v);
    } 
    static float clamp01(float v) { return clamp(v, 0.0f, 1.0f); } 
    static float abs(float v)    { return v < 0 ? -v : v; } 
    static float min(float a, float b) { return a < b ? a : b; } 
    static float max(float a, float b) { return a > b ? a : b; } 
    static float floor(float v)  { return (float)(int)v - (v < (int)v ? 1.f : 
    0.f); } 
    static float frac(float v)   { return v - NkMathUtils::floor(v); } 
    static float deg2rad(float d){ return d * Deg2Rad; } 
    static float rad2deg(float r){ return r * Rad2Deg; } 
    // Interpolation easing 
    static float easeInQuad(float t)  { return t*t; } 
    static float easeOutQuad(float t) { return 1-(1-t)*(1-t); } 
    static float easeInOutQuad(float t) { 
    return t < 0.5f ? 2*t*t : 1-2*(1-t)*(1-t); 
    } 
}; 