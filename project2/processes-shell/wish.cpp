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

void errorrout(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(stderr);
}

int main(int argc, char* argv[]){
    if(argc > 2){
        //too many arguments
        errorrout();
        exit(1);
    }
    vector<string> paths;
    paths.push_back("/bin");
    istream* in = argc == 2 ? new std::ifstream(argv[1]) : &std::cin;
    if(in->fail()){
        errorrout();
        exit(1);
    }
    string line;
    while(true){
        
        if(argc == 1){
            cout << "wish> ";
            getline(cin, line);

        }else if (!std::getline(*in, line)){
            break;
        }
        
        std::istringstream ss(line);
        std::vector<pid_t> children;
        string command = "";
        while(getline(ss, command, '&')){
            bool err = false;
            istringstream ss2(command);
            vector<string> parsed;
            string arg;
            string outputloc = "";
            
            while (ss2 >> arg) {
                if(arg.find(">") != string::npos ){
                    size_t pos = arg.find(">");
                    if(pos != 0){
                        parsed.push_back(arg.substr(0, pos));
                    }
                    outputloc = arg.substr(pos + 1);
                    if(outputloc.empty() && !(ss2 >> outputloc)){
                        errorrout();
                        err = true;
                        break;
                    }
                    if(ss2 >> arg){
                        errorrout();
                        err = true;
                        break;
                    }
                    break;
                }
                parsed.push_back(arg);
            }
            if (parsed.empty() && !outputloc.empty()){
                errorrout();
                err = true;
            }
            if (err){
                continue;
            }
            if(!parsed.empty()){
                if(parsed[0] == "exit"){
                    if(parsed.size() > 1){
                        errorrout();
                    }
                    exit(0);
                }else if(parsed[0] == "cd"){
                    if(parsed.size() > 2){
                        errorrout();
                    }
                    else if(parsed.size() == 1){
                        errorrout();
                    }else{
                        int succ = chdir(parsed[1].c_str());
                        if (succ == -1){
                            errorrout();
                        }
                    }
                }else if(parsed[0] == "path"){
                    //delete old paths
                    paths.clear();
                    //parse path
                    for(long unsigned int i = 1; i < parsed.size(); i++){
                        paths.push_back(parsed[i]);
                    }
                }else{
                    pid_t pid = fork();
                    if(pid == 0){
                        bool found = false;
                        string command = "";
                        for(long unsigned int j = 0; j < paths.size(); j++){
                            command = paths[j] + "/" + parsed[0];
                            int succ = access(command.c_str(), X_OK);
                            if(succ == 0){
                                found = true;
                                break;
                            }
                        }
                        char *args[parsed.size() + 1];
                        for(long unsigned int i = 0; i < parsed.size(); i++){
                            args[i] = (char*)parsed[i].c_str();
                        }
                        args[parsed.size()] = NULL;
                        if(found){
                            if(!outputloc.empty()){
                                int fd = open(outputloc.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                                if(fd == -1){
                                    errorrout();
                                    exit(1);
                                }
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                            }
                            execv(command.c_str(), args);
                            _exit(errno);
                        }
                        else {
                            // Redirect standard error to the parent's standard error
                            dup2(STDERR_FILENO, STDERR_FILENO);
                            if(execv(command.c_str(), args) == -1){
                                errorrout();
                                _exit(errno);
                            }
                        }
                    }else if(pid > 0){
                        children.push_back(pid);
                    }else{
                        errorrout();
                        _exit(0);
                    }
                }
            }
        }    
    
        for (std::vector<pid_t>::iterator it = children.begin(); it != children.end(); ++it) {
            int status;
            waitpid(*it, &status, 0);
        }
    }
    return 0;
}