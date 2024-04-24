#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("USAGE %s [-f filename] read|write text_string", argv[0]);
        return 1;
    }

    char filename[100];
    char op[100], oparg[100];

    if (strcmp(argv[1], "-f") == 0) {
        sprintf(filename, "%s", argv[2]);
        printf("file name has been set to %s\n", argv[2]);
        strcpy(op, argv[3]);
        if (strcmp(op, "write") == 0) {
            strcpy(oparg, argv[4]);
        }
    } else {
        strcpy(op, argv[1]);
        strcpy(oparg, argv[2]);
    }

    int fd = open(filename, O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    char str[100];
    memset(str, '\0', 100);
    write(fd, str, 100);
    
    struct stat sb;

    fstat(fd, &sb);

    char *mm = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (strcmp(op, "read") == 0) {
        printf("%ld", strlen(mm));
        printf("%s", mm);        
    } else if (strcmp(op, "write") == 0) {
        sprintf(mm, "%s", oparg);
    } else {
        printf("Operation not found\n");
    }

    munmap(mm, sb.st_size);
    close(fd);
    return 0;
}