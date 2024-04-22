#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>


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
            vector <int> paralleltokens;
            for (int i = 0; i < parsed.size(); i++) {
                if(parsed[i] == "&"){
                    paralleltokens.push_back(i);
                }
            }
            vector <string> parallelcommands;
            int ppid = getpid();
            for(int i = 0; i <= paralleltokens.size(); i++){
                
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
                
            }


        }
    }else if(argc == 2){
        //batch mode
        string batch = argv[1];
        int commands = open(batch.c_str(), O_RDONLY);
        if(commands == -1){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        char buffer[1];
        int bytesread = read(commands, buffer, 1);
        string bufferst = "";
        vector <string> parsed;
        while(bytesread > 0){
            while(bytesread != 0 && buffer[0] != '\n'){
                if(buffer[0] != ' '){
                    bufferst += buffer[0];
                }
                bytesread = read(commands, buffer, 1);
                if(buffer[0] == ' '){
                    parsed.push_back(bufferst);
                    bufferst = "";
                }
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
                
            }
            

        }
        
        
    }else{
        //too many arguments
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}