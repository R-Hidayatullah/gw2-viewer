#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace gw2::foundation
{
    class FileReader
    {
    public:
        static std::vector<std::uint8_t> ReadBinary(const std::string &path);
    };
}
