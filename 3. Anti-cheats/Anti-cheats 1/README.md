# Running processes scanning

Now that you have learnt to develop cheats, next is to be able to detect the use of them and do something about it.

A good way of thinking about anti-cheats is like anti-virus software. Where the anti-cheats/virus is trying to find bad processes or something suspicious happening with another process and stopping it.

Anti-cheat usually is integrated into the game itself as well as externally to the process. This makes it harder to differentiate between game code and anti-cheating functions and providing direct access to important information in the game, lowering the ability for it to be masked by some cheats.

This won't be done as while Quake's source code is open, it would be exponentially harder to do and explain. The code would have to be integrated into the game's own code, which would also have to be explained. And compiling the game's code itself would also have to be done.

These will have to be external with the process hooking being done at the start like the cheats before.

This first anti-cheat detection program will be a simple process scanner. Checking all the running processes for ones it knows are bad, and then stopping the game if it detects one running.

## Coding

Before any code is done, a list needs to be created of "known bad processes". While this could be done inside the code itself, it isn't very good for adding new items to it. As most anti-cheats connect to a server to receive or compare known processes which gets updated. As the uptime for that costs money, a text file will be used as a demonstration.

![Known processes text file](https://gcdnb.pbrd.co/images/q4FW0lmsZT7k.png "Known processes text file")

This file then gets opened by the code, with all the entries being converted into wide character strings so they work with the previously used [`_wcsicmp`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/stricmp-wcsicmp-mbsicmp-stricmp-l-wcsicmp-l-mbsicmp-l?view=msvc-170 "_wcsicmp") function, and then put into a vector.

```cpp
// Sets up the file with the known cheats and vector to put the data into
std::ifstream knownCheatsFile("known_cheats.txt");
std::vector<std::wstring> knownCheatsList;
```

```cpp
// Gets the strings from the file, converts them into wide characters and then puts them into the vector
std::string tmpStr;
while (std::getline(knownCheatsFile, tmpStr)) {
    std::wstring tmpWStr( tmpStr.begin(), tmpStr.end() );
    knownCheatsList.push_back(tmpWStr);
    std::wcout << tmpWStr << std::endl;
}

knownCheatsFile.close(); // Closes file, as it isn't needed anymore
```

The [`getline`](https://cplusplus.com/reference/string/string/getline/ "getline") function in the standard library gets the next line in a input stream (in this case being the file) and puts it into a string (in this case being `tmpStr`).

Once the list is loaded, the code now continually checks the running processes to try find one it doesn't like. This is done by creating a snapshot and looping through the process entries and string comparing the names. Similar to the code for hooking into the main process.

```cpp
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
```

The [`TerminateProcess`](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/stricmp-wcsicmp-mbsicmp-stricmp-l-wcsicmp-l-mbsicmp-l?view=msvc-170 "TerminateProcess") function is used to close Quake if it detects a cheat. The code also exits out when this is done, but it is only for demonstrational purposes and probably wouldn't happen if it was a real external anti-cheat. For more information on the other parts of this code, check the "Hooking into the process" section.

## Potential improvements

- Only processes are looked at in the code, and a cheat could be hidden as a module used by a legitimate process. Can you also add support for scanning modules?

    Difficulty: 2/5

    Hint: Look at the process hooking code for inspiration

- The code looks at names of processes, but this isn't the best as process names can be easily changes. Can you make it look at hashes instead?

    Difficulty: 4/5
    
    Hint: Have a look at the [`PROCESSENTRY32`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32 "PROCESSENTRY32") and [`MODULEENTRY32`](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-moduleentry32 "MODULEENTRY32") structs and compile as 64bit to make it easier. And for getting the hash, either look at the openssl libraries or getting the code to execute powershell commands.

------------

### Click [here](https://github.com/AberFray/how-to-make-game-cheats-and-anticheats/tree/main/3.%20Anti-cheats/Anti-cheats%202 "Anti-cheats 2") for the next lesson

(The link in the bottom left of this is for feedback to GitHub itself, to give feedback on this repository, click [here](https://forms.office.com/e/r9Mdy3stif "Survey"))