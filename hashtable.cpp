#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <regex>
#include <array>

// Node structure
struct Node {
    std::string key; // domain name
    std::string value; // IP address
    Node* next;
};

// Initialize the node
void setNode(Node* node, const std::string& key, const std::string& value) {
    node->key = key;
    node->value = value;
    node->next = nullptr;
}

// Hash Map structure
struct HashMap {
    int numOfElements, capacity;
    Node** arr;
};

// Initialize the hash map
void initializeHashMap(HashMap* mp, int capacity = 100) {
    mp->capacity = capacity;
    mp->numOfElements = 0;
    mp->arr = (Node**)malloc(sizeof(Node*) * mp->capacity);
    for (int i = 0; i < mp->capacity; ++i) {
        mp->arr[i] = nullptr;
    }
}

// Simple hash function for strings
int hashFunction(HashMap* mp, const std::string& key) {
    int hash = 0;
    int prime = 31;
    for (char c : key) {
        hash = (hash * prime + c) % mp->capacity;
    }
    return hash;
}

// Insert key-value pair into the hash map
void insert(HashMap* mp, const std::string& key, const std::string& value) {
    int bucketIndex = hashFunction(mp, key);
    Node* newNode = (Node*)malloc(sizeof(Node));
    setNode(newNode, key, value);
    newNode->next = mp->arr[bucketIndex];
    mp->arr[bucketIndex] = newNode;
    mp->numOfElements++;
}

// Delete key from the hash map
void deleteKey(HashMap* mp, const std::string& key) {
    int bucketIndex = hashFunction(mp, key);
    Node* currNode = mp->arr[bucketIndex];
    Node* prevNode = nullptr;
    while (currNode != nullptr) {
        if (currNode->key == key) {
            if (prevNode == nullptr) {
                mp->arr[bucketIndex] = currNode->next;
            } else {
                prevNode->next = currNode->next;
            }
            free(currNode);
            mp->numOfElements--;
            return;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }
}

// Search for a key in the hash map
std::string search(HashMap* mp, const std::string& key) {
    int bucketIndex = hashFunction(mp, key);
    Node* currNode = mp->arr[bucketIndex];
    while (currNode != nullptr) {
        if (currNode->key == key) {
            return currNode->value;
        }
        currNode = currNode->next;
    }
    return "Oops! No data found.\n";
}

// Function to execute a system command and return the output
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

// Function to retrieve DNS cache

std::unordered_map<std::string, std::string> retrieveDNSCache() {
    std::unordered_map<std::string, std::string> dnsCache;
    std::string output = exec("ipconfig /displaydns");

    // Print the output for debugging
    std::cout << "Output: " << output << std::endl;

    std::regex rgx("Record Name\\s+\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\s+(.+)\n.*?Record Type\\s+\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\s+A\n.*?Record Data\\s+\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\s+(.+)\n");
    std::smatch matches;
    std::string::const_iterator searchStart(output.cbegin());
    while (std::regex_search(searchStart, output.cend(), matches, rgx)) {
        dnsCache[matches[1].str()] = matches[2].str();
        searchStart = matches.suffix().first;
    }

    // Test Print the DNS cache to terminal
    std::cout << "DNS Cache:" << std::endl;
    for (const auto& pair : dnsCache) {
        //Only comment or uncomment the line below to test print the DNS cache. At this point the DNS cache is not yet inserted into the hash map.
        // std::cout << "Record Name: " << pair.first << ", Record Data: " << pair.second << std::endl;
    }

    return dnsCache;
}


// Write hash map to a file
void writeHashMapToFile(HashMap* mp, const std::string& fileName) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        for (int i = 0; i < mp->capacity; ++i) {
            Node* currNode = mp->arr[i];
            while (currNode != nullptr) {
                outFile << currNode->key << " " << currNode->value << "\n";
                currNode = currNode->next;
            }
        }
        outFile.close();
    } else {
        std::cerr << "Unable to open file for writing: " << fileName << std::endl;
    }
}

// Function to print the hash map
void printHashMap(HashMap* mp) {
    for (int i = 0; i < mp->capacity; ++i) {
        Node* currNode = mp->arr[i];
        while (currNode != nullptr) {
            std::cout << "Key: " << currNode->key << ", Value: " << currNode->value << std::endl;
            currNode = currNode->next;
        }
    }
}

int main() {
    HashMap* mp = (HashMap*)malloc(sizeof(HashMap));
    initializeHashMap(mp);

    // Retrieve DNS cache and insert into the hash map
    auto dnsCache = retrieveDNSCache();
    for (const auto& entry : dnsCache) {
        insert(mp, entry.first, entry.second);
    }
    // Print the hash map to terminal
    printHashMap(mp);
    writeHashMapToFile(mp, "dns_cache.txt");

    // Test search of a key in the hash map
    std::cout << "Search result for google.com: " << search(mp, "google.com") << std::endl;

    // Test deletion of a key from the hash map
    deleteKey(mp, "google.com");
    std::cout << "Search result for google.com after deletion: " << search(mp, "google.com") << std::endl;

    free(mp->arr);
    free(mp);
    return 0;
}
