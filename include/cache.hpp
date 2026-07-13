#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <list>
#include <cstddef>

struct CacheEntry{
    std::string response;
    std::chrono::steady_clock::time_point expires_at;
    std::list<std::string>::iterator lru_position;
};




class ResponseCache{
private:
    explicit ResponseCache();
    std::unordered_map<std::string, CacheEntry> entries;
    std::mutex cache_mutex;
    std::list<std::string> lru_order; // (mru) front -> back (lru)
    std::size_t capacity;
    void insert_entry(CacheEntry& entry);
    void remove_entry(std::unordered_map<std::list<std::string>::iterator entry_it);
    void evict_lru();
    

public:
    bool get(const std::string& key, std::string& response);
    void put(const str::string& key, 
            const std::string& response
            std::chrono::seconds ttl);
    size_t size();
};