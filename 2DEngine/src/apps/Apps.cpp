#include "NKWindow/Core/NkMain.h"
#include "NKWindow/NkWindow.h"
#include "NKLogger/NkLog.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKMath/NkColor.h"

#if defined(NKENTSEU_PLATFORM_WINDOWS)
    #include <windows.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace nkentseu;

namespace {

class Framebuffer {
public:
    explicit Framebuffer(NkWindow& window) {
        const auto size = window.GetSize();
        Resize(size.width, size.height, window);
    }

    ~Framebuffer() {
        CloseBackend();
    }

    bool Resize(uint32 width, uint32 height, NkWindow& window) {
        CloseBackend();

        mWidth = width;
        mHeight = height;
        mStrideBytes = mWidth * 4u;

        if (mWidth == 0 || mHeight == 0) {
            mPixels.clear();
            return true;
        }

        const std::size_t totalBytes =
            static_cast<std::size_t>(mStrideBytes) * static_cast<std::size_t>(mHeight);

        mPixels.assign(totalBytes, 0);

        return InitBackend(window);
    }

    bool IsValid() const {
#if defined(NKENTSEU_PLATFORM_WINDOWS)
        return mWidth > 0 && mHeight > 0 && !mPixels.empty() && mDibBits != nullptr && mDibDC != nullptr && mWindowDC != nullptr;
#else
        return mWidth > 0 && mHeight > 0 && !mPixels.empty();
#endif
    }

    uint32 GetWidth() const { return mWidth; }
    uint32 GetHeight() const { return mHeight; }

    void SetPixel(int x, int y, const math::NkColor& color) {
        if (x < 0 || y < 0 || x >= static_cast<int>(mWidth) || y >= static_cast<int>(mHeight)) {
            return;
        }

        const std::size_t index =
            static_cast<std::size_t>(y) * static_cast<std::size_t>(mStrideBytes) +
            static_cast<std::size_t>(x) * 4u;

#if defined(NKENTSEU_PLATFORM_WINDOWS)
        // Win32 DIB memory is BGRA, so convert from logical RGBA here.
        mPixels[index + 0] = color.b;
        mPixels[index + 1] = color.g;
        mPixels[index + 2] = color.r;
        mPixels[index + 3] = color.a;
#else
        mPixels[index + 0] = color.r;
        mPixels[index + 1] = color.g;
        mPixels[index + 2] = color.b;
        mPixels[index + 3] = color.a;
#endif
    }

    void Clear(const math::NkColor& color) {
#if defined(NKENTSEU_PLATFORM_WINDOWS)
        const uint8 p0 = color.b;
        const uint8 p1 = color.g;
        const uint8 p2 = color.r;
        const uint8 p3 = color.a;
#else
        const uint8 p0 = color.r;
        const uint8 p1 = color.g;
        const uint8 p2 = color.b;
        const uint8 p3 = color.a;
#endif

        for (std::size_t i = 0; i + 3 < mPixels.size(); i += 4) {
            mPixels[i + 0] = p0;
            mPixels[i + 1] = p1;
            mPixels[i + 2] = p2;
            mPixels[i + 3] = p3;
        }
    }

    void DrawFilledRect(int x, int y, int width, int height, const math::NkColor& color) {
        if (width <= 0 || height <= 0) {
            return;
        }

        const int startX = std::max(0, x);
        const int startY = std::max(0, y);
        const int endX = std::min(static_cast<int>(mWidth), x + width);
        const int endY = std::min(static_cast<int>(mHeight), y + height);

        for (int py = startY; py < endY; ++py) {
            for (int px = startX; px < endX; ++px) {
                SetPixel(px, py, color);
            }
        }
    }

