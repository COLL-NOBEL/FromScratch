#pragma once
// engine/graphics/ShaderLoader.hpp
// Loads GLSL shader source from a file on disk.

#include <string>

namespace engine {

struct ShaderSource {
    std::string vertex;
    std::string fragment;
    std::string geometry;  // empty if not used
    bool        ok = false;
};

class ShaderLoader {
public:
    // Load a single text file and return its contents
    static std::string LoadFile(const std::string& path);

    // Load vertex + fragment (and optionally geometry) from separate files
    static ShaderSource LoadFiles(const std::string& vertPath,
                                  const std::string& fragPath,
                                  const std::string& geomPath = "");

    // Embed a shader source string directly (pass-through helper)
    static ShaderSource FromSource(const std::string& vert,
                                   const std::string& frag,
                                   const std::string& geom = "");
};

} // namespace engine
