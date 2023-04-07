# Bot like consistency detection

As cheats and anti-cheats are an ever evolving battle. It gets harder for the both of them to detect and evade each other, but one thing is known for certain about the cheats. The cheats, will be used to cheat.

This may seem something pointless to say, but what it means is that the cheats will be used on the game, such as aim cheats will lock onto another player or bunny hop scripts will consistently bunny hop. And we can use this to detect if a player is acting suspiciously consistent in some regards.

Now it could be possible to trigger these in normal play for someone who isn't using cheats, that’s why the "punishments" for these shouldn't be severe to upset legitimate players.

This kind of code would be great to be internal as it could get direct access to game timing and values quicker than an external program, which the importance of which will be shown.

# Getting the needed values

As this is being made externally, the values desired will need to be found. While as mentioned it would work well as an internal tool, it was discussed in `Anti-Cheats 1: Running processes scanning` why the anti-cheats are being made externally.

This program focuses on bunny hop cheat detection, so while some of the addresses are already known from making them, more will be needed for the players velocity.

The player’s velocity isn't just a single value, it's a vector of 3 values for the X (left and right), Y (forward and backwards) and Z (up and down) speed of the player. So in memory it will likely be close to each other.

As usual, it could be possible to check for a 0 for player velocity values and then a "Increased Value" when moving. But having a little game knowledge goes a long way. With the game's [`sv_maxspeed`](https://www.quakewiki.net/console/console-commands/quake-console-commands/#c-sv_maxspeed "sv_maxspeed") value, the player can walk at full speed in 1 direction and the value will be known. Additionally the value will most likely be a float for smoothness of movement and considering previous value types.

