//independently test DNS cache retrieval and writing to a file. 
#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <fstream>
#include <filesystem>

class DNSCache {
public:
    void retrieveAndWriteToFile(const std::string& fileName) {
        std::string output = exec("ipconfig /displaydns");
        std::cout << output << std::endl;
        writeToFile(fileName, output);
    }

private:
    void writeToFile(const std::string& fileName, const std::string& data) {
        std::filesystem::path filePath = std::filesystem::current_path() / fileName;
        std::ofstream outFile(filePath);
        if (outFile.is_open()) {
            outFile << data;
            outFile.close();
            std::cout << "Data written to file: " << filePath << std::endl;
        } else {
            std::cerr << "Unable to open file for writing: " << filePath << std::endl;
        }
    }

    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        if (!pipe) throw std::runtime_error("popen() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
};

int main() {
    DNSCache dnsCache;
    dnsCache.retrieveAndWriteToFile("dns_cache.txt");
    return 0;
}

