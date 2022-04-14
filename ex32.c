// Hadas Babayov 322807629

#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>

/**
 * This function get path and check if this is folder.
 *
 * @param filePath
 * @return 1 - folder, 0 - not folder.
 */
int isFolder(char *filePath, int stdOut) {
    struct stat checkStat;
    int callStat = stat(filePath, &checkStat);
    if (callStat == -1) {
        write(stdOut, "Error in: stat\n", 15);
    }
    if (callStat != 0) {
        return 0;
    }
    return S_ISDIR(checkStat.st_mode);
}

/**
 * This function get configuration file path and read the file to some buffer in order to get the arguments.
 *
 * @param path
 * @param std - if we want to write something to the screen.
 * @return buffer with text of the file.
 */
char *getConf(char *path, int std) {
    // Open the file.
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        write(std, "Error in: open\n", 15);
        close(fd);
        exit(-1);
    }

    // Call stat in order to get file size.
    struct stat stat_conf;
    if (stat(path, &stat_conf) == -1) {
        write(std, "Error in: stat\n", 15);
        close(fd);
        exit(-1);
    }

    // Creat buffer for read.
    char *buffer = (char *) malloc(stat_conf.st_size + 1);
    buffer[stat_conf.st_size] = '\0';
    if (buffer == NULL) {
        close(fd);
        exit(-1);
    }

    // Read the file.
    int checkRead = read(fd, buffer, stat_conf.st_size);
    if (checkRead == -1) {
        write(std, "Error in: read\n", 15);
        close(fd);
        exit(-1);
    }
    close(fd);
    return buffer;
}

/**
 * This function creat c file path.
 *
 * @param partOfPath - first part for this path.
 * @param directoryName
 * @param childdir
 * @return the finally path.
 */
char *pathCFile(char *partOfPath, char *directoryName, char *childdir) {
    char *pathToReturn = (char *) malloc(200);
    strcpy(pathToReturn, partOfPath);
    strcat(pathToReturn, "/");
    strcat(pathToReturn, directoryName);
    strcat(pathToReturn, "/");
    strcat(pathToReturn, childdir);
    return pathToReturn;
}

/**
 * This function run c file and compare the output with the given output.
 *
 * @param fileName
 * @param inputPath
 * @param outputPath
 * @param path
 * @param std - if we want to write something to the screen.
 * @return - 1 if the files is equal, 3 - if they similar, 2 - if they different and 20 if the running take more
 * than 5 seconds.
 */
