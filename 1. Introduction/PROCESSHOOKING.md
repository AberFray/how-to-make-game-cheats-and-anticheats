# Hooking into the process

## User Account control

As getting a hook into a process for writing memory requires administrator privileges. The .exe files will run to run under that for all the created programs here. This can be done in two ways, the first of which is to simply just right click the executable and select "Run as administrator".

The second option is to have it ask by default. There are a few ways this can be done, but the easiest for this situation would be using Visual Studio.

To do this, create a project in Visual studio for the code (or open it if you have already done this) and then click the "Project" tab in the top left of the window. In the drop down menu select the properties item at the bottom of the list. Finally, in the new window select "Linker" in the "Configuration Properties" path and then select "Manifest File". Then inside the options on the right section, change "UAC Execution Level" to "requireAdministrator".

![Changing UAC default](https://gcdnb.pbrd.co/images/XGKhtPiCHGXq.png "Changing UAC default")

## Code

With most cheats, they need to interact with the game in some way.
It is possible to avoid this and create a macro for automating simple things, such as a script that presses the space bar every millisecond or a recoil control macro. But these have limited usages and can be fairly obvious.

The way that this is done is by "hooking" into a process, by creating a handle to it. If that sounds confusing then don't worry, it will be explained shortly.

Before writing an algorithm to do this, first is to import the needed libraries:
```cpp
#include <iostream> // For general input and output
#include <Windows.h> // Windows API for interacting with windows processes
#include <TlHelp32.h> // For getting snapshots
```
As indicated by the comments, `Windows.h` is used for interacting with the processes although it can do much more than that. `TlHelp32.h` is for getting "snapshots" of processes, which will be demonstrated in a moment. And finally, while not strictly necessary, is `iostream` which is used to output information to the console.

Since that is done, now onto writing code.
Most of the time when a program is running, it is running as a process in the operating system. This is true for games as well, so the first step is to get the process ID of the game.

To start, the `CreateToolhelp32Snapshot` function is used to get a "snapshot" of all the processes, by setting the "TH32CS_SNAPPROCESS" flag in the arguments:
```cpp
HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS , 0); // Creates a "snapshot" of the current processes (Essentially a list of the processes)
```
For more information on this function, and if you ever are confused about a Windows function. The Windows online documentation is a good place to go. [CreateToolhelp32Snapshot](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot "CreateToolhelp32Snapshot")

Next is to iterate through the captured processes to find the one we want. This is first done by setting up a variable to be the placeholder for the current process being checked:
```cpp
// Creates a placeholder process entry, that will hold each process for checking
PROCESSENTRY32 currentProc;
currentProc.dwSize = sizeof(PROCESSENTRY32); // Has to have it's "sizeof" member set, or else the loop fails
```
For more information, check the [documentation](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32 "documentation").

Once that is done, then next is to loop through all the processes using the placeholder:
```cpp
// Loops through each process in the snapshot until it finds Winquake.exe
if (Process32First(procSnap, &currentProc)) { // Starts by getting the first process
    do {
        if (!_wcsicmp(currentProc.szExeFile, L"Winquake.exe")) { // Checks if the process is Winquake, using a Wide Character String Compare function
            // If found, update the variable and break out the loop
            gameProcID = currentProc.th32ProcessID;
            break;
        }
    } while (Process32Next(procSnap, &currentProc)); // While there are still more processes, keep checking
}
CloseHandle(procSnap); // Close the snapshot. Good memory management :)
```
This is first started by the [`Process32First`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32first "Process32First") function to get the first process from the snapshot, then followed by using [`Process32Next`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32next "Process32Next") repeatedly in the `do ... while` loop to get the next process. With it stopping when `Process32Next` returns 0;

Inside the loop, the code checks the current process' exe file name and compares it with the file name game exe we want to check (In this example, Winquake.exe is being used). The [`_wcsicmp`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/stricmp-wcsicmp-mbsicmp-stricmp-l-wcsicmp-l-mbsicmp-l?view=msvc-170 "_wcsicmp") function is needed as the returned value from `currentProc.szExeFile` is a wide character string, that's also why the L was put before `"Winquake.exe"` to make it a wide character string. With the ! being used because `_wcsicmp` returns 0 if they are the same string.

If the process is found, then the process ID is saved into the variable `gameProcID`, which was previously initialized to -1, and then the loop is broken out of. Otherwise if `gameProcID` is still -1, which means that the process hasn't been found, then it prints a message and quits out:

```cpp
// If it can't find the process, print and then close
if (gameProcID == -1) {
    std::cout << "Couldn't find process" << std::endl;
    exit(1);
}
```

Now that the process ID has been found, the desired module needs to be found so that the base address for it can be retrieved.
Processes can have multiple modules that are used by the program, such as .dll files. Different games use different modules for their functionality, in this case for the example game, it uses the exe as the module for this.

The code to find the modules is rather similar to the previous one to find the processes. By creating a snapshot of the modules in the process via its process ID. And looping through all the different modules in the snapshot:
```cpp
uintptr_t gameBaseAddr = -1; // Variable to hold the base memory address of the module we want to find, Starts as -1 (which is invalid) so it is known if the module wasn't found
HANDLE modSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, gameProcID); // Creates a snapshot of all the modules inside the process

// Creates a placeholder module entry, that will hold each module for checking
MODULEENTRY32 currentMod;
currentMod.dwSize = sizeof(currentMod); // Has to have it's "sizeof" member set, or else the loop fails.

// Loops through each module in the snapshot until it finds Winquake.exe
if (Module32First(modSnap, &currentMod)) { // Starts by getting the first module
    do {
        if (!_wcsicmp(currentMod.szModule, L"Winquake.exe")) { // Checks if the module is Winquake, using a Wide Character String Compare fucntion
            // If found, update the variable and break out the loop
            gameBaseAddr = (uintptr_t)currentMod.modBaseAddr;
        }
    } while (Module32Next(modSnap, &currentMod)); // While there are still more modules, keep checking
}
CloseHandle(modSnap); // Close the snapshot. Good memory management

// If it can't find the module, print and then close
if (gameBaseAddr == -1) {
    std::cout << "Couldn't find module inside game. Perhaps you have the wrong 'Winquake.exe' executable" << std::endl;
    exit(1);
}
```

With the difference being it using [`Module32First`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-module32first "Module32First"), [`Module32Next`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-module32next "Module32Next") and [`MODULEENTRY32`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-moduleentry32 "MODULEENTRY32") instead of `Process32First`, `Process32Next` and `PROCESSENTRY32`. Plus it gets the base address in memory of the module as opposed to getting the process ID.

Finally is to create a handle to the process using the [`OpenProcess`](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess "OpenProcess") function with the game process ID. With the module base address being used later:
```cpp
HANDLE gameProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, gameProcID); // Opens a handle to attach to the Winquake process and module. With full access rights
```
The first argument is the access requested of the program, with `PROCESS_ALL_ACCESS` being used, which causes the program to require administrator privileges to run.
While it is most likely better to run with more specific access rights for specific situations, this code is an example and isn't meant for production use.

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats "Main page") to go back to the main page

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))