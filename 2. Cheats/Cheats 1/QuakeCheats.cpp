#include <iostream> // For general input and output
#include <Windows.h> // Windows API for interacting with windows processes
#include <TlHelp32.h> // For getting snapshots
#include <thread> // For multithreading

boolean cheatsRunning = true; // Global variable used to check if the game cheats are running. Global because it is used by all threads

/*
    printMenu function
    ... Does what it says on the tin
*/

void printMenu() {

    // Print the title
    std::cout <<
        ":'#######::'##::::'##::::'###::::'##:::'##:'########::::'########::'##::::'##::'#######::'########:::::'########:'##::::'##::::'###::::'##::::'##:'########::'##:::::::'########:\n"
        "'##.... ##: ##:::: ##:::'## ##::: ##::'##:: ##.....::::: ##.... ##: ##:::: ##:'##.... ##: ##.... ##:::: ##.....::. ##::'##::::'## ##::: ###::'###: ##.... ##: ##::::::: ##.....::\n"
        " ##:::: ##: ##:::: ##::'##:. ##:: ##:'##::: ##:::::::::: ##:::: ##: ##:::: ##: ##:::: ##: ##:::: ##:::: ##::::::::. ##'##::::'##:. ##:: ####'####: ##:::: ##: ##::::::: ##:::::::\n"
        " ##:::: ##: ##:::: ##:'##:::. ##: #####:::: ######:::::: ########:: #########: ##:::: ##: ########::::: ######:::::. ###::::'##:::. ##: ## ### ##: ########:: ##::::::: ######:::\n"
        " ##:'## ##: ##:::: ##: #########: ##. ##::: ##...::::::: ##.... ##: ##.... ##: ##:::: ##: ##.....:::::: ##...:::::: ## ##::: #########: ##. #: ##: ##.....::: ##::::::: ##...::::\n"
        " ##:.. ##:: ##:::: ##: ##.... ##: ##:. ##:: ##:::::::::: ##:::: ##: ##:::: ##: ##:::: ##: ##::::::::::: ##:::::::: ##:. ##:: ##.... ##: ##:.:: ##: ##:::::::: ##::::::: ##:::::::\n"
        ": ##### ##:. #######:: ##:::: ##: ##::. ##: ########:::: ########:: ##:::: ##:. #######:: ##::::::::::: ########: ##:::. ##: ##:::: ##: ##:::: ##: ##:::::::: ########: ########:\n"
        ":.....:..:::.......:::..:::::..::..::::..::........:::::........:::..:::::..:::.......:::..::::::::::::........::..:::::..::..:::::..::..:::::..::..:::::::::........::........::\n"
    << std::endl;
    
    // Print instructions
    std::cout << "\n - Press the \\ key to auto jump -" << std::endl;
    std::cout << "\n - Press the E key to auto strafe -" << std::endl;
}

/*
    autoStrafe function
    It's called by a new thread as it contains lots of sleep commands which would mess with the other things being run there
    runStrafe is the reference to the flag for when it should run
*/

