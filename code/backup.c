
//SPDX-FileCopyrightText: © 2024 Junsu Lee <junsulee119@gmail.com>
//SPDX-License-Identifier: GNU Affero General Public License v3.0

#include <stdio.h>      // For standard I/O functions
#include <stdlib.h>     // For general purpose functions like exit()
#include <unistd.h>     // For getopt() and other Unix standard functions
#include <string.h>     // For string manipulation functions
#include <sys/stat.h>   // For file and directory metadata functions
#include <sys/types.h>  // For data types used in system calls
#include <dirent.h>     // For directory entry manipulation
#include <time.h>       // For timestamp generation
#include <errno.h>      // For error reporting
#include <limits.h>     // For PATH_MAX definition
#include <pwd.h>        // For getting home directory

#define DEFAULT_TARGET_DIR "/media/pi/piBackup" // Default directory for backups
#define CONFIG_FILE_PATH "%s/.config/backup_tool.conf" // Path for configuration file

// Define ANSI escape codes for colors
#define RESET "\033[0m"
#define WHITE "\033[37m"    // For default system output
#define RED "\033[31m"  // For [ERROR] and [FATAL] and perror
#define YELLOW "\033[33m"   // For [WARNING]
#define GRAY "\033[90m" // For [DEBUG] and [INFO]

// Function prototypes
void create_timestamped_dir(const char *base_path, char *timestamped_dir);  // Create a timestamped directory
void copy_file(const char *src, const char *dest);                          // Copy a single file
void copy_directory(const char *src, const char *dest);                     // Copy a directory recursively
void handle_error(const char *msg);                                         // Handle errors and print messages
void read_default_backup_dir(char *default_target_dir);                     // Read default backup directory from config file
void write_default_backup_dir(const char *new_default_dir);                 // Write new default backup directory to config file
void ensure_config_dir_exists(const char *config_path);                     // Ensure config directory exists
void random_delay();                                                        // To make things look more profesional :)

int main(int argc, char *argv[]) {
    char target_dir[PATH_MAX];  // Writable buffer for target directory
    strcpy(target_dir, DEFAULT_TARGET_DIR); // Initialize with default value

    char source_dir[PATH_MAX];  // Buffer for source directory path
    char backup_dir[PATH_MAX];  // Buffer for backup directory path
    int opt;                    // Variable for getopt options
    int update_default = 0;     // Flag to determine if default directory should be updated

    fprintf(stderr, GRAY "[DEBUG] Starting backup tool.\n" RESET);

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            random_delay();
            fprintf(stderr, GRAY "[DEBUG] -t option provided with argument: %s\n" RESET, optarg);
            if (realpath(optarg, target_dir) == NULL) {
                perror(RED "Invalid target directory" RESET);
                exit(EXIT_FAILURE);
            }
            update_default = 1; // Mark for updating the default target directory
            break;
        default:
            random_delay();
            fprintf(stderr, RED "   [ERROR] Invalid usage.\n" RESET);
            fprintf(stderr, "Usage: %s [-t target_dir] source_dir\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (update_default) {
        random_delay();
        fprintf(stderr, GRAY "[DEBUG] Updating default backup directory to: %s\n" RESET, target_dir);
        write_default_backup_dir(target_dir);
        random_delay();
        printf("Updated default backup directory to: %s\n", target_dir);
        return 0;
    }

    // Read default backup directory
    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Reading default backup directory.\n" RESET);
    read_default_backup_dir(target_dir);

    // Ensure a source directory is provided
    if (optind >= argc) {
        random_delay();
        fprintf(stderr, RED "   [ERROR] Expected source_dir after options.\n" RESET);
        fprintf(stderr, "Usage: %s [-t target_dir] source_dir\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(source_dir, argv[optind]);   // Get source directory from command-line arguments

    // Validate source directory
    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Validating source directory: %s\n" RESET, source_dir);
    struct stat src_stat;
    if (stat(source_dir, &src_stat) != 0 || !S_ISDIR(src_stat.st_mode)) {
        perror(RED "Invalid source directory" RESET);
        exit(EXIT_FAILURE);
    }

    // Create timestamped backup directory
    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Creating timestamped backup directory in: %s\n" RESET, target_dir);
    create_timestamped_dir(target_dir, backup_dir);
    if (mkdir(backup_dir, 0755) != 0) {
        perror(RED "Failed to create backup directory" RESET);
        exit(EXIT_FAILURE);
    }

    random_delay();
    printf("Backing up '%s' to '%s'\n", source_dir, backup_dir);
    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Starting backup process.\n" RESET);

    // Copy the source directory
    copy_directory(source_dir, backup_dir);

    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Backup process completed successfully.\n" RESET);
    random_delay();
    printf("Backup completed successfully!\n");
    return 0;
}


// Function to read default backup directory from config file
void read_default_backup_dir(char *default_target_dir) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), CONFIG_FILE_PATH, homedir);
    
    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Reading config file: %s\n" RESET, config_path);

    FILE *config_file = fopen(config_path, "r");
    if (config_file) {
        if (fgets(default_target_dir, PATH_MAX, config_file) != NULL) {
            // Remove trailing newline
            default_target_dir[strcspn(default_target_dir, "\n")] = 0;
            fclose(config_file);
            random_delay();
            fprintf(stderr, GRAY "[DEBUG] Default target directory read: %s\n" RESET, default_target_dir);
            return;
        }
        fclose(config_file);
    }
    
    // Fallback to original default if config file doesn't exist
    random_delay();
    fprintf(stderr, YELLOW "   [WARNING] Config file not found or empty. Using default target directory.\n" RESET);
    strcpy(default_target_dir, DEFAULT_TARGET_DIR);
}

