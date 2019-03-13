/**
 * rogaura
 * Copyright (c) 2019 Will Roberts
 *
 * Author:        Will Roberts <wildwilhelm@gmail.com> (WKR)
 * Creation Date: 13 March 2019
 *
 * Description:
 *    RGB keyboard control for Asus ROG laptops
 *
 * Revision Information:
 *
 *    (WKR) 13 March 2019
 *          - Boilerplate header added.
 *
 * \file rogaura.c
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define MESSAGE_LENGTH 17
#define MAX_NUM_MESSAGES 6
#define MAX_NUM_COLORS 4
#define MAX_FUNCNAME_LEN 32

// https://stackoverflow.com/a/14251257/1062499
#define DEBUG
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif


// ------------------------------------------------------------
//  Data structures
// ------------------------------------------------------------

typedef struct {
    uint8_t nRed;
    uint8_t nGreen;
    uint8_t nBlue;
} Color;

typedef enum {Slow = 1, Medium, Fast} Speed;

typedef struct {
    Color colors[MAX_NUM_COLORS];
    Speed speed;
} Arguments;

typedef struct {
    int nMessages;
    uint8_t messages[MAX_NUM_MESSAGES][MESSAGE_LENGTH];
} Messages;

typedef struct {
    const char *szName;
    void (*function)(const Arguments *args, Messages *outputs);
    int nColors;
    int nSpeed;
} FunctionRecord;


// ------------------------------------------------------------
//  USB protocol for RGB keyboard
// ------------------------------------------------------------

const uint8_t SPEED_BYTE_VALUES[] = {0xe1, 0xeb, 0xf5};

uint8_t speedByteValue(Speed speed) {
    return SPEED_BYTE_VALUES[speed - 1];
}

const uint8_t MESSAGE_SET[] = {0x5d, 0xb5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t MESSAGE_APPLY[] = {0x5d, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void initMessage(uint8_t *msg) {
    memset(msg, 0, MESSAGE_LENGTH);
    msg[0] = 0x5d;
    msg[1] = 0xb3;
}

void single_static(const Arguments *args, Messages *outputs) {
    D(printf("single_static\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[4] = args->colors[0].nRed;
    m[5] = args->colors[0].nGreen;
    m[6] = args->colors[0].nBlue;
}

void single_breathing(const Arguments *args, Messages *outputs) {
    D(printf("single_breathing\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 1;
    m[4] = args->colors[0].nRed;
    m[5] = args->colors[0].nGreen;
    m[6] = args->colors[0].nBlue;
    m[7] = speedByteValue(args->speed);
    m[9] = 1;
    m[10] = args->colors[1].nRed;
    m[11] = args->colors[1].nGreen;
    m[12] = args->colors[1].nBlue;
}

void single_colorcycle(const Arguments *args, Messages *outputs) {
    D(printf("single_colorcycle\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 2;
    m[4] = 0xff;
    m[7] = speedByteValue(args->speed);
}

void multi_static(const Arguments *args, Messages *outputs) {
    D(printf("multi_static\n"));
    outputs->nMessages = 4;
    for (int i = 0; i < 4; ++i) {
        uint8_t *m = outputs->messages[i];
        initMessage(m);
        m[2] = i + 1;
        m[4] = args->colors[i].nRed;
        m[5] = args->colors[i].nGreen;
        m[6] = args->colors[i].nBlue;
        m[7] = 0xeb;
    }
}

void multi_breathing(const Arguments *args, Messages *outputs) {
    D(printf("multi_breathing\n"));
    outputs->nMessages = 4;
    for (int i = 0; i < 4; ++i) {
        uint8_t *m = outputs->messages[i];
        initMessage(m);
        m[2] = i + 1;
        m[3] = 1;
        m[4] = args->colors[i].nRed;
        m[5] = args->colors[i].nGreen;
        m[6] = args->colors[i].nBlue;
        m[7] = speedByteValue(args->speed);
    }
}


// ------------------------------------------------------------
//  Command line argument parsing
// ------------------------------------------------------------

const FunctionRecord FUNCTION_RECORDS[] = {
    {"single_static", &single_static, 1, 0},
    {"single_breathing", &single_breathing, 2, 1},
    {"single_colorcycle", &single_colorcycle, 0, 1},
    {"multi_static", &multi_static, 4, 0},
    {"multi_breathing", &multi_breathing, 4, 1},
};

const int NUM_FUNCTION_RECORDS = (int)(sizeof(FUNCTION_RECORDS) / sizeof(FUNCTION_RECORDS[0]));

void usage() {
    printf("rogaura - RGB keyboard control for Asus ROG laptops\n");
    printf("(c) 2019 Will Roberts\n\n");
    printf("Usage:\n");
    printf("   rogaura COMMAND ARGUMENTS\n\n");
    printf("COMMAND should be one of:\n");
    for (int i = 0; i < NUM_FUNCTION_RECORDS; ++i) {
        printf("   %s\n", FUNCTION_RECORDS[i].szName);
    }
}

void parseColor(char *arg, Color *result) {
    D(printf("parse color %s\n", arg));
    uint32_t v = 0;
    if (strlen(arg) != 6) goto fail;
    for (int i = 0; i < 6; ++i) {
        if (!isxdigit(arg[i])) goto fail;
    }
    v = (uint32_t)strtol(arg, 0, 16);
    if (errno == ERANGE) goto fail;
    result->nRed = (v >> 16) & 0xFF;
    result->nGreen = (v >> 8) & 0xFF;
    result->nBlue = v & 0xFF;
    D(printf("interpreted color %d %d %d\n", result->nRed, result->nGreen, result->nBlue));
    return;
fail:
    printf("Could not interpret color parameter value %s\n", arg);
    printf("Please give this value as a six-character hex string like ff0000.\n");
    exit(1);
}

void parseSpeed(char *arg, Speed *result) {
    D(printf("parse speed %s\n", arg));
    long nSpeed = strtol(arg, 0, 0);
    if (errno == ERANGE || nSpeed < 1 || nSpeed > 3) {
        printf("Could not interpret speed parameter value %s\n", arg);
        printf("Please give this value as an integer: 1 (slow), 2 (medium), or 3 (fast).\n");
        exit(1);
    }
    *result = (Speed)nSpeed;
}

void parseArguments(int argc, char **argv, Messages *messages) {
    // identify the function the user has asked for
    const FunctionRecord *desired = 0;
    if (argc > 1) {
        for (int i = 0; i < NUM_FUNCTION_RECORDS; ++i) {
            if (!strncmp(argv[1], FUNCTION_RECORDS[i].szName, MAX_FUNCNAME_LEN)) {
                desired = &(FUNCTION_RECORDS[i]);
                break;
            }
        }
    }
    if (!desired) {
        usage();
        exit(1);
    }
    // check that the function signature is satisfied
    if (argc != (1 + 1 + desired->nColors + desired->nSpeed)) {
        usage();
        printf("\nFunction %s takes ", desired->szName);
        if (desired->nColors > 0) {
            if (desired->nSpeed) {
                printf("%d color(s) and a speed", desired->nColors);
            } else {
                printf("%d color(s)", desired->nColors);
            }
        } else {
            if (desired->nSpeed) {
                printf("a speed");
            } else {
                printf("no arguments");
            }
        }
        printf(":\n   rogaura %s ", desired->szName);
        for (int i = 0; i < desired->nColors; i++) {
            printf("color%d ", i+1);
        }
        if (desired->nSpeed) {
            printf("speed");
        }
        printf("\n\ncolor arguments should be given as hex values like ff0000\n");
        printf("speed argument should be given as an integer: 1, 2, or 3\n");
        exit(1);
    }
    // parse the argument values
    Arguments args;
    int nColors = 0;
    for (int i = 2; i < argc; ++i) {
        if (nColors < desired->nColors) {
            parseColor(argv[i], &(args.colors[nColors]));
            nColors++;
        } else {
            parseSpeed(argv[i], &args.speed);
        }
    }
    D(printf("args:\n"));
    for (int i = 0; i < MAX_NUM_COLORS; ++i) {
        D(printf("color%d %d %d %d\n", i + 1, args.colors[i].nRed, args.colors[i].nGreen, args.colors[i].nBlue));
    }
    D(printf("speed %d\n", args.speed));
    // call the function the user wants
    desired->function(&args, messages);
    D(printf("constructed %d messages:\n", messages->nMessages));
    for (int i = 0; i < messages->nMessages; ++i) {
        D(printf("message %d: ", i));
        for (int j = 0; j < MESSAGE_LENGTH; j++)
        {
            D(printf("%02x ", messages->messages[i][j] & 0xff));
        }
        D(printf("\n"));
    }
}


// ------------------------------------------------------------
//  Main function
// ------------------------------------------------------------

int main(int argc, char **argv) {
    Messages messages;
    parseArguments(argc, argv, &messages);
}
