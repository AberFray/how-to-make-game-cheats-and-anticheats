#include <iostream> // For general input and output
#include <fstream> // For dealing with files
#include <Windows.h> // Windows API for interacting with windows processes
#include <TlHelp32.h> // For getting snapshots
#include <vector> // Adds the vector class to use
#include <string> // Used for some string transformation functions

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

    // Sets up the file with the known cheats and vector to put the data into
    std::ifstream knownCheatsFile("known_cheats.txt");
    std::vector<std::wstring> knownCheatsList;

    // Gets the strings from the file, converts them into wide characters and then puts them into the vector
    std::string tmpStr;
    while (std::getline(knownCheatsFile, tmpStr)) {
        std::wstring tmpWStr( tmpStr.begin(), tmpStr.end() );
        knownCheatsList.push_back(tmpWStr);
        std::wcout << tmpWStr << std::endl;
    }

    knownCheatsFile.close(); // Closes file, as it isn't needed anymore

    while (true) { // Keep running the main loop to check for cheats

        HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Creates a "snapshot" of the current processes (Essentially a list of the processes)

        // Current process variable. Basically copied from previous code above
        PROCESSENTRY32 currentProc;
        currentProc.dwSize = sizeof(PROCESSENTRY32); 

        if (Process32First(procSnap, &currentProc)) { // Check through each process
            do {
                for (std::wstring i : knownCheatsList) { // For each process, check through each item in the known cheats vector and see if it matches. If so, then kill Quake
                    if (!_wcsicmp(currentProc.szExeFile, i.c_str())) {
                        TerminateProcess(gameProc, 42069);
                        CloseHandle(procSnap); // Close the snapshot. Good memory management :)
                        exit(0); // This is only here for demo purposes. An actual anti-cheat wouldn't die as soon as it caught something
                    }
                }
            } while (Process32Next(procSnap, &currentProc)); // While there are still more processes, keep checking
        }
        CloseHandle(procSnap); // Close the snapshot. Good memory management :)

    }

}
