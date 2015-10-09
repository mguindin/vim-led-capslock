# Vim LED Caps Lock Control
## Control Caps Lock LED based off of VIM insert mode state

Only works on OS X (uses Core.Framework)

Add to vim
```
  au InsertEnter * silent !/Users/mguindin/led_control
  au InsertLeave * silent !/Users/mguindin/led_control 0
```

Modified from original Apple code: [Apple HID LED test tool](https://developer.apple.com/library/mac/samplecode/HID_LED_test_tool/Introduction/Intro.html)

## Building
Build using XCode or just use the compiled binary, led_control, which will work
on OS X 10.10 and 10.11
