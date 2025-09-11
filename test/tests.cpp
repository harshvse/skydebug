#include <sys/signal.h>

#include <catch2/catch_test_macros.hpp>
#include <libskydebug/error.hpp>
#include <libskydebug/process.hpp>

namespace {
bool process_exists(pid_t pid) {
  auto ret = kill(pid, 0);
  return ret != -1 and errno != ESRCH;
}
}  // namespace

TEST_CASE("skydebug::process::launch success", "process") {
  auto proc = skydebug::process::launch("yes");
  REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("skydebug::process::launch no such program", "process") {
  REQUIRE_THROWS_AS(skydebug::process::launch("get_gud_at_everything"),
                    skydebug::error);
}
