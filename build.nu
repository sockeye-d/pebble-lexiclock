#! /usr/bin/nu

use generate-package.nu

export def rebuild [] {
    generate-package export
    pebble build
}

export def "rebuild clean" [] {
    pebble clean
    rebuild
}

export def emulator [--emulator: list<string> = ["basalt"]] {
    let local_properties = open local.yaml

    rebuild
    ($local_properties.emulator | default $env.emulator? | default { $emulator }) | par-each {|it| pebble install --emulator $it }
}

export def phone [--ip: string] {
    let local_properties = open local.yaml

    rebuild
    pebble install --phone ($ip | default $env.phone_ip? | default { $local_properties.phone_ip })
}

export def main [] {
    rebuild
}
