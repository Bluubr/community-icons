#pragma once

#include <string>

struct IconPack {
    std::string id;
    std::string name;
    std::string author;
    std::string gamemode;
    std::string imageUrl;
    std::string plistUrl;
    std::string graphicsType;
    std::string uploadedAt;
    int downloads = 0;
};
