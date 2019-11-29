/**
 * rogauracore
 * Copyright (c) 2019 Will Roberts
 * Copyright (c) 2019 Josh Ventura
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
 *    (JPV) 28 November 2019
 *          - Added support for brightness adjustment.
 *          - Generalized speed specification mechanism to accomodate brightness
 *            or other integer values.
 *    (JPV) 29 November 2019
 *          - Added support for single_pulsing and the official rainbow effect.
 *          - Added support for optional arguments and usage printing.
 *          - Generalized argument formats.
 *
 * \file rogauracore.c
 */

// sudo apt install libusb-1.0-0 libusb-1.0-0-dev

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#if STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif /*STDC_HEADERS*/

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif /*HAVE_ERRNO_H*/
#ifndef errno
/* Some systems #define this! */
extern int errno;
#endif

#include <ctype.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>

#define MESSAGE_LENGTH 17
#define MAX_NUM_MESSAGES 6
#define MAX_NUM_ARGUMENTS 8
#define MAX_FUNCNAME_LEN 32

// verbose output
int verbose = 0;
#define V(x) if (!verbose); else x


// ------------------------------------------------------------
//  Data structures
// ------------------------------------------------------------

typedef struct {
    uint8_t nRed;
    uint8_t nGreen;
    uint8_t nBlue;
} Color;

typedef struct {
    const char *name;
    Color color;
} NamedColor;

typedef struct {
    const char *name;
    int value;
} NamedScalar;

typedef enum {
    AK_UNSPECIFIED,
    AK_COLOR,
    AK_SCALAR,
    AK_AUTOMATIC
} ArgumentKind;

typedef struct {
    ArgumentKind type;
    union {
      Color color;
      int scalar;
    };
} Argument;

typedef Argument Arguments[MAX_NUM_ARGUMENTS];

typedef struct {
    const char *name;
    ArgumentKind kind;
    int (*parse)(const char *value, const Argument *defaultValue, Argument *out);
    Argument defaultValue;
} ArgumentDef;

typedef struct {
    int nMessages;
    uint8_t messages[MAX_NUM_MESSAGES][MESSAGE_LENGTH];
    int setAndApply;
} Messages;


typedef struct {
    const char *szName;
    void (*function)(Arguments args, Messages *outputs);
    int nArgs;
    ArgumentDef args[MAX_NUM_ARGUMENTS];
} FunctionRecord;

// ------------------------------------------------------------
//  USB protocol for RGB keyboard
// ------------------------------------------------------------

const uint8_t SPEED_BYTE_VALUES[] = {0xe1, 0xeb, 0xf5};

uint8_t speedByteValue(int speed) {
    return SPEED_BYTE_VALUES[speed - 1];
}

const int BRIGHTNESS_OFFSET = 4;
uint8_t MESSAGE_BRIGHTNESS[MESSAGE_LENGTH] = {0x5a, 0xba, 0xc5, 0xc4};
uint8_t MESSAGE_SET[MESSAGE_LENGTH] = {0x5d, 0xb5};
uint8_t MESSAGE_APPLY[MESSAGE_LENGTH] = {0x5d, 0xb4};

void
initMessage(uint8_t *msg) {
    memset(msg, 0, MESSAGE_LENGTH);
    msg[0] = 0x5d;
    msg[1] = 0xb3;
}

