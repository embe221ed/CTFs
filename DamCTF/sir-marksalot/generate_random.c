#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("[!] Please pass x_coord and y_coord\n");
        return 0;
    }
    int x_coord = atoi(argv[1]);
    int y_coord = atoi(argv[2]);
    int val = 200;
    if (strcpy(argv[3], "GDB")) {
        val = 1500;
    }
    if (strcpy(argv[3], "REMOTE")) {
        val = 2000;
    }
    int ctime = time(NULL) - val;
    
    // printf("%d %d\n", x_coord, y_coord);
    // srand(time);
    int results[4] = {0};

    for (int i=0; i<val; ++i) {
        ctime++;
        srand(ctime);
        for (int j=0; j<4; ++j) {
            results[j] = rand() % 0x28;
        }
        if (results[0] == y_coord && results[1] == x_coord) {
            printf("%d, %d\n", results[2], results[3]);
        }
    }

    return 0;
}
