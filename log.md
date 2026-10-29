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

## 23 October 2026 — ChatGPT
Asked: I am starting Stage 1 (DFS). I am using a `std::vector` as a stack. I know that `push_back` adds to the end and `pop_back` removes from the end (LIFO). But if I push the neighbours in the N, E, S, W order, why does the algorithm explore West first?
Got:   Explained that because a stack is Last-In-First-Out, the last item pushed is the first one popped. Since West is pushed last (N, E, S, W), West ends up on top of the stack and gets popped first.
Changed by me: I tested this by writing a small `for` loop to push N, E, S, W and printed the stack. I saw that West was at the back of the vector. I made sure my loop iterates from 0 to 3 using the `DR[]` and `DC[]` arrays so the order is exactly correct.

## 23 October 2026 — ChatGPT
Asked: In DFS, why do we mark a cell as visited when we POP it from the stack, and not when we PUSH it? I remember in BFS we mark on push. What is the difference, and how does `trace::discard` fit into this?
Got:   Explained that in DFS, a cell can be pushed multiple times before it is ever popped. If we mark it on push, we might miss a shorter path or block a valid route. So we wait until it is popped to mark it. If we pop a cell and see it is already visited, it means it was a duplicate entry, so we throw it away using `trace::discard`.
Changed by me: I implemented the visited check exactly at the top of the `while` loop. If `visited[curr_idx] == true`, I call `trace::discard` and `continue` to skip it. I tested this on the `m04-rooms-35` map and saw that the `discard` hook was being called, which matched the test expectations.

## 23 October 2026 — ChatGPT
Asked: I am trying to reconstruct the path from the goal back to the start using the parent array. But when I trace it back, the start cell has a parent of -1, so my loop gets stuck or crashes. How do I write the while loop safely?
Got:   Explained that we need to store the parent inside the stack entry itself (like a struct with `cell` and `parent`). Then, when we pop the cell, we save the parent into the `parent_cells` array. When reconstructing, we loop until we reach the start cell, not until we hit -1, because the start cell's parent is -1.
Changed by me: I created a `StackItem` struct to hold both the cell and its parent. I wrote the while loop to break when the current cell equals the start cell. Then I used a `for` loop to reverse the path so it starts at the start and ends at the goal. I manually traced it on a 3x3 map to make sure it didn't include the start cell twice.

## 23 October 2026 — ChatGPT
Asked: The handout says DFS must be iterative, but my friend tried a recursive one to see what will happen and it worked on the small maps. We tried to discuss about the concept of recursion on the DFS and we knew the problem might be related to stack overflow since it is the common problem on recursions. Is really that? Why does the project explicitly forbid recursion? What happens if I use it?
Got:   Explained that one of the test maps (`e02-corridor-41`) is a single corridor 798 cells long. A recursive DFS would call the function 798 times, which would overflow the call stack and crash the program. Also, `maxFrontier` is meant to measure the memory of the frontier, and if the frontier is the call stack, there is nothing to measure.
Changed by me: I made sure my implementation is completely iterative using a `std::vector` as the stack. I ran the test on the `e02-corridor-41` map specifically to confirm it doesn't crash.

## 23 October 2026 — ChatGPT
Asked: I finished BFS, but I noticed a huge difference from DFS. In DFS, my `maxFrontier` on the `m09-open-61` map was huge. In BFS, the `maxFrontier` is much smaller. Why does BFS grow like a ring while DFS grows like a line? And what exactly happens if I mark cells as seen when I pop them (like in DFS) instead of when I push them?
Got:   Explained that BFS explores layer by layer, so the frontier is the boundary of the explored area (like a ring). DFS dives deep, so it holds almost the whole map on its stack. If I marked cells on pop in BFS, the same cell would be queued multiple times by different neighbours, causing the queue size (and `maxFrontier`) to explode to the size of the whole map, failing the `[frontier]` test.
Changed by me: I made sure to mark `seen[nbr_idx] = true` exactly inside the `if` block before I pushed it to the queue. I ran `make test-bfs` and checked the `[frontier]` test output to confirm my `maxFrontier` was small and the ring shape was correct.

