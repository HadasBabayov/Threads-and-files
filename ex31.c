// Hadas Babayov 322807629

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * This function remove spaces, '\n'and tabs, and make character to lower case.
 * That's help us to check if the files is similar.
 *
 * @param str - buffer
 * @return the buffer after the change.
 */
char *removeSpacesAndMakeLower(char *str) {
    int i = 0, j = 0;
    // Copy the char if it isn't a space and make it lower.
    while (str[i]) {
        if (str[i] != ' ' && !isspace(str[i])) {
            str[j++] = (char) tolower(str[i]);
        }
        i++;
    }
    str[j] = '\0';
    return str;
}

/**
 * This function compare between two files.
 * similar files --> same chars in the same order but not the same spaces and upper\ lower case.
 *
 * @param firstFilePath
 * @param secondFilePath
 * @return 1 - if they are equal, 3 - if they are similar, 2 - they are different.
 */
int cmpFiles(char *firstFilePath, char *secondFilePath) {
    // Open the files.
    int first_fd = open(firstFilePath, O_RDONLY);
    int second_fd = open(secondFilePath, O_RDONLY);

    // Check if the open failed.
    if (first_fd == -1 || second_fd == -1) {
        if (write(1, "Error in: open\n", 15) != 15) {
            exit(-1);
        }
        close(first_fd);
        close(second_fd);
        exit(-1);
    }

    // Check the size of the files.
    struct stat stat_firstFile;
    struct stat stat_secondFile;

    if (stat(firstFilePath, &stat_firstFile) == -1 || stat(secondFilePath, &stat_secondFile) == -1) {
        if (write(1, "Error in: stat\n", 15) != 15) {
            exit(-1);
        }
        close(first_fd);
        close(second_fd);
        exit(-1);
    }

    // Two buffers in order to read the files.
    char *firstFileBuffer = (char *) malloc(stat_firstFile.st_size + 1);
    char *secondFileBuffer = (char *) malloc(stat_secondFile.st_size + 1);
    firstFileBuffer[stat_firstFile.st_size] = '\0';
    secondFileBuffer[stat_secondFile.st_size] = '\0';
    if (firstFileBuffer == NULL || secondFileBuffer == NULL) {
        close(first_fd);
        close(second_fd);
        exit(-1);
    }

    // Read the files and check if read failed.
    int checkFirstRead, checkSecondRead;
    checkFirstRead = read(first_fd, firstFileBuffer, stat_firstFile.st_size);
    checkSecondRead = read(second_fd, secondFileBuffer, stat_secondFile.st_size);
    if (checkFirstRead == -1 || checkSecondRead == -1) {
        if (write(1, "Error in: read\n", 15) != 15) {
            exit(-1);
        }
        close(first_fd);
        close(second_fd);
        exit(-1);
    }
    // If the files is equal - return 1.
    if (!strcmp(firstFileBuffer, secondFileBuffer)) {
        free(firstFileBuffer);
        free(secondFileBuffer);
        close(first_fd);
        close(second_fd);
        return 1;
    }
    // Change the buffer in order to check if the files is similar.
    char *firstToCmp = removeSpacesAndMakeLower(firstFileBuffer);
    char *secondToCmp = removeSpacesAndMakeLower(secondFileBuffer);
    // If the files is similar - return 3.
    if (!strcmp(firstToCmp, secondToCmp)) {
        free(firstFileBuffer);
        free(secondFileBuffer);
        return 3;
    }

    // else - the files are different - return 2.
    free(firstFileBuffer);
    free(secondFileBuffer);
    return 2;
}

/**
 * This program get two files path and compare between this files.
 *
 * @param argc
 * @param argv
 * @return 1 - if they are equal, 3 - if they are similar, 2 - they are different.
 */
int main(int argc, char *argv[]) {

    if (argc != 3) {
        exit(-1);
    }

    char *firstFilePath = argv[1];
    char *secondFilePath = argv[2];

    return cmpFiles(firstFilePath, secondFilePath);
}
