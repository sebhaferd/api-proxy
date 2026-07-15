#include "../../include/rate-limiter.hpp"

RateLimiter::RateLimiter(double capacity, double refill_rate)
    : capacity(capacity), refill_rate(refill_rate){}

bool RateLimiter::allow(const std::string& client_ip){
    //lock map for thread safety
    std::lock_guard(std::mutex) lock(bucket_mutex);
    const auto now = Clock::now();
    auto it = buckets.find(client_ip);
    //if new client allow and set bucket
    if (it == buckets.end){
        Bucket new_bucket = {capacity - 1.0, now}
        buckets.emplace(client_ip, new_bucket);
        return true;
    }
    //compute tokens to add and if they have any tokens allow else limit
    Bucket& bucket = it->second;
    std::chrono::duration<double> elapsed = now - bucket.last_refill;
    double tokens_to_add = elapsed.count * refill_rate;
    bucket.tokens = std::min(capacity, tokens_to_add+bucket.tokens);
    bucket.last_refill = now;
    if (bucket.tokens < 1.0){
        return false;
    }
    bucket.tokens-=1.0;
    return true;
}