## 23 October 2026 — ChatGPT
Asked: How does BFS guarantee the shortest path in steps? I understand the queue is FIFO, but can you write out the proof as if I am inventing it? And why does this proof break the moment we introduce terrain costs in Stage 3?
Got:   Explained that because it's a FIFO queue, everything at distance \( d \) from the start is discovered before anything at distance \( d+1 \). So the first time we reach the goal, it is guaranteed to be the shortest path. The proof breaks in Stage 3 because "distance" stops meaning "number of steps" and starts meaning "total cost", so the first path found is not necessarily the cheapest.
Changed by me: I wrote this proof down in my notebook. I also added comments in my code above the `while` loop explaining that BFS explores in concentric rings. This made me realize why the `[optimal]` test exists.

## 23 October 2026 — ChatGPT
Asked: In DFS, I had `trace::discard` for stale entries. But in my BFS code, I don't have any discards and the tests pass. Why does BFS never have discards? Is there ever a case where I need to discard in BFS?
Got:   Explained that because we mark cells as seen on *push*, a cell is only ever pushed into the queue exactly once. Therefore, when we pop a cell, it is impossible for it to be a duplicate. In DFS, we push duplicates (because we mark on pop), so we need to discard the stale copies.
Changed by me: I verified this by checking the `[frontier]` test in `tests/test_bfs.cpp`, which has a specific test called "nothing is discarded because nothing is queued twice". I confirmed my code has no `trace::discard` calls and passed the test.