void ensure_config_dir_exists(const char *config_path) {
    // Extract the directory path from the full config file path
    char dir_path[PATH_MAX];
    strncpy(dir_path, config_path, PATH_MAX);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';  // Terminate string at last slash to get directory path
    }

    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Ensuring config directory exists: %s\n" RESET, dir_path);

    // Create directory if it doesn't exist
    // 0755 provides read, write, execute permissions for owner, 
    // and read and execute for group and others
    if (mkdir(dir_path, 0755) != 0) {
        // If mkdir fails, check if it's because the directory already exists
        if (errno != EEXIST) {
            perror(RED "Failed to create config directory" RESET);
        } else {
            random_delay();
            fprintf(stderr, GRAY "[DEBUG] Config directory already exists.\n" RESET);
        }
    }
}

// Function to write new default backup directory to config file
void write_default_backup_dir(const char *new_default_dir) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), CONFIG_FILE_PATH, homedir);
    
    // Ensure the config directory exists
    ensure_config_dir_exists(config_path);
    
    random_delay();
    fprintf(stderr, GRAY "[DEBUG] Writing new default directory to config file: %s\n" RESET, config_path);

    FILE *config_file = fopen(config_path, "w");
    if (config_file) {
        random_delay();
        fprintf(config_file, "%s", new_default_dir);
        fclose(config_file);
        random_delay();
        fprintf(stderr, GRAY "[DEBUG] Config file updated successfully.\n" RESET);
    } else {
        perror(RED "Failed to update default backup directory" RESET);
    }
}

