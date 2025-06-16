# GBARunner3

<div align="center">
    <img src="docs/images/GBARunner3_Logo.png" width="1000"/>
</div>    

![Build](https://img.shields.io/badge/build-passing-brightgreen)
![License](https://img.shields.io/github/license/gericom/gbarunner3)
![Platform](https://img.shields.io/badge/platform-Nintendo_DS-blue)

**GBARunner3** is the successor to [GBARunner2](https://github.com/Gericom/GBARunner2), designed as a hypervisor to run Game Boy Advance games on the **Nintendo DS in DS mode**, and on the **DSi/3DS in DSi mode**.

> **Note:** GBARunner3 is still in current development, experimental state, use at your own risk.

While GBARunner2 started as a proof of concept and evolved into a large-scale project, it eventually suffered from maintainability and architectural limitations.
GBARunner3 addresses these problems with a cleaner, more maintainable, and better-documented codebase, along with modern features and broader hardware support.

> **Note:** 3DS users should instead consider [open_agb_firm](https://github.com/profi200/open_agb_firm) to play GBA games.

## Table of Contents
- [Project Highlights](#project-highlights)
- [How to Compile](#how-to-compile)
- [Setup Instructions](#setup-instructions)
- [Configuration](#configuration-via-gbarunner3json)
- [GBA Color Correction](#true-gba-color-correction)
- [Troubleshooting](#troubleshooting)
- [Known Issues](#known-issues)
- [FAQ](#faq)
- [Contributing](#contributing)
- [Compatibility](#compatibility-list)
- [Credits](#credits)


## Project Highlights

### Improvements Over GBARunner2

- Native SD card support for DSi and 3DS.
- Clean, maintainable, and well-documented codebase.
- Compatible with modern devkitARM versions (pre-Calico).
- Improved sound emulation on DS and DSi.
- Support for high ROM address execution ("hi-code") beyond the first 2 MB.
- Real time and manual JIT patching for enhanced compatibility.
- Option to disable the unused screen to reduce power consumption.
- More control over the running game.
- True GBA color correction (multiple selectable profiles).

### Planned / Requested Features

- BlocksDS support.
- In-game Configuration menu.
- GBA Wireless Adapter emulation (RFU).
- Real-Time Clock (RTC).
- Solar Sensor emulation (GPIO).
- Rumble emulation (GPIO).
- Save states and cheat support.
- Sleep mode (via lid close or software).
- Automatic IPS patching.
- Custom button remapping.

## How to Compile

1. Install **DevKitPro**. Make sure to use a version prior to 2.0 (Calico).
2. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/Gericom/GBARunner3.git
   ```
3. Build from the root folder:
   ```bash
   make
   ```
   The output will be located inside the `/out`directory.

## Setup Instructions

### TwilightMenu++ Users

**Requirements:**
- [TwilightMenu++ v27.5.0+](https://github.com/DS-Homebrew/TWiLightMenu/releases)
- A valid GBA BIOS (see [this guide](https://wiki.ds-homebrew.com/gbarunner2/bios.html))

**Steps:**
1. Place your GBARunner3.nds in `/_nds/TWiLightMenu/emulators/`.
2. Place your GBA BIOS in `_gba/` and rename it to `bios.bin`
3. Place the config folder in `_gba/configs/`
4. Launch TwilightMenu++ and navigate to your GBA ROMs.
5. Launch a ROM. You should see the GBARunner3 splash screen, followed by the GBA BIOS animation and the Nintendo logo.

### Flashcard Kernel Users (via gbar3-frontend)

**Requirements:**
- [gbar3-frontend.nds](https://github.com/flashcarts/gbar3-frontend/releases)
- A valid GBA BIOS (see [this guide](https://wiki.ds-homebrew.com/gbarunner2/bios.html))

**Steps:**
1. Copy the contents of the `out/` folder to the root of your SD card.
2. Place your GBA BIOS in `_gba/` and rename it to `bios.bin`
3. Place `gbar3-frontend.nds` anywhere on your SD card.
4. Launch `gbar3-frontend.nds`, navigate to your GBA ROMs location and launch them.
5. You should see the GBARunner3 splash animation, followed by the GBA BIOS animation with the Nintendo logo.

> Bonus feature: `gbar3-frontend` will automatically boot to `roms/gba` if the folder exists.

## Configuration via `GBARunner3.json`

This config file must be stored in `/_gba/GBARunner3.json`, the default one is already included inside the `/out` folder after building GBARunner3.    
Some games require specific configurations, refer to the [Compatibility List](#compatibility-list) for details.    
For more on `gbaColorCorrection` and `gbaDisplayGamma`, see the [GBA Color Correction](#true-gba-color-correction) section.

```json
{
    "runSettings": {
        "enableJit": true,
        "enableRomICache": true,
        "enableWramICache": true,
        "enableIWramDCache": false,
        "enableEwramDCache": true,
        "forceDSModeArm9Clock": false,
        "skipBiosIntro": false
    },
    "displaySettings": {
        "gbaScreen": "top",
        "gbaColorCorrection": "Oxy001",
        "gbaDisplayGamma": 0,
        "gbaScreenBrightness": 16,
        "enableCenterAndMask": true,
        "centerOffsetX": 8,
        "centerOffsetY": 16,
        "maskWidth": 240,
        "maskHeight": 160,
        "borderImage": "default"
    },
    "gameSettings": {
        "saveType": "Auto"
    }
}
```

#### runSettings - Settings Related to GBARunner3’s Emulation Performance
- `enableJit`:
    - `[Boolean]` Enables the real-time JIT. Should be enabled by default. Some games only work when this is disabled.
- `enableRomICache`:
    - `[Boolean]` Enables ROM instruction cache. Should be enabled by default. Some games only work when this is disabled.
- `enableWramICache`:
    - `[Boolean]` Enables WRAM instruction cache. Disable if the ROM uses self-modifying code.
- `enableIWramDCache`:
    - `[Boolean]` Enables IWRAM data cache. Improves performance in titles using bitmap rendering (e.g., 3D graphics), but may break others. Should be disabled by default.
- `enableEwramDCache`:
    - `[Boolean]` Enables EWRAM data cache. Should be enabled by default. Fixes sound issues in certain games.
- `forceDSModeArm9Clock`:
    - `[Boolean]` Forces the ARM9 CPU to run at 67 MHz in DSi mode. Fixes timing issues in some games.
- `skipBiosIntro`:
    - `[Boolean]` Skips the GBA boot animation.

#### displaySettings - Settings Related to GBARunner3's Display Output
- `gbaScreen`: Specifies which DS screen displays the GBA output.
    - `"top"`
    - `"bottom"`
- `gbaColorCorrection`: Specifies the color correction profile to simulate various GBA screens.
    - `"none"`: No color correction.
    - `"Agb001"`: Original GBA (AGB-001).
    - `"Ags101"`: GBA SP Bright (AGS-101).
    - `"Oxy001"`: Game Boy Micro (OXY-001).
    - `"Ntr001"`: NDS Phat (NTR-001).
    - `"Usg001"`: NDS Lite (USG-001).
    - `"Psp01g"`: PSP 1000 (PSP-01g).
    - `"NswIps"`: Nintendo Switch Classics GBA shader (IPS model).
    - `"NswOle"`: Nintendo Switch Classics GBA shader (OLED model).
    - `"VbaEmu"`: VisualBoy Advance Emulator shader.
    - `"NoCash"`: No$GBA Emulator shader.
    - `"mGba01"`: mGBA Emulator shader.
- `gbaDisplayGamma`:
    - `0 ~ 4`: Applies a gamma correction level (only if color correction is enabled). 0 is brightest, 4 is darkest.
- `gbaScreenBrightness`: 
    - `1 ~ 16`: Master brightness level for the display. 1 is darkest, 16 is brightest.
- `enableCenterAndMask`: 
    - `[Boolean]` Hides DS overscan by centering and masking the image (adds 1 frame of latency).
- `centerOffsetX`: 
    - `0 ~ 8`: Horizontal centering offset (requires `enableCenterAndMask`).
- `centerOffsetY`: 
    - `0 ~ 16`: Vertical centering offset (requires `enableCenterAndMask`).
- `maskWidth`: 
    - `0 ~ 256`: Horizontal masking width. `240` matches GBA width (requires `enableCenterAndMask`).
- `maskHeight`: 
    - `0 ~ 192`: Vertical masking height. `160` matches GBA height (requires `enableCenterAndMask`).
- `borderImage`: Custom border image (requires `enableCenterAndMask`).
    - `"default"`: `256x192 8bpp BMP` named `border.bmp`, inside `_gba/`. 
    - `"game"`: BMP named after the game’s Title ID (e.g. `BPEE.bmp`) inside `_gba/borders/`.
    - `"none"`: Disables the border.

#### gameSettings - Game related settings
- `saveType`: Specifies if game saving is performed. Fixes AP issues in certain titles.
    - `"Auto"`: Default, save type gets auto-detected.
    - `"none"`: Forces save handling off.

### Per-game Settings
You can override global settings with per-game configurations:
- Create a JSON file in the `_gba/configs/` directory.
- Name it using the game’s **Title ID (TID)** and **Revision (REV)** in the format:
```bash
TIDREV.json
```
- `TID`: 4-character internal game code (offset `0xAC` in the ROM header).
- `REV`: 1-byte revision number (offset `0xBC`).

Example:
If TID = `BPEE` and REV = `01`, name the file:
```bash
BPEE01.json
```
Unset per-game config single options will fallback to general options found in `GBARunner3.json`.    
If no per-game config is found, GBARunner3 will fall back to the global `GBARunner3.json`.

<!-- TODO: Link to guides in separate readme files inside the configs folder
## Manual JIT patches
## Self-modifying code patches    -->

## True GBA Color Correction    

<div align="center">
    <img src="docs/images/GBARunner3_GBA_Models.jpg" width="300"/>
</div>    

The original GBA (AGB-001) uses a non-backlit, reflective LCD display. It not only requires external light to be visible, but also features an unconventional RGB subpixel layout:      

<div align="center">
    <img src="docs/images/GBARunner3_GBA_Pixels.jpg" width="250"/>
</div>    

As a result, many early GBA games were designed with oversaturated and overly bright palettes, often deviating from true color accuracy to remain visible. These look normal on the AGB-001, but on later models like the GBA SP (AGS-101) and Nintendo DS Lite (USG-001), which use standard backlit RGB panels, these appear washed out or color shifted.

Interestingly, models like the DS Phat (NTR-001) and the Game Boy Micro (OXY-001), which, despite having backlit screens, still use reflective LCDs with a pixel arrangement closer to that of the AGB-001, preserving the original look more accurately.      

<div align="center">
    <img src="docs/images/GBARunner3_ColorCorrection_1.webp" width="400"/>
</div>    

Emulators commonly apply shaders to simulate the AGB-001’s look on modern displays, recreating how games were originally meant to be seen on a real GBA display. But since the Nintendo DS lacks shader support, GBARunner3 takes a different approach: **it intercepts palette writes at runtime and uses a precomputed LUT to apply color correction.**

The LUT is generated at boot based on selected color profile matrices, derived from libretro shaders by **hunterk and Pokefan531**. Below are direct feed screenshots from GBARunner3, showcasing the available color correction profiles.  

<p align="center">
    <img src="docs/images/GBARunner3_ColorLut_1.webp" width="450">
    <img src="docs/images/GBARunner3_ColorLut_2.webp" width="450">
</p>    

<p align="center">
    <img src="docs/images/GBARunner3_ColorLut_3.webp" width="450">
    <img src="docs/images/GBARunner3_ColorLut_4.webp" width="450">
</p>    

#### Gamma Correction Levels    

Gamma correction further improves visuals by deepening contrast and color intensity. This is especially useful for games with washed-out palettes. The image below shows each available gamma level in GBARunner3:  

<div align="center">
    <img src="docs/images/GBARunner3_GammaLut_1.webp" width="900"/>
</div>   

## Troubleshooting

### Hicode & ROM Compatibility
To maximize performance, GBARunner3 loads the ROM into DS RAM. However, only the first 2MB is loaded linearly at startup, due to RAM limitations.
If a game uses additional **executable code beyond 2MB** (known as *hicode*), it will cause GBARunner3 to halt. Such games are currently incompatible unless using the [cache-hicode branch](https://github.com/Gericom/GBARunner3/tree/feature/cache-hicode).

Check the [Compatibility List](#compatibility-list) for known cases.

### BIOS Checksums

Ensure your BIOS is valid:
- CRC32: `81977335`
- MD5: `a860e8c0b6d573d191e4ec7db1b1e4f6`
- SHA1: `300c20df6731a33952ded8c436f7f186d25d3492`
- SHA256: `fd2547724b505f487e6dcb29ec2ecff3af35a841a77ab2e85fd87350abd36570`

### SD Card Fragmentation & Quality  

Performance issues (e.g., audio crackling, slowdowns) are often due to poor SD card I/O speed. Causes include:

- Cheap or unreliable SD cards.
- Fragmentation.
- Poor DLDI support in some flashcards (e.g., EZFlash Parallel).

**Recommendations**:

- Use reputable SD card brands.
- Reformat the card properly using [this guide.](https://dsi.cfw.guide/sd-card-setup.html)
- Consider upgrading to a better flashcart, see recommendations [here.](https://www.flashcarts.net/ds-quick-start-guide)

## Known Issues

- Some games may have flickering graphics, due to IRQ latency.
- Sound stereo desync/crackling randomly. A temporary fix in some games is to perform an ingame save.
- Some games require manual JIT patches to work (guide coming soon).
- NES Classics and Famicom Mini ROMs are not supported.
- Some GBA Video titles don't work yet.
- 64MB ROMs (e.g. GBA movies) are currently unsupported.

## FAQ

**Q: My ROM doesn’t run. Why?**  
A: It likely uses hicode. Try the [cache-hicode branch](https://github.com/Gericom/GBARunner3/tree/feature/cache-hicode).

**Q: What's "hicode"**  
A: See the [Troubleshooting](#troubleshooting) section.

**Q: I still get a white screen using the hicode-cache branch!**  
A: The game may need JIT or self-modifying code patches (Guides coming soon).

**Q: Can I make such patches?**  
A: Yes, if you’re familiar with no$GBA or ARM7 debugging tools. Ask for help in the Discord server.

**Q: Does [romhack name] work?**  
A: Test it yourself! Support focuses on official GBA releases.

**Q: Can I still use GBARunner2?**  
A: Yes, GBARunner2 and GBARunner3 can coexist.

**Q: Do I need to uninstall GBARunner2 after installing 3?**  
A: No.

<!-- **Q: Does Pokémon Unbound work?**  
A: Pokémon Unbound works! [Follow this guide](https://discord.com/channels/1289261839804272712/1289261840613900370/1298412255469244416) then set the in-game audio to "medium". -->

## Contributing

- Report issues in the [GitHub issue tracker](https://github.com/Gericom/GBARunner3/issues).
- You are welcome to submit your PRs to the repository.
- Help optimize or document the code.
- Share your suggestions or ideas.

## Compatibility List

To contribute testing results, use the official [Compatibility Sheet](https://docs.google.com/spreadsheets/d/1PTf9kW7L3MTIUU5WXvOnvSLTmgNG4K-CzDNe2U8Rd6Y/edit?usp=sharing). Request access from "Kaisaan" using your Google Account.

<!-- TODO: Add a license
## License
GBARunner3 is licensed under the [zLib license](LICENSE). -->

## Credits

- **Gericom** – Lead developer.
- **profi200** – DSi SD driver, color LUT base code.
- **Dartz150** – Logo/splash art design, manual JIT patches and thorough testing.
- **VeaNika** – Thorough DS mode testing and manual JIT patches, real-time color correction and profiles code, documentation.
- **hunterk and Pokefan531** - Libretro color correction shaders authors.
- **endrift** - mGBA lead developer.
- [DSi mode Hacking! Discord server](https://discord.gg/fCzqcWteC4) users who keep testing GBARunner3!
- ...all contributors of the DS homebrew community!

---
Copyright (C) 2025 Gericom
