export def main [] {
    let bold_media = 0..<26 | each {|num|
        [
            {
                type: bitmap,
                name: $"IOSEVKA_ATLAS_($num)",
                file: font-atlas/($num).png,
                memoryFormat: 8Bit,
                spaceOptimization: memory,
                targetPlatforms: [
                    basalt,
                    emery,
                ]
            },
            {
                type: bitmap,
                name: $"IOSEVKA_ATLAS_($num)",
                file: font-atlas/($num).png,
                memoryFormat: 1Bit,
                spaceOptimization: memory,
                targetPlatforms: [
                    aplite,
                    diorite,
                ]
            },
            {
                type: bitmap,
                name: $"IOSEVKA_ATLAS_BOLD_($num)",
                file: font-atlas-bold/($num).png,
                memoryFormat: 8Bit,
                spaceOptimization: memory,
                targetPlatforms: [
                    basalt,
                    emery,
                ]
            },
        ]
    } | flatten | sort-by name --natural
    {
        name: lexiclock,
        author: fishy,
        version: "1.4.0",
        keywords: [ "pebble-watchface" ],
        private: true,
        dependencies: {
            pebble-clay: "^1.0.4"
        },
        capabilities: [
            configurable,
        ],
        pebble: {
            displayName: lexiclock,
            uuid: 1c6a1f0b-8194-491f-acc4-fc06a6f9966d,
            sdkVersion: "3",
            enableMultiJS: true,
            targetPlatforms: [
                aplite,
                diorite,
                basalt,
                emery,
            ],
            watchapp: { watchface: true },
            messageKeys: [
                bg_color,
                faint_color,
                bright_color,
                faint_bold,
                bright_bold,
                light_on_dark,
                use_fainter_dithering,
                british_mode,
            ],
            resources: {
                media: ($bold_media ++ [
                    {
                        type: bitmap,
                        menuIcon: true,
                        name: IMAGE_MENU_ICON,
                        file: launcher.png
                    },
                    {
                        type: bitmap,
                        name: DITHER_PATTERN_1,
                        file: dither1.png,
                        memoryFormat: 1Bit,
                        spaceOptimization: memory,
                        targetPlatforms: [
                            aplite,
                            diorite,
                        ]
                    },
                    {
                        type: bitmap,
                        name: DITHER_PATTERN_2,
                        file: dither2.png,
                        memoryFormat: 1Bit,
                        spaceOptimization: memory,
                        targetPlatforms: [
                            aplite,
                            diorite,
                        ]
                    },
                ]),
            },
        },
    }
}

export def json [] {
    main | to json
}

export def export [] {
    main | to json | save package.json --force
}
