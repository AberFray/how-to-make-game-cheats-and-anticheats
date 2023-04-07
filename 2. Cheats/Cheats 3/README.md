# Aim cheats

Aim cheats are some of the most commonly seen cheats for player versus player first person shooter games online.

These cheats automatically aim for the player onto another player. Essentially playing the game itself, with the user just having to click on the other person.

These are usually toggled on and off depending on how well the user is doing.

Another reason for their popularity is their relative ease of creating, as will be demonstrated.

## Getting the players data

There are 3 pieces of information needed to create this hack:
- The users viewing angles
- The users position
- The position of other players

These will be used to calculate the angle to view the player and then put in that value. As for the 3rd item in the list, this will require another player to connect to. This can be done with most games by creating a local server for both to join.

Now it is possible to brute force finding the values. With most games using degrees to represent player looking angle and scanning being done by moving the players looking angle and position and re-scanning. And with the values most likely being stored as floats to avoid jittery movement.

But here is where a bit of game knowledge can help. Specifically two things:
- [`-condebug`](https://www.quakewiki.net/console/console-commands/quake-console-commands/#p--condebug "-condebug")
 A command line parameter for Quake that saves the output of the command line to the file "qconsole.log" inside the "id1" folder.
- [`entities`](https://www.quakewiki.net/console/console-commands/quake-console-commands/#c-entities "entities")
 A command that lists the current entities in the game inside the console. It provides the model used by it, along with the **orientation angle** and **location**

The reason for the command line parameter is that the output of the `entities` command is too big to fit the screen most of the time, and as it shows things from closest to furthest, the information in the console isn't very helpful

![entities command output to console](https://gcdnb.pbrd.co/images/2H6if064BnGC.png "entities command output to console")

As opposed to the full info from the text file

![entities command output to file](https://gcdnb.pbrd.co/images/3AOhdsCXJo4h.png "entities command output to file")

Next is to get the data. For the location, this is rather simple. Just move, use the entities command, check the file, scan for the new value, repeat. There may be multiple values, so just pick one that works and is a static address.

For the viewing angle, this isn't so easy. It needs to be a memory address that changes the players viewing angle when itself is changed. This can be checked by selecting the addresses, right clicking and choosing "Add selected addresses to the addresslist". Then double clicking the values for the addresses in the addresslist and changing them.

![The player's yaw addresses](https://gcdnb.pbrd.co/images/TJOQk79ehtQr.png "The player's yaw addresses")

Also sometimes the players angle can be inverted in memory.

With all the information needed, it's now onto coding the hacks.

## Coding

First is to set up all the information into variables to be used.

```cpp
// Setting up the variables and memory addresses for the needed data to calculate where to aim
float playerYaw = 0;
uintptr_t playerYawAddr = gameBaseAddr + 0x?????;
float playerX = 0;
uintptr_t playerXAddr = gameBaseAddr + 0x?????;
float playerY = 0;
uintptr_t playerYAddr = gameBaseAddr + 0x?????;
float player2X = 0;
uintptr_t player2XAddr = gameBaseAddr + 0x?????;
float player2Y = 0;
uintptr_t player2YAddr = gameBaseAddr + 0x?????;
```

As always, the actual memory locations won't be given out for free.

There are also some more variables to be used later.

```cpp
// Sets up extra variables for the calculations
float relposX = 0;
float relposY = 0;
```

Along with creating a variable to track when to run the cheats, which gets past to a function via another thread to handle input

```cpp
bool aimToggle = false; // Flag to toggle the cheats
std::thread inputThread(checkForInput, std::ref(toggle)); // Creates a thread for handling input checking input, 
```

```cpp
void checkForInput(bool aimToggle) {
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
 Sleep(500); // Sleep to stop re-toggles
 }
}
```

This also uses a global variable checking if the cheats are running.

Back to the main code, it runs a constant loop where it first grabs the players location data

```cpp
//Get the X and Y coords for both player 1 and player 2
ReadProcessMemory(gameProc, (BYTE*)playerXAddr, &playerX, sizeof(playerX), 0);
ReadProcessMemory(gameProc, (BYTE*)playerYAddr, &playerY, sizeof(playerY), 0);

ReadProcessMemory(gameProc, (BYTE*)player2XAddr, &player2X, sizeof(player2X), 0);
ReadProcessMemory(gameProc, (BYTE*)player2YAddr, &player2Y, sizeof(player2Y), 0);
```

The next part of the code requires some explaining.

What the primary goal of our cheats is to calculate the angle from the user to another player and then put that into their view angle. This can be done just by using both players position.

Suppose there are two players, one at location (1,2) and the other at (6,5) on a (X, Y) grid.

![Example location grid 1](https://gcdnb.pbrd.co/images/NSATdeGd1y3P.png "Example location grid 1")

To make things easier and for most math calculations. Only the difference in position will be considered, player A's position will be (0,0) and player B's location will change accordingly (5,3).
6 - 1 = 5
5 - 2 = 3

![Example location grid 2](https://gcdnb.pbrd.co/images/1sTOhRoWK6WK.png "Example location grid 2")

This is done in the code also.

```cpp
// Get their relative positions
relposX = player2X - playerX;
relposY = player2Y - playerY;
```

Considering the information known, we can now draw a right triangle with the two points. With the bottom and right side having the length of 5 and 3 respectively (We can figure out the hypotenuse, but it isn't important for this so there is no need).

![Example location grid 3](https://gcdnb.pbrd.co/images/6j61Z76RFanM.png "Example location grid 3")

Now this is where it starts to sound a little scary as the next step is to use some trigonometry. But this isn't very complex trigonometry and will be explained.

The trigonometric function that will be used it `tan` (sort of).

Remember that the objective is to find the angle from player A to player B. This starts at player A's location, which would be (0,0) in this example as only the difference in position is needed right now. The angle value to be found will be referred to as `X`. So the objective is to find `X`.

![Example location grid 4](https://gcdnb.pbrd.co/images/0H3zGRpjVP0g.png "Example location grid 4")

So the way `tan` works is as follows:

`tan(θ) = opposite/adjacent`

`θ` represents any angle, so in this case it could be replaced by `X`

`opposite` and `adjacent` are the side of the angle in relation to the angles location, in this instance `X`. So the opposite side to `X` would be the side of 3 length, and the adjacent side would be the side of 5 length. So `opposite` and `adjacent` could be replaced by `3` and `5`

So for this situation, `tan` would be:

`tan(X) = 3/5`

This is fine for finding `tan` of X, but this isn't the objective. X itself needs to be found, and this is where the previous "(sort of)" comes into play.

`tan⁻¹` or `arc tan` is the way of finding `θ`, `X` in this case, by knowing `opposite` and `adjacent`, `3` and `5` in this case.

This function will give us the needed angle (`X`) if we have the players' location differences (`3` and `5`). And it's inside of the `math.h` library.

```cpp
#include // For the trigonometry stuff yey
```

Some of the more math knowledgeable might have noticed some problems which would need to be accounted for if some of the differences were to be negative. Luckily the function that will be used, [`atan2f`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/atan-atanf-atanl-atan2-atan2f-atan2l?view=msvc-170 "atan2f"), already deals with this and will give out the correct angle.

Readers who actually looked into `atan2f` may have also noticed that the results are returned in radians. This will be dealt with by converting it to degrees as soon as it is handled.

With all that explaining out the way, here is the next line of code:

```cpp
// Calculate the view angle, by getting the tangent using the function from math.h
// The function returns in radians, so it is converted into degrees
playerYaw = (float)((atan2f(relposY, relposX)) * (180 / 3.1415));
```

Starting from left to right:
- `playerYaw` is the variable to hold the angle to the other player
- `(float)(...)` Makes sure the number stored in the variable is a float
- `(atan2f(relposY, relposX))` is the aforementioned function with the relative positions of the players to each other being passed as parameters. This is bracketed for readability.
- `* (180 / 3.1415)` The math needed to convert radians into degrees

And after that, it's just writing the value into memory to be done, and then to run the loop again.

```cpp
// The calculated angle is put into the game
WriteProcessMemory(gameProc, (BYTE*)playerYawAddr, &playerYaw, sizeof(playerYaw), 0);
```

And finally is the code to join the thread and print the close message. This will be triggered by `cheatsRunning` being set to false by the other thread.

```cpp
inputThread.join();
std::cout << "- Cheats closed -" << std::endl; // End message
```

## Potential improvements

- Could you change the code to be able to select from multiple players? How would you choose which one to look at?

    Difficulty: 2/5

    Hint: You may have noticed that some of the players' location memory addresses found were quite close to the other player's addresses.
- The code only changes the players yaw (left and right), could you add functionality for up and down (pitch)?

    Difficulty: 1/5

    Hint: Mostly just a copy paste job, although the memory value for pitch is a bit weird.

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats "Main page") to go back to the main page

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))