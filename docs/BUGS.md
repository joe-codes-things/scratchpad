# Bug Log

Track reported bugs and design issues here until they are resolved. Use the
stable identifiers `B001`, `B002`, `B003`, and so on. Never reuse an identifier;
keep existing IDs when updating an entry. Move resolved entries to the
Resolved Bugs section rather than deleting them.

## Open Bugs

### B001 — Windows plugin title appearance

- **Status:** Open
- **Platform:** Windows VST3
- **Issue:** The plugin title at the top left looks less polished and lower
  resolution than the same title in the original macOS version. The other
  Windows plugin text is acceptable and is not part of this report.
- **Ideas to investigate:**
  - Explicitly select the title's typeface and weight instead of relying on
    each platform's default font.
  - Compare the title's font metrics, rendering, and display scaling between
    macOS and Windows in the target DAW hosts.
  - If needed, bundle a suitable redistributable font and use it for the title
    on both platforms to make its appearance more consistent.
  - Check whether title label dimensions or positioning contribute to the
    stretched appearance before changing the font size or layout.

## Resolved Bugs

### B002 — AU editor window resizing in Logic

- **Status:** Resolved
- **Platform:** macOS AU in Logic
- **Issue:** The macOS AU editor window did not resize in Logic 12, although
  VST3 resizing worked in Cubase 15 on macOS and Windows.
- **Resolution:** Enabled the editor's bottom-right resize handle only when
  hosted as AU. Confirmed working in Logic; VST3 behavior remains unchanged.
