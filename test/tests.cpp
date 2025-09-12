#include <sys/signal.h>

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <libskydebug/error.hpp>
#include <libskydebug/process.hpp>

namespace {
bool process_exists(pid_t pid) {
  auto ret = kill(pid, 0);
  return ret != -1 and errno != ESRCH;
}
char get_process_status(pid_t pid) {
  std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
  std::string data;

  std::getline(stat, data);
  auto index_of_last_parenthesis = data.rfind(')');
  auto index_of_status_indicator = index_of_last_parenthesis + 2;
  return data[index_of_status_indicator];
}

}  // namespace

TEST_CASE("skydebug::process::launch success", "[process]") {
  auto proc = skydebug::process::launch("yes");
  REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("skydebug::process::launch no such program", "[process]") {
  REQUIRE_THROWS_AS(skydebug::process::launch("get_gud_at_everything"),
                    skydebug::error);
}

TEST_CASE("skydebug::process::attach success", "[process]") {
  auto target = skydebug::process::launch(
      std::filesystem::absolute("targets/run_endlessly").string(), false);
  auto proc = skydebug::process::attach(target->pid());
  REQUIRE(get_process_status(target->pid()) == 't');
}

TEST_CASE("skydebug::process::attach invalid PID", "[process]") {
  REQUIRE_THROWS_AS(skydebug::process::attach(0), skydebug::error);
}

TEST_CASE("process::resume success", "[process]") {
  {
    auto proc = skydebug::process::launch(
        std::filesystem::absolute("targets/run_endlessly").string(), true);
    proc->resume();
    auto status = get_process_status(proc->pid());
    auto success = status == 'R' or status == 'S';
    REQUIRE(success);
  }

  {
    auto target = skydebug::process::launch(
        std::filesystem::absolute("targets/run_endlessly").string(), false);
    auto proc = skydebug::process::attach(target->pid());
    proc->resume();
    auto status = get_process_status(proc->pid());
    auto success = status == 'R' or status == 'S';
    REQUIRE(success);
  }
}

TEST_CASE("process::resume already terminated", "[process]") {
  auto proc = skydebug::process::launch("targets/end_immediately");
  proc->resume();
  proc->wait_on_signal();
  REQUIRE_THROWS_AS(proc->resume(), skydebug::error);
}