// Create a timestamped directory name
void create_timestamped_dir(const char *base_path, char *timestamped_dir) {
    time_t now = time(NULL);                      // Get the current time
    struct tm tm_info;                            // Struct for local time information
    char timestamp[30];                           // Ensure buffer is large enough for timestamp and null terminator

    // Use `localtime_r` for thread safety and to ensure the struct is properly filled
    if (localtime_r(&now, &tm_info) == NULL) {
        perror(RED "Failed to get local time" RESET);
        exit(EXIT_FAILURE);
    }

    // Format the timestamp safely
    if (strftime(timestamp, sizeof(timestamp), "Backup %Y-%m-%d %H-%M-%S", &tm_info) == 0) {
        random_delay();
        fprintf(stderr, RED "   [ERROR] Failed to format timestamp.\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Construct the full path with base path and timestamp
    if (snprintf(timestamped_dir, PATH_MAX, "%s/%s", base_path, timestamp) >= PATH_MAX) {
        random_delay();
        fprintf(stderr, "Path is too long\n");
        exit(EXIT_FAILURE);
    }
}

// Copy a single file
void copy_file(const char *src, const char *dest) {
    FILE *src_file = fopen(src, "rb"); // Open the source file in binary mode for reading
    if (!src_file) {
        perror(RED "Failed to open source file" RESET); // Print error if the source file cannot be opened
        random_delay();
        fprintf(stderr, RED "   [ERROR] Could not open source file: %s\n" RESET, src);
        return;
    }
    random_delay();
    fprintf(stderr, GRAY "   [INFO] Opened source file: %s\n" RESET, src);

    FILE *dest_file = fopen(dest, "wb"); // Open the destination file in binary mode for writing
    if (!dest_file) {
        perror(RED "Failed to open destination file" RESET); // Print error if the destination file cannot be opened
        random_delay();
        fprintf(stderr, RED "   [ERROR] Could not open destination file: %s\n" RESET, dest);
        fclose(src_file); // Close the source file to avoid resource leaks
        return;
    }
    random_delay();
    fprintf(stderr, GRAY "   [INFO] Created destination file: %s\n" RESET, dest);

    char buffer[4096]; // Buffer to temporarily store file data
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) { // Read data into the buffer
        if (fwrite(buffer, 1, bytes, dest_file) != bytes) { // Write buffer contents to destination
            perror(RED "Failed to write to destination file" RESET);
            random_delay();
            fprintf(stderr, RED "   [ERROR] Write error occurred while copying file: %s -> %s\n" RESET, src, dest);
            break; // Exit loop on write failure
        }
    }

    fclose(src_file); // Close the source file
    fclose(dest_file); // Close the destination file
    random_delay();
    fprintf(stderr, GRAY "   [INFO] File copy completed: %s -> %s\n" RESET, src, dest);

    // Copy file permissions from source to destination
    struct stat src_stat;
    if (stat(src, &src_stat) == 0) { // Retrieve source file metadata
        if (chmod(dest, src_stat.st_mode) != 0) { // Apply the same permissions to the destination file
            perror(RED "Failed to set file permissions" RESET);
            random_delay();
            fprintf(stderr, YELLOW "   [WARNING] Permissions not set correctly for: %s\n" RESET, dest);
        } else {
            random_delay();
            fprintf(stderr, GRAY "   [INFO] Permissions set successfully for: %s\n" RESET, dest);
        }
    }
}

// Copy a directory recursively
void copy_directory(const char *src, const char *dest) {
    DIR *dir = opendir(src); // Open the source directory for reading
    if (!dir) {
        perror(RED "Failed to open source directory" RESET);
        random_delay();
        fprintf(stderr, RED "   [ERROR] Could not open directory: %s\n" RESET, src);
        return;
    }
    fprintf(stderr, GRAY "   [INFO] Opened source directory: %s\n" RESET, src);

    // Attempt to create the destination directory
    if (mkdir(dest, 0755) != 0 && errno != EEXIST) { // Handle case where directory might already exist
        perror(RED "Failed to create destination directory" RESET);
        random_delay();
        fprintf(stderr, RED "   [ERROR] Could not create destination directory: %s\n" RESET, dest);
        closedir(dir);
        return;
    }
    random_delay();
    fprintf(stderr, GRAY "   [INFO] Destination directory created or already exists: %s\n" RESET, dest);

    struct dirent *entry; // To iterate over directory entries
    char src_path[PATH_MAX]; // Full path for the source file/directory
    char dest_path[PATH_MAX]; // Full path for the destination file/directory

    while ((entry = readdir(dir)) != NULL) { // Read each entry in the directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip current and parent directory entries
        }

        snprintf(src_path, PATH_MAX, "%s/%s", src, entry->d_name); // Construct full source path
        snprintf(dest_path, PATH_MAX, "%s/%s", dest, entry->d_name); // Construct full destination path

        struct stat entry_stat;
        if (stat(src_path, &entry_stat) == 0) { // Retrieve metadata for the source entry
            if (S_ISDIR(entry_stat.st_mode)) { // Check if the entry is a directory
                random_delay();
                fprintf(stderr, GRAY "   [INFO] Found directory: %s\n" RESET, src_path);
                copy_directory(src_path, dest_path); // Recursively copy subdirectory
            } else if (S_ISREG(entry_stat.st_mode)) { // Check if the entry is a regular file
                random_delay();
                fprintf(stderr, GRAY "   [INFO] Found file: %s\n" RESET, src_path);
                copy_file(src_path, dest_path); // Copy the file
            } else {
                random_delay();
                fprintf(stderr, YELLOW "   [WARNING] Skipped unknown entry type: %s\n" RESET, src_path);
            }
        } else {
            perror("Failed to retrieve file metadata");
            random_delay();
            fprintf(stderr, YELLOW "   [WARNING] Could not stat entry: %s\n" RESET, src_path);
        }
    }

    closedir(dir); // Close the directory stream
    random_delay();
    fprintf(stderr, GRAY "   [INFO] Finished processing directory: %s\n" RESET, src);
}

// Handle errors and exit
void handle_error(const char *msg) {
    perror(msg); // Print the error message
    random_delay();
    fprintf(stderr, RED "[FATAL] Program terminating due to error: %s\n" RESET, msg);
    exit(EXIT_FAILURE); // Exit the program
}

void random_delay() {
    // Seed the random number generator with the current time
    srand(time(NULL));

    // Generate a random delay between 50ms (50000us) and 500ms (500000us)
    int delay = (rand() % (500 - 50 + 1) + 50) * 1000; // in microseconds

    // Introduce the delay
    usleep(delay);
}