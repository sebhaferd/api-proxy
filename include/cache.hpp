#pragma once

#include <chrono>
#include <cstddef>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

struct CacheEntry{
    std::string response;
    std::chrono::steady_clock::time_point expires_at;
    std::list<std::string>::iterator lru_position;
};




class ResponseCache{
private:
    explicit ResponseCache(std::size_t capacity);
    std::unordered_map<std::string, CacheEntry> entries;
    std::mutex cache_mutex;
    std::list<std::string> lru_order; // (mru) front -> back (lru)
    std::size_t capacity;
    void insert_entry(CacheEntry& entry);
    void evict_lru();
    

public:
    bool get(const std::string& key, std::string& response);
    void put(const std::string& key, 
            const std::string& response,
            std::chrono::seconds ttl);
    size_t size();
};