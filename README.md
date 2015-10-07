## Control Caps Lock LED based off of VIM insert mode state

Only works on OS X (uses Core.Framework)

Add to vim
```
  au InsertEnter * silent !/Users/mguindin/led_control
  au InsertLeave * silent !/Users/mguindin/led_control 0
```
