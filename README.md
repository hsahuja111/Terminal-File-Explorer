# Terminal Based File Explorer

This repository contains code for file explorer which is written in C language.

A file explorer, also known as a file manager, is a software application or tool that allows users to navigate and manage files and directories on a computer system.
Here, it  provides a graphical user interface (GUI) for navigating through the files and  command-line interface (CLI) for interacting with the file system and perform various operations.

## How to run
Open a terminal of the Operating System.

**Compile code** : ```g++ main.cpp```

**Run Code** : ```./a.out```

## Working and Commands
Our File Explorer should work in two modes :-

1. **Normal mode (default mode**) - used to explore the current directory and navigate the filesystem
2. **Command mode** - used to enter shell commands

This application displays data starting from the top-left corner of the terminal window, line-by- line. Text rendering is handled vertically if the terminal window is resized. 
The last line of the display screen is used as a status bar, and it prints the current mode on the terminal window.

### Normal mode:
Normal mode is the default mode of out application. It has the following functionalities -

1. Display a list of directories and files in the current folder

   a. Every file in the directory should be displayed on a new line with the following attributes for each file -
   i. File Name
   ii. File Size
   iii. Ownership (user and group) and Permissions    
   iv. Last modified

   b. The file explorer shows entries “.” and “..” for current and parent directory respectively
   
   c. The file explorer handles scrolling using the up and down arrow keys.

   d. User is able to navigate up and down in the file list using the corresponding up and down arrow keys. The up and down arrow keys should 
      also handle scrolling during vertical overflow.


2. Open directories and files When enter key is pressed -
   
   a. Directory - Clear the screen and navigate into the directory and show the directory contents as specified in point 1
   
   b. File - Open the file in vi editor

4. Traversal

   a. Go back - Left arrow key should take the user to the previously visited directory
   
   b. Go forward - Right arrow key should take the user to the next directory
   
   c. Up one level - Backspace key should take the user up one level
   
   d. Home – h key should take the user to the home folder



### Command Mode:
The application should enter the Command button whenever “:” (colon) key is pressed. In the command mode, the user should be able to enter different commands. All commands appear in the status bar at the bottom.

1. Copy –
```$ copy <source_file(s)> <destination_directory>```
Move –
```$ move <source_file(s)> <destination_directory>```
Rename –
```$ rename <old_filename> <new_filename>```
        
   a.Eg–
   ```$ copy foo.txt bar.txt baz.mp4 ~/foobar```
   ```$ move foo.txt bar.txt baz.mp4 ~/foobar``` 
   ```$ rename foo.txt bar.txt```
   
   b. Assume that the destination directory exists, and you have write permissions.
   
   c. Copying/Moving directories should also be implemented
   
   d. The file ownership and permissions should remain intact

2. Create File –
```$ create_file <file_name> <destination_path>```

Create Directory –
```$ create_dir <dir_name> <destination_path>```

Eg – ‘$ create_file foo.txt ~/foobar ‘$ create_dir foo ~/foobar’

4. Delete File –
```$ delete_file <file_path>```

Delete Directory –
```$ delete_dir <dir_path>```

On deleting directory, you must recursively delete all content present inside it .

6. Goto –
```$ goto <location>```

Eg – ‘$ goto <directory_path>’

8. Search –
```$ search <file_name>```
or
```$ search <directory_name>```

   a. Search for a given file or folder under the current directory recursively.
   b. Output should be True or False depending on whether the file or folder exists
 
10. On pressing ESC key, the application should go back to Normal Mode
    
11. On pressing q key in normal mode, the application should close. Similarly, entering the ‘quit’
command in command mode should also close the application.



