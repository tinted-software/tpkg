extern "C" {
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
}

import std;

class Task {
public:
  Task(const std::string &name, const std::vector<std::string> &command)
      : name(name), command(command), duration(0), completed(false), pid(-1) {}
  std::string name;
  std::vector<std::string> command;
  int duration;
  bool completed;
  pid_t pid;
  int fd;
  std::queue<std::string> logs;

  void add_log(const std::string &log) { logs.push(log); }
};

class ProgressBar {
public:
  ProgressBar(std::vector<Task> &tasks) : tasks(tasks), total_time(0) {}

  void update() {
    std::cout << "\033[2J\033[H"; // Clear screen and move cursor to top-left
    printLogs();
    printProgress();
  }

private:
  std::vector<Task> &tasks;
  int total_time;

  void printLogs() {
    for (const auto &task : tasks) {
      std::queue<std::string> temp_logs = task.logs;
      while (!temp_logs.empty()) {
        std::cout << task.name << "> " << temp_logs.front() << "\n";
        temp_logs.pop();
      }

      std::cout << "\n";
    }
  }

  void printProgress() {
    std::cout << "┏━ Dependency Graph:\n";
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
      const auto &task = *it;
      std::cout << "┃ \e[0;33m⏵ " << task.name << "\e[0m ⏱ " << task.duration
                << "s\n";
    }
    std::cout << "┣━━━ Builds\n";

    int completed = 0;
    int paused = 0;
    for (auto it = tasks.begin(); it != tasks.end();) {
      const auto &task = *it;
      if (task.completed) {
        completed++;
        it = tasks.erase(it);
      } else {
        ++it;
      }
    }

    total_time += 1;
    std::cout << "┗━ ∑ \e[0;33m⏵ " << tasks.size() << " \e[0;32m│ ✔ "
              << completed << " \e[0;34m│ ⏸ " << paused << "\e[0m │ ⏱ "
              << total_time << "s\n";
  }
};

void run_command(Task &task) {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    throw std::runtime_error("pipe");
  }

  auto temporary_directory =
      std::filesystem::temp_directory_path() / ("tpkg-" + task.name);
  std::filesystem::create_directory(temporary_directory);

  pid_t pid = fork();
  if (pid == -1) {
    throw std::runtime_error("fork");
  } else if (pid == 0) {
    // Child process
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    std::filesystem::current_path(temporary_directory);

    std::vector<char *> args;
    for (const auto &arg : task.command) {
      args.push_back(const_cast<char *>(arg.c_str()));
    }
    args.push_back(nullptr);

    execvp(args[0], args.data());
    throw std::runtime_error("Command failed to run");
  } else {
    // Parent process
    close(pipefd[1]);
    task.pid = pid;
    task.fd = pipefd[0];

    // Set the pipe to non-blocking mode
    int flags = fcntl(task.fd, F_GETFL, 0);
    fcntl(task.fd, F_SETFL, flags | O_NONBLOCK);
  }
}

bool update_task_status(Task &task) {
  int status;
  pid_t result = waitpid(task.pid, &status, WNOHANG);
  if (result == 0) {
    return false; // Task is still running
  } else if (result == -1) {
    return true; // Assume task is completed due to error
  } else {
    task.completed = true;
    return true;
  }
}

void readTaskOutput(Task &task) {
  char buffer[1024];
  ssize_t bytesRead;

  while ((bytesRead = read(task.fd, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[bytesRead] = '\0';
    task.add_log(std::string(buffer));
  }
}

int main() {
  std::vector<Task> tasks = {
      Task("cmake-bootstrap-minimal", {"nu", "-c", "sleep 5sec"}),
  };

  ProgressBar progress_bar(tasks);

  for (auto &task : tasks) {
    run_command(task);
  }

  while (true) {
    bool allCompleted = true;
    for (auto &task : tasks) {
      if (!task.completed) {
        task.duration++;
        readTaskOutput(task);
        if (update_task_status(task)) {
          readTaskOutput(task); // Read any remaining output
        } else {
          allCompleted = false;
        }
      }
    }

    progress_bar.update();

    if (allCompleted) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}
