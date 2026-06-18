// engine/graphics/ShaderLoader.cpp

#include "ShaderLoader.hpp"
#include "NKLogger/NkLog.h"
#include <fstream>
#include <sstream>

namespace engine {

using namespace nkentseu;

std::string ShaderLoader::LoadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger.Errorf("[ShaderLoader] Cannot open file: %s\n", path.c_str());
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

ShaderSource ShaderLoader::LoadFiles(const std::string& vertPath,
                                     const std::string& fragPath,
                                     const std::string& geomPath)
{
    ShaderSource src;
    src.vertex   = LoadFile(vertPath);
    src.fragment = LoadFile(fragPath);
    if (!geomPath.empty()) src.geometry = LoadFile(geomPath);
    src.ok = !src.vertex.empty() && !src.fragment.empty();
    if (!src.ok) {
        logger.Errorf("[ShaderLoader] Failed to load shaders: %s / %s\n",
                      vertPath.c_str(), fragPath.c_str());
    }
    return src;
}

ShaderSource ShaderLoader::FromSource(const std::string& vert,
                                      const std::string& frag,
                                      const std::string& geom)
{
    ShaderSource src;
    src.vertex   = vert;
    src.fragment = frag;
    src.geometry = geom;
    src.ok       = !vert.empty() && !frag.empty();
    return src;
}

} // namespace engine
