#include "NKWindow/Core/NkMain.h"
#include "NKWindow/NkWindow.h"
#include "NKPlatform/NkPlatformDetect.h"
#include "NKLogger/NkLog.h"
// #include "NKMath/NKMath.h"

#if defined(NKENTSEU_PLATFORM_WINDOWS)
    #include <windows.h>
#endif

using namespace nkentseu;

struct Framebuffer {
    private:
        NkVector<uint8> pixels;
        math::NkVec2u size;
        uint32 stride = 0;

        #if defined(NKENTSEU_PLATFORM_WINDOWS)
            HWND hwnd      = nullptr;  // HWND
            HDC hdc       = nullptr;  // HDC
            HBITMAP dibBitmap = nullptr;  // HBITMAP DIBSection
            HDC dibDC     = nullptr;  // HDC mémoire
            void* dibBits   = nullptr;  // Pointeur pixels DIB
        #endif

        bool InitBackend(NkWindow& window) {
            #if defined(NKENTSEU_PLATFORM_WINDOWS)
                this->hwnd = window.GetSurfaceDesc().hwnd;
                // this->hwnd  = static_cast<HWND>(window.GetSurfaceDesc().hwnd);
                this->hdc  = GetDC(this->hwnd);

                BITMAPINFO bmi = {};
                bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth       = (LONG)size.width;
                bmi.bmiHeader.biHeight      = -(LONG)size.height; // top-down
                bmi.bmiHeader.biPlanes      = 1;
                bmi.bmiHeader.biBitCount    = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                void* bits = nullptr;
                this->dibBitmap = CreateDIBSection(static_cast<HDC>(this->hdc), &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
                if (!this->dibBitmap) { 
                    logger.Error("CreateDIBSection failed"); 
                    return false; 
                }

                this->dibBits = bits;
                this->dibDC   = CreateCompatibleDC(static_cast<HDC>(this->hdc));
                SelectObject(static_cast<HDC>(this->dibDC), static_cast<HBITMAP>(this->dibBitmap));
                return true;
            #else
                return false;
            #endif
        }

        void CloseBackend() {
            #if defined(NKENTSEU_PLATFORM_WINDOWS)
                if (this->dibDC) { 
                    DeleteDC(static_cast<HDC>(this->dibDC));
                    this->dibDC = nullptr;
                }

                if (this->dibBitmap) { 
                    DeleteObject(static_cast<HBITMAP>(this->dibBitmap)); 
                    this->dibBitmap = nullptr;
                }

                if (this->hdc && this->hwnd) {
                    ReleaseDC(static_cast<HWND>(this->hwnd), static_cast<HDC>(this->hdc));
                    this->hdc = nullptr;
                }
            #endif
        }

    public:
        Framebuffer(NkWindow& window) : size(window.GetSize()) {
            if (!IsValide()) return;
            stride = size.width * 4u;

            pixels.Reserve(size.width * size.height * 4);

            InitBackend(window);
        }

        ~Framebuffer() {
            CloseBackend();
        }

        bool Resize(uint32 w, uint32 h, NkWindow& window) {
            if (w == 0 || h == 0) return true;
            
            CloseBackend();

            stride = w * 4u;
            size.width = w;
            size.height = h;

            const usize pixelCount = static_cast<usize>(stride) * static_cast<usize>(h);
            pixels.Assign(static_cast<uint8>(0), static_cast<NkVector<uint8>::SizeType>(pixelCount));

            if (!InitBackend(window)) {
                return false;
            }
            return true;
        }

        bool IsValide() {
            if (size.width <= 0 || size.height <= 0) return false;
            return true;
        }

        uint8*       RowPtr(uint32 y)       { return pixels.Data() + y * stride; }
        const uint8* RowPtr(uint32 y) const { return pixels.Data() + y * stride; }

        void SetPixel(int x, int y, const math::NkColor& color) {
            if (x < 0 || x >= size.width || y < 0 || y >= size.height) return;

            uint8* p = RowPtr(y) + x * 4u;

            p[0] = color.r;
            p[1] = color.g;
            p[2] = color.b;
            p[3] = color.a;
        }

        math::NkColor GetPixel(int x, int y) {
            if (x < 0 || x >= size.width || y < 0 || y >= size.height) return math::NkColor::Transparent();

            uint8* p = RowPtr(y) + x * 4u;
            return math::NkColor(p[0], p[1], p[2], p[3]);
        }

        void Clear(const math::NkColor& color) {
            for (usize i = 0; i < pixels.Size(); i += 4) {
                pixels[i + 0] = color.r;
                pixels[i + 1] = color.g;
                pixels[i + 2] = color.b;
                pixels[i + 3] = color.a;
            }
        }
        
        // There are four points on a square, then we start by taking the indices of the first and fourth x values
        // Then we take the indices of the first and fourth y values.
        void drawRectangle (const int point1x, const int point4x, const int point1y, const int point4y, const math::NkColor& color) {

            for (int i = point1x; i < point4x; ++i) {
                for (int j = point1y; j < point4y; ++j) {
                    this->SetPixel(i, j, color);
                }
            }
        }


        void Present() {
            #if defined(NKENTSEU_PLATFORM_WINDOWS)
                if (this->dibBits && this->dibDC && this->hdc) {
                    memcpy(this->dibBits, pixels.Data(), pixels.Size());
                    BitBlt(static_cast<HDC>(this->hdc), 0, 0, (int)size.width, (int)size.height, static_cast<HDC>(this->dibDC), 0, 0, SRCCOPY);
                }
            #endif
        }
};

// =============================================================================
// Point d'entrée — nkmain (appelé par NkMetalEntryPoint.mm sur Apple,
//                           ou directement depuis main() sur Windows/Linux)
// =============================================================================
int nkmain(const NkEntryState& state) {
    // -------------------------------------------------------------------------
    // 2. Fenetre
    // -------------------------------------------------------------------------
    NkWindowConfig cfg;
    cfg.title       = "Trial-Apps";
    cfg.width       = 1280;
    cfg.height      = 720;
    cfg.centered    = true;
    cfg.resizable   = true;
    cfg.dropEnabled = true;

    bool resize;
    math::NkVec2u size(1280, 720);

    NkWindow window;
    if (!window.Create(cfg)) {
        logger.Error("[APPS] Window creation FAILED");
        return -2;
    }

    Framebuffer* fb = new Framebuffer(window);
    if (fb == nullptr) {
        return -3;
    }

    // -------------------------------------------------------------------------
    // 5. Boucle principale
    // -------------------------------------------------------------------------
    auto& eventSystem = NkEvents();

    bool running = true;
    float timeSeconds = 0.f;
    NkChrono chrono;
    NkElapsedTime elapsed;

    while (running) {
        NkElapsedTime e = chrono.Reset();

        // --- Pattern A : Dispatcher typA (OnEvent pour chaque event)
        while (NkEvent* event = eventSystem.PollEvent()) {
            if (auto wcl = event->As<NkWindowCloseEvent>()) {
                if (wcl->GetWindowId() == window.GetId()) { 
                    running = false;
                    break;
                }
            }

            if (auto wcl = event->As<NkWindowResizeEvent>()){
                resize = true;
                size.width = wcl->GetWidth();
                size.height = wcl->GetHeight();
            }

            if (auto wcl = event->As<NkWindowMaximizeEvent>()){
                resize = true;
                size = window.GetSize();
            }

            if (auto wcl = event->As<NkWindowMinimizeEvent>()){
                resize = true;
                size.width = 0;
                size.height = 0;
            }
        }

        if (!running) break;

        if (size.width == 0 || size.height == 0) continue;

        if (resize) {
            fb->Resize(size.width, size.height, window);
        }

        if (fb == nullptr) continue;

        fb->Clear(math::NkColor::Red);
        fb->SetPixel(100, 100, math::NkColor::White());

        fb->drawRectangle(150, 300, 150, 300, math::NkColor::White());
        fb->Present();


        // --- Cap 60 fps ---
        elapsed = chrono.Elapsed();
        if (elapsed.milliseconds < 16)
            NkChrono::Sleep(16 - elapsed.milliseconds);
        else
            NkChrono::YieldThread();
    }

    delete fb;

    window.Close();

    return 0;
}