int compileAndRunFile(char *fileName, char *inputPath, char *outputPath, char *path, int std) {

    // Compile and run the c file.
    char *compileCommand[5] = {"gcc", fileName, "-o", "compile.out", NULL};
    char *runCommand[3] = {"./compile.out", fileName, NULL};

    // Call fork for the compile.
    int compile_pid = fork();
    if (compile_pid < 0) {
        write(std, "Error in: fork\n", 15);
        exit(-1);
    }
    // Call exec for the compile.
    if (compile_pid == 0) {
        int check = execvp(compileCommand[0], compileCommand);
        if (check < 0) {
            write(std, "Error in: execvp\n", 17);
            exit(-1);
        }
    }
    // initialize status to check if the compile failed.
    int compile_status = 0;
    // Call wait for the compile.
    if (compile_pid > 0) {
        if (waitpid(compile_pid, &compile_status, 0) < 0) {
            write(std, "Error in: waitpid\n", 18);
            exit(-1);
        }
    }

    int ex31value = 10, isTwenty = 0;
    // If the compile succeeded, run this C file and save the output to compare.
    if (WEXITSTATUS(compile_status) != 1) {
        char returnPath[200];
        getcwd(returnPath, sizeof(returnPath));
        chdir(path);
        // Open the input file to this c file.
        int fd_inputFile = open(inputPath, O_RDONLY);
        if (fd_inputFile == -1) {
            write(std, "Error in: open\n", 15);
            remove("compile.out");
            exit(-1);
        }
        dup2(fd_inputFile, 0);

        // Open the output file to save this output.
        int fd_outputFile = open("output", O_CREAT | O_RDWR, 0666);
        if (fd_outputFile == -1) {
            write(std, "Error in: open\n", 15);
            exit(-1);
        }
        dup2(fd_outputFile, 1);
        chdir(returnPath);

        // Call fork in order to run the c file.
        int run_pid = fork();
        if (run_pid < 0) {
            write(std, "Error in: fork\n", 15);
            exit(-1);
        }
        // Call exec
        if (run_pid == 0) {
            int check = execvp(runCommand[0], runCommand);
            if (check < 0) {
                write(std, "Error in: execvp\n", 17);
                exit(-1);
            }
        }
        // Call wait
        if (run_pid > 0) {
            // If the running take more than 5 seconds - we save 20 in return value.
            time_t beforeWait = time(NULL);
            if (waitpid(run_pid, NULL, 0) < 0) {
                write(std, "Error in: waitpid\n", 18);
                exit(-1);
            }
            if ((time(NULL) - beforeWait) > 5) {
                isTwenty = 1;
                ex31value = 20;
            }
        }

        // Creat command in order to run ex31.c
        char *runEx31Command[4] = {"./comp.out", outputPath, "output", NULL};

        chdir(path);
        // Call fork
        int run31_pid = fork();
        if (run31_pid < 0) {
            write(std, "Error in: fork\n", 15);
            exit(-1);
        }
        // Call exec
        if (run31_pid == 0) {
            int check = execvp(runEx31Command[0], runEx31Command);
            if (check < 0) {
                write(std, "Error in: execvp\n", 17);
                exit(-1);
            }
        }
        // Call wait
        int status = 0;
        if (run31_pid > 0) {
            if (waitpid(run31_pid, &status, 0) < 0) {
                write(std, "Error in: waitpid\n", 18);
                exit(-1);
            } else {
                // Save the return value of ex31 - that compare between the files.
                if (!isTwenty) {
                    ex31value = WEXITSTATUS(status);
                }
            }
        }
        // Close all the files that open.
        close(fd_outputFile);
        remove("output");
        chdir(returnPath);
        remove("compile.out");
        close(fd_inputFile);
    }

    return ex31value;
}

/**
 * This function creat string to write un the csv.
 *
 * @param nameOfDir
 * @param grade
 * @param explanation
 * @return the string after the connect.
 */
char *connectStringsToCSV(char *nameOfDir, char *grade, char *explanation) {
    char *returnVal = (char *) malloc(200);
    strcpy(returnVal, nameOfDir);
    strcat(returnVal, ",");
    strcat(returnVal, grade);
    strcat(returnVal, ",");
    strcat(returnVal, explanation);
    returnVal[strlen(returnVal)] = '\n';
    return returnVal;
}

/**
 * This function check the validity of paths.
 * @param inputPath
 * @param outputPath
 * @param std - if we want to write something to the screen.
 */
void checkValidityOfPath(char *inputPath, char *outputPath, int std) {

    // Open the file s- if the open failed - the file isn't exist.
    int fd_inputFile = open(inputPath, O_RDONLY);
    if (fd_inputFile == -1) {
        write(std, "Input file not exist\n", 21);
        exit(-1);
    }
    close(fd_inputFile);

    int fd_outputFile = open(outputPath, O_RDONLY);
    if (fd_outputFile == -1) {
        write(std, "Output file not exist\n", 22);
        exit(-1);
    }
    close(fd_outputFile);
}