    void DrawFilledCircle(int centerX, int centerY, int radius, const math::NkColor& color) {
        if (radius <= 0) {
            return;
        }

        const int radiusSquared = radius * radius;

        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                if (x * x + y * y <= radiusSquared) {
                    SetPixel(centerX + x, centerY + y, color);
                }
            }
        }
    }

    void Present() {
#if defined(NKENTSEU_PLATFORM_WINDOWS)
        if (mDibBits == nullptr || mDibDC == nullptr || mWindowDC == nullptr || mPixels.empty()) {
            return;
        }

        std::memcpy(mDibBits, mPixels.data(), mPixels.size());
        BitBlt(
            static_cast<HDC>(mWindowDC),
            0,
            0,
            static_cast<int>(mWidth),
            static_cast<int>(mHeight),
            static_cast<HDC>(mDibDC),
            0,
            0,
            SRCCOPY
        );
#endif
    }

private:
    bool InitBackend(NkWindow& window) {
#if defined(NKENTSEU_PLATFORM_WINDOWS)
        mWindowHandle = window.GetSurfaceDesc().hwnd;
        if (mWindowHandle == nullptr) {
            logger.Error("[2DEngine] Missing HWND for framebuffer backend");
            return false;
        }

        mWindowDC = GetDC(static_cast<HWND>(mWindowHandle));
        if (mWindowDC == nullptr) {
            logger.Error("[2DEngine] GetDC failed");
            return false;
        }

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(mWidth);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(mHeight); // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* dibBits = nullptr;
        mDibBitmap = CreateDIBSection(
            static_cast<HDC>(mWindowDC),
            &bmi,
            DIB_RGB_COLORS,
            &dibBits,
            nullptr,
            0
        );

        if (mDibBitmap == nullptr || dibBits == nullptr) {
            logger.Error("[2DEngine] CreateDIBSection failed");
            CloseBackend();
            return false;
        }

        mDibBits = dibBits;
        mDibDC = CreateCompatibleDC(static_cast<HDC>(mWindowDC));
        if (mDibDC == nullptr) {
            logger.Error("[2DEngine] CreateCompatibleDC failed");
            CloseBackend();
            return false;
        }

        SelectObject(static_cast<HDC>(mDibDC), static_cast<HBITMAP>(mDibBitmap));
        return true;
#else
        (void)window;
        return false;
#endif
    }

    void CloseBackend() {
#if defined(NKENTSEU_PLATFORM_WINDOWS)
        if (mDibDC != nullptr) {
            DeleteDC(static_cast<HDC>(mDibDC));
            mDibDC = nullptr;
        }

        if (mDibBitmap != nullptr) {
            DeleteObject(static_cast<HBITMAP>(mDibBitmap));
            mDibBitmap = nullptr;
        }

        if (mWindowDC != nullptr && mWindowHandle != nullptr) {
            ReleaseDC(static_cast<HWND>(mWindowHandle), static_cast<HDC>(mWindowDC));
            mWindowDC = nullptr;
        }

        mDibBits = nullptr;
        mWindowHandle = nullptr;
#endif
    }

private:
    uint32 mWidth = 0;
    uint32 mHeight = 0;
    uint32 mStrideBytes = 0;
    std::vector<uint8> mPixels;

#if defined(NKENTSEU_PLATFORM_WINDOWS)
    void* mWindowHandle = nullptr;
    void* mWindowDC = nullptr;
    void* mDibDC = nullptr;
    void* mDibBitmap = nullptr;
    void* mDibBits = nullptr;
#endif
};

struct Bullet {
    float x = 0.0f;
    float y = 0.0f;
    bool alive = true;
};

struct Enemy {
    float x = 0.0f;
    float y = 0.0f;
    bool alive = true;
};

} // namespace