![Scanning for velocity values](https://gcdnb.pbrd.co/images/fUmZ8HsDHaJg.png "Scanning for velocity values")

Once searched, it’s possible that there will be multiple entries. What is wanted is a address that can be written to and make a difference. A good way to check this is by selecting the tick box under "Active" next to the value, which will freeze it at the original value or one that is set.

![Entries in the address list](https://gcdnb.pbrd.co/images/VuhNVyTnHD6f.png "Entries in the address list")

It is possible that the value found is not a static address, in this case a pointer scan will be done. Which can be achieved by right clicking the address in the address list and selecting "Pointer scan for this address". Then selecting OK in the new window and saving the data to a sensible location.

![Pointer scan](https://gcdnb.pbrd.co/images/wlgCvUYvQzkX.png "Pointer scan")

What should be looked at first is the base addresses. One with the executable's name should be used as they are static to it, and the number after the "+" should be noted. After that should be the offsets, which should also be noted.

## Code

(As a heads up, for this program, compiler optimizations had to be disabled as it wouldn't act correctly with them. This can be done by going to "Project"->"Properties"->"Configuration Properties"->"C/C++"->"Optimization". And in the "Optimization" option selecting "Disabled".)

![Disable optimizations](https://gcdnb.pbrd.co/images/D2Rpjmyk9XB0.png "Disable optimizations")

Unsurprisingly, the variables need to be set up first and there is a lot of them.

```cpp
BYTE onGround = 0; // Variable to store the state of the player if they are on the ground or not. Is a byte because the whole byte is sent by the ReadProcessMemory function
uintptr_t onGroundAddr = gameBaseAddr + 0x?????; // A pointer to the address in memory where the onGround state is. This was found beforehand manually in testing
unsigned int groundLen = 0; // Counter for how long the player was on the ground
```

This first part is taken from `Cheats 1: Bunny hop script`, as the code is checking for this.

```cpp
bool timingGround = true; // Flag to start timing on the ground
unsigned int timingList[10] = {0}; // Stores the last 10 jump times
char timingListIndex = 0; // An index variable to check which is index of the timingList to change
bool consistencyCheck = false; // A flag too see if the timings were suspiciously consistent
```

The next part sets up a list of timings and some other values. This is for the main way this program suspects cheats are being used.
The time it takes for when the player lands on the ground until they jump is recorded and put into a list. The list is then checked and if all the recorded times are similar enough to each other than the player's velocity is set to 0 which halts all their momentum.

The handling of checking if the jump button is pressed will be done in a separate thread to get more accurate timings.

```cpp
std::thread spacebarChecker(checkForSpacebar, &timingGround); // Runs a thread for helping with timings with the spacebar
```

Now is to get the memory address of the velocity. As this might not be a static address it will have to be explained how finding dynamic addresses are done.
There are two main pieces of information needed. A starting address and a list of offsets.

```cpp
// The memory location data for the players velocity
// The location isn't static, so a static pointer too it is used
uintptr_t playerVelBaseAddrPtr = gameBaseAddr + 0x?????; // Pointer to initial address
uintptr_t playerVelOffsets[] = { 0x??, ... }; // The offsets (might be multiple, might be just one)
uintptr_t playerVelBaseAddr = playerVelBaseAddrPtr; // Actual address holder
```

What happens with these is that the memory in the first address is read, which contains another memory address, then add the current offset and repeat with the new address plus offset until the memory address with wanted data is found.

![Dynamic pointer resolving diagram](https://gcdnb.pbrd.co/images/Mpnqe6b9BmWV.png "Dynamic pointer resolving diagram")

```cpp
// Loop through all the offsets to find the actual address
for (int i = 0; i < sizeof(playerVelOffsets) / sizeof(uintptr_t); i++) { // For every offset
    ReadProcessMemory(gameProc, (BYTE*)playerVelBaseAddr, &playerVelBaseAddr, sizeof(playerVelBaseAddr), 0); // Resolve the actual address and put it into the actual address variable
    playerVelBaseAddr = playerVelBaseAddr + playerVelOffsets[i]; // Adding the offset
}
```

Next is the main loop, which starts by reading if the player is on the ground or not and if so it executes some code.

```cpp
// Main loop
while (true) {

    // Reads memory to check if the player is on ground, if so then start timing
    ReadProcessMemory(gameProc, (BYTE*)onGroundAddr, &onGround, sizeof(onGround), 0);
    if (onGround) {
		...
```

The first thing it does when it knows the player is on the ground is it starts timing.

```cpp
// Set the timing flag, that will be changed in the other thread, and then start timing
timingGround = true;
while (timingGround) {
    groundLen++;
}
```

This will only exit when `timingGround` is set to false by the thread code, as it is passed by reference.

```cpp
void checkForSpacebar(bool* flag) {
    while (true) {
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            *flag = false;
        }
    }
}
```

Unfortunately this couldn't be done with actual time measurements as Windows only works on milliseconds and calling and setting that all up would take too long to get a useful measurement. In this situation if it was an internal anti-cheat. Then it could simply run on every game update, which would make things easier.

Anyways, once it has broken out, it saves it to the list of recordings. Overwriting the first one if it has filled the rest in.

```cpp
// Add the timing to the list
timingList[timingListIndex] = groundLen;
timingListIndex++;
if (timingListIndex >= 10) { timingListIndex = 0; } // Wrap around if the index has gone too far
```

If the list has 10 records, then it checks each one and compares it with the previous. If all the times are within 4 times higher or lower than the previous entry, then it is deemed suspiciously consistent, if one of them is out however, then it breaks out of the check and carries on.

```cpp
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
	...
```

Next if it is suspiciously consistent, then it sets the players velocity to zero. Negating the speed of any bunny hops.

```cpp
// If found to be too consistent, set the player velocity to 0;
if (consistencyCheck) { 
    WriteProcessMemory(gameProc, (BYTE*)playerVelBaseAddr, &zero, sizeof(zero), 0);
    WriteProcessMemory(gameProc, (BYTE*)playerVelBaseAddr + sizeof(zero), &zero, sizeof(zero), 0);
    WriteProcessMemory(gameProc, (BYTE*)playerVelBaseAddr + sizeof(zero) * 2, &zero, sizeof(zero), 0);
    std::cout << ":(" << std::endl; // for showing the values for demonstation. Wouldn't be in actual game
}
```

Finally, it prints out the recorded time (although this is only for the demo and wouldn't be used in an actual production situation) and resets the values. It also sleeps for a millisecond as it causes the `ReadProcessMemory` results to be random.

```cpp
	}

    // Output time for debugging, and reset values
    std::cout << groundLen << std::endl;
    groundLen = 0;
    timingGround = true;
}
Sleep(1); // Needs this or it starts to read weird onground values
```

## Potential improvements

- This covers bunny hops, can you make a version that works for aim cheats?

	Difficulty: 3.5/5

	Hint: Use your imagination... How would you do this?

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats "Main page") to go back to the main page

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))