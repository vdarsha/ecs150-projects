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

int main(int argc, char *argv[]){
    // Check if there are any arguments
    if (argc == 1) {
        cout << "wunzip: file1 [file2 ...]" << endl;
        return 1;
    }
    // Loop through all the files if there are any
    if (argc > 1) {
        unordered_map<int,int> verify;
        for (int i = 1; i < argc; i++) {
            verify[i] = open(argv[i], O_RDONLY);
            
            // check if open was successful
            if(verify[i] < 0){
                cout << "wunzip: cannot open file" << endl;
                exit(1);
            }
        }
        // read & print the file
        int j = 1;
        int count = 0;
        int bytesread = 0;
        char letter[1];
        
        while(j <= argc - 1 || (bytesread = read(verify[j], &count, 4)) != 0){
            if(bytesread == 0 && j == argc - 1){
                break;
            }
            read(verify[j], letter, 1);
            
            for (int i = 0; i < count; i++){
                write(STDOUT_FILENO, letter, 1);
            }
            
            
            if(bytesread == 0 && j < argc - 1){
                ++j;
                if(j == argc - 1){
                    break;
                }
            }
            bytesread = read(verify[j], &count, 4);
        }
        for(int i = 1; i < argc; i++){    
            // close the files
            close(verify[i]);
        }
    }

    
    return 0;
}