int nkmain(const NkEntryState& /*state*/) {
    NkWindowConfig cfg;
    cfg.title = "2D Shooter";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.centered = true;
    cfg.resizable = true;
    cfg.dropEnabled = true;

    NkWindow window;
    if (!window.Create(cfg)) {
        logger.Error("[2DEngine] Window creation failed");
        return -2;
    }

    Framebuffer framebuffer(window);
    if (!framebuffer.IsValid()) {
        logger.Error("[2DEngine] Framebuffer initialization failed");
        window.Close();
        return -3;
    }

    uint32 viewportWidth = framebuffer.GetWidth();
    uint32 viewportHeight = framebuffer.GetHeight();

    constexpr float kPlayerHalfSize = 22.0f;
    constexpr float kPlayerSpeed = 420.0f;
    constexpr float kBulletRadius = 5.0f;
    constexpr float kBulletSpeed = 760.0f;
    constexpr float kEnemyRadius = 20.0f;
    constexpr float kFireIntervalSeconds = 0.16f;

    const float playerX = 40.0f + kPlayerHalfSize;
    float playerY = static_cast<float>(viewportHeight) * 0.5f;

    bool moveUpHeld = false;
    bool moveDownHeld = false;
    bool shootHeld = false;

    float fireCooldown = 0.0f;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    auto resetEnemies = [&]() {
        enemies.clear();

        const int enemyCount = 8;
        const float startX = static_cast<float>(viewportWidth) - 180.0f;
        const float laneOffset = 80.0f;
        const float topY = 90.0f;
        const float bottomY = std::max(topY, static_cast<float>(viewportHeight) - 90.0f);

        for (int i = 0; i < enemyCount; ++i) {
            const float t = (enemyCount > 1)
                ? static_cast<float>(i) / static_cast<float>(enemyCount - 1)
                : 0.5f;

            Enemy enemy;
            enemy.x = startX + ((i % 2 == 0) ? 0.0f : laneOffset);
            enemy.y = topY + (bottomY - topY) * t;
            enemy.alive = true;
            enemies.push_back(enemy);
        }
    };

    resetEnemies();

    NkChrono frameChrono;
    auto& events = NkEvents();
    bool running = true;

    while (running) {
        const NkElapsedTime frameElapsed = frameChrono.Reset();
        const float deltaSeconds = std::clamp(static_cast<float>(frameElapsed.seconds), 0.0f, 0.05f);

        bool resized = false;

        while (NkEvent* event = events.PollEvent()) {
            if (auto* closeEvent = event->As<NkWindowCloseEvent>()) {
                if (closeEvent->GetWindowId() == window.GetId()) {
                    running = false;
                    break;
                }
            }

            if (auto* resizeEvent = event->As<NkWindowResizeEvent>()) {
                viewportWidth = resizeEvent->GetWidth();
                viewportHeight = resizeEvent->GetHeight();
                resized = true;
            }

            if (auto* maximizeEvent = event->As<NkWindowMaximizeEvent>()) {
                (void)maximizeEvent;
                const auto size = window.GetSize();
                viewportWidth = size.width;
                viewportHeight = size.height;
                resized = true;
            }

            if (auto* minimizeEvent = event->As<NkWindowMinimizeEvent>()) {
                (void)minimizeEvent;
                viewportWidth = 0;
                viewportHeight = 0;
                resized = true;
            }

            auto handleKeyDown = [&](NkKey key) {
                if (key == NkKey::NK_UP) {
                    moveUpHeld = true;
                } else if (key == NkKey::NK_DOWN) {
                    moveDownHeld = true;
                } else if (key == NkKey::NK_SPACE) {
                    shootHeld = true;
                }
            };

            if (auto* keyPressEvent = event->As<NkKeyPressEvent>()) {
                handleKeyDown(keyPressEvent->GetKey());
            }

            if (auto* keyRepeatEvent = event->As<NkKeyRepeatEvent>()) {
                handleKeyDown(keyRepeatEvent->GetKey());
            }

            if (auto* keyReleaseEvent = event->As<NkKeyReleaseEvent>()) {
                const NkKey key = keyReleaseEvent->GetKey();
                if (key == NkKey::NK_UP) {
                    moveUpHeld = false;
                } else if (key == NkKey::NK_DOWN) {
                    moveDownHeld = false;
                } else if (key == NkKey::NK_SPACE) {
                    shootHeld = false;
                }
            }
        }

        if (!running) {
            break;
        }

        if (resized && viewportWidth > 0 && viewportHeight > 0) {
            if (!framebuffer.Resize(viewportWidth, viewportHeight, window)) {
                logger.Error("[2DEngine] Framebuffer resize failed");
                break;
            }

            playerY = std::clamp(
                playerY,
                kPlayerHalfSize,
                std::max(kPlayerHalfSize, static_cast<float>(viewportHeight) - kPlayerHalfSize)
            );

            resetEnemies();
        }

        if (viewportWidth == 0 || viewportHeight == 0 || !framebuffer.IsValid()) {
            NkChrono::Sleep(16.0);
            continue;
        }

        if (moveUpHeld) {
            playerY -= kPlayerSpeed * deltaSeconds;
        }

        if (moveDownHeld) {
            playerY += kPlayerSpeed * deltaSeconds;
        }

        const float minPlayerY = kPlayerHalfSize;
        const float maxPlayerY = std::max(kPlayerHalfSize, static_cast<float>(viewportHeight) - kPlayerHalfSize);
        playerY = std::clamp(playerY, minPlayerY, maxPlayerY);

        fireCooldown = std::max(0.0f, fireCooldown - deltaSeconds);
        if (shootHeld && fireCooldown <= 0.0f) {
            Bullet bullet;
            bullet.x = playerX + kPlayerHalfSize + 8.0f;
            bullet.y = playerY;
            bullets.push_back(bullet);

            fireCooldown = kFireIntervalSeconds;
        }

        for (Bullet& bullet : bullets) {
            if (!bullet.alive) {
                continue;
            }

            bullet.x += kBulletSpeed * deltaSeconds;

            if (bullet.x - kBulletRadius > static_cast<float>(viewportWidth)) {
                bullet.alive = false;
            }
        }

        const float collisionDistance = kBulletRadius + kEnemyRadius;
        const float collisionDistanceSquared = collisionDistance * collisionDistance;

        for (Bullet& bullet : bullets) {
            if (!bullet.alive) {
                continue;
            }

            for (Enemy& enemy : enemies) {
                if (!enemy.alive) {
                    continue;
                }

                const float dx = bullet.x - enemy.x;
                const float dy = bullet.y - enemy.y;
                const float distanceSquared = dx * dx + dy * dy;

                if (distanceSquared <= collisionDistanceSquared) {
                    bullet.alive = false;
                    enemy.alive = false;
                    break;
                }
            }
        }

        bullets.erase(
            std::remove_if(
                bullets.begin(),
                bullets.end(),
                [](const Bullet& bullet) { return !bullet.alive; }
            ),
            bullets.end()
        );

        framebuffer.Clear(math::NkColor::Black());

        framebuffer.DrawFilledRect(
            static_cast<int>(playerX - kPlayerHalfSize),
            static_cast<int>(playerY - kPlayerHalfSize),
            static_cast<int>(kPlayerHalfSize * 2.0f),
            static_cast<int>(kPlayerHalfSize * 2.0f),
            math::NkColor::Blue
        );

        for (const Bullet& bullet : bullets) {
            framebuffer.DrawFilledCircle(
                static_cast<int>(bullet.x),
                static_cast<int>(bullet.y),
                static_cast<int>(kBulletRadius),
                math::NkColor::Yellow
            );
        }

        for (const Enemy& enemy : enemies) {
            if (!enemy.alive) {
                continue;
            }

            framebuffer.DrawFilledCircle(
                static_cast<int>(enemy.x),
                static_cast<int>(enemy.y),
                static_cast<int>(kEnemyRadius),
                math::NkColor::Red
            );
        }

        framebuffer.Present();

        const NkElapsedTime postFrame = frameChrono.Elapsed();
        if (postFrame.milliseconds < 16) {
            NkChrono::Sleep(16.0 - static_cast<double>(postFrame.milliseconds));
        } else {
            NkChrono::YieldThread();
        }
    }

    window.Close();

    return 0;
}
