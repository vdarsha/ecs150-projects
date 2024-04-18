#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <unordered_map>

using namespace std;

int main(int argc, char *argv[]) {
    // Check if there are any arguments
    if (argc == 1) {
        cout << "wzip: file1 [file2 ...]" << endl;
        return 1;
    }
    // Loop through all the files if there are any
    if (argc > 1) {
        unordered_map<int,int> verify;
        for (int i = 1; i < argc; i++) {
            verify[i] = open(argv[i], O_RDONLY);
            
            // check if open was successful
            if(verify[i] < 0){
                cout << "wzip: cannot open file" << endl;
                exit(1);
            }
        }
            // read & print the file
            int j = 1;
            char buffer[1];
            int bytesread = read(verify[1], buffer, sizeof(buffer));
            char prev = buffer[0];
            int count = 0;
            while(j < argc){
                if (bytesread == 0 && j == argc - 1) {
                    write(STDOUT_FILENO, &count, sizeof(count));
                    write(STDOUT_FILENO, &prev, sizeof(prev));
                    break;
                }
                else if (bytesread == 0 && j < argc - 1) {
                    j++;
                    bytesread = read(verify[j], buffer, sizeof(buffer));
                    continue;
                }
                else if (buffer[0] == prev) {
                    count++;
                    bytesread = read(verify[j], buffer, sizeof(buffer));
                    continue;
                    
                } 
                else {
                    write(STDOUT_FILENO, &count, sizeof(count));
                    write(STDOUT_FILENO, &prev, sizeof(prev));
                    prev = buffer[0];
                    count = 0;
                    continue;
                }
                
            }
        for(int i = 1; i < argc; i++){    
            // close the files
            close(verify[i]);
        }
    }
    return 0;
}