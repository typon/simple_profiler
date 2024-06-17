#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>


namespace spf {

struct ProfileData {
  std::int64_t count = 0;
  std::int64_t sum = 0;
  std::int64_t sum_sq = 0;
  std::int64_t min = std::numeric_limits<std::int64_t>::max();
  std::int64_t max = 0;
};

struct ProfileDataPlus {
  double mean;
  double stddev;
  std::string scope_name;
  std::int64_t count;
  std::int64_t sum;
  std::int64_t min;
  std::int64_t max;
};

struct ScopeInfo {
  std::size_t thread_id;
  std::string name;
  inline bool operator==(const ScopeInfo& other) const {
    return this->thread_id == other.thread_id and this->name == other.name;
  };
};

} // end namespace spf

namespace std {
template <> struct hash<spf::ScopeInfo> {
  std::size_t operator()(const spf::ScopeInfo& obj) const noexcept {
    std::size_t h1 = std::hash<std::size_t>()(obj.thread_id);
    std::size_t h2 = std::hash<std::string>()(obj.name);
    return h1 ^ (h2 << 1);
  }
};
} // namespace std

namespace spf {

class SimpleProfiler {
public:
  // Disallow copying and assigning
  SimpleProfiler(const SimpleProfiler&) = delete;
  SimpleProfiler& operator=(const SimpleProfiler&) = delete;

  static inline SimpleProfiler& get_instance() {
    static SimpleProfiler instance;
    return instance;
  }

  inline void start_profiling(const std::string& scope_name) {
    std::scoped_lock lock(mutex_);
    ScopeInfo scope_info = {
        std::hash<std::thread::id>{}(std::this_thread::get_id()), scope_name};
    timers_[scope_info] = std::chrono::high_resolution_clock::now();
  }

  inline void end_profiling(const std::string& scope_name) {
    std::scoped_lock lock(mutex_);
    auto end_time = std::chrono::high_resolution_clock::now();
    ScopeInfo scope_info = {
        std::hash<std::thread::id>{}(std::this_thread::get_id()), scope_name};

    auto start_time = timers_.at(scope_info);
    timers_.erase(scope_info);
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                        end_time - start_time)
                        .count();

    auto& data = aggregated_data_[scope_name];
    data.count += 1;
    data.sum += duration;
    data.sum_sq += duration * duration;
    data.min = std::min(data.min, duration);
    data.max = std::max(data.max, duration);
  }

  inline static std::vector<ProfileDataPlus> get_aggregated_data() {
    auto& profiler = get_instance();
    std::scoped_lock lock(profiler.mutex_);

    std::vector<ProfileDataPlus> result;

    for (const auto& pair : profiler.aggregated_data_) {
      const auto& scope_name = pair.first;
      const auto& data = pair.second;
      double mean = data.sum / data.count;
      double variance = (data.sum_sq / data.count) - (mean * mean);
      double stddev = std::sqrt(variance);
      result.push_back(ProfileDataPlus{.mean = mean,
                                       .stddev = stddev,
                                       .scope_name = scope_name,
                                       .count = data.count,
                                       .sum = data.sum,
                                       .min = data.min,
                                       .max = data.max});
    }
    return result;
  }

private:
  SimpleProfiler() = default;
  ~SimpleProfiler() = default;

  std::unordered_map<ScopeInfo, std::chrono::high_resolution_clock::time_point>
      timers_;
  std::unordered_map<std::string, ProfileData> aggregated_data_;
  std::mutex mutex_;
};

#define PROFILE_SCOPE(scope_name)                                              \
  spf::SimpleProfiler::get_instance().start_profiling(scope_name);             \
  std::shared_ptr<void> _profile_scope_guard(nullptr, [&](...) {               \
    spf::SimpleProfiler::get_instance().end_profiling(scope_name);             \
  })

} // end namespace spf
