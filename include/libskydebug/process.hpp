#pragma once

#include <sys/types.h>

#include <filesystem>
#include <memory>

namespace skydebug {
class process {
  static std::unique_ptr<process> launch(std::filesystem::path path);
  static std::unique_ptr<process> attach(pid_t pid);

  process() = delete;
  process(const process&) = delete;
  process& operator=(const process&) = delete;

  void resume();
  /* think more about what this function should be returning */ void
  wait_on_signal();
  pid_t pid() const { return pid_; }

 private:
  pid_t pid_ = 0;
};
}  // namespace skydebug