void autoStrafe(boolean &runStrafe) {

    // Sets up the input stages to be run for strafing
    // Each one is similar to the jump inputs, the only important info is 0x44 is the "d" Key and 0x41 is the "a" key
    INPUT strafeInputsRight[2] = {};
    strafeInputsRight[0].type = strafeInputsRight[1].type = INPUT_KEYBOARD;
    strafeInputsRight[0].ki.wScan = MapVirtualKeyA(0x44, MAPVK_VK_TO_VSC);
    strafeInputsRight[0].ki.dwFlags = strafeInputsRight[1].ki.dwFlags = KEYEVENTF_SCANCODE;
    strafeInputsRight[1].ki.wScan = MapVirtualKeyA(VK_RIGHT, MAPVK_VK_TO_VSC);

    INPUT strafeInputsRightStop[2] = {};
    strafeInputsRightStop[0].type = strafeInputsRightStop[1].type = INPUT_KEYBOARD;
    strafeInputsRightStop[0].ki.wScan = MapVirtualKeyA(0x44, MAPVK_VK_TO_VSC);
    strafeInputsRightStop[0].ki.dwFlags = strafeInputsRightStop[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    strafeInputsRightStop[1].ki.wScan = MapVirtualKeyA(VK_RIGHT, MAPVK_VK_TO_VSC);
    
    INPUT strafeInputsLeft[2] = {};
    strafeInputsLeft[0].type = strafeInputsLeft[1].type = INPUT_KEYBOARD;
    strafeInputsLeft[0].ki.wScan = MapVirtualKeyA(0x41, MAPVK_VK_TO_VSC);
    strafeInputsLeft[0].ki.dwFlags = strafeInputsLeft[1].ki.dwFlags = KEYEVENTF_SCANCODE;
    strafeInputsLeft[1].ki.wScan = MapVirtualKeyA(VK_LEFT, MAPVK_VK_TO_VSC);

    INPUT strafeInputsLeftStop[2] = {};
    strafeInputsLeftStop[0].type = strafeInputsLeftStop[1].type = INPUT_KEYBOARD;
    strafeInputsLeftStop[0].ki.wScan = MapVirtualKeyA(0x41, MAPVK_VK_TO_VSC);
    strafeInputsLeftStop[0].ki.dwFlags = strafeInputsLeftStop[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    strafeInputsLeftStop[1].ki.wScan = MapVirtualKeyA(VK_LEFT, MAPVK_VK_TO_VSC);

    while (cheatsRunning) { // Keep running this code until the cheat program halts
        if (runStrafe) { // If the autoStrafeFlag is true then run the commands

            // Initial half strafe to the left to start
            SendInput(ARRAYSIZE(strafeInputsLeft), strafeInputsLeft, sizeof(INPUT)); // Strafe to the left
            if (!runStrafe) { SendInput(ARRAYSIZE(strafeInputsLeftStop), strafeInputsLeftStop, sizeof(INPUT)); break; } // Check the runStrafe value, if it has stopped then end the strafing
            Sleep(200); // Strafe for 0.2 seconds
            SendInput(ARRAYSIZE(strafeInputsLeftStop), strafeInputsLeftStop, sizeof(INPUT)); // Stop strafing to the left

            while (runStrafe) {
                // Strafe to the right
                SendInput(ARRAYSIZE(strafeInputsRight), strafeInputsRight, sizeof(INPUT)); // Strafe to the right
                if (!runStrafe) { SendInput(ARRAYSIZE(strafeInputsRightStop), strafeInputsRightStop, sizeof(INPUT)); break; } // Check the runStrafe value, if it has stopped then end the strafing
                Sleep(400); // Strafe for 0.4 seconds

                // Stop strafing to the right and switch strafing to the left
                SendInput(ARRAYSIZE(strafeInputsRightStop), strafeInputsRightStop, sizeof(INPUT)); // Stop strafing to the right
                if (!runStrafe) { break; }
                SendInput(ARRAYSIZE(strafeInputsLeft), strafeInputsLeft, sizeof(INPUT)); // Strafe to the left
                if (!runStrafe) { SendInput(ARRAYSIZE(strafeInputsLeftStop), strafeInputsLeftStop, sizeof(INPUT)); break; } // Check the runStrafe value, if it has stopped then end the strafing
                Sleep(400);// Strafe for 0.4 seconds
                
                // Stop strafing to the left
                SendInput(ARRAYSIZE(strafeInputsLeftStop), strafeInputsLeftStop, sizeof(INPUT)); // Stop strafing to the left
            }
        }
        Sleep(5); // Sleep for 5 miliseconds, so to not hog resources as updating at 200 times a second is fine enough
    }
}

int main()
{
    /*
        Get the process ID to attach later
    */

    DWORD gameProcID = -1; // Variable to hold the process ID, has to be a DWORD because the windows API wants it that way. Starts as -1 (which is invald) so it is known if the process wasn't found
    HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS , 0); // Creates a "snapshot" of the current processes (Essentially a list of the processes)
    
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

    // Sets up auto jump variables
    BYTE onGround = 0; // Variable to store the state of the player if they are on the ground or not. Is a byte because the whole byte is sent by the ReadProcessMemory function
    uintptr_t onGroundAddr = gameBaseAddr + 0x?????; // A pointer to the address in memory where the onGround state is. This was found beforehand manually in testing

    // Stores an array of the keyboard inputs to be used to make the player jump
    INPUT jumpInputs[2] = {};
    jumpInputs[0].type = jumpInputs[1].type = INPUT_KEYBOARD; // The type of input
    jumpInputs[0].ki.wScan = jumpInputs[1].ki.wScan = MapVirtualKeyA(VK_SPACE, MAPVK_VK_TO_VSC); // The key that will be pressed (Spacebar)
    jumpInputs[0].ki.dwFlags = KEYEVENTF_SCANCODE; // First input is pressing down
    jumpInputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP; // Second is pulling up

    // Sets up auto strafe variables
    boolean autoStrafeFlag = false; // A flag used by the seporate thread to check if autostrafing is being done
    std::thread autoStrafeThread(autoStrafe, std::ref(autoStrafeFlag)); // Sets up and runs the seporate autoStrafe thread, which gets the autoStrafeFlag variable by reference so it can be checked when changed in the main thread

    // Prints the menu to the console if used
    printMenu();

    /*
        Main game loop
    */

    while (cheatsRunning) { // Keep on running until the cheatsRunning global variable is false
        ReadProcessMemory(gameProc, (BYTE*)onGroundAddr, &onGround, sizeof(onGround), 0); // Read the memory of Winquake.exe, where the on ground state is and put it into the onGround variable
        
        if (GetAsyncKeyState(VK_OEM_5) & 0x8000 && onGround) { // If the "\" key is being pressed down, and the player is on the ground
            std::cout << "AUTO JUMP" << std::endl; // Print that the player has used auto jump
            SendInput(ARRAYSIZE(jumpInputs), jumpInputs, sizeof(INPUT)); // Send the saved keyboard inputs, so that the player will jump
        }

        if (GetAsyncKeyState(0x45) & 0x8000 && !autoStrafeFlag) { // If the "e" key is being pressed down and the autoStrafeFlag isn't already set to true
            std::cout << "AUTO STRAFE ENABLED" << std::endl; // Print that the player is using auto strafe
            autoStrafeFlag = true; // Set the autoStrafeFlag variable to true, so that the autoStrafe thread can run its loop
        }
        else if ( !(GetAsyncKeyState(0x45) & 0x8000) && autoStrafeFlag){ // If the key isn't being pressed and the autoStrafeFlag is on
            std::cout << "AUTO STRAFE DISABLED" << std::endl; // Print that the player has stopped using auto strafe
            autoStrafeFlag = false; // Set the autoStrafeFlag variable to true, so that the autoStrafe thread can stop its loop
        }
        if (GetAsyncKeyState(VK_END) & 0x8000) { // If the "END" key is pressed
            cheatsRunning = false; // Set the global running variable to false to cleanly stop the program
        }
        
        Sleep(5); // Sleep for 5 milliseconds, so to not hog resources as updating at 200 times a second is fine enough
    }

    autoStrafeThread.join(); // Wait for the threads to close

    std::cout << "- Cheats closed -" << std::endl; // End message

}
