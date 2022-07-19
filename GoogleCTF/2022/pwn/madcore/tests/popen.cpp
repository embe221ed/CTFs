#include <iostream>
#include <cstdio>

int main(int argc, char *argv[]) {
    FILE *fp;
    fp = popen(argv[1], "r");
    char buffer[100];
    fread(buffer, 1, 100, fp);
    printf(buffer);
}
