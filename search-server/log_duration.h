#pragma once

#include <chrono>
#include <iostream>
#include <string>

class LogDuration {
public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
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
