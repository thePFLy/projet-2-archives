#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

void test_check_archive(int fd) {
    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);
    lseek(fd, 0, SEEK_SET);
}

void test_exists(int fd, const char *path) {
    int ret = exists(fd, (char *)path);
    printf("exists('%s') returned %d\n", path, ret);
    lseek(fd, 0, SEEK_SET);
}

void test_is_dir(int fd, const char *path) {
    int ret = is_dir(fd, (char *)path);
    printf("is_dir('%s') returned %d\n", path, ret);
    lseek(fd, 0, SEEK_SET);
}

void test_is_file(int fd, const char *path) {
    int ret = is_file(fd, (char *)path);
    printf("is_file('%s') returned %d\n", path, ret);
    lseek(fd, 0, SEEK_SET);
}

void test_is_symlink(int fd, const char *path) {
    int ret = is_symlink(fd, (char *)path);
    printf("is_symlink('%s') returned %d\n", path, ret);
    lseek(fd, 0, SEEK_SET);
}

void test_list(int fd, const char *path) {
    char *entries[10];
    for (int i = 0; i < 10; i++) {
        entries[i] = malloc(100);
    }
    size_t no_entries = 10;
    int ret = list(fd, (char *)path, entries, &no_entries);

    printf("list('%s') returned %d\n", path, ret);
    printf("Number of entries: %zu\n", no_entries);
    for (size_t i = 0; i < no_entries; i++) {
        printf("Entry %zu: %s\n", i, entries[i]);
        free(entries[i]);
    }
    lseek(fd, 0, SEEK_SET);
}

void test_read_file(int fd, const char *path, size_t offset) {
    uint8_t buffer[512];
    size_t len = sizeof(buffer);
    ssize_t ret = read_file(fd, (char *)path, offset, buffer, &len);

    printf("read_file('%s', %zu) returned %zd\n", path, offset, ret);
    lseek(fd, 0, SEEK_SET);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    // Test check_archive
    test_check_archive(fd);

    // Test exists
    test_exists(fd, "lib_tar.h");
    test_exists(fd, "file.txt");

    // Test is_dir
    test_is_dir(fd, "test_dir");
    test_is_dir(fd, "file.txt");

    // Test is_file
    test_is_file(fd, "lib_tar.c");
    test_is_file(fd, "test_dir");

    // Test is_symlink
    test_is_symlink(fd, "lien_symb.c");
    test_is_symlink(fd, "file.txt");

    // Test list
    test_list(fd, "test_dir");

    // Test read_file
    test_read_file(fd, "lib_tar.h", 0);
    test_read_file(fd, "lib_tar.h", 10);

    close(fd);
    return 0;
}