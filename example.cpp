#include "simple_profiler.hpp"

void bar() {
  PROFILE_SCOPE("bar");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void foo() {
  PROFILE_SCOPE("foo");
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  for (int i = 0; i < 1000; i++) {
    bar();
  }
}

int main() {
  {
    PROFILE_SCOPE("main");
    for (int i = 0; i < 10; i++) {
      foo();
    }
  }

  for (const auto& profile_data : spf::SimpleProfiler::get_aggregated_data()) {
    std::cout << profile_data.scope_name << ": "
              << "Mean: " << profile_data.mean << " μs, "
              << "Std Dev: " << profile_data.stddev << " μs, "
              << "Min: " << profile_data.min << " μs, "
              << "Max: " << profile_data.max << " μs, "
              << "Count: " << profile_data.count << std::endl;
  }

  return 0;
}
