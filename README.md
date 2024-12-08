# Backup.c

## Overview

**Backup.c** is a robust and user-friendly utility designed to back up files and directories to an external storage device, such as a USB drive. With a focus on simplicity and functionality, this tool provides a seamless way to secure your data on Linux-based systems.

- **Core Feature**: Backup to external storage devices (e.g., USB drives).
- **Development Duration**: November 12 - November 24, 2024.
- **License**: [GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.html).
- **GitHub Repository**: [Backup.c](https://github.com/junsulee119/backup.c)

## Features

### 1. Backup Functionality
- Backs up a source directory to a target location.
- Automatically creates a timestamped folder (e.g., `Backup YYYY-MM-DD HH-MM-SS`) in the target directory for better organization.
- Recursively copies files and directories while maintaining permissions and structure.

### 2. Custom Target Directory
- Allows specifying a custom target directory using the `-t` command-line option:
  ```bash
  ./backup -t /custom/target /path/to/source
  ```
- Saves the specified directory as the new default for future backups.

### 3. Enhanced Aesthetics
- Includes visual and functional details, such as random delays and color-coded debug logs, to enhance the user experience.
- Outputs informative messages and warnings for better transparency.

## Installation

### Prerequisites
- A Linux-based operating system (e.g., Raspberry Pi OS or other Unix-like systems).
- A C compiler (e.g., `gcc`).

### Compilation
Compile the source code with the following command:
```bash
gcc -o backup backup.c
```

### Usage
Run the program as follows:
```bash
./backup [OPTIONS] SOURCE_DIR
```

#### Options
- `-t TARGET_DIR`: Specifies a custom target directory for backups. Saves this directory as the new default.

#### Example
Backup the `/home/user/Documents` directory to the default target:
```bash
./backup /home/user/Documents
```
Specify a custom target directory:
```bash
./backup -t /mnt/usb /home/user/Documents
```

## Design Highlights

### Default Target Directory
The program reads the default backup directory from a configuration file located in the user's home directory (e.g., `~/.config/backup_tool.conf`). If no configuration exists, the default directory is `/media/pi/piBackup`.

### Timestamped Directories
Each backup is stored in a uniquely named folder based on the current date and time, ensuring backups remain organized and traceable.

### Error Handling
The program validates directories, handles unexpected conditions, and provides detailed error messages for troubleshooting.

## Future Improvements
- Extend compatibility to other Linux distributions by dynamically detecting the system's default external storage path.
- Install the program globally with root privileges for execution without `./` and from any directory.
- Add features like a progress bar or dramatic warning displays for enhanced interactivity.

## Why It Was Built
- **60%**: Because it seemed cool.
- **40%**: For project submission.
- **15%**: To manage GitHub contributions.
- **5%**: Out of necessity.

## Acknowledgments
- Developed by **Junsu Lee**.
- Contact: [junsulee119@gmail.com](mailto:junsulee119@gmail.com)

Thank you for using **Backup.c**!