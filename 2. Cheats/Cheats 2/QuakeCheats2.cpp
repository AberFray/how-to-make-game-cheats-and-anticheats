#include <iostream> // For general input and output
#include <Windows.h> // Windows API for interacting with windows processes
#include <TlHelp32.h> // For getting snapshots

boolean cheatsRunning = true; // Global variable used to check if the game cheats are running. Global because it is used by all threads

/*
    autoStrafe function
    It's called by a new thread as it contains lots of sleep commands which would mess with the other things being run there
    runStrafe is the reference to the flag for when it should run
*/

void printMenu() {

    // Print the title
    std::cout <<
        ":'#######::'##::::'##::::'###::::'##:::'##:'########::::'##:::::'##::::'###::::'##:::::::'##::::::::::'########:'##::::'##::::'###::::'##::::'##:'########::'##:::::::'########:\n"
        "'##.... ##: ##:::: ##:::'## ##::: ##::'##:: ##.....::::: ##:'##: ##:::'## ##::: ##::::::: ##:::::::::: ##.....::. ##::'##::::'## ##::: ###::'###: ##.... ##: ##::::::: ##.....::\n"
        " ##:::: ##: ##:::: ##::'##:. ##:: ##:'##::: ##:::::::::: ##: ##: ##::'##:. ##:: ##::::::: ##:::::::::: ##::::::::. ##'##::::'##:. ##:: ####'####: ##:::: ##: ##::::::: ##:::::::\n"
        " ##:::: ##: ##:::: ##:'##:::. ##: #####:::: ######:::::: ##: ##: ##:'##:::. ##: ##::::::: ##:::::::::: ######:::::. ###::::'##:::. ##: ## ### ##: ########:: ##::::::: ######:::\n"
        " ##:'## ##: ##:::: ##: #########: ##. ##::: ##...::::::: ##: ##: ##: #########: ##::::::: ##:::::::::: ##...:::::: ## ##::: #########: ##. #: ##: ##.....::: ##::::::: ##...::::\n"
        " ##:.. ##:: ##:::: ##: ##.... ##: ##:. ##:: ##:::::::::: ##: ##: ##: ##.... ##: ##::::::: ##:::::::::: ##:::::::: ##:. ##:: ##.... ##: ##:.:: ##: ##:::::::: ##::::::: ##:::::::\n"
        ": ##### ##:. #######:: ##:::: ##: ##::. ##: ########::::. ###. ###:: ##:::: ##: ########: ########:::: ########: ##:::. ##: ##:::: ##: ##:::: ##: ##:::::::: ########: ########:\n"
        ":.....:..:::.......:::..:::::..::..::::..::........::::::...::...:::..:::::..::........::........:::::........::..:::::..::..:::::..::..:::::..::..:::::::::........::........::\n"
        << std::endl;

    // Print instructions
    std::cout << "\n - Press the R key to toggle wallhacks -" << std::endl;
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

    // Setting up variables for modifying game code
    bool wallhacksActive = false; // Toggle variable
    unsigned char nops[2] = { 0x90, 0x90 }; // Replacement code for the je command
    unsigned char actualCode[2] = { 0x74, 0x?? }; // The original je command
    uintptr_t r_draworderStopper = gameBaseAddr + 0x?????; // The address of the je command

    // Prints the menu to the console if used
    printMenu();

    /*
        Main game loop
    */
    while (true) {

        // Check if the R key is being pressed
        if (GetAsyncKeyState(0x52) & 0x8000) { // If R key pressed

            // Check which state the wall hacks are in, then switch state
            if (!wallhacksActive) {
                WriteProcessMemory(gameProc, (BYTE*)r_draworderStopper, &nops, sizeof(nops), 0); // Write 2 bytes of memory so the game doesn't perform a jump command
                wallhacksActive = true;
                Sleep(500); // Sleep to stop rebounce
            }
            else if (wallhacksActive) {
                WriteProcessMemory(gameProc, (BYTE*)r_draworderStopper, &actualCode, sizeof(actualCode), 0); // Write 2 bytes of memory to restore the game to the original code
                wallhacksActive = false;
                Sleep(500); // Sleep to stop rebounce
            }

        }
        if (GetAsyncKeyState(VK_END) & 0x8000) { // If END key pressed

            // Restore original code and exit
            if (wallhacksActive) {
                WriteProcessMemory(gameProc, (BYTE*)r_draworderStopper, &actualCode, sizeof(actualCode), 0); // Write 2 bytes of memory to restore the game to the original code
            }
            break;
        }
    }

    std::cout << "- Cheats closed -" << std::endl; // End message

}