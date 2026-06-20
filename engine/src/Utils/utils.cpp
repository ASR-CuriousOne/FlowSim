#include <Ghost/Utils/utils.hpp>
#include <fstream>

namespace Ghost::Utils {

std::vector<std::byte> readFile(const std::filesystem::path &filepath) {
    std::ifstream shaderFile(filepath, std::ios::ate | std::ios::binary);

    if (!shaderFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }

    std::streamsize fileSize = shaderFile.tellg();
    if (fileSize == -1) {
        throw std::runtime_error("Failed to determine file size: " +
                                 filepath.string());
    }
    std::vector<std::byte> buffer(fileSize);

    shaderFile.seekg(0);
    if (!shaderFile.read(reinterpret_cast<char *>(buffer.data()), fileSize)) {
        throw std::runtime_error("Failed to read file: " + filepath.string());
    }
    shaderFile.close();

    return buffer;
}

std::unordered_map<std::string, std::string>
loadEnvFile(const std::filesystem::path &filepath) {
    std::unordered_map<std::string, std::string> envVars;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        return envVars;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        auto delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            envVars[key] = value;
        }
    }
    return envVars;
}

} // namespace Ghost::Utils
