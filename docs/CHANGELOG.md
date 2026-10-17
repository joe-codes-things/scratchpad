# ScratchPad Plugin Changelog

## v0.0.23 (2026-05-27)
### Final UI Polish
- **Gear button hover**: Removed elliptical glow artifact — gear icon now simply brightens on hover, matching the "Nine Lives Audio" label behavior
- **Caret color**: Text cursor now matches text color (`#e6f1ff`) instead of neon blue
- **Spacing**: Increased gap between top bar and notepad/sidebar from 5px to 9px

## v0.0.22 (2026-05-27)
### UI Polish & Bug Fixes

#### Hover Effects
- **Gear button**: Added neon green hover glow matching the other top bar buttons
- **Font size dropdown**: Added neon green hover glow and brightened background on hover, matching other buttons
- **Font size dropdown**: Fixed hover glow persistence bug — glow now clears immediately when mouse leaves, regardless of which component it moves to next (implemented via `HoverComboBox` subclass with `mouseExit` override)

#### Popup Improvements
- **Escape key**: All three popups (Plugin Info, About, NLA) now close on Escape key
- **ScratchPad Guide**: Clicking "ScratchPad Guide" in the Plugin Info popup no longer dismisses the popup — it stays open (guide functionality to be added later)

#### Dropdown Menu Styling
- **Hover highlight**: Item highlight rectangle now uses `surface3` background with neon blue side borders matching the dropdown border color
- **Text color**: Hovered items stay white — no longer turn neon blue on hover

#### Note Title Editing
- **Escape to revert**: Pressing Escape while editing a note title now correctly reverts to the original name (fixed by adding `suppressFocusLost` flag to prevent `focusLost` from saving on top of the revert)
- **Blank titles**: Blank note titles are now allowed — empty string is a valid name
- **Right-click disabled**: Note title buttons no longer respond to right-click; left mouse button only
- **Click-to-position cursor**: Clicking inside the rename text field positions the cursor at the click location
- **Horizontal scroll**: Text field scrolls horizontally for long note titles

## v0.0.21 (2026-05-27)
### Major UI Redesign
- **New Layout**: Moved note buttons from horizontal grid to vertical sidebar on right side
- **Expanded Window**: Increased default width from 600px to 800px to accommodate new layout
- **Centered Controls**: Control buttons (New Note, Delete Note, Export Note, Font Size) now centered in top bar with increased spacing (10px between buttons)
- **Vertical Scrolling**: Note list now scrolls vertically instead of horizontally
- **Fixed Button Width**: All note buttons in sidebar are uniform width (135px)
- **Improved Spacing**: More breathing room throughout the UI

### Layout Details
- Plugin window: 800x500px (default), resizable from 600x250 to 1400x1000
- Right sidebar: 150px wide for note buttons
- Note buttons: 135px wide x 30px tall, stacked vertically with 5px spacing
- Control buttons: Centered with 10px spacing between each button
- Notepad area: Takes remaining space on the left side

## v0.0.20 (2026-05-27)
### Visual Improvements
- Removed blue text color for selected notes - all note text stays white
- Selected notes now only show neon blue glow border (no text color change)
- Hover state keeps text white (only adds lime green glow)

### Version System Overhaul
- Changed version numbering from 2.7.x to 0.0.x format
- Renamed all old builds in `builds/Old/` folder to new version format (v0.0.1 through v0.0.19)
- Updated CMakeLists.txt to version 0.0.20
- Cleaned up old version folders from main builds directory

### Bug Fixes
- Fixed button text display - now shows "New Note", "Delete Note", "Export Note" instead of symbols
- Added explicit `setButtonText()` calls to ensure button labels are applied correctly

## v0.0.19 (Previous Session)
### UI Improvements
- Changed "Style Guide" menu item to "ScratchPad Guide" (placeholder, no functionality)
- Increased edge padding from 5px to 10px for better visual breathing room
- Improved popup dismissal - clicking anywhere in plugin window now closes popups
- Fixed popup dismissal in text editor area using proper coordinate translation

### Button Styling
- Changed export button border color from neon purple to neon blue (matching note title text)
- Updated button text from symbols to descriptive labels:
  - `+` → "New Note"
  - `-` → "Delete Note"
  - `↑` → "Export Note"
- Adjusted button widths to accommodate text labels (80-85px each)
- Reduced control button font size from 16pt to 12pt

### Crash Fixes
- Fixed segmentation fault crashes in Logic Pro (AU) and Cubase (VST3)
- Added null pointer checks for `gearButton_` and `nlaLabel_` in `resized()` method
- Root cause: Components were being dereferenced before initialization
- Added missing callback declarations for `TabRenameEditor` class

## Design Philosophy
The plugin follows the NLA style guide with:
- Dark layered surfaces (Deep Background: #0A0E14)
- Neon accent colors (Blue: #00E6FF, Cyan: #00FFC6, Green: #39FF14, Purple: #B300FF, Magenta: #FF00F0)
- Soft glowing effects for active states
- White text (#E6F1FF) that doesn't change color on hover or selection
- Minimal, modern interface with professional audio software aesthetics
