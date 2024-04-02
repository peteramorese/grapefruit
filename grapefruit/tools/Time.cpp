#include "tools/Time.h"

#include "tools/Logging.h"

namespace GF{

double convert(const std::chrono::microseconds& duration, TimeUnit unit) {
    switch (unit) {
        case TimeUnit::us:
            return static_cast<double>(duration.count());
        case TimeUnit::ms:
            return static_cast<double>(duration.count()) / 1000;
        case TimeUnit::s:
            return static_cast<double>(duration.count()) / 1000000;
    }
    ERROR("Unrecognized time unit");
    return 0.0;
}

}

double GF::Profiler::getMostRecentProfile(const std::string& key, TimeUnit unit) {
    auto it = s_profiles.find(key);
    if (it == s_profiles.end()) {
        ERROR("Profile key '" << key << "' not found, has a Timer object been created with that key?");
        return 0.0;
    }
    return convert(it->second.most_recent_duration, unit);
}

double GF::Profiler::getTotalProfile(const std::string& key, TimeUnit unit) {
    auto it = s_profiles.find(key);
    if (it == s_profiles.end()) {
        ERROR("Profile key '" << key << "' not found, has a Timer object been created with that key?");
        return 0.0;
    }
    return convert(it->second.total_duration, unit);
}

GF::Profiler::Data& GF::Profiler::getData(const std::string& key) {
    auto it = s_profiles.find(key);
    if (it == s_profiles.end()) {
        GF::Profiler::Data data{std::chrono::microseconds(0), std::chrono::microseconds(0)};
        it = s_profiles.insert(std::make_pair(key, data)).first;
    }
    return it->second;
}

GF::Timer::Timer(const std::string& key) 
    : m_key(key)
    , m_start(std::chrono::system_clock::now()) {}

void GF::Timer::stop() {
    GF::Profiler::Data& data = Profiler::getData(m_key);
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
    data.most_recent_duration = duration;
    data.total_duration += duration;
    m_key.clear();
}

double GF::Timer::now(TimeUnit unit) {
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
    return convert(duration, unit);
}

GF::Timer::~Timer() {
    if (!m_key.empty()) {
        stop();
    }
}