/**
 * This program get file with 3 arguments.
 * first one - path to directory.
 * second one - path to input file.
 * third one - path to output file.
 *
 * @param argc
 * @param argv
 * @return csv file and error file.
 * looping through the directory files and inside to the folders.
 * if there is c file - we run it and compare the output with the output we get by ex31.c.
 * By the results we full the CSV file. if we have some errors we write it to erros.txt.
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        exit(-1);
    }

    // Save the stdout if we want to write something to the screen and not to some file.
    int saveStdOut = dup(1);
    char *path = argv[1];
    // Get the arguments.
    char *conf = getConf(path, saveStdOut);

    // Save them in buffers.
    char *directoryPath = strtok(conf, "\n");
    char *inputFilePath = strtok(NULL, "\n");
    char *outputFilePath = strtok(NULL, "\n");

    // Save current path.
    char currPath[250];
    getcwd(currPath, sizeof(currPath));

    // Get the full path to input and output files.
    checkValidityOfPath(inputFilePath, outputFilePath, saveStdOut);

    // Check if the first argument is a folder.
    if (!isFolder(directoryPath, saveStdOut)) {
        write(1, "Not a valid directory\n", 22);
        exit(-1);
    }

    DIR *pDir;
    struct dirent *pDirent;
    // Open the folder.
    if ((pDir = opendir(directoryPath)) == NULL) {
        write(saveStdOut, "Error in: opendir\n", 18);
        exit(-1);
    }

    // Open CSV file to write the results.
    int fd_CSV = open("results.csv", O_CREAT | O_RDWR, 0666);
    if (fd_CSV == -1) {
        write(saveStdOut, "Error in: open\n", 15);
        close(fd_CSV);
        exit(-1);
    }

    // Creat the error file.
    int fd_errorFile = open("errors.txt", O_CREAT | O_RDWR, 0666);
    if (fd_errorFile == -1) {
        write(saveStdOut, "Error in: open\n", 15);
        exit(-1);
    }
    // Move the writing to the errors file.
    dup2(fd_errorFile, 2);


    // Move to the directory.
    if (chdir(directoryPath) != 0) {
        write(saveStdOut, "Error in: chdir\n", 16);
        exit(-1);
    }

    char *grade = "0";
    char *explanation;

    // Looping through the directory.
    while ((pDirent = readdir(pDir)) != NULL) {
        if (!strcmp(pDirent->d_name, ".") || !strcmp(pDirent->d_name, "..")) {
            continue;
        }
        // Look for folders.
        if (isFolder(pDirent->d_name, saveStdOut)) {
            DIR *innerDir;
            struct dirent *innerDirent;
            // Open the folder
            if ((innerDir = opendir(pDirent->d_name)) == NULL) {
                write(saveStdOut, "Error in: opendir\n", 18);
                exit(-1);
            }
            // looping through the directory files and look for c file.
            int containCFile = 0;
            while ((innerDirent = readdir(innerDir)) != NULL) {
                // Move to child folder.
                char *pathToCFile = pathCFile(currPath, directoryPath, pDirent->d_name);
                chdir(pathToCFile);

                // If it's folder - continue to the next file (we look for c file).
                if (isFolder(innerDirent->d_name,saveStdOut)) {
                    chdir("..");
                    continue;
                }
                // If there is c file- compile, run it and compare to output file we get.
                int lenOfFileName = strlen(innerDirent->d_name);
                if (innerDirent->d_name[lenOfFileName - 2] == '.' && innerDirent->d_name[lenOfFileName - 1] == 'c') {
                    containCFile = 1;

                    int returnValueOfEx31 = compileAndRunFile(innerDirent->d_name, inputFilePath, outputFilePath,
                                                              currPath, saveStdOut);

                    // According to the return value, we full the csv file.
                    if (returnValueOfEx31 == 10) {
                        grade = "10";
                        explanation = "COMPILATION_ERROR";
                    }
                    if (returnValueOfEx31 == 1) {
                        grade = "100";
                        explanation = "EXCELLENT";
                    }
                    if (returnValueOfEx31 == 2) {
                        grade = "50";
                        explanation = "WRONG";
                    }
                    if (returnValueOfEx31 == 3) {
                        grade = "75";
                        explanation = "SIMILAR";
                    }
                    if (returnValueOfEx31 == 20) {
                        grade = "20";
                        explanation = "TIMEOUT";
                    }
                }
                // Move to the original directory.
                if (chdir("..") != 0) {
                    write(saveStdOut, "Error in: chdir\n", 16);
                }
            }
            // If the readdir is failed.
            if (!DT_BLK) {
                write(saveStdOut, "Error in: readdir\n", 18);
            }
            // If we don't have c file in some child folder.
            if (!containCFile) {
                grade = "0";
                explanation = "NO_C_FILE";
            }
            // Write the finally result of this child folder in csv.
            char *writeInCSV = connectStringsToCSV(pDirent->d_name, grade, explanation);
            write(fd_CSV, writeInCSV, strlen(writeInCSV));
            // Close child directory.
            closedir(innerDir);
        }
    }
    if (!DT_BLK) {
        write(saveStdOut, "Error in: readdir\n", 18);
    }

    // Close CSV file.
    close(fd_CSV);
    // Close errors file.
    close(fd_errorFile);
    // Close the original directory.
    closedir(pDir);
    // And move to original path.
    chdir(currPath);
}