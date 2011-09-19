// Double quotes, spaces OK.
#define PLUG_MFR "Leet Powered"

#define BUNDLE_DOMAIN "com.leetPowered"


#if defined VST_API
    #define PLUG_NAME "Hush"
    #define BUNDLE_NAME "Hush"
#elif defined AU_API
    #define PLUG_NAME "Hush"
    #define BUNDLE_NAME "Hush"
#else
    #define PLUG_NAME "Hush"
    #define BUNDLE_NAME "Hush"
#endif



// No quotes or spaces.
#define PLUG_CLASS_NAME PlugHush

// OSX crap.
// - Manually edit the info.plist file to set the CFBundleIdentifier to the either the string 
// "BUNDLE_DOMAIN.audiounit.BUNDLE_NAME" or "BUNDLE_DOMAIN.vst.BUNDLE_NAME".
// Double quotes, no spaces. BUNDLE_DOMAIN must contain only alphanumeric
// (A-Z,a-z,0-9), hyphen (-), and period (.) characters.


// - Manually create a PLUG_CLASS_NAME.exp file with two entries: _PLUG_ENTRY and _PLUG_VIEW_ENTRY
// (these two defines, each with a leading underscore).
// No quotes or spaces.
#define PLUG_ENTRY PlugHush_Entry
#define PLUG_VIEW_ENTRY PlugHush_ViewEntry
// The same strings, with double quotes.  There's no other way, trust me.
#define PLUG_ENTRY_STR "PlugHush_Entry"
#define PLUG_VIEW_ENTRY_STR "PlugHush_ViewEntry"
// This is the exported cocoa view class, some hosts display this string.
// No quotes or spaces.
#define VIEW_CLASS PlugHush_View
#define VIEW_CLASS_STR "PlugHush_View"

// This is interpreted as 0xMAJR.MN.BG
#define PLUG_VER 0x00010000

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes.

#define PLUG_UNIQUE_ID 'Hush'
#define PLUG_MFR_ID 'ltPW'

#define PLUG_CHANNEL_IO "1-1 2-2"

#define PLUG_LATENCY 0
#define PLUG_IS_INST 0
#define PLUG_DOES_MIDI 1
#define PLUG_DOES_STATE_CHUNKS 0

// Unique IDs for each image resource.
#define BG_ID		     100
#define IMG_MIDI_ID      101
#define IMG_MIDIBG_ID    102
#define IMG_LED_ID       103
#define IMG_KNOB1_ID     104
#define IMG_KNOB2_ID     105
#define IMG_HELPICON_ID  106
#define IMG_HELPDESC_ID  107



// Image resource locations for this plug.
#define BG_FN		     "img/background.png"
#define IMG_MIDI_FN      "img/midi.png"
#define IMG_MIDIBG_FN    "img/midi-bg.png"
#define IMG_LED_FN       "img/led_stack.png"
#define IMG_KNOB1_FN     "img/knob_type1_shadow_stack.png"
#define IMG_KNOB2_FN     "img/knob_type2_shadow_stack.png"
#define IMG_HELPICON_FN  "img/help-icon.png"
#define IMG_HELPDESC_FN  "img/help-desc.png"
