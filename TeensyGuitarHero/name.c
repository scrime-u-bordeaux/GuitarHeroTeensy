#include "usb_names.h"

// #define MIDI_NAME   {'M','y',' ','M','I','D','I'}
#define MIDI_NAME {'G','i','b','s','o','n',' ','H','e','r','o',' ','C','o','n','t','r','o','l','l','e','r'}
#define MIDI_NAME_LEN 22

// Do not change this part.  This exact format is required by USB.

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + MIDI_NAME_LEN * 2,
        3,
        MIDI_NAME
};