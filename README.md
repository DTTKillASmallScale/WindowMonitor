### WindowMonitor

WindowMonitor displays a scalable DWM thumbnail for a selected window.
\([Download](https://github.com/Temetra/WindowMonitor/releases)\)

#### Controls
* Resize window to set scale
* Drag with `Left Mouse` to move window
* Hold `Shift` and drag `Left Mouse` to pan view
* Hold `Control` and drag `Left Mouse` to crop view
* `Ctrl+Tab` and `Shift+Ctrl+Tab` cycles through available sources
* `Right Mouse` click or `Shift+F10` opens context menu
* `F5` resets view
* `F6` toggles fullscreen mode

#### Main features for v1.1.0
* Presets context menu
* Fullscreen menu option
* Checkmark indicators for selected menu options
* Preset is saved when enter is pressed while editing title

#### Secret features
* `F12` toggles window click-though mode (disables mouse interaction with window)
* Optional blacklist.txt for basic additional window filtering

Format is `<title_regex><tab><windowclass_regex>`, one line per entry. Either `<title_regex>` or `<windowclass_regex>` can be omitted, but the tab character must be included if omitting `<title_regex>`.

Example 1, filtering Visual Studio:
```
^.*Microsoft Visual Studio.*$	^HwndWrapper\[.*\]$
```

Example 2, filtering Explorer windows:
```
    	^CabinetWClass$
```