void
single_static(Arguments args, Messages *outputs) {
    V(printf("single_static\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[4] = args[0].color.nRed;
    m[5] = args[0].color.nGreen;
    m[6] = args[0].color.nBlue;
}

void
single_breathing(Arguments args, Messages *outputs) {
    V(printf("single_breathing\n"));
    outputs->nMessages = 1;
    if (args[1].type == AK_AUTOMATIC) args[1] = args[0];
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 1;
    m[4] = args[0].color.nRed;
    m[5] = args[0].color.nGreen;
    m[6] = args[0].color.nBlue;
    m[7] = speedByteValue(args[2].scalar);
    m[9] = 1;
    m[10] = args[1].color.nRed;
    m[11] = args[1].color.nGreen;
    m[12] = args[1].color.nBlue;
}

void
single_pulsing(Arguments args, Messages *outputs) {
    V(printf("single_pulsing\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 0x0a;
    m[4] = args[0].color.nRed;
    m[5] = args[0].color.nGreen;
    m[6] = args[0].color.nBlue;
    m[7] = speedByteValue(args[1].scalar);
}

void
single_colorcycle(Arguments args, Messages *outputs) {
    V(printf("single_colorcycle\n"));
    outputs->nMessages = 1;
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 2;
    m[4] = 0xff;
    m[7] = speedByteValue(args[0].scalar);
}

void
multi_static(Arguments args, Messages *outputs) {
    V(printf("multi_static\n"));
    outputs->nMessages = 4;
    for (int i = 0; i < 4; ++i) {
        uint8_t *m = outputs->messages[i];
        initMessage(m);
        m[2] = i + 1;
        m[4] = args[i].color.nRed;
        m[5] = args[i].color.nGreen;
        m[6] = args[i].color.nBlue;
        m[7] = 0xeb;
    }
}

void
multi_breathing(Arguments args, Messages *outputs) {
    V(printf("multi_breathing\n"));
    outputs->nMessages = 4;
    for (int i = 0; i < 4; ++i) {
        uint8_t *m = outputs->messages[i];
        initMessage(m);
        m[2] = i + 1;
        m[3] = 1;
        m[4] = args[i].color.nRed;
        m[5] = args[i].color.nGreen;
        m[6] = args[i].color.nBlue;
        m[7] = speedByteValue(args[0].scalar);
    }
}

void
rainbow(Arguments args, Messages *outputs) {
    V(printf("rainbow\n"));
    uint8_t *m = outputs->messages[0];
    initMessage(m);
    m[3] = 0x03;
    m[4] = 0x08;
    m[5] = 0xff;
    m[6] = 0xf0;
    m[7] = speedByteValue(args[0].scalar);
    outputs->nMessages = 1;
}

void
set_brightness(Arguments args, Messages *outputs) {
    V(printf("single_static\n"));
    memcpy(outputs->messages[0], MESSAGE_BRIGHTNESS, MESSAGE_LENGTH);
    outputs->messages[0][BRIGHTNESS_OFFSET] = args[0].scalar;
    outputs->nMessages = 1;
    outputs->setAndApply = 0;
}

// ------------------------------------------------------------
//  Command line argument parsing
// ------------------------------------------------------------

int
parseColor(const char *arg, const Argument *defaultValue, Argument *pResult);
int
parseSpeed(const char *arg, const Argument *defaultValue, Argument *pResult);
int
parseBrightness(const char *arg, const Argument *defaultValue, Argument *pResult);

const Argument ZERO =  { .type = AK_SCALAR, .scalar = 0 };
const Argument ONE =   { .type = AK_SCALAR, .scalar = 1 };
const Argument TWO =   { .type = AK_SCALAR, .scalar = 2 };
const Argument THREE = { .type = AK_SCALAR, .scalar = 3 };
const Argument AUTO =  { .type = AK_AUTOMATIC };

const ArgumentDef COLOR =  {"COLOR", AK_COLOR, parseColor};
const ArgumentDef COLOR1 = {"COLOR1", AK_COLOR, parseColor};
const ArgumentDef COLOR2 = {"COLOR2", AK_COLOR, parseColor};
const ArgumentDef COLOR3 = {"COLOR3", AK_COLOR, parseColor};
const ArgumentDef COLOR4 = {"COLOR4", AK_COLOR, parseColor};
const ArgumentDef SPEED =  {"SPEED", AK_SCALAR, parseSpeed};
const ArgumentDef BRIGHTNESS = {"BRIGHTNESS", AK_SCALAR, parseBrightness };

const ArgumentDef COLOR2_OR_AUTO = {"COLOR2", AK_COLOR, parseColor, AUTO};
const ArgumentDef SPEED_OR_TWO =  {"SPEED", AK_SCALAR, parseSpeed, TWO};
const ArgumentDef SPEED_OR_THREE =  {"SPEED", AK_SCALAR, parseSpeed, THREE};

const FunctionRecord FUNCTION_RECORDS[] = {
    {"single_static", &single_static, 1, {
        COLOR
    }},
    {"single_breathing", &single_breathing, 3, {
        COLOR1, COLOR2_OR_AUTO, SPEED_OR_TWO
    }},
    {"single_pulsing", &single_pulsing, 2, {
        COLOR, SPEED
    }},
    {"single_colorcycle", &single_colorcycle, 1, {
        SPEED
    }},
    {"multi_static", &multi_static, 4, {
        COLOR1, COLOR2, COLOR3, COLOR4
    }},
    {"multi_breathing", &multi_breathing, 5, {
        COLOR1, COLOR2, COLOR3, COLOR4, SPEED
    }},
    {"rainbow", &rainbow, 1, {
        SPEED_OR_THREE
    }},
    {"brightness", &set_brightness, 1, {
        BRIGHTNESS
    }},
};

const int NUM_FUNCTION_RECORDS = (int)(sizeof(FUNCTION_RECORDS) / sizeof(FUNCTION_RECORDS[0]));

const NamedColor NAMED_COLORS[] = {
    {"red",      { 0xff, 0x00, 0x00 }},
    {"orange",   { 0xff, 0x80, 0x00 }},
    {"yellow",   { 0xff, 0xff, 0x00 }},
    {"lime",     { 0x80, 0xff, 0x00 }},
    {"green",    { 0x00, 0xff, 0x00 }},
    {"teal",     { 0x00, 0xff, 0x40 }},
    {"turquoise",{ 0x00, 0xff, 0x80 }},
    {"cyan",     { 0x00, 0xff, 0xff }},
    {"skyBlue",  { 0x00, 0x80, 0xff }},
    {"blue",     { 0x00, 0x00, 0xff }},
    {"indigo",   { 0x40, 0x00, 0xff }},
    {"violet",   { 0x80, 0x00, 0xff }},
    {"magenta",  { 0xff, 0x00, 0xff }},
    {"pink",     { 0xff, 0x40, 0xe0 }},
    {"deepPink", { 0xff, 0x00, 0x80 }},
    {"hotPink",  { 0xff, 0x00, 0x40 }},
    {"white",    { 0xff, 0xff, 0xff }},
    {"black",    { 0x00, 0x00, 0x00 }},
};

const int NUM_NAMED_COLORS =
    (int)(sizeof(NAMED_COLORS) / sizeof(*NAMED_COLORS));

const NamedScalar NAMED_SPEEDS[] = {
    {"slow", 1}, {"low", 1}, {"medium", 2}, {"fast", 3}, {"high", 3}
};
const int NUM_NAMED_SPEEDS =
    (int)(sizeof(NAMED_SPEEDS) / sizeof(*NAMED_SPEEDS));

const NamedScalar NAMED_BRIGHTNESSES[] = {
    {"off", 0}, {"low", 1}, {"dim", 1}, {"medium", 2}, {"high", 3}, {"bright", 3}
};
const int NUM_NAMED_BRIGHTNESSES =
    (int)(sizeof(NAMED_BRIGHTNESSES) / sizeof(*NAMED_BRIGHTNESSES));

void
printFuncUsage(const FunctionRecord *func) {
    printf("%s", func->szName);
    int num_closing = 0;
    for (int i = 0; i < func->nArgs; ++i) {
        const ArgumentDef *arg = &func->args[i];
        printf(arg->defaultValue.type ? " [" : " ");
        printf(arg->name);
        if (arg->defaultValue.type) printf("]");
    }
    printf("\n");
}

inline void
printColumns(int nItems, const void *items, int itemSize, int nColumns) {
    const int nRows = (nItems + nColumns - 1) / nColumns;
    int printed = 0;
    for (int i = 0; i < nRows; ++i) {
        printf("\n   ");
        for (int j = 0; i + j < nItems; j += nRows) {
            printf("%-12s", *(const char**)(items + itemSize * (i + j)));
        }
    }
}

void
usage() {
    printf("rogauracore - RGB keyboard control for Asus ROG laptops\n");
    printf("(c) 2019 Will Roberts\n\n");
    printf("Usage:\n");
    printf("   rogauracore COMMAND ARGUMENTS\n\n");
    printf("Supported commands and usage:\n");
    for (int i = 0; i < NUM_FUNCTION_RECORDS; ++i) {
        printf("   ");
        printFuncUsage(&FUNCTION_RECORDS[i]);
    }
    printColumns(NUM_NAMED_COLORS, NAMED_COLORS, sizeof(*NAMED_COLORS), 6);
    printf("\n\n\
COLOR argument(s) should be given as color names, or hex values like ff0000.\n\
SPEED argument should be given as slow, medium, or fast, or integers 1 - 3.\n\
BRIGHTNESS values should be given as off, low, medium, high, or integers 0 - 3.\
\n\n");
}

void printArg(const Argument *arg) {
    switch (arg->type) {
        case AK_UNSPECIFIED:
            printf("[unspecified]");
            return;
        case AK_COLOR:
            printf("rgb(%d, %d, %d)",
                   arg->color.nRed, arg->color.nGreen, arg->color.nBlue);
            return;
        case AK_SCALAR:
            printf("%d", arg->scalar);
            return;
        case AK_AUTOMATIC:
            printf("[auto]");
            return;
    }
}

void
printMessages(const Messages *messages) {
    printf("constructed %d messages:\n", messages->nMessages);
    for (int i = 0; i < messages->nMessages; ++i) {
        printf("message %d: ", i);
        for (int j = 0; j < MESSAGE_LENGTH; j++) {
            printf("%02x ", messages->messages[i][j]);
        }
        printf("\n");
    }
}

int
parseColor(const char *arg, const Argument *defaultValue, Argument *pResult) {
    V(printf("parse color %s\n", arg));
    pResult->type = AK_COLOR;
    Color *col = &pResult->color;
    for (int i = 0; i < NUM_NAMED_COLORS; ++i) {
        if (!strcasecmp(arg, NAMED_COLORS[i].name)) {
            col->nRed =   NAMED_COLORS[i].color.nRed;
            col->nGreen = NAMED_COLORS[i].color.nGreen;
            col->nBlue =  NAMED_COLORS[i].color.nBlue;
            return 0;
        }
    }
    if (strlen(arg) != 6) goto fail;
    uint32_t v = 0;
    for (int i = 0; i < 6; ++i) {
        if (!isxdigit(arg[i])) goto fail;
    }
    v = (uint32_t)strtol(arg, 0, 16);
    if (errno == ERANGE) goto fail;
    pResult->color.nRed = (v >> 16) & 0xff;
    pResult->color.nGreen = (v >> 8) & 0xff;
    pResult->color.nBlue = v & 0xff;
    V(printf("Interpreted color %d %d %d\n",
             col->nRed, col->nGreen, col->nBlue));
    return 0;
fail:
    if (defaultValue->type) {
      *pResult = *defaultValue;
      return 1;
    }
    fprintf(stderr, "Could not interpret color parameter value `%s`\n", arg);
    fprintf(stderr, "Please name a color, or give this value as a six-character"
                    " hex string like ff0000.\n");
    return -1;
}

int
parseScalar(const char *arg, int num_named_vals, const NamedScalar *named_vals,
            int min, int max, const Argument *defaultValue, Argument *pResult) {
    for (int i = 0; i < num_named_vals; ++i) {
        if (!strcasecmp(arg, named_vals[i].name)) {
            pResult->scalar = named_vals[i].value;
            return 0;
        }
    }
    long nScalar = strtol(arg, 0, 0);
    if (errno == ERANGE || nScalar < min || nScalar > max) {
        if (defaultValue->type) {
          *pResult = *defaultValue;
          return 1;
        }
        return -1;
    }
    V(printf("Parsed as %d\n", nScalar));
    pResult->scalar = nScalar;
    return 0;
}

int
parseSpeed(const char *arg, const Argument *defaultValue, Argument *pResult) {
    V(printf("parse speed %s\n", arg));
    int r = parseScalar(arg, NUM_NAMED_SPEEDS, NAMED_SPEEDS, 1, 3,
                        defaultValue, pResult);
    if (r < 0) {
        fprintf(stderr, "Could not interpret speed parameter value `%s`\n"
                "Please give this value as 1 (slow), 2 (medium), or 3 (fast).\n",
                arg);
    }
    return r;
}

int
parseBrightness(const char *arg, const Argument *defValue, Argument *pResult) {
    V(printf("parse brightness %s\n", arg));
    int r = parseScalar(arg, NUM_NAMED_BRIGHTNESSES, NAMED_BRIGHTNESSES, 0, 3,
                        defValue, pResult);
    if (r < 0) {
        fprintf(stderr, "Could not interpret brightness parameter value `%s`\n"
                "Please give this value as 0 (off), 1 (dim), 2 (medium), or 3 "
                "(bright).\n", arg);
    }
    return r;
}

int
parseArguments(int argc, char **argv, Messages *messages) {
    int                   nRetval;
    Arguments             args = {};
    int                   nArgs         = 0;
    int                   nArgsRead     = 0;
    const FunctionRecord *pDesiredFunc  = 0;

    // check for command line options
    while ((nRetval = getopt(argc, argv, "v")) != -1) {
        switch (nRetval) {
        case 'v':
            verbose = 1;
            break;
        default: /* '?' */
            usage();
            return -1;
        }
    }
    nArgs = argc - optind;

    // identify the function the user has asked for
    if (nArgs > 0) {
        for (int i = 0; i < NUM_FUNCTION_RECORDS; ++i) {
            if (!strncmp(argv[optind], FUNCTION_RECORDS[i].szName, MAX_FUNCNAME_LEN)) {
                pDesiredFunc = &(FUNCTION_RECORDS[i]);
                break;
            }
        }
    } else {
        usage();
        return -1;
    }
    if (!pDesiredFunc) {
        for (int i = 0; i < NUM_NAMED_COLORS; ++i) {
            if (!strcasecmp(argv[optind], NAMED_COLORS[i].name)) {
                args[0].color = NAMED_COLORS[i].color;
                single_static(args, messages);
                V(printMessages(messages));
                return 0;
            }
        }
    }
    if (!pDesiredFunc) {
        usage();
        return -1;
    }
    // check that the function signature is satisfied
    int min_args = 0;
    for (int i = 0; i < pDesiredFunc->nArgs; ++i) {
      if (!pDesiredFunc->args[i].defaultValue.type) ++min_args;
    }
    if (nArgs - 1 < min_args || nArgs - 1 > pDesiredFunc->nArgs) {
        usage();
        if (min_args == pDesiredFunc->nArgs) {
            printf("\nFunction %s takes %d arguments:\n",
                   pDesiredFunc->szName, pDesiredFunc->nArgs);
        } else {
            printf("\nFunction %s takes %d-%d arguments:\n",
                   pDesiredFunc->szName, min_args, pDesiredFunc->nArgs);
        }
        printf("   rogauracore ", pDesiredFunc->szName);
        printFuncUsage(pDesiredFunc);
        return -1;
    }
    // parse the argument values
    nArgsRead = 0;
    for (int i = optind + 1; i < argc; ++i) {
        int r;
        V(printf("Reading argument %d (%s)\n", optind, argv[optind]));
        for (;;) {
            const ArgumentDef *arg = &pDesiredFunc->args[nArgsRead];
            if (nArgsRead >= pDesiredFunc->nArgs) {
                fprintf(stderr, "Extra (unrecognized) argument `%s`.\n"
                        "Please check your argument order and correct or remove"
                        " this argument.\n", argv[i]);
                printFuncUsage(pDesiredFunc);
                return -1;
            }
            if (!arg->parse) {
                fprintf(stderr, "Internal error: no parser for %s argument.\n",
                        arg->name ? arg->name : "null");
                return -1;
            }
            r = arg->parse(argv[i], &pDesiredFunc->args[nArgsRead].defaultValue,
                           &args[nArgsRead]);
            ++nArgsRead;
            if (!r) break;
            if (r != 1) {
                V(printf("Parser didn't like the value and argument isn't "
                         "optional; giving up.\n"));
                return r;
            }
        }
    }
    
    while (nArgsRead < pDesiredFunc->nArgs) {
        args[nArgsRead] = pDesiredFunc->args[nArgsRead].defaultValue;
        ++nArgsRead;
    }
    V(printf("args:\n"));
    for (int i = 0; i < MAX_NUM_ARGUMENTS; ++i) {
        V(printArg(&args[i]));
    }
    // call the function the user wants
    pDesiredFunc->function(args, messages);
    V(printMessages(messages));
    return 0;
}


// ------------------------------------------------------------
//  Libusb interface
// ------------------------------------------------------------

const uint16_t ASUS_VENDOR_ID = 0x0b05;
const uint16_t ASUS_PRODUCT_IDS[] = { 0x1854, 0x1869, 0x1866 };
const int NUM_ASUS_PRODUCTS = (int)(sizeof(ASUS_PRODUCT_IDS) / sizeof(ASUS_PRODUCT_IDS[0]));

int
checkDevice(libusb_device *pDevice) {
    struct libusb_device_descriptor devDesc;
    libusb_get_device_descriptor(pDevice, &devDesc);
    V(printf("Checking device %04x:%04x, address %d\n",
             devDesc.idVendor, devDesc.idProduct,
             libusb_get_device_address(pDevice)));
    if (devDesc.idVendor == ASUS_VENDOR_ID) {
        for (int i = 0; i < NUM_ASUS_PRODUCTS; ++i)
        {
            if (devDesc.idProduct == ASUS_PRODUCT_IDS[i]) return 1;
        }
    }
    return 0;
}

int
controlTransfer(libusb_device_handle *pHandle, unsigned char *sData, uint16_t wLength) {
    int nRetval = libusb_control_transfer(
        pHandle,
        0x21 /* bmRequestType */,
        9 /* bRequest */,
        0x035d /* wValue */,
        0 /* wIndex */,
        sData,
        wLength,
        0 /* standard device timeout */
        );
    if (nRetval < 0) {
        fprintf(stderr, "Control transfer error: %s\n", libusb_error_name(nRetval));
    }
    return nRetval;
}

int
handleUsb(Messages *pMessages) {
    int                              nRetval;
    libusb_device                  **deviceList       = 0;
    int                              nDevices         = 0;
    libusb_device                   *device           = 0;
    libusb_device                   *auraCoreDevice   = 0;
    libusb_device_handle            *pHandle          = 0;
    uint8_t                          bInterfaceNumber = 0;
    struct libusb_config_descriptor *pConfig          = 0;
    // Try to initialise the libusb library
    V(printf("Initialising libusb\n"));
    if (libusb_init(0) < 0) {
        fprintf(stderr, "Could not initialise libusb.\n");
        nRetval = -1; goto exit;
    }
    V(printf("Initialised libusb.\n"));

    // Lets try to find our HID device that controls backlight LEDs.
    nDevices = libusb_get_device_list(0, &deviceList);
    if (nDevices < 0) {
        fprintf(stderr, "Could not fetch USB device list.\n");
        nRetval = -1; goto deinit;
    }
    V(printf("Found %d USB devices.\n", nDevices));
    for (int i = 0; i < nDevices; i++) {
        device = deviceList[i];
        if (checkDevice(device)) {
            V(printf("Found ROG Aura Core keyboard.\n"));
            auraCoreDevice = device;
            break;
        }
    }
    if (!auraCoreDevice) {
        fprintf(stderr, "Could not find ROG Aura Core keyboard.\n");
        nRetval = -1; goto freelist;
    }
    nRetval = libusb_open(auraCoreDevice, &pHandle);
    if (nRetval < 0) {
        fprintf(stderr, "Could not open ROG Aura Core keyboard: %s\n", libusb_error_name(nRetval));
        goto freelist;
    }
    V(printf("Opened USB device.\n"));

    // Detach kernel drivers before USB communication
    nRetval = libusb_set_auto_detach_kernel_driver(pHandle, 1);
    if (nRetval < 0) {
        fprintf(stderr, "Could not set auto detach kernel mode: %s\n",
                libusb_error_name(nRetval));
    } else {
        V(printf("Auto detach kernel mode set.\n"));
    }

    // Get configuration descriptor
    nRetval = libusb_get_active_config_descriptor(auraCoreDevice, &pConfig);
    if (nRetval < 0) {
        fprintf(stderr, "Could not get configuration descriptor: %s.\n", libusb_error_name(nRetval));
        goto close;
    }
    V(printf("Got configuration descriptor.\n"));

    // We want to claim the first interface on the device
    if (pConfig->bNumInterfaces == 0) {
        fprintf(stderr, "No interfaces defined on the USB device.\n");
        nRetval = -1; goto freedesc;
    }
    V(printf("Found %d interfaces on the USB device.\n", pConfig->bNumInterfaces));
    if (pConfig->interface[0].num_altsetting == 0) {
        fprintf(stderr, "No interface descriptors for the first interface of the USB device.\n");
        nRetval = -1; goto freedesc;
    }
    bInterfaceNumber = pConfig->interface[0].altsetting[0].bInterfaceNumber;

    // Claim the interface
    nRetval = libusb_claim_interface(pHandle, bInterfaceNumber);
    if(nRetval < 0) {
        fprintf(stderr, "Could not claim interface: %s.\n", libusb_error_name(nRetval));
        goto freedesc;
    }
    V(printf("Claimed interface %d.\n", bInterfaceNumber));

    // Send the control messages
    for (int i = 0; i < pMessages->nMessages; ++i) {
        nRetval = controlTransfer(pHandle, pMessages->messages[i], MESSAGE_LENGTH);
        if (nRetval < 0) {
          fprintf(stderr, "Sending message %d of %d failed.\n", i, pMessages->nMessages);
          goto release;
        }
    }
    if (nRetval < 0) goto release;
    V(printf("Successfully sent all messages.\n"));
    if (pMessages->setAndApply) {
        nRetval = controlTransfer(pHandle, MESSAGE_SET, MESSAGE_LENGTH);
        if (nRetval < 0) goto release;
        V(printf("Sent SET message.\n"));
        nRetval = controlTransfer(pHandle, MESSAGE_APPLY, MESSAGE_LENGTH);
        if (nRetval < 0) goto release;
        V(printf("Sent APPLY message.\n"));
    }

release:
    libusb_release_interface(pHandle, bInterfaceNumber);
freedesc:
    libusb_free_config_descriptor(pConfig);
close:
    libusb_close(pHandle);
freelist:
    libusb_free_device_list(deviceList, 1);
deinit:
    libusb_exit(0);
exit:
    return nRetval;
}

// ------------------------------------------------------------
//  Main function
// ------------------------------------------------------------

int
main(int argc, char **argv) {
    Messages messages;
    messages.setAndApply = 1;
    if (parseArguments(argc, argv, &messages) == 0) {
        handleUsb(&messages);
    }
}
