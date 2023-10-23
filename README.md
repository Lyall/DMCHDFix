# Devil May Cry HD Collection Fix
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/W7W01UAI9)</br>
[![Github All Releases](https://img.shields.io/github/downloads/Lyall/DMCHDFix/total.svg)](https://github.com/Lyall/DMCHDFix/releases)

This is a work-in-progress fix that aims to add ultrawide support to Devil May Cry HD Collection.<br />
As of (21/10/23) it is still in testing and a public release build should be up in the next few days.

## Features
- Unlocks 1080p resolution limit in the launcher.
- Corrects gameplay aspect ratio.
- Scales HUD to 16:9.
- Correctly scales FMVs to 16:9.

## Installation
- Grab the latest release of DMCHDFix from [here.](https://github.com/Lyall/DMCHDFix/releases)
- Extract the contents of the release zip in to the the Win64 folder.<br />(e.g. "**steamapps\common\Devil May Cry HD Collection**" for Steam).

## Configuration
- See **DMCHDFix.ini** to adjust settings for the fix.

## Known Issues
Please report any issues you see.
This list will likely contain minor bugs which may or may not be fixed.

### Launcher
- Some text is not scaled correctly.
- HUD scale does not update with resolution changes.
- Attract FMV is not scaled correctly. (Thanks Mechanical Paladin!)

### DMC 1
- Door opening zoom effect looks janky.
- Cutscene letterbox does not span screen.
- Cutscenes with certain screen effects are scaled incorrectly. (Thanks Mechanical Paladin!)
- Death screen overlay does not span screen. (Thanks Mechanical Paladin!)
- Aspect ratio is incorrect underwater. (Thanks Mechanical Paladin!)

### DMC 2
- Targeting reticle is squished (wrong aspect ratio).

### DMC 3
- Targeting reticle is misaligned.
- Combo meter is misaligned.
- Main menu is not scaled correctly on first boot.

## Screenshots

| ![ezgif-4-0e4ab45437](https://github.com/Lyall/DMCHDFix/assets/695941/ab35cd0e-3db6-409c-8110-ec28bf3dbbf8) |
|:--:|
| DMC HD Collection Launcher |

| ![ezgif-4-6dc922d9c9](https://github.com/Lyall/DMCHDFix/assets/695941/d5f68896-c6d1-4905-9f95-819bd8224500) |
|:--:|
| DMC 1 |

| ![ezgif-4-40643537c3](https://github.com/Lyall/DMCHDFix/assets/695941/466b2005-5967-4fc9-88e9-852e316223bf) |
|:--:|
| DMC 2 |

| ![ezgif-4-72aa490107](https://github.com/Lyall/DMCHDFix/assets/695941/49c1e545-ccfb-4f27-95c7-801a0739040f) |
|:--:|
| DMC 3 |

## Credits
[Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading. <br />
[inipp](https://github.com/mcmtroffaes/inipp) for ini reading. <br />
[Loguru](https://github.com/emilk/loguru) for logging. <br />
[length-disassembler](https://github.com/Nomade040/length-disassembler) for length disassembly.
