# Battery Text Behavior Usage Guide

## Overview

The `battery_text` behavior outputs battery percentage information as typed text when triggered. It displays battery levels for both the central (left) and peripheral (right) halves of your split keyboard.

## Output Format

When triggered, the behavior outputs: `L:XX% R:YY%`

- `L:XX%` - Left (central) side battery percentage
- `R:YY%` - Right (peripheral) side battery percentage

Example output: `L:85% R:90%`

## Requirements

- ZMK firmware with battery reporting enabled (`CONFIG_ZMK_BATTERY_REPORTING=y`)
- Split keyboard configured as central (the side that connects to your computer)
- For split keyboards with BLE, ensure `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y` is enabled

## Usage in Keymap

### Basic Usage

Add the behavior to any key in your keymap:

```dts
/ {
    keymap {
        compatible = "zmk,keymap";

        default_layer {
            bindings = <
                // ... other keys ...
                &bat_text    // Press this key to output battery info
                // ... other keys ...
            >;
        };
    };
};
```

### With Layer Toggle

Combine with a layer to access on a function layer:

```dts
/ {
    keymap {
        compatible = "zmk,keymap";

        default_layer {
            bindings = <
                // ... your keys ...
                &mo 1    // Hold for layer 1
                // ... more keys ...
            >;
        };

        function_layer {
            bindings = <
                // ... other function keys ...
                &bat_text    // Battery info on function layer
                // ... more keys ...
            >;
        };
    };
};
```

### In a Macro

You can also include it in a macro sequence:

```dts
/ {
    macros {
        check_status: check_status {
            compatible = "zmk,behavior-macro";
            #binding-cells = <0>;
            bindings = <&bat_text>;
        };
    };
};
```

## Example: Sofle Configuration

For a Sofle keyboard, you might add it to an upper layer:

```dts
// In your sofle.keymap file

/ {
    keymap {
        compatible = "zmk,keymap";

        default_layer {
            bindings = <
                &kp GRAVE &kp N1 &kp N2 &kp N3 &kp N4 &kp N5                      &kp N6 &kp N7 &kp N8    &kp N9   &kp N0   &kp BSPC
                &kp TAB   &kp Q  &kp W  &kp E  &kp R  &kp T                       &kp Y  &kp U  &kp I     &kp O    &kp P    &kp BSLH
                &kp LCTRL &kp A  &kp S  &kp D  &kp F  &kp G                       &kp H  &kp J  &kp K     &kp L    &kp SEMI &kp SQT
                &kp LSHFT &kp Z  &kp X  &kp C  &kp V  &kp B  &kp C_MUTE &none     &kp N  &kp M  &kp COMMA &kp DOT  &kp FSLH &kp RSHFT
                              &kp LGUI &kp LALT &mo 1 &kp SPACE &kp RET  &kp SPACE &mo 2  &kp RALT &kp RGUI &kp RCTRL
            >;
        };

        lower_layer {
            bindings = <
                &kp ESC  &kp F1   &kp F2   &kp F3    &kp F4    &kp F5                       &kp F6    &kp F7   &kp F8    &kp F9    &kp F10  &kp F11
                &trans   &trans   &trans   &trans    &trans    &trans                       &trans    &trans   &kp UP    &trans    &trans   &kp F12
                &trans   &trans   &trans   &trans    &trans    &trans                       &trans    &kp LEFT &kp DOWN  &kp RIGHT &trans   &trans
                &trans   &trans   &trans   &trans    &trans    &trans   &trans    &trans    &trans    &trans   &trans    &trans    &trans   &trans
                                  &trans   &trans    &trans    &trans   &trans    &trans    &trans    &trans   &trans    &bat_text
            >;
        };
    };
};
```

In this example, pressing the bottom-right key on layer 1 (accessed via `&mo 1`) will output the battery information.

## Configuration

### Enable Battery Reporting

Make sure your keyboard configuration has battery reporting enabled. In your keyboard's `.conf` file:

```conf
CONFIG_ZMK_BATTERY_REPORTING=y
```

### Enable Split Battery Fetching (for split keyboards)

For split keyboards using Bluetooth, add this to your central side configuration:

```conf
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y
```

## Troubleshooting

### No output when pressing the key
- Verify battery reporting is enabled in your config
- Check that you're on the central (computer-connected) side of your split keyboard
- Ensure the behavior is included in ZMK build (verify `CONFIG_ZMK_BATTERY_REPORTING=y`)

### Only shows "L:XX% R:0%"
- The peripheral battery fetching may not be enabled
- Add `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y` to your config
- Ensure both halves of your split keyboard are connected

### Wrong characters appear
- This is likely a keyboard layout mismatch
- The behavior uses US keyboard layout key codes
- If you use a different OS keyboard layout, characters may not match

## Technical Details

- **Behavior name**: `&bat_text`
- **Binding cells**: 0 (no parameters needed)
- **Dependencies**:
  - `CONFIG_ZMK_BATTERY_REPORTING`
  - `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING` (for split keyboards)
- **Output timing**: 10ms tap time, 10ms wait between characters
- **Total output time**: Approximately 180ms for full string

## Building

After adding the behavior to your keymap, rebuild your ZMK firmware:

```bash
# If using GitHub Actions, commit and push your changes
git add config/
git commit -m "Add battery text behavior to keymap"
git push

# If building locally
west build -b your_board_name
```

## Notes

- The behavior only works on the central side of split keyboards (the side connected to the computer)
- Battery percentages are whole numbers (0-100)
- The peripheral battery reading may be 0 if the peripheral hasn't sent an update yet
- Battery updates are sent periodically (default: every 60 seconds)
