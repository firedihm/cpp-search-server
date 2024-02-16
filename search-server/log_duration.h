#pragma once

#include <chrono>
#include <iostream>
#include <string>

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;
    
    LogDuration(const std::string& msg) : msg_(msg) {}
    
    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;
        
        std::cerr << msg_ << ": "s << duration_cast<milliseconds>(Clock::now() - start_time_).count() << " ms"s << std::endl;
    }
private:
    const Clock::time_point start_time_ = Clock::now();
    const std::string msg_;
};
