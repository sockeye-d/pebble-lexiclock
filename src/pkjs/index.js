const Clay = require('pebble-clay');

// bg_color
// faint_color
// bright_color
// faint_bold
// bright_bold

new Clay(
    [
        {
            type: "color",
            messageKey: "bg_color",
            defaultValue: "0x000000",
            label: "Background color",
            capabilities: ["color"],
        },
        {
            type: "color",
            messageKey: "faint_color",
            defaultValue: "0x555555",
            label: "Faint text color",
            capabilities: ["color"],
        },
        {
            type: "color",
            messageKey: "bright_color",
            defaultValue: "0xAAFF55",
            label: "Bright text color",
            capabilities: ["color"],
        },
        {
            type: "toggle",
            messageKey: "faint_bold",
            defaultValue: 0,
            label: "Bold faint text",
        },
        {
            type: "toggle",
            messageKey: "bright_bold",
            defaultValue: 1,
            label: "Bold bright text",
        },
        {
            type: "toggle",
            messageKey: "light_on_dark",
            defaultValue: 0,
            label: "Use a light on dark theme",
            capabilities: ["bw"],
        },
        {
            type: "submit",
            defaultValue: "Save settings and exit",
        },
    ]
)
