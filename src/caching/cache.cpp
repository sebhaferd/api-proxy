#include "../../include/cache.hpp"

using Clock = std::chrono::steady_clock;

ResponseCache::ResponseCache(int cap)
    : capacity(cap) {
}

void ResponseCache::insert_entry(CacheEntry& entry) {
    lru_order.splice(
        lru_order.begin(),
        lru_order,
        entry.lru_position
    );

    entry.lru_position = lru_order.begin();
}

void ResponseCache::evict_lru() {
    if (lru_order.empty()) {
        return;
    }

    const std::string lru_key = lru_order.back();

    entries.erase(lru_key);
    lru_order.pop_back();
}

// Search entries cache for hit or miss
bool ResponseCache::get(
    const std::string& key,
    std::string& response
) {
    std::lock_guard<std::mutex> lock(cache_mutex);

    auto it = entries.find(key);

    if (it == entries.end()) {
        return false;
    }

    if (Clock::now() >= it->second.expires_at) {
        lru_order.erase(it->second.lru_position);
        entries.erase(it);
        return false;
    }

    insert_entry(it->second);

    response = it->second.response;
    return true;
}

void ResponseCache::put(
    const std::string& key,
    const std::string& response,
    std::chrono::seconds ttl
) {
    std::lock_guard<std::mutex> lock(cache_mutex);

    if (capacity == 0) {
        return;
    }

    auto it = entries.find(key);

    if (it != entries.end()) {
        it->second.response = response;
        it->second.expires_at = Clock::now() + ttl;

        insert_entry(it->second);
        return;
    }

    if (entries.size() >= capacity) {
        evict_lru();
    }

    lru_order.push_front(key);

    CacheEntry entry{
        response,
        Clock::now() + ttl,
        lru_order.begin()
    };

    entries.emplace(key, std::move(entry));
}