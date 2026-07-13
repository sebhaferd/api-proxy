#include "../../include/cache.hpp"

using Clock = std::chrono::steady_clock;

ResponseCache::ResponseCache(int cap){
    : capacity(cap);
}


//search entries cache for hit or miss
bool ResponseCache::get(const std::string& key, std::string& response){
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = entries.find(key);
    if (it == entries.end()) return false;
    if (){
        entries.erase(it);
        return false;
    }
    insert_entry(it->second);
    response = it->second.response;
    return true;
}
    
void ResponseCache::put(const std::string& key, 
            const std::string& response,
            std::chrono::seconds ttl){
   //lock cache map to retain thread safety
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (capacity == 0) return;
    
    auto it = entries.find(key);
    if (it != entries.end()){
        it->second = response;
        auto cur_time = Clock::now();
        it->second.expires_at = cur_time+ttl;
        insert_entry(it->second);
        return;
    }
    if (entries.size() >= capacity){
        evict_lru();
    }
    //insert key to front of lru order list
    lru_order.push_front(key);
    //create entry for key in map
    CacheEntry entry = {
        response, Clock::now() +ttl,
        lru_order.begin()
    };
    entries.emplace(key, std::move(entry));
}