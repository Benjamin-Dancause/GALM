# GALM

**GALM** stands for **GNOME App Launcher Maker**. It is a small Qt 6 desktop app for creating GNOME application launchers (`.desktop` files). Enter a name, choose an executable and an icon, and save a launcher for your account or for all users.

## Requirements

- Linux with a graphical desktop session
- A C++23-capable compiler
- CMake 3.16 or newer
- Qt 6 development libraries: Core, Gui, and Widgets
- A build tool supported by CMake, such as Make or Ninja

Qt 6 runtime libraries must also be available on the machine running the app.

## Build and run

From the project directory:

```sh
cmake -S . -B build
cmake --build build --parallel
./build/GALM
```

The launcher template is embedded in the executable during the build. You can launch the app from any working directory without copying `Template.desktop` alongside it.

## Usage

1. Enter an **App name**. This becomes both the displayed name and the `.desktop` filename.
2. Use the **Exec Path** browse button to select the program to launch.
3. Use the **Icon Path** browse button to select its icon file.
4. Optionally select **Make systemwide launcher**.
5. Click **Create app launcher**.

All three fields are required. Names cannot be blank or contain `/`, line breaks, or null characters. The app closes after a successful save. If saving fails, it displays an error and stays open.

### Save locations

| Scope | Destination |
| --- | --- |
| Current user (default) | `~/.local/share/applications/<App name>.desktop` |
| Systemwide | `/usr/share/applications/<App name>.desktop` |

The app creates the destination directory if needed. Systemwide creation requires write permission to `/usr/share/applications`; the checkbox does not request elevated privileges.

Creating a launcher with the same name replaces the existing file at that location without a confirmation prompt. Files are saved atomically, so a failed write does not replace an existing launcher with incomplete content.

## Generated launcher

For example, a launcher might contain:

```ini
[Desktop Entry]
Name=My App
Exec=/home/user/apps/my-app
Icon=/home/user/Pictures/my-app.png
Terminal=false
Type=Application
```

The executable and icon are referenced at their selected paths; they are not copied. Keep those files in place after creating the launcher.

## Current limitations

- The file picker checks that the selected program is a regular file, but does not check or grant executable permission.
- Executable paths are inserted without quoting or escaping. Paths containing spaces or special characters may need manual editing in the generated `.desktop` file.
- The interface does not offer command-line arguments or a terminal option. The template uses `Terminal=false`.

## Troubleshooting

- **Create button disabled:** Enter a valid name and select both an executable and an icon.
- **Could not create or save launcher:** Check write permissions for the destination and available disk space. For a personal launcher, leave the systemwide option unchecked.
- **Launcher does not start:** Check that the executable still exists, has execute permission, and that its `Exec` entry is correctly quoted if the path contains spaces or special characters.
- **Empty files from an older build:** Rebuild and run the updated executable, then recreate the affected launchers. Older builds depended on finding `Template.desktop` in the current working directory.

## Project files

- `main.cpp`: Qt interface, input validation, and launcher saving.
- `Template.desktop`: Launcher template. Rebuild after changing it.
- `resources.qrc`: Embeds the template as a Qt resource.
- `CMakeLists.txt`: Build configuration.
