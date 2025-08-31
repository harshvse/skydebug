#include <cstdio>
#include <iostream>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <wait.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <sstream>
#include <vector>

namespace{
    pid_t attach(int argc, const char** argv){
        pid_t pid = 0;

        if(argc == 3 && argv[1] == std::string_view("-p")){
            // passed pid
            pid = std::atoi(argv[2]);
            if(pid <= 0){
                std::cerr << "Invalid process id" << std::endl;
                return -1;
            }

            if(ptrace(PTRACE_ATTACH, pid, /*addr=*/nullptr, /*data=*/nullptr) < 0){
                std::perror("Could not attach to pid");
                return -1;
            }

        }else{
            // passed program path
            const char* program_path = argv[1];
            if((pid = fork()) < 0){
                std::perror("fork failed");
                return -1;
            }

            if(pid == 0){
                if(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0){
                    std::perror("Tracing failed");
                    return -1;
                }
                if(execlp(program_path, program_path, nullptr) < 0){
                    std::perror("Exec Failed");
                    return -1;
                }
            }
        }
        return pid;
    }

    std::vector<std::string> split(std::string_view str, char delimiter){
        std::vector<std::string> out{};
        std::stringstream ss {std::string{str}};
        std::string item;

        while(std::getline(ss, item, delimiter)){
            out.push_back(item);
        }
        return out;

    }
    bool is_prefix(std::string_view str, std::string_view of){
        if(str.size()> of.size())return false;
        return std::equal(str.begin(),str.end(), of.begin());
    }
    void resume(pid_t pid){
        if(ptrace(PTRACE_CONT, pid, nullptr, nullptr) < 0){
            std::perror("Could'nt continue \n");
            std::exit(-1);
        }
    }
    void wait_on_signal(pid_t pid){
        int wait_status;
        int options = 0;
        if(waitpid(pid, &wait_status, options) < 0){
            std::perror("waitpid failed");
            std::exit(-1);
        }
    }

    void handle_command(pid_t pid, std::string_view line){
        auto args = split(line, ' ');
        auto command = args[0];

        if(is_prefix(command, "continue")){
            resume(pid);
            wait_on_signal(pid);
        }
        else{
            std::cerr << "Unkown Command \n";
        }

    }
}

int main(int argc, const char** argv){

    if(argc == 1){
        std::cerr << "No arguments given" << std::endl;
        return -1;
    } 

    pid_t pid = attach(argc,argv);

    int wait_status;
    int options = 0;

    if(waitpid(pid, &wait_status, options) < 0){
        std::perror("wait pid failed");
        return -1;
    }

    char * line = nullptr;
    while((line = readline("skydebug> ")) != nullptr){
        std::string line_str;

        if(line == std::string_view("")){
            free(line);
            if(history_length > 0){
                line_str = history_list()[history_length -1]->line;
            }
        }
        else{
            line_str = line;
            add_history(line);
            free(line);
        }
        if(!line_str.empty()){
            handle_command(pid, line_str);
        }
   }

    return 0;
}