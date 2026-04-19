const app_version = "1.6.0"

export def main [] {
    let media = 0..<37 | each {|num|
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
        name: 'lexiclock 2',
        author: fishy,
        version: $app_version,
        keywords: [ "pebble-watchface" ],
        private: true,
        dependencies: {
            pebble-clay: "^1.0.4"
        },
        capabilities: [
            configurable,
        ],
        pebble: {
            displayName: 'lexiclock 2',
            uuid: 7e68af47-c9c4-4f0a-b40e-ce13f813ec2f,
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
                date_mode,
                british_mode,
                shake_enabled,
                shake_animation,
            ],
            resources: {
                media: ([
                    {
                        menuIcon: true
                        name: IMAGE_MENU_ICON
                        file: launcher.png
                        type: png
                        targetPlatforms: [
                            aplite,
                            diorite,
                            basalt,
                            emery,
                        ]
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
                ] ++ $media),
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
