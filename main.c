#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stage.h"

static const short ARGCOUNT[5] = {0, 3, 4, 4, 4};

// Checks if the arguments are in the correct format 
int argCheck(int argc, char **argv) {
    if (argc == 1) {
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    // Check if code is 1 character long
    if (argv[1][1] != '\0') {
        printf("Invalid Stage!\n");
        exit(EXIT_FAILURE);
    }
    
    int stage;
    switch (argv[1][0]) {
        case '1':
            stage = 1;
            break;
        case '2':
            stage = 2;
            break;
        case '3':
            stage = 3;
            break;
        case '4':
            stage = 4;
            break;
        default:
            printf("Invalid Stage!\n");
            exit(EXIT_FAILURE);
            break;
    }

    if (argc != ARGCOUNT[stage] + 1) {
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    return stage;
}

// Runs the corresponding stage given the stage number
void runStage(int stage, char **argv) {
    switch (stage) {
        case 1:
            stage1(argv[2], argv[3]);
            break;
        case 2:
            stage2(argv[2], argv[3], argv[4]);
            break;
        case 3:
            stage34(argv[2], argv[3], argv[4], false);
            break;
        case 4:
            stage34(argv[2], argv[3], argv[4], true);
            break;
        default:
            printf("Invalid Stage!\n");
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    int stage = argCheck(argc, argv);
    runStage(stage, argv);
}
