# Code integrity checking

This type of anti-cheat is a direct counter to the 2nd cheat covered in the resource. That being the "Wallhack" that modified the games code during runtime.

The program will continually check for changes in the code for the game and stop it if detected.

Realistically this would be done by a service with very high privileges, possibly as a driver. That the main code would have to check to make sure is running and connected at all times, as this wouldn't be feasible to do in the main games code as for the code itself it would require admin privileges which the game process itself shouldn't have.

As stated previously and discussed further in the "Discussion + Extra" segments, there won't be any materials on how to make drivers as the writer isn't proficient in making them.

While this is being done externally, there could be code written internally to check for certain variables and values and if they should be a certain value or not. But this is outside the scope of this specific lesson.

Finally, this will require information from the 2nd cheats lesson. Namely memory addresses and relevant bytecode.

## Code

The code for this type of anti-cheat is remarkably small, with discounting the whitespace lines in-between segments of code, it only comes to 11 lines.

Starting off, as always, is to set up the relevant variables.

```cpp
// Setting up variables for modifying game code
unsigned char actualCode[CODE_SIZE] = { 0x74, 0x?? }; // The original jmp command
unsigned char readCode[CODE_SIZE];
uintptr_t r_draworderStopper = gameBaseAddr + 0x?????; // The address of the jmp command
```

Once again, the question marks are to obscure the memory addresses, so you'll have to find them yourself.
The first line creates an array that should contain the original jump code that gets replaced, while the third line is a pointer to what should be the address where the jump code is. The second line is a buffer for the actual bytes that are continually read to be put. The constant `CODE_SIZE` is set to 2 earlier in the code.

Finally, the code loops until it detects a change in the game's runtime code. If it does, then it closes the game and itself.

```cpp
// Main loop
while (true) {
    ReadProcessMemory(gameProc, (BYTE*)r_draworderStopper, &readCode, sizeof(readCode), 0);
    for (int i = 0; i < CODE_SIZE; i++) {
        if (actualCode[i] != readCode[i]) {
            TerminateProcess(gameProc, r_draworderStopper + i);
            exit(0);
        }
    }
}
```

## Potential improvements

- The anti cheat only looks at one specific part of the code. As other parts could be changed, and all the code is stored in a section. Could the code be changed to look at all the code?

    Difficulty: 4.25/5

    Hint: There's a fair amount to this one. First of all is to get information about the `.text` section to the executable, this can be done inside CheatEngine by opening it up and attaching the process, then click "Memory View" and then in the top left of the new window "Tools" -> "Dissect PE Headers". Once there, select the game .exe file and then click "info".

    ![PE headers hint](https://gcdnb.pbrd.co/images/oC9BiHECCGqN.png "PE headers hint")

    Of particular interest are the first 4 items in the list. With "Virtual" meaning in memory, while "raw" is the exe itself.
    Using this information, the exe code can be compared with the code in memory. Although this will have to be tested, as some parts of the code are meant to change from the writers testing, as a heads up.

- Cheats could be implemented into the .exe itself, could you verify if the .exe's code is legitimate?

    Difficulty 2.5/5

    Hint: Possibly look at some of the file hashing ideas in Anti-Cheats 1

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats/tree/main/3.%20Anti-cheats/Anti-cheats%203 "Anti-cheats 3") for the next lesson

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))
