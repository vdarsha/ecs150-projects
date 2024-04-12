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
        cout << "wzip: file1 [file2 ...]" << endl;
        return 1;
    }
    // Loop through all the files if there are any
    if (argc == 2) {
        for (int i = 1; i < argc; i++) {
            int verify = open(argv[i], O_RDONLY);
            // check if open was successful
            if(verify < 0){
                cout << "wzip: cannot open file" << endl;
                exit(1);
            }
            // read & print the file
            char buffer[1];
            int bytesread = read(verify, buffer, sizeof(buffer));
            string bufferst = "";
            char prev = buffer[0];
            int count = 1;
            while (bytesread > 0) {
                bytesread = read(verify, buffer, sizeof(buffer));
                if (bytesread == 0) {
                    break;
                }
                if (buffer[0] == prev) {
                    count++;
                } else {
                    write(STDOUT_FILENO, &count, sizeof(count));
                    write(STDOUT_FILENO, &prev, sizeof(prev));
                    prev = buffer[0];
                    count = 1;
                }
            }
            // close the file
            verify = close(verify);
        }
    }
    return 0;
}