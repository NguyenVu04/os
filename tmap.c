#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#define MAXSIZE 1024

char *memmap;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *sender (void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        char message[MAXSIZE];
        fprintf(stdout, "Enter a message: ");
        fgets(message, MAXSIZE, stdin);
        int len = strlen(message);
        for (int i = 0; i < len; i++) {
            *(memmap + i) = message[i];
        }
        *(memmap + len - 1) = '\0';
        pthread_mutex_unlock(&lock);
        fprintf(stdout, "Message sent\n");
        sleep(10);
    }
    pthread_exit(NULL);
}

void *receiver (void *argc) {
    while (1) {
        if (pthread_mutex_trylock(&lock) == 0) {
            char message[MAXSIZE];
            int i = 0;
            while (memmap[i] != '\0') {
                message[i] = *(memmap + i);
                *(memmap + i) = '\0';
                i++;
            }
            message[i] = '\0';
            fprintf(stdout, "Message received: %s\n", message);
            pthread_mutex_unlock(&lock);
            sleep(10);
        } else {
            fprintf(stdout, "Doing something\n");
            sleep(10);
        }
    }
    pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
    pthread_t sender_thread, receiver_thread;
    size_t page_size = getpagesize();
    char buffer[page_size];
    memset(buffer, '\0', page_size);
    int fd = open("memmap", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }
    if (write(fd, buffer, page_size) == -1) {
        perror("write");
        exit(-1);
    }
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        exit(-1);
    }
    memmap = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memmap == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }
    pthread_create(&sender_thread, NULL, sender, NULL);
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    if (pthread_join(sender_thread, NULL) == -1) {
        perror("pthread_join");
        exit(-1);
    }
    if (pthread_join(receiver_thread, NULL) == -1) {
        perror("pthread_join");
        exit(-1);
    }    
    munmap(memmap, page_size);
    close(fd);
    pthread_mutex_destroy(&lock);
    return 0;
}
