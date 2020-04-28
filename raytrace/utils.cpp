#include <fstream>
#include <memory>

#include "string.h"

std::string LoadFile(const std::string& fileName) {
    std::ifstream ifs;
    ifs.open(fileName, std::fstream::binary | std::fstream::in);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    return content;
}

void SaveFile(const std::string& fileName, const std::string& data) {
    std::ofstream ofs;
    ofs.open(fileName, std::fstream::binary | std::fstream::out);
    ofs.write(data.c_str(), data.size());
}
