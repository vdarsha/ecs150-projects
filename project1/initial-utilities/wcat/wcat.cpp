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
        //cout << "wcat: file1 [file2 ...]" << endl;
        return 0;
    }

    // Loop through all the arguments
    for (int i = 1; i < argc; i++) {
        int verify = open(argv[i], O_RDONLY);
        // check if open was successful
        if(verify < 0){
            cout << "wcat: cannot open file" << endl;
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