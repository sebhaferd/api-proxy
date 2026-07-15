#pragma once
#include <iostream> 
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

class RateLimiter{
public:
    RateLimiter(double capacity, double refill_rate);
    bool allow(const std::string& client_ip);

private:
    using Clock = std::chrono::steady_clock;
    //bucket to store number of tokens per ip
    struct Bucket{
        double tokens;
        Clock::time_point last_refill;
    };

    std::unordered_map<std::string, Bucket> buckets;
    std::mutex bucket_mutex;
    double capacity;
    double refill_rate;
};