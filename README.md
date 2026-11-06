# Custom Shop Maker — source update

**Target:** Geometry Dash 2.2081, Geode 5.11.2, Android 64-bit (and Windows metadata target).

This is source code for you to build with the matching Geode SDK. It is not a compiled `.geode` file, and it has not been compiled against your exact Android game binary in this environment.

## Implemented in this update

- Main-menu entry opens Custom Shop Maker.
- Create and persist multiple independent shops.
- Shop design editor: shop name, keeper display name, greeting and click dialogue.
- Eight selectable background palette indices and eight text palette indices, saved with the shop.
- Three independently cycling keeper-part preset indices (head, mouth, body), saved with the shop.
- Add, edit and delete merchandise.
- Merchandise fields: title, price in orbs, stock (`-1` means unlimited), icon type, icon ID, reward kind (`icon`, `more-icons`, or `custom`), and repeatable flag.
- Shop preview/visit screen with buy buttons.
- Orb balance validation and orb deduction for supported purchases.
- Finite stock decrement, unlimited stock, repeat-purchase rules, and saved ownership IDs.
- Vanilla Geometry Dash icon rewards call the game's icon unlock method after purchase.
- Shop designs, item data, stock, and custom ownership persist using Geode saved values.

## Not yet complete / integration blockers

These parts are intentionally not faked:

1. **More Icons unlock integration:** The `more-icons` reward kind is editable and recognized, but purchases are blocked until a real adapter for the installed More Icons mod is connected. Its icon registry/ownership API is not interchangeable with vanilla `GameManager::unlockIcon`; passing those IDs to the vanilla API could unlock the wrong thing or corrupt the intended mapping.
2. **Scratch's shop-room injection:** The current entry point is the main menu. The custom shops can be opened and used from there, but this source does not yet add merchant doors/NPCs inside Scratch's shop room. That requires a verified hook for the exact Scratch shop layer in the 2.2081 bindings.
3. **Keeper part rendering:** The head/mouth/body preset indices are stored and editable, but the source does not yet compose the selected parts into a rendered shopkeeper sprite. The correct sprite-frame names/atlas mapping must be confirmed from the game assets.
4. **Visual palette fidelity:** The shop preview uses an eight-color background palette and applies an eight-color text palette to its shop labels. This is a simple palette implementation, not an exact recreation of Scratch's original shop UI styling.
5. **Build/runtime verification:** The source has not been compiled or run against your local Geode SDK/game installation. API signature mismatches may need fixing during the first build.

## Build on Android

Use a Geode SDK/build environment that supports Android arm64 and the exact GD 2.2081 binary. From the project directory:

```sh
geode sdk install
geode sdk install-binaries
geode build
```

If the CLI/environment you use cannot build Android arm64 mods, it will not produce a working tablet `.geode` just because the source targets Android. Do not install a Windows or x86 build on the tablet.

## Notes

- `icon` items are treated as vanilla GD icons.
- `more-icons` items are deliberately blocked from purchase until the external adapter is implemented.
- `custom` items currently act as persistent shop-owned rewards/collectibles; they do not grant a separate gameplay ability by themselves.
- The shop manager is accessed from the new main-menu button. This is not yet the Scratch-room merchant replacement/injection.
