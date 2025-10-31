# Debugging Battery Text Behavior

## Quick Fix

**The most common issue**: The behavior device name was wrong in the initial implementation. Make sure you have the latest version where `behavior_dev = "kp"` (not `"key_press"`).

## Enable Debug Logging

To see what's happening when you press the battery text key, enable debug logging:

### Option 1: Via Config File (Recommended)

Add these lines to your keyboard's `.conf` file (e.g., `config/sofle.conf`):

```conf
# Enable USB logging (if using USB connection)
CONFIG_ZMK_USB_LOGGING=y

# Set log level to debug
CONFIG_LOG_DEFAULT_LEVEL=4
CONFIG_ZMK_LOG_LEVEL_DBG=y

# Optionally, enable specific module logging
CONFIG_LOG=y
```

### Option 2: Enable in West Build

If building locally with west:

```bash
west build -b your_board -- -DCONFIG_ZMK_USB_LOGGING=y -DCONFIG_LOG_DEFAULT_LEVEL=4
```

## View Logs

### On Linux/macOS:
```bash
# Find your keyboard device (usually /dev/ttyACM0 or similar)
ls /dev/tty* | grep -i acm

# Read logs (replace with your device)
cat /dev/ttyACM0
```

### On Windows (WSL):
```powershell
# In PowerShell, use PuTTY or similar terminal emulator
# Or from WSL:
sudo cat /dev/ttyACM0
```

### Using West:
```bash
west build -t menuconfig  # Configure logging
west build -t flash       # Flash firmware
west attach              # Attach to serial console
```

## What to Look For

When you press the battery text key, you should see output like:

```
[00:01:23.456,000] <dbg> zmk: Battery text behavior pressed
[00:01:23.457,000] <dbg> zmk: Local battery level: 85%
[00:01:23.458,000] <dbg> zmk: Peripheral battery level: 90%
[00:01:23.459,000] <dbg> zmk: Battery text output queued: L:85% R:90%
```

## Common Issues and Solutions

### 1. Nothing in logs when pressing the key

**Problem**: The behavior isn't being triggered at all.

**Solutions**:
- Check your keymap - ensure `&bat_text` is correctly placed
- Verify you're pressing the right key
- Make sure the firmware actually compiled with the new behavior
- Check if you're on the correct layer

**Test**: Try adding this to a simple key on your base layer:
```dts
&bat_text  // Temporarily bind to a key you use often
```

### 2. "Battery text behavior pressed" appears but no output

**Problem**: The behavior runs but keys aren't being queued.

**Solutions**:
- The bug was likely `behavior_dev = "key_press"` instead of `"kp"` - verify this is fixed
- Check that CONFIG_ZMK_BATTERY_REPORTING=y is set
- Ensure the behavior queue is working

**Verify the fix**:
```c
// In behavior_battery_text.c, line ~57, should be:
.behavior_dev = "kp",  // ✓ Correct
// NOT:
.behavior_dev = "key_press",  // ✗ Wrong
```

### 3. Shows "L:0% R:0%"

**Problem**: Battery levels are reading as zero.

**Solutions**:
- Battery reporting might not be initialized yet after boot - wait 60 seconds and try again
- Check CONFIG_ZMK_BATTERY_REPORTING=y is in your config
- Verify your board has a battery configured in its device tree
- Check battery sensor is working: logs should show periodic battery updates

**What to check in logs**:
```
# You should see these periodically (every 60s by default):
[00:01:00.000,000] <inf> zmk: Battery level: 85%
```

### 4. Shows "L:XX% R:0%" on split keyboard

**Problem**: Peripheral battery isn't reporting.

**Solutions**:
- Add `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y` to central config
- Ensure both halves are connected (check Bluetooth connection)
- Peripheral must send battery update at least once (wait ~60 seconds after connection)
- This feature requires `CONFIG_ZMK_SPLIT_BLE` to be enabled

**Check the logs**:
```
[00:01:23.458,000] <dbg> zmk: Failed to get peripheral battery level: -2
# or
[00:01:23.458,000] <dbg> zmk: Split battery fetching not enabled, peripheral will show 0%
```

### 5. Wrong characters appear

**Problem**: You see weird characters instead of the expected format.

**Solutions**:
- Your OS keyboard layout is not US layout
- The behavior uses US keyboard codes (HID_USAGE_KEY_*)
- Either change your OS layout to US temporarily, or modify the behavior to match your layout
- Verify modifiers (Shift) are being sent correctly

**Workaround**: Switch OS keyboard to US layout before testing

### 6. Characters appear delayed or in wrong order

**Problem**: Timing issue with keystroke output.

**Solutions**:
- Increase `tap_ms` and `wait_ms` values (currently 10ms each)
- Some systems need longer delays between keypresses

**Modify in behavior_battery_text.c**:
```c
const uint32_t tap_ms = 20;   // Increase from 10
const uint32_t wait_ms = 20;  // Increase from 10
```

## Testing Steps

1. **Verify build includes the behavior**:
   ```bash
   # Check if behavior is compiled
   grep -r "battery_text" build/
   ```

2. **Test with logging enabled**: Flash with logging config and watch serial output

3. **Test on base layer**: Temporarily bind `&bat_text` to an easy-to-reach key

4. **Test battery reading**: Check if battery values are sensible (0-100)

5. **Test split connection**: Ensure both halves show in Bluetooth as connected

## Advanced Debugging

### Check if behavior is registered

Look for this in logs during boot:
```
[00:00:01.234,000] <dbg> zmk: Behavior battery_text initialized
```

### Verify binding in keymap

Use ZMK Studio or check the compiled keymap to ensure `&bat_text` is in the right position.

### Manual battery check

You can check battery levels without the behavior:
- Some systems show battery in Bluetooth settings
- On the keyboard: logs show periodic battery updates
- Use ZMK Studio if available

## Still Not Working?

If you've tried everything above:

1. **Share your logs**: Copy the serial output when pressing the key
2. **Share your config**: Post your `.conf` and `.keymap` files
3. **Check ZMK version**: Ensure you're using a recent version of ZMK
4. **Verify hardware**: Make sure battery sensor is actually working on your board

## Example Working Configuration

**File: `config/sofle.conf`**
```conf
CONFIG_ZMK_BATTERY_REPORTING=y
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y
CONFIG_ZMK_USB_LOGGING=y
CONFIG_LOG_DEFAULT_LEVEL=4
```

**File: `config/sofle.keymap`** (excerpt)
```dts
default_layer {
    bindings = <
        // ... other keys ...
        &bat_text  // Should output "L:85% R:90%" when pressed
    >;
};
```

After making these changes, rebuild and flash, then check serial output!
