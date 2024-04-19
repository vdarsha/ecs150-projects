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
    

    if(argc == 1){
        //no arguments, boot into interactive mode
        while(true){
            cout << "wish> ";
            string input;
            getline(cin, input);
            stringstream ss(input);
            vector<string> parsed;
            while(getline(ss, input, ' ')){
                parsed.push_back(input);
            }
            if(parsed[0] == "exit"){
                if(parsed.size() > 1){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                exit(0);
            }
            if(parsed[0] == "cd"){
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
            }
            if(parsed[0] == "path"){
                //parse path
                for(int i = 1; i < parsed.size(); i++){
                    paths.push_back(parsed[i]);
                }
                //print path
                for(int i = 0; i < paths.size(); i++){
                    cout << paths[i] << endl;
                }
                
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
        char buffer[1024];
        
        
    }else{
        //too many arguments
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}