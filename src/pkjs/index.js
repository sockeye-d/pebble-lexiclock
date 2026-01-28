const Clay = require('pebble-clay');

new Clay(
    [
        {
            type: "color",
            messageKey: "bg_color",
            defaultValue: "0x000000",
            label: "Background color",
            capabilities: ["COLOR"],
        },
        {
            type: "color",
            messageKey: "faint_color",
            defaultValue: "0x555555",
            label: "Faint text color",
            capabilities: ["COLOR"],
        },
        {
            type: "color",
            messageKey: "bright_color",
            defaultValue: "0xAAFF55",
            label: "Bright text color",
            capabilities: ["COLOR"],
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
            capabilities: ["BW"],
        },
        {
            type: "toggle",
            messageKey: "use_fainter_dithering",
            defaultValue: 0,
            label: "Use a fainter dithering pattern on inactive words",
            capabilities: ["BW"],
        },
        {
            type: "toggle",
            messageKey: "british_mode",
            defaultValue: 0,
            label: "Use a British English locale (by Connope's request)",
        },
        {
            type: "select",
            messageKey: "date_mode",
            label: "Date order",
            defaultValue: "0",
            options: [
                {
                    label: "month day year",
                    value: "0"
                },
                {
                    label: "day month year",
                    value: "1"
                },
                {
                    label: "year month day",
                    value: "2"
                },
            ]
        },
        {
            type: "submit",
            defaultValue: "Save settings and exit",
        },
    ]
)
