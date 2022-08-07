#include "pch.h"
#include "Shader.h"

namespace gfx
{
    using namespace std::string_literals;

    const bool Shader::PathEndsWithCSO(const char* path) const
    {
        static std::string cso = std::string(".cso");
        std::string sPath = std::string(path);
        return (0 == sPath.compare(sPath.length() - cso.length(), cso.length(), cso));
    }
}