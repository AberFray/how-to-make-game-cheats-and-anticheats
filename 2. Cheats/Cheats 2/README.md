# Wall hacks*

Wall hacks or x-ray cheats as sometimes referred to, is a cheat that allows a player to see other players through walls and other objects. Giving them an extra positional awareness advantage.

Some keen-eyed viewers may have noticed the asterisk next to "Wall hacks". This is because this tutorial won't be showing the traditional way to create these.

This is for two main reasons:
- Complexity
 The exact methods to do this is rather in-depth and complicated. Which would be a jump compared to the other material. Plus the method going to be used is quite good for teaching.
- Compatibility
 The traditional method usually involves modifying the z-buffer via executing functions or variables used in the graphics library the game is running. But as the example game being tested is WinQuake, which doesn't use a external graphics library. This wouldn't work and would have to be done differently.

The method that will be used is by abusing some functionality in the game and adjusting the code in memory to enable something that should be disabled during multiplayer.

This functionality in particular is one of the commands in the game. Some games allow the player to use a command prompt inside it to edit certain things if given the right permissions for certain actions. The one of interest in this case is the [`r_draworder`](https://www.quakewiki.net/console/console-commands/quake-console-commands/#c-r_draworder "r_draworder") value. This value, if set to anything other than the default 0, will change the order of how the surfaces of the map are drawn. Drawing the farthest away walls and allowing the player see through the closest.

This powerful command is disabled in multiplayer games to stop players cheating, but is it possible to re-enable it through cheating? This is what the tutorial will demonstrate.

## Finding the code

Following the steps taken in the previous tutorial to get a hook into the game with cheat engine. The next step is to find the address holding the `r_draworder` value. This can be done quite simply by changing the values in the console and scanning for that exact value. This should be done in single player mode or whatever mode allows you to change it.

![Finding r_draworder in memory](https://gcdnb.pbrd.co/images/571cgJ40mGi1.png "Finding r_draworder in memory")

Once that is found, then the code that accesses it needs to be found using the "Find out what accesses this address" option when right clicking the memory address.

![Getting what accesses the address (Allowed access)](https://gcdnb.pbrd.co/images/oDY5CnhS0lKx.png "Getting what accesses the address (Allowed access)")

This information gives two leads to look at, the first one seems more interesting as it is directly accessing the address.

Right clicking the entry and choosing "Show this address in disassembler" shows what seems to be code that does something depending on the value.

![Unmodified code in memory](https://gcdnb.pbrd.co/images/HI9wDDMgTj7r.png "Unmodified code in memory")

From the looks of it, it seems to be determining the values of some variables depending on if `r_draworder` is 0 or not. This could be the code to effect if the game is rendered with the function or normally. To test this, the "jump if equal" `je` operation and its parameters were changed to no operation opcodes, referred to as `nop`. This was done by right clicking the code and selecting "Replace with code that does nothing".

![Modified code in memory](https://gcdnb.pbrd.co/images/H25Lr3eb4TBq.png "Modified code in memory")

Seeing that this worked, it's now time to automate this.

## Coding

Like with all code, the first thing to do is to get the variables set up

```cpp
// Setting up variables for modifying game code
bool wallhacksActive = false; // Toggle flag
unsigned char nops[2] = { 0x90, 0x90 }; // Replacement code for the je command
unsigned char actualCode[2] = { 0x74, 0x?? }; // The original je command
uintptr_t r_draworderStopper = gameBaseAddr + 0x?????; // The address of the je command
```

The `actualCode` will be found in the "Bytes" section inside the CheatEngine memory viewer, this will be the original command that was replaced with nops (Which have the byte code of 0x90). You can get the original code easily by right clicking the nops in CheatEngine and selecting "Restore with original code". And as always, the actual addresses will need to be found.

Once this has been completed, the rest of the process is rather trivial. With just a loop being done to check if the correct key is being pressed and if so to write to the code in memory.

```cpp
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
```

This being facilitated through the [`WriteProcessMemory`](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-writeprocessmemory "WriteProcessMemory") Windows API call.

## Potential improvements

- To avoid some program detection, could you modify the exe file to change the code instead of doing it in memory?

    Difficulty: 3.5/5

    Hint: Copy the bytes around the area of the code and try searching in a hex editor inside the exe file.
    
- Try actually write a traditional aimhack that draws the players over walls.

    Difficulty: 5/5

    Hint: I tried for a bit and got a decent understanding on how it worked, but couldn't stop it from crashing when I tried messing with it. [This](https://github.com/id-Software/Quake "Quake source code") could be of some help.

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats/tree/main/2.%20Cheats/Cheats%203 "Cheats 3") for the next lesson

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))