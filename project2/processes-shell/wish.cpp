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
    if(argc == 1){
        //no arguments, boot into interactive mode
        while(true){

        }
    }else{
        //batch mode
        string batch = argv[1];
        int commands = open(batch.c_str(), O_RDONLY);
        if(commands == -1){
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        char buffer[1024];
    }
}