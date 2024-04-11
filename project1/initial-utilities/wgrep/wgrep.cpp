#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

using namespace std;
int main(int argc, char *argv[]) {
    // Check if there are any arguments
    if (argc == 1) {
        cout << "wgrep: searchterm [file ...]" << endl;
        return 1;
    }
    string searchterm = argv[1];
    // Loop through all the files if there are any
    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            int verify = open(argv[i], O_RDONLY);
            // check if open was successful
            if(verify < 0){
                cout << "wgrep: cannot open file" << endl;
                exit(1);
            }
            // read & print the file
            char buffer[1];
            int bytesread = read(verify, buffer, sizeof(buffer));
            string bufferst = "";
            while (bytesread > 0) {
            bufferst += buffer[0];
            if(buffer[0] == '\n'){
                if(bufferst.find(searchterm) != string::npos){
                    write(STDOUT_FILENO, bufferst.c_str(), bufferst.size());
                }
                bufferst = "";
            }
            bytesread = read(verify, buffer, sizeof(buffer));
        }
            // close the file
            verify = close(verify);
        }
    }
    else{
        // read from stdin
        char buffer[1];
        int bytesread = read(STDIN_FILENO, buffer, sizeof(buffer));
        string bufferst = "";
        while (bytesread > 0) {
            bufferst += buffer[0];
            if(buffer[0] == '\n'){
                if(bufferst.find(searchterm) != string::npos){
                    write(STDOUT_FILENO, bufferst.c_str(), bufferst.size());
                }
                bufferst = "";
            }
            bytesread = read(STDIN_FILENO, buffer, sizeof(buffer));
        }
    }   
    return 0;
}