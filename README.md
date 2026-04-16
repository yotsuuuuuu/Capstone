# Audioscape
[TODO: not Done]

Audioscape is an interactive 3D technical demo that bridges the gap between audio and visual experience. Players are able to load any audio track of their choosing by adding it to the audio folder, freely explore the generated terrain on foot, and personalise the visual appearance of the world through a built in colour editor.

The terrain is formed from the song's frequency data at the point of generation, producing a static landscape that is a physical representation of the song's sonic characteristics. Meanwhile, the lighting system reacts continuously to the beat, with lights emitting bloom that pulses in real time to the rhythm, bringing the environment to life around the player.

Audioscape is less a traditional game and more an audio-visual experience. The goal is to make the player feel as though they are not just listening to music, but physically walking through it.

---

## Getting Started

There are two ways to get Audioscape running: downloading the installer via itch.io or cloning the repository locally.

---

### Option 1: itch.io / Installer

1. Head to the itch.io page at [Audioscape](https://gu88.itch.io/audioscape)
2. Download the installer
3. Run the installer and follow the on screen instructions
4. Launch Audioscape

---

### Option 2: Clone from GitHub

#### Prerequisites

Before cloning, make sure the following are installed:

| Dependency | Details |
|---|---|
| **Visual Studio 2022** | Windows only. Tested on Visual Studio 2022, should be compatible with later versions. |
| **Vulkan SDK 1.3.296.0** | This is the specific version the project was built and tested against. It is the last version to support both 32-bit and 64-bit builds. Download it here: [LunarG Vulkan SDK](https://vulkan.lunarg.com/) |
| **FMOD Studio API 2.03.09** | The project is configured to use FMOD's default install path. If FMOD is installed to a custom location, the include, lib and environment paths will need to be updated manually in Visual Studio under **Project Properties**. Download here: [fmod.com](https://www.fmod.com) |
| **Mesh OBJ Files** | The mesh assets are not included in the repository. Download the zip here: **[LINK TO MESH OBJ ZIP]** and extract the contents to `\ComponentFramework\meshes` |
| **Audio Files** | Audioscaper ships with one free licensed track to get you started. To add your own music, place tracks in the `audio` folder in the project directory. Supported formats: **MP3, FLAC, WAV** |

All other dependencies are included in the repository.

---

#### Build Instructions

1. Clone the repository
2. Open the Visual Studio solution file (`.sln`) in the root of the repository
3. Set the environment paths under **Project Properties → Debugging → Environment** for your target platform:

   **32-bit (x86):**
   ```
   PATH=%PATH%;$(ProjectDir)third_party\OpenGL\Lib32;$(ProjectDir)third_party\SDL3\Lib32;C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\core\lib\x86;C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\studio\lib\x86;$(ProjectDir)third_party\FFTW\x86
   ```

   **64-bit (x64):**
   ```
   PATH=%PATH%;$(ProjectDir)third_party\OpenGL\Lib64;$(ProjectDir)third_party\SDL3\Lib64;C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\core\lib\x64;C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\studio\lib\x64;$(ProjectDir)third_party\FFTW\x64
   ```

   > [!NOTE]
   > Paths prefixed with `$(ProjectDir)` are relative to the repository root and do not need to be changed. Only the FMOD paths need to be updated if FMOD was installed to a custom location.

4. Set the build configuration to **[DEBUG/RELEASE]** and platform to **[x64/x86]**
5. Build the solution
6. Run the project

---

## Controls

| Key / Input | Action |
|---|---|
| W A S D | Move |
| Mouse Movement | Look / Aim |
| Space | Jump |
| Left Shift | Run |
| TAB | Open / Close Main Menu |
| ESC | Quit Application |

---

## Credits

Andres Herrera

Kevin Valencia

Vicent Addante

**Humber Polytechnic** — **2026**
