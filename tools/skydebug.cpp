// clang-format off
#include <cstdio>
#include <readline/history.h>
#include <readline/readline.h>

//clang-format on
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libskydebug/process.hpp>
#include "libskydebug/error.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::unique_ptr<skydebug::process> attach(int argc, const char** argv) {

  if (argc == 3 && argv[1] == std::string_view("-p")) {
    // passed pid
    pid_t pid = std::atoi(argv[2]);
    if (pid <= 0) {
      std::cerr << "Invalid process id" << std::endl;
    }
    std::unique_ptr<skydebug::process>proc = skydebug::process::attach(pid);
    return proc;
  } else {
    const char* program_path = argv[1];
    std::unique_ptr<skydebug::process>proc = skydebug::process::launch(program_path);
    return proc;
  }
}

void print_stop_reason(const std::unique_ptr<skydebug::process> &process, skydebug::stop_reason reason){
  std::cout << "Process " << process->pid() << std::endl;
  switch(reason.reason){
    case skydebug::process_state::exited:
      std::cout << "exited with status: " << static_cast<int>(reason.info);
      break;
    case skydebug::process_state::terminated:
      std::cout << "terminated with signal: " << sigabbrev_np(reason.info);
      break;
    case skydebug::process_state::stopped:
      std::cout << "stopped with signal: " << sigabbrev_np(reason.info);
      break;

    default:
      break;
  }
}

std::vector<std::string> split(std::string_view str, char delimiter) {
  std::vector<std::string> out{};
  std::stringstream ss{std::string{str}};
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }
  return out;
}

bool is_prefix(std::string_view str, std::string_view of) {
  if (str.size() > of.size()) return false;
  return std::equal(str.begin(), str.end(), of.begin());
}

void handle_command(std::unique_ptr<skydebug::process> &process, std::string_view line) {
  auto args = split(line, ' ');
  auto command = args[0];

  if (is_prefix(command, "continue")) {
    process->resume();
    auto reason = process->wait_on_signal();
    print_stop_reason(process, reason);
  } else {
    std::cerr << "Unkown Command \n";
  }
}

void main_loop(std::unique_ptr<skydebug::process> &process){
  char* line = nullptr;
  while ((line = readline("skydebug> ")) != nullptr) {
    std::string line_str;

    if (line == std::string_view("")) {
      free(line);
      if (history_length > 0) {
        line_str = history_list()[history_length - 1]->line;
      }
    } else {
      line_str = line;
      add_history(line);
      free(line);
    }
    if (!line_str.empty()) {
      try{
        handle_command(process, line_str);
      }catch(const skydebug::error &err){
        std::cout << err.what() << '\n';
      }
    }
  }

}

}  // namespace

int main(int argc, const char** argv) {
  if (argc == 1) {
    std::cerr << "No arguments given" << std::endl;
    return -1;
  }

  try{
    std::unique_ptr<skydebug::process> process = attach(argc, argv);
    main_loop(process);
  }catch(const skydebug::error &err){
    std::cout << err.what() << '\n';
  }

  return 0;
}
