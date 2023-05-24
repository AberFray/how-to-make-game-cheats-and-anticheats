#include <iostream> // For general input and output
#include <Windows.h> // Windows API for interacting with windows processes
#include <TlHelp32.h> // For getting snapshots
#include <thread> // For multithreading
#include <math.h> // For the trigonometry stuff yey

boolean cheatsRunning = true; // Global variable used to check if the game cheats are running. Global because it is used by all threads
bool aimToggle = false; // Global flag for turning the cheats on and off. Tried doing this local variables passed by reference, but it was having problems

/*
    printMenu function
    ... Does what it says on the tin
*/

void printMenu() {

    // Print the title
    std::cout <<
        ":'#######::'##::::'##::::'###::::'##:::'##:'########:::::::'###::::'####:'##::::'##::::'########:'##::::'##::::'###::::'##::::'##:'########::'##:::::::'########:\n"
        "'##.... ##: ##:::: ##:::'## ##::: ##::'##:: ##.....:::::::'## ##:::. ##:: ###::'###:::: ##.....::. ##::'##::::'## ##::: ###::'###: ##.... ##: ##::::::: ##.....::\n"
        " ##:::: ##: ##:::: ##::'##:. ##:: ##:'##::: ##:::::::::::'##:. ##::: ##:: ####'####:::: ##::::::::. ##'##::::'##:. ##:: ####'####: ##:::: ##: ##::::::: ##:::::::\n"
        " ##:::: ##: ##:::: ##:'##:::. ##: #####:::: ######::::::'##:::. ##:: ##:: ## ### ##:::: ######:::::. ###::::'##:::. ##: ## ### ##: ########:: ##::::::: ######:::\n"
        " ##:'## ##: ##:::: ##: #########: ##. ##::: ##...::::::: #########:: ##:: ##. #: ##:::: ##...:::::: ## ##::: #########: ##. #: ##: ##.....::: ##::::::: ##...::::\n"
        " ##:.. ##:: ##:::: ##: ##.... ##: ##:. ##:: ##:::::::::: ##.... ##:: ##:: ##:.:: ##:::: ##:::::::: ##:. ##:: ##.... ##: ##:.:: ##: ##:::::::: ##::::::: ##:::::::\n"
        ": ##### ##:. #######:: ##:::: ##: ##::. ##: ########:::: ##:::: ##:'####: ##:::: ##:::: ########: ##:::. ##: ##:::: ##: ##:::: ##: ##:::::::: ########: ########:\n"
        ":.....:..:::.......:::..:::::..::..::::..::........:::::..:::::..::....::..:::::..:::::........::..:::::..::..:::::..::..:::::..::..:::::::::........::........::\n"
        << std::endl;

    // Print instructions
    std::cout << "\n - Press the middle mouse button to toggle aimbot -" << std::endl;
}


/*
    Handles input to toggle the cheats
*/
void checkForInput() {
    while (true) {
        if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) {
            if (aimToggle) {
                aimToggle = false;
            }
            else {
                aimToggle = true;
            }
        }
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            cheatsRunning = false;
            break;
        }
        Sleep(200); // Sleep to stop re-toggles
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

    printMenu(); // Prints the menu to the console if used

    // Setting up the variables and memory addresses for the needed data to calculate where to aim
    float playerYaw = 0;
    uintptr_t playerYawAddr = gameBaseAddr + 0xEDF10;
    float playerX = 0;
    uintptr_t playerXAddr = gameBaseAddr + 0xD0B80;
    float playerY = 0;
    uintptr_t playerYAddr = gameBaseAddr + 0xD0B84;
    float player2X = 0;
    uintptr_t player2XAddr = gameBaseAddr + 0xD11B0;
    float player2Y = 0;
    uintptr_t player2YAddr = gameBaseAddr + 0xD11B4;

    // Sets up extra variables for the calculations
    float relposX = 0;
    float relposY = 0;

    std::thread inputThread(checkForInput); // Creates a thread for handling input checking input, 

    /*
        Main game loop
    */
    while (cheatsRunning) { // While the cheats are still running

        if (aimToggle) { // If the toggle is set to run the aim cheats

            //Get the X and Y coords for both player 1 and player 2
            ReadProcessMemory(gameProc, (BYTE*)playerXAddr, &playerX, sizeof(playerX), 0);
            ReadProcessMemory(gameProc, (BYTE*)playerYAddr, &playerY, sizeof(playerY), 0);

            ReadProcessMemory(gameProc, (BYTE*)player2XAddr, &player2X, sizeof(player2X), 0);
            ReadProcessMemory(gameProc, (BYTE*)player2YAddr, &player2Y, sizeof(player2Y), 0);

            // Get their relative positions
            relposX = player2X - playerX;
            relposY = player2Y - playerY;

            // Calculate the view angle, by getting the tangent using the function from math.h
            // The function returns in radians, so it is converted into degrees
            playerYaw = (float)((atan2f(relposY, relposX)) * (180 / 3.1415));

            // The calculated angle is put into the game
            WriteProcessMemory(gameProc, (BYTE*)playerYawAddr, &playerYaw, sizeof(playerYaw), 0);
        }
    }

    inputThread.join(); // Joins the final input checking thread

    std::cout << "- Cheats closed -" << std::endl; // End message

}