## 23 October 2026 — ChatGPT
Asked: I noticed a test called `[optimal]` checks if `r.path.size() - 1 == fewestSteps`. What is the `fewestSteps` function doing in the test harness? Why is it so important that it matches my BFS answer exactly?
Got:   Explained that `fewestSteps` is an independent "oracle" (a separate BFS written by the lecturers) inside `harness.h`. It is used to verify that my BFS actually found the true minimum steps, not just a valid path. The test ensures my code isn't just returning a random path, but the mathematically shortest one.
Changed by me: I looked at `fewestSteps` in the test file. I traced it manually on a small 3x3 map and saw it uses a `std::deque` instead of `std::queue`. It gave me confidence that my code is correct because two different implementations of BFS (mine and the oracle's) return the exact same step count.

## 24 October 2026 — ChatGPT
Asked: I am starting Stage 3 (Dijkstra) and it is very hard. Please create HTML notes for me to read. I don't understand the `std::priority_queue`. How does it sort the smallest cost first? Can you put diagrams in the HTML notes so I can see how the heap works? Please use simple English.
Got:   Created HTML notes with a step-by-step diagram of the min-heap. Showed me the `struct` and `operator()` using full `if/else` so I can read it easily.
Changed by me: I read the HTML notes on my laptop. I drew the heap tree on paper to see how the numbers go up and down. Then I typed the `CompareState` struct into my code. I tested it and the smallest cost comes out first.

## 24 October 2026 — ChatGPT
Asked: The HTML notes explain the tie-breaking rule, but I am still a little bit confused. How do I write the code so that if the cost is the same, it picks the smaller row, and then the smaller column? Can you add a table to the notes showing this?
Got:   Added a new section to the HTML notes with a table showing the ties. Showed me the code step by step using standard `if` statements.
Changed by me: I looked at the table in the notes. I traced a 3x3 map on paper to see why row 1 comes out before row 2. I put the `if` statements in my code. My search now follows the exact order the project wants.

## 24 October 2026 — ChatGPT
Asked: I read the notes about "lazy deletion". I am very confused. The handout says `priority_queue` cannot change a value already inside. The notes say we must push a new entry and throw away the old one. Can you add a diagram to the HTML notes showing the old entry and the new entry inside the queue?
Got:   Updated the HTML notes with a diagram showing the queue with two entries for the same cell (one old, one new). Explained the `settled` array and how `trace::discard` is used.
Changed by me: I looked at the diagram and saw the old entry is like a duplicate. I added the `settled` check in my code. I ran the test and saw the `discard` events happen when the old entry comes out.

## 24 October 2026 — ChatGPT
Asked: In the HTML notes, it says Dijkstra has "no relax" events. I am very confused. If we find a cheaper path, why is there no relax? Please explain in simple English and add a flow diagram in the notes.
Got:   Explained the math simply. Because we pay when we enter a cell, the first time we pop the cell from the queue, it is already the cheapest. Added a flow diagram in the HTML notes showing why it cannot be cheaper later.
Changed by me: I read the explanation three times. I drew the cost on paper to see it myself. I checked my code and made sure I don't call `trace::relax` at all. The test `this cost model never needs a cell to be updated` passed.

## 24 October 2026 — ChatGPT
Asked: The tests say Dijkstra must take more steps than BFS on terrain maps. But in my head, the best path should also be the shortest? Why does it take more steps? The HTML notes didn't explain this enough.
Got:   Added a new section to the HTML notes with a picture of a small map. Showed how BFS goes straight through the high numbers (cost 9) and Dijkstra goes around them.
Changed by me: I ran `./run bfs t02` and `./run dijkstra t02` in my terminal. I watched the screen. I saw BFS go straight through the red block, and Dijkstra take a longer way around. I understand now that fewer steps is not always cheaper.

## 24 October 2026 — ChatGPT
Asked: My code crashes when I try to trace the path back. The start cell parent is (-1, -1) and my loop breaks. How do I fix this? Can you update the HTML notes with a picture of the parent array?
Got:   Showed me the `if` check for `-1`. Added a simple diagram of the parent array to the HTML notes so I can see the chain from the goal to the start.
Changed by me: I added the check `if (p.r == -1 && p.c == -1)` inside my `while` loop. I ran the test for the `e03-adjacent-11` map (start next to goal) and it doesn't crash anymore.

## 25 October 2026 — ChatGPT
Asked: I am starting Stage 4 (A*). I am very confused about the formula `f = g + h`. Can you make HTML notes with diagrams to explain what `g` and `h` mean? Also, why is Manhattan distance the correct `h`?
Got:   Created HTML notes with a picture of a grid showing `g` (cost so far), `h` (straight line distance to goal), and `f` (the total). Explained that Manhattan distance is `|r1 - r2| + |c1 - c2|` and showed why it never overestimates.
Changed by me: I read the notes and drew the `g`, `h`, `f` on a small map on paper. I wrote the `h` function in my code using standard `if` statements and `std::abs` math. I tested it on a 3x3 open room and the search went straight to the goal.

## 25 October 2026 — ChatGPT
Asked: I am confused about the `dist` array. In Dijkstra, `dist` was the total cost. In A*, the notes say I must keep `dist` as only `g`, not `f`. But the priority queue uses `f`. Why? If I put `f` in `dist`, my test fails with a wrong cost.
Got:   Explained that `dist` must hold the real cost to enter the cell (`g`). If we put `f` in `dist`, we are adding the guess to the real cost, and the final path cost will be wrong. Added a diagram in the HTML notes showing the difference between the queue (which uses `f`) and the `dist` array (which uses `g`).
Changed by me: I changed my code to save `new_g` in the `dist` array, but I used `new_g + h(...)` for the priority queue. I tested it and my final cost is now correct.

## 25 October 2026 — ChatGPT
Asked: In Dijkstra, I never used `trace::relax`. But the test for A* says "unlike Dijkstra A* does find cheaper routes to known cells". Why does A* need a relax event? Please explain with a drawing in the HTML notes.
Got:   Added a new section to the notes with a drawing. Explained that because A* pops by `f` (g + h), it might pop a cell with a high `g` but very low `h` before a cell with a low `g` but high `h`. This means we can find a cheaper path to a cell that is already on the frontier. The `relax` hook fires when this happens.
Changed by me: I added a check `if (dist[nbr_idx] < INF)` to know if the cell was already on the frontier. If it was, I call `trace::relax`. I ran the test and saw the relax events were happening.

## 25 October 2026 — ChatGPT
Asked: What does "admissible" mean? The handout says Manhattan distance is admissible. Does that mean it is always correct? What happens if I use a different formula that overestimates?
Got:   Explained that admissible means the guess is never bigger than the real cost. If it overestimates, A* might think a path is too expensive and skip it, even if it is the cheapest path. So the search would return a wrong answer. Added a drawing of an overestimating guess in the notes.
Changed by me: I wrote a comment in my code above the `h` function explaining that Manhattan distance is admissible because you can only move N, E, S, W. I checked the test `Manhattan distance never overestimates` and it passed.

## 25 October 2026 — ChatGPT
Asked: My code is failing the `[work]` test that says "on an open map it commits to far fewer cells than Dijkstra". My `expanded` is almost the same as Dijkstra. Why is my search not saving work? Is my `h` returning 0 by mistake?
Got:   Explained that if `h` returns 0, it is just Dijkstra. I should check my `h` function to make sure it is calculating the distance. Also, the tie-breaking in the queue must be exactly correct, otherwise it spreads out like Dijkstra.
Changed by me: I printed my `h` value for the start cell and saw it was not 0. I looked at my `CompareState` struct and saw I forgot to add the tie-breaking for the row and column. I fixed it, and my `expanded` dropped below 880 on the cave map. The test passed.
