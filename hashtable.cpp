#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <regex>
#include <fstream>
#include <filesystem>
#include <array>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

const int TABLE_SIZE = 100; // Size of the hash table

// Structure to store each hash table entry
struct HashEntry {
    std::string key;
    std::string value;
};

// Simple hash table class with chaining for collision resolution
class SimpleHashTable {
private:
    std::vector<std::list<HashEntry>> table; // Vector of lists to handle collisions with chaining

public:
    SimpleHashTable() {
        table.resize(TABLE_SIZE); // Initialize the table with empty lists
    }

    // Hash function to compute index for a given key
    int hashFunction(const std::string& key) {
        int hash = 0;
        int prime = 31;
        for (char c : key) {
            hash = (hash * prime + c) % TABLE_SIZE;
        }
        return hash;
    }

    // Insert function to add key-value pairs to the hash table
    void insert(const std::string& key, const std::string& value) {
        int index = hashFunction(key); // Compute the index for the key
        table[index].push_back({key, value}); // Add the key-value pair to the list at the computed index
    }

    // Contains function to check if a key is present in the hash table
    bool contains(const std::string& key) {
        int index = hashFunction(key); // Compute the index for the key
        for (auto& entry : table[index]) { // Iterate over the list at the computed index
            if (entry.key == key) {
                return true;
            }
        }
        return false;
    }

    // Get function to retrieve the value associated with a key
    std::string get(const std::string& key) {
        int index = hashFunction(key); // Compute the index for the key
        for (auto& entry : table[index]) { // Iterate over the list at the computed index
            if (entry.key == key) {
                return entry.value;
            }
        }
        return "Key not found";
    }

    // Print function to display the contents of the hash table
    void printHashTable() const {
        std::cout << "\nPrinting the entire hash table:\n";
        for (int i = 0; i < TABLE_SIZE; i++) {
            for (auto& entry : table[i]) { // Iterate over each list in the table
                if (!entry.key.empty()) {
                    std::cout << "Index: " << i << ", Key: " << entry.key << ", Value: " << entry.value << std::endl;
                }
            }
        }
    }
};

// Class to retrieve DNS cache and write it to a file
class DNSCache {
public:
    void retrieveAndWriteToFile(const std::string& fileName) {
        std::string output = exec("ipconfig /displaydns"); // Execute the command to retrieve DNS cache
        std::cout << output << std::endl; // Print the output to the terminal
        writeToFile(fileName, output); // Write the output to a file
    }

private:
    // Function to execute a system command and return the output
    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
        if (!pipe) throw std::runtime_error("_popen() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    // Function to write data to a file
    void writeToFile(const std::string& fileName, const std::string& data) {
        std::filesystem::path filePath = std::filesystem::current_path() / fileName; // Get the current path and append the file name
        std::ofstream outFile(filePath);
        if (outFile.is_open()) {
            outFile << data; // Write data to the file
            outFile.close();
            std::cout << "Data written to file: " << filePath << std::endl;
        } else {
            std::cerr << "Unable to open file for writing: " << filePath << std::endl;
        }
    }
};

// Function to populate the hash table from the data in the file
void populateHashTable(SimpleHashTable& hashTable, const std::string& fileName) {
    std::ifstream inFile(fileName);
    if (!inFile.is_open()) {
        std::cerr << "Unable to open file for reading: " << fileName << std::endl;
        return;
    }

    // Read the entire file content
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    // Regular expression to match DNS records
    std::regex rgx(R"(Record Name\s+\.\.\.\s+(.+)\n.*?Record Type\s+\.\.\.\s+A\n.*?Record Data\s+\.\.\.\s+(.+))");
    std::smatch matches;

    // Iterate over the file content to find all matches
    std::string::const_iterator searchStart(content.cbegin());
    while (std::regex_search(searchStart, content.cend(), matches, rgx)) {
        std::string key = matches[1].str(); // Extract key (record name)
        std::string value = matches[2].str(); // Extract value (record data)
        hashTable.insert(key, value); // Insert key-value pair into the hash table
        searchStart = matches.suffix().first; // Move the search start to the next position
    }
}

int main() {
    DNSCache dnsCache;
    dnsCache.retrieveAndWriteToFile("dns_cache.csv"); // Retrieve DNS cache and write to file

    SimpleHashTable simpleHashTable;
    populateHashTable(simpleHashTable, "dns_cache.csv"); // Populate the hash table from the file

    simpleHashTable.printHashTable(); // Print the hash table

    return 0;
}
