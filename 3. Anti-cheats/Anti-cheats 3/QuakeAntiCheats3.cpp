#include <iostream> // For general input and output
#include <Windows.h> // Windows API for interacting with windows processes
#include <TlHelp32.h> // For getting snapshots
#include <thread> // Used to check for space bar presses in a seporate thread

/*
    checkForSpacebar function
    Called by a new thread and constantly checks if the space bar has been pressed, if so then sets the flag given to it via the parameter to false
    The flag is passed by reference, so it will be changed by this function in the main thread)
*/

void checkForSpacebar(bool* flag) {
    while (true) {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            *flag = false;
        }
    }
}

int main()
{
    /*
        Get the process ID to attach later
    */

    DWORD gameProcID = -1; // Variable to hold the process ID, has to be a DWORD because the windows API wants it that way. Starts as -1 (which is invald) so it is known if the process wasn't found
    HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Creates a "snapshot" of the current processes (Essentially a list of the processes)

    // Creates a placeholder process entry, that will hold each process for checking
    PROCESSENTRY32 currentProc;
    currentProc.dwSize = sizeof(PROCESSENTRY32); // Has to have it's "sizeof" member set, or else the loop fails. 

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

    // If it can't find the process, print and then close
    if (gameProcID == -1) {
        std::cout << "Couldn't find process" << std::endl;
        exit(1);
    }

    /*
        Get the module inside the found process (Quite similar to finding the process)
    */

    uintptr_t gameBaseAddr = -1; // Variable to hold the base memory address of the module we want to find, Starts as -1 (which is invald) so it is known if the module wasn't found
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

    /*
        Sets up the variables needed before the main loop
    */

    HANDLE gameProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, gameProcID); // Opens a handle to attach to the Winquake process and module. With full access rights

    BYTE onGround = 0; // Variable to store the state of the player if they are on the ground or not. Is a byte because the whole byte is sent by the ReadProcessMemory function
    uintptr_t onGroundAddr = gameBaseAddr + 0x?????; // A pointer to the address in memory where the onGround state is. This was found beforehand manually in testing
    unsigned int groundLen = 0; // Counter for how long the player was on the ground
    bool timingGround = true; // Flag to start timing on the ground
    unsigned int timingList[10] = { 0 }; // Stores the last 10 jump times
    char timingListIndex = 0; // An index variable to check which is index of the timingList to change
    bool consistencyCheck = false; // A flag too see if the timings were suspiciously consistent

    std::thread spacebarChecker(checkForSpacebar, &timingGround); // Runs a thread for helping with timings with the spacebar

    // The memory location data for the players velocity
    // The location isn't static, so a static pointer too it is used
    uintptr_t playerVelBaseAddrPtr = gameBaseAddr + 0x?????; // Pointer to intial address
    uintptr_t playerVelOffsets[] = { 0x??, ... }; // The offsets (might be multiple, might be just one)
    uintptr_t playerVelBaseAddr = playerVelBaseAddrPtr; // Actual address holder

    // Loop through all the offsets to find the actual address
    for (int i = 0; i < sizeof(playerVelOffsets) / sizeof(uintptr_t); i++) { // For every offset
        ReadProcessMemory(gameProc, (BYTE*)playerVelBaseAddr, &playerVelBaseAddr, sizeof(playerVelBaseAddr), 0); // Resolve the actual address and put it into the actual address variable
        playerVelBaseAddr = playerVelBaseAddr + playerVelOffsets[i]; // Adding the offset
    }

    float zero = 0; // Setting up a float with the value zero, to make things a bit easier with functions

    // Main loop
    while (true) {

        // Reads memory to check if the player is on ground, if so then start timing
        ReadProcessMemory(gameProc, (BYTE*)onGroundAddr, &onGround, sizeof(onGround), 0);
        if (onGround) {

            // Set the timinng flag, that will be changed in the other thread, and then start timing
            timingGround = true;
            while (timingGround) {
                groundLen++;
            }

            // Add the timing to the list
            timingList[timingListIndex] = groundLen;
            timingListIndex++;
            if (timingListIndex >= 10) { timingListIndex = 0; } // Wrap around if the index has gone too far

            if (timingList[9] != 0) { // Only check once at least 10 jumps have been recorded

                // Go through each entry and compare it too the last once to see if it is consistant
                consistencyCheck = true;
                for (int i=1; i < 10; i++) {
                    if (timingList[i] > timingList[i - 1] * 4 || timingList[i] < timingList[i - 1] / 4) { // The leniency for the timings is 4 times its value
                        // If inconsistant, break out and set flag
                        consistencyCheck = false;
                        break;
                    }
                }

                // If found to be too consistent, set the player velocity to 0;
                if (consistencyCheck) { 
                    WriteProcessMemory(gameProc, (BYTE*)playerVelBaseAddr, &zero, sizeof(zero), 0);
                    WriteProcessMemory(gameProc, (BYTE*)playerVelBaseAddr + sizeof(zero), &zero, sizeof(zero), 0);
                    WriteProcessMemory(gameProc, (BYTE*)playerVelBaseAddr + sizeof(zero) * 2, &zero, sizeof(zero), 0);
                    std::cout << ":(" << std::endl; // for showing the values for demonstation. Wouldn't be in actual game
                }
            }

            // Output time for debugging, and reset values
            std::cout << groundLen << std::endl;
            groundLen = 0;
            timingGround = true;
        }
        Sleep(1); // Needs this or it starts to read weird onground values
    }

}
