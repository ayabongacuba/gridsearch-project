# Project Log

## 23 October 2026 — ChatGPT
Asked: Do I really need to install Ubuntu for this project? The README says something about WSL. Also, what are the rules for using AI and GitHub here?
Got:   Explained that yes, on Windows we must use WSL (Windows Subsystem for Linux) to get the Ubuntu terminal, because the project uses `make` and standard C++ compilers which don't work well on Windows directly. Also explained the rules in `AI_CONTEXT.md`: we can use AI to help, but we must log it and we cannot edit files outside the `src/` folder.
Changed by me: First started learning basics of md in Youtube. I installed WSL and Ubuntu and ran `sudo apt install build-essential make`. I checked the `AI_CONTEXT.md` file to see the exact rules. I also made a GitHub repo to save my work, private.

## 23 October 2026 — ChatGPT
Asked: I am looking at Stage 0. The `Grid` class uses `std::vector<std::string> map`. I never mastered 2D arrays fully, so can you explain how a vector of strings acts like a 2D grid? how does `DR` and `DC` work in memory? And recommend videos to watch?
Got:   Explained that `std::vector<std::string>` is basically an array of arrays. Each string in the vector is one row, and each character inside the string is a column. It gave me an Abdul Bari video on 1D and 2D array memory layout so I could see how rows and columns sit next to each other in memory.
Changed by me: I watched related videos on Youtube. Now I understand that even though the map looks like a 2D grid on the screen, the computer stores it as a line of strings, I knew that before, but I never had a picture of the layout in the memory. I traced out a small 3x3 map on paper to see where each row starts and ends.

## 23 October 2026 — ChatGPT
Asked: I am reminding myself how `std::getline(...)` works because I struggled with it in the past , specifically the concept of reading a new line to getline before the actual text. Can I just write `#include <iostream>` and `using namespace std;` and then just type `getline`, `cin`, `cout` without the `std::`? What is the professional recommended method? Also I got confused with the `dummy` variable and why we use `in` instead of `cin`.
Got:   Explained that `using namespace std` is fine for beginners and small projects, but in professional C++ code it is usually avoided in headers because it can cause name clashes. It's better to use `std::` every time. Also explained that `in` is just the name of the input stream passed to the function, which could be `cin` or a file stream. The `dummy` is needed to "eat" the newline character left behind by `in >> cols` so `getline` doesn't read an empty line.
Changed by me: I decided to use `std::` everywhere to build good habits (This is a big project also). I added `std::string dummy; std::getline(in, dummy);` after reading the rows and cols. I tested it with a map file and the first row read correctly instead of being skipped. I searched more about the dummy variable on google.

## 23 October 2026 — ChatGPT
Asked: For the `index` function, what is the formula for `r * cols + c`? What is the math behind it, and how does this relate to the memory layout?
Got:   Explained that memory is just a straight line of boxes. To find the index in a 2D grid, you skip all the previous rows (`r * cols`), then walk forward to the correct column (`+ c`). It also showed the inverse for the `at` function: `r = i / cols` and `c = i % cols`.
Changed by me: I wrote out the code using `int` variables. I manually traced a 3x5 grid on paper to see that `index(1, 3)` becomes `1 * 5 + 3 = 8`, and `at(8)` gives back row `8/5 = 1` and column `8 % 5 = 3`. It matches perfectly.

## 23 October 2026 — ChatGPT
Asked: How do I implement `neighbours()` using the `DR[]` and `DC[]` arrays? I was told it's like moving on a grid like a chess board game with coordinates, but I'm not sure how that helps with the code.
Got:   Explained that `DR` is "Delta Row" (how much the row changes) and `DC` is "Delta Column". For example, moving North means row goes down by 1 (DR = -1) and column doesn't change (DC = 0). It showed a simple `for` loop that goes 4 times to check all 4 directions, and uses an `if` statement with `passable()` to only add real neighbors.
Changed by me: I wrote the loop using standard `if/else` logic since I am more comfortable in standard methods than the advanced ones. I made sure the loop goes from 0 to 3, which naturally gives me North, East, South, West in order. I tested it on a corner cell to make sure it doesn't go out of bounds.

## 23 October 2026 — ChatGPT
Asked: I need to add error checking in `readGrid`. Why does the project require `std::exit(2)` if a row is the wrong length or if there are fewer than 1 stops? Why specifically 2 and not other codes ?
Got:   Explained that `exit(2)` is a convention for the marker to know the file is broken. Showed how to use `row.length() != g.cols` with an `if` statement to catch bad rows, and `if (k < 1)` to catch bad stop counts.
Changed by me: I added the checks. I tested it by making a bad map file with a missing character. The program printed the error and stopped. I also tested the `k < 1` check by changing the stop count to 0.

## 23 October 2026 — ChatGPT
Asked: In `readGrid`, why must we use `std::exit(2)` instead of `return 2` when the map is malformed? I tried `return 2`and it gave me an error.
Got:   Explained that the function returns a `Grid` object, not an integer, so `return 2` would cause a compile error. `std::exit(2)` immediately terminates the whole program and sends the exit code `2` to the operating system, which is what the automated marker checks for.
Changed by me: I verified this by looking at the function signature. I left the `std::exit(2)` in my code to make sure the marker catches the bad input correctly.

## 23 October 2026 — ChatGPT
Asked: I have two things left for Stage 0. First, without the use of big words and analogies, how do I create the `makeCostTable` with the zero diagonal? Second, how do I write the `renderPath` function to draw the arrows? Also, why is `INF` defined as `LONG_MAX / 4` and not just `LONG_MAX`?
Got:   Provided a `for` loop to fill the table with `INF` and a second loop for the `0` diagonal. For `renderPath`, showed how to compare `prev` and `curr` cells to pick the right arrow character. Explained that `LONG_MAX / 4` prevents overflow when two `INF` values are added in Stage 5, because `LONG_MAX + LONG_MAX` would wrap around and become a negative number.
Changed by me: I implemented both functions using basic `if/else` logic for the arrows instead of a switch statement which I never spent too much time on. I checked the math myself: if `LONG_MAX` is the biggest number a long can hold, adding two of them would overflow. Dividing by 4 keeps it safe, so the program won't think a wall is the cheapest path.

## 23 October 2026 — ChatGPT
Asked: When I ran `make test-util`, it gave me this error: `/bin/sh: 1: pkg-config: not found`. What does this mean and how do I fix it?
Got:   Explained that the Makefile checks for `pkg-config` to see if the optional SDL2 visualiser library is installed. Since Ubuntu was fresh, it wasn't installed. Told me to run `sudo apt install pkg-config` and then run `make clean` to clear the half-finished build.
Changed by me: I installed `pkg-config`, ran `make clean`, and ran `make test-util` again. It compiled successfully and printed "All tests passed (74 assertions in 13 test cases)". I then pushed all my Stage 0 code to GitHub.