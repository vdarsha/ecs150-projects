#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>

using namespace std;



int main(int argc, char* argv[]){
    vector<string> paths;
    paths.push_back("/bin");
    if(argc == 1){
        //no arguments, boot into interactive mode
        while(true){
            cout << "wish> ";
            string input;
            getline(cin, input);
            vector<string> parsed;
            int start, end;
            start = end = 0;
            char dl = ' ';
            while ((start = input.find_first_not_of(dl, end))!= string::npos) {
                end = input.find(dl, start);
                parsed.push_back(input.substr(start, end - start));
            }

            if(parsed[0] == "exit"){
                if(parsed.size() > 1){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                exit(0);
            }else if(parsed[0] == "cd"){
                if(parsed.size() > 2){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                if(parsed.size() == 1){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }else{
                    int succ = chdir(parsed[1].c_str());
                    if (succ == -1){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
            }else if(parsed[0] == "path"){
                //delete old paths
                paths.clear();
                //parse path
                for(int i = 1; i < parsed.size(); i++){
                    paths.push_back(parsed[i]);
                }
            }else{
                bool found = false;
                string command = "";
                for(int p = 0; p < paths.size(); p++){
                    command = paths[p] + "/" + parsed[0];
                    int succ = access(command.c_str(), X_OK);
                    if(succ == 0){
                        found = true;
                        break;
                    }
                }
                if(found){
                    char *args[parsed.size() + 1];
                    for(int i = 0; i < parsed.size(); i++){
                        args[i] = (char*)parsed[i].c_str();
                    }
                    args[parsed.size()] = NULL;
                    int pid = fork();
                    if(pid == 0){
                        int ex = execv(command.c_str(), args);
                        if(ex == -1){
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                        }
                    }else{
                        wait(&pid);
                    }
                }else{
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }


        }
    }else if(argc == 2){
        //batch mode
        istream* in = new ifstream(argv[1]);
        if (in->fail()) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        if(!getline(*in, line)){
            break;
        }
        std::istringstream ss(line);
        std::vector<pid_t> children;
        string command = "";
        while(getline(ss, command, '&')){
            isstringstream ss2(command);
            vector<string> parsed;
            string arg;
            string outputloc = "";
            bool err = false;
            while (ss2 >> arg) {
                if(arg.find(">") != string::npos ){
                    size_t pos = arg.find(">");
                    if(pos != 0){
                        args.push_back(arg.substr(0, pos));
                    }
                    outputloc = arg.substr(pos + 1);
                    if(outputloc.empty() && !(ss2 >> outputFile)){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        err = true;
                        break;
                    }
                    if(ss2 >> arg){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        err = true;
                        break;
                    }
                    break;
                }
                parsed.push_back(arg);
            }
            if (parsed.empty() && !outputloc.empty()){
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                err = true;
            }
            if (err){
                continue;
            }
            if(!parsed.empty()){
                if(parsed[0] == "exit"){
                    if(parsed.size() > 1){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    exit(0);
                }else if(parsed[0] == "cd"){
                    if(parsed.size() > 2){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    else if(parsed.size() == 1){
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }else{
                        int succ = chdir(parsed[1].c_str());
                        if (succ == -1){
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                        }
                    }
                }else if(parsed[0] == "path"){
                    //delete old paths
                    paths.clear();
                    //parse path
                    for(int i = 1; i < parsed.size(); i++){
                        paths.push_back(parsed[i]);
                    }
                }else{
                    pid_t pid = fork();
                    if(pid == 0){
                        bool found = false;
                        string command = "";
                        for(int p = 0; p < paths.size(); p++){
                            command = paths[p] + "/" + parsed[0];
                            int succ = access(command.c_str(), X_OK);
                            if(succ == 0){
                                found = true;
                                break;
                            }
                        }
                        char *args[parsed.size() + 1];
                        for(int i = 0; i < parsed.size(); i++){
                            args[i] = (char*)parsed[i].c_str();
                        }
                        argv[parsed.size()] = NULL;
                        if(found){
                            if(!outputloc.empty()){
                                int fd = open(outputloc.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                if(fd == -1){
                                    char error_message[30] = "An error has occurred\n";
                                    write(STDERR_FILENO, error_message, strlen(error_message));
                                    exit(1);
                                }
                                dup2(fd, STDOUT_FILENO);
                                dup2(fd, STDERR_FILENO);
                                close(fd);
                            }
                            
                            int ex = execv(command.c_str(), args);
                            if(ex == -1){
                                char error_message[30] = "An error has occurred\n";
                                write(STDERR_FILENO, error_message, strlen(error_message));
                            }

                        }
                    }else if(pid > 0){
                        children.push_back(pid);

                    }else{
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
            }

            
        }
        for (std::vector<pid_t>::iterator it = children.begin(); it != children.end(); ++it) {
            int status;
            waitpid(*it, &status, 0);
        }
        
        
    }else{
        //too many arguments
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    return 0;
}