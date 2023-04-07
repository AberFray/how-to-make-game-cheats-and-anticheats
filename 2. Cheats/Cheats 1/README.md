# Bunny hop script

A bunny hop script is a program to automate bunny hopping in video games.

Bunny hopping itself is a technique available in some games (mostly games that run on an engine based off the Quake engine), in which you can travel faster than the normal maximum character speed. This is done by side strafing in the air continually and jumping as soon as you hit the ground.

This is due to the game giving the player a max walking velocity, but the velocity of a player can go above this when they are air strafing. Their speed slows down to normal though when they hit the ground but is cancelled when jumping. So the players speed stays high if they keep jumping just as they land.

This task is rather simple but can be difficult to execute, thus a script is used to do it for them. Which is what this tutorial will do:

## Retrieving memory addresses
(At this point it is assumed the user has read the introduction and installed the necessary tools)

Considering how most game logic works, there is most likely a variable in memory to indicate if the player is on the ground. Usually to determine if the player can jump, e.g.:
```cpp
while (true) {
	if (GetAsyncKeyState(VK_SPACE) > 1) {// Space bar is pressed
		if (playerGrounded == true) {
			jump();
		}
	}
}
```

To try find this variable, Cheat Engine will be used.

Starting off, Cheat Engine needs to get a hook into the running game. This can be done by opening up Cheat Engine, clicking "File"->"Open Process" and then finding the game process and clicking "Open".

Now that this has been done, a memory scan needs to be performed. Considering the type of data being searched for, it will probably be a boolean, as the player can be on ground or not. Thus being a boolean it will be a single byte so that is what will be searched (This does involve some amount of guess work which could be incorrect. If so more generic search parameters would be used, but being specific can speed up the process).

It isn't exactly known if 0 means on the ground or in the air, so both might need to be tested. But what should be done now is to select one of the values and perform an initial scan.

![First scan](https://gcdnb.pbrd.co/images/C6S4aI9XaDXJ.png "First scan")

From there, the next step is to perform more scans by using the "Next Scan" button, to whittle down the results. This should be done while moving the character, changing the viewing angle or other things but making sure that the player is on the ground when scanning. To scan when in the air, change the value to the other boolean value and then while airborne perform another scan. To help with this a hotkey could be used by going into the settings or the game could be paused and you might be frozen in the air depending on the game.

![Getting the memory address](https://gcdnb.pbrd.co/images/12IKqw9Ssl31.png "Getting the memory address")

The values of the memory addresses are updated regularly, so it can help determine which is the right address, addresses in green are static, so they will always be the same regardless of reboot.

Once a good memory address is found, now onto writing the script.

## Coding

(Make sure you understand the process hooking described in the Introduction before continuing)

First of all is to create a pointer to the memory address previously retrieved and then to create a variable to store the value:
```cpp
// Sets up auto jump variables
BYTE onGround = 0; // Variable to store the state of the player if they are on the ground or not. Is a byte because the whole byte is sent by the ReadProcessMemory function
uintptr_t onGroundAddr = gameBaseAddr + 0x?????; // A pointer to the address in memory where the onGround state is. This was found beforehand manually in testing
```

Remember that the memory address is based off of the virtual memory of the process, and that "0x?????" isn't a valid memory address and is just a placeholder, with the reader having to find the actual one.

Next is to get the input for jumping, which is a spacebar down followed by a spacebar up:

```cpp
// Stores an array of the keyboard inputs to be used to make the player jump
INPUT jumpInputs[2] = {};
jumpInputs[0].type = jumpInputs[1].type = INPUT_KEYBOARD; // The type of input
jumpInputs[0].ki.wScan = jumpInputs[1].ki.wScan = MapVirtualKeyA(VK_SPACE, MAPVK_VK_TO_VSC); // The key that will be pressed (Spacebar)
jumpInputs[0].ki.dwFlags = KEYEVENTF_SCANCODE; // First input is pressing down
jumpInputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP; // Second is pulling up
```

The input data is saved into the `jumpInputs[]` array, with the [`MapVirtualKeyA`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapvirtualkeya "MapVirtualKeyA") function, creating a [`KEYBDINPUT`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-keybdinput "KEYBDINPUT") object. 

There are two main types of input codes: virtual-key codes and scan codes. Usually it is recommended to use virtual-key codes, but this game doesn't recognize those inputs, so they are converted to scan codes.

For extra functionality, an auto strafing function will be created. This will need to be done in a separate thread to avoid blocking the other parts of the code.

```cpp
// Sets up auto strafe variables
boolean autoStrafeFlag = false; // A flag used by the seporate thread to check if autostrafing is being done
std::thread autoStrafeThread(autoStrafe, std::ref(autoStrafeFlag)); // Sets up and runs the seporate autoStrafe thread, which gets the autoStrafeFlag variable by reference so it can be checked when changed in the main thread
```

A flag is passed through by reference to tell the thread when to start running some code.

Onto the code in the separate thread, it runs the newly created function `autosave`. In the function it starts out by mapping the needed inputs.

```cpp
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
```

(Only the first half is shown to save space, the same code is done for the left side)

Then once initialized, the code runs a loop based on a global flag which signifies if the program is still running, then it checks if the auto strafing has been triggered. If so, it starts an initial half drift and then loops through the full drift until the flag is turned off. With it checking the flag during multiple points when running and closing cleanly if so.

```cpp
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
```

The emulated key presses are sent to the operating system via the [`SendInput`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput "SendInput") function which takes an array of inputs along with some other information about the array.

Back to the main code, it runs its own loop based on the `cheatsRunning` flag, in which it reads if the player is on the ground and then performs certain actions depending on what keys are being pressed.

```cpp
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
```

The value of the players ground status is retrieved by the [`ReadProcessMemory`](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-readprocessmemory "ReadProcessMemory") function. This data is then put into the specified pointer to a variable, with other parameters being the game process handle, the memory address of the value and how much to read.

The key presses are read using the [`GetAsyncKeyState`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getasynckeystate "GetAsyncKeyState") function with it being provided with a [virtual key code constant](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes "virtual key code constant"). As stated in the documentation, the function returns a 1 on the most significant bit if the button is being pressed, and as it returns a `SHORT` this value would be `0x8000`.

The main functionality key checks are for holding the auto jump key `/`, the auto strafe key `e` and if the auto strafe key has been released.

If the end key is pressed, this ends the cheats themselves by setting the `cheatsRunning` flag to false. This will then wait for the other thread to join and then closes.

```cpp
autoStrafeThread.join(); // Wait for the threads to close
std::cout << "- Cheats closed -" << std::endl; // End message
```

------------

## Potential improvements

- The user might not have `a` and `d` as their strafe keys. Can you make the code change depending on what controls the player has?

	Difficulty: 2/5

	Hint: Try find the config.cfg file, and look into [`VkKeyScanExA`](https://learn.microsoft.com/en-gb/windows/win32/api/winuser/nf-winuser-vkkeyscanexa "VkKeyScanExA").

- Some games don't support movement with the keyboard keys. Can you change the program to move the mouse instead?

	Difficulty 2/5

	Hint: Look at [`MOUSEINPUT`](https://learn.microsoft.com/en-gb/windows/win32/api/winuser/ns-winuser-mouseinput "MOUSEINPUT").

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats/tree/main/2.%20Cheats/Cheats%202 "Cheats 2") for the next lesson

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))