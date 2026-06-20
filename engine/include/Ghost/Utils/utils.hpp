#pragma once
#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ghost::Utils {
std::vector<std::byte> readFile(const std::filesystem::path &filepath);

std::unordered_map<std::string, std::string> loadEnvFile(const std::filesystem::path &envFilepath);
}
