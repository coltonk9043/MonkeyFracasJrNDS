# Monkey Fracas Jr. for the Nintendo DS
An NDS port of the game Monkey Fracas Jr from Futurama.

## Prequisites
This is not a standalone application and requires libnds in order to work. 
If you plan to compile this, you must ensure that either libnds or devkitPro is installed on your machine. 
A download link for devkitPro can be found at: https://github.com/devkitPro/installer/releases

Compiling this code will generate a .nds file, which can be emulated or loaded onto a homebrewed device.

## Setup
1. Copy this folder (or symbolic link) to the path of devkitPro under 'C:/devkitPro/MonkeyFracasJrNDS'. This ensures that the library can be found when compiling / modifying code.
2. Go into the folder 'MonkeyFracasJrNDS', change line 'Project Directory' in the .prj file to the root directory of the project.
3. Run 'make'. Assuming that the library is installed and working, the project should compile.
4. After the 'make' command is finished, you should have a '.nds' file in the root directory of the project.
5. Open the '.nds' file in an emulator or a homebrewed NDS device. You should be good to go! :)
