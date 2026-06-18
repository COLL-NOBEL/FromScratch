#pragma once

#include <cstdint>

struct NkColor { 
    uint8_t mR, mG, mB, mA; 

    // Constructeurs 
    NkColor() : mR(0), mG(0), mB(0), mA(255) {} 
    NkColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) 
        : mR(r), mG(g), mB(b), mA(a) {} 

    // Conversion vers/depuis uint32_t 
    uint32_t toUint32() const { 
        return ((uint32_t)mA << 24) | 
                ((uint32_t)mR << 16) | 
                ((uint32_t)mG << 8)  | 
                ((uint32_t)mB); 
    } 

    static NkColor fromUint32(uint32_t c) { 
        return NkColor( 
            (c >> 16) & 0xFF, 
            (c >> 8)  & 0xFF, 
            (c >> 0)  & 0xFF, 
            (c >> 24) & 0xFF 
        ); 
    } 

    // Construction depuis des floats [0,1] 
    static NkColor fromFloat(float r, float g, float b, float a = 1.0f) { 
        return NkColor( 
            (uint8_t)(r * 255.0f), 
            (uint8_t)(g * 255.0f), 
            (uint8_t)(b * 255.0f),
            (uint8_t)(a * 255.0f) 
        ); 
    } 

    // Conversion vers couleur windows
    static NkColor toUint32(float r, float g, float b, float a = 1.0f) { 
        return NkColor( 
            (uint8_t)(a * 255.0f), 
            (uint8_t)(b * 255.0f), 
            (uint8_t)(g * 255.0f),
            (uint8_t)(r * 255.0f) 
        ); 
    } 

    // Couleurs prédéfinies 
    static NkColor Red()   { return NkColor(255, 0,   0,   255); } 
    static NkColor Green() { return NkColor(0,   255, 0,   255); } 
    static NkColor Blue()  { return NkColor(0,   0,   255, 255); } 
    static NkColor White() { return NkColor(255, 255, 255, 255); } 
    static NkColor Black() { return NkColor(0,   0,   0,   255); } 
}; 
 
