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
    if (__argc == 1) {
        cout << "wgrep: searchterm [file ...]" << endl;
        return 1;
    }
    // Loop through all the arguments
    for (int i = 2; i < __argc; i++) {
        int verify = open(argv[i], O_RDONLY);
        // check if open was successful
        if(verify < 0){
            cerr << "wgrep: cannot open file" << endl;
            return 1;
        }
        // read & print the file
        char buffer[1024];
        int bytesread = read(verify, buffer, sizeof(buffer));
        while (bytesread > 0) {
            write(STDOUT_FILENO, buffer, bytesread);
            bytesread = read(verify, buffer, sizeof(buffer));
        }   
        // close the file
        verify = close(verify);
    }
    return 0;
}