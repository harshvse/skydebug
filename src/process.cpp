#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <libskydebug/error.hpp>
#include <libskydebug/pipe.hpp>
#include <libskydebug/process.hpp>

std::unique_ptr<skydebug::process> skydebug::process::launch(
    std::filesystem::path path) {
  pipe channel(true);
  pid_t pid = 0;
  if ((pid = fork()) < 0) {
    // Error: fork failed
    error::send_errno("fork failed");
  }

  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
      // Error: Tracing failed
      error::send_errno("tracing failed");
    }
    if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
      // Error: Failed to execute
      error::send_errno("failed to execute application - " + path.string());
    }
  }

  std::unique_ptr<process> proc(new process(pid, true));
  proc->wait_on_signal();

  return proc;
}

std::unique_ptr<skydebug::process> skydebug::process::attach(pid_t pid) {
  if (pid == 0) {
    // Error: Invalid PID
    error::send("invalide PID");
  }
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    // Error: Could not attach
    error::send_errno("could not attach to pid");
  }
  std::unique_ptr<skydebug::process> proc(new process(pid, false));
  return proc;
}

void skydebug::process::resume() {
  if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
    error::send_errno("could not resume the process");
  }
  state_ = process_state::running;
}

skydebug::stop_reason::stop_reason(int wait_status) {
  if (WIFEXITED(wait_status)) {
    reason = process_state::exited;
    info = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    reason = process_state::terminated;
    info = WTERMSIG(wait_status);
  } else if (WIFSTOPPED(wait_status)) {
    reason = process_state::stopped;
    info = WSTOPSIG(wait_status);
  }
}

skydebug::stop_reason skydebug::process::wait_on_signal() {
  int wait_status;
  int options = 0;
  if (waitpid(pid_, &wait_status, options) < 0) {
    error::send_errno("process waitpid failed");
  }
  stop_reason reason(wait_status);
  state_ = reason.reason;
  return reason;
}

skydebug::process::~process() {
  if (pid_ != 0) {
    int status;
    if (state_ == process_state::running) {
      kill(pid_, SIGSTOP);
      waitpid(pid_, &status, 0);
    }
    ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
    kill(pid_, SIGCONT);

    if (terminate_on_end_) {
      kill(pid_, SIGKILL);
      waitpid(pid_, &status, 0);
    }
  }
}
