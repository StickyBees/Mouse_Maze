// This program simulates a mouse searching through a maze to find the exit.
// The maze will be displayed as well as the mouses movements through the maze, keeping track of all the paths explored.
// Once solved, the fastest route and the time it took to compute moving through the maze (not including printing) is displayed.

#include <windows.h>		// for sleep()
#include <iostream>
#include <conio.h>
#include <assert.h>
#include <vector>
#include <string>
#include <stack>
#include <utility>
#include <list>

#include "cMaze.h"
#include "cTimer.h"
#include "cVector2.h"
#include "Tile.h"
#include "Branch.h"
#include "Mouse.h"
#include "CustomMaze.h"

const int UP_KEY = 72;
const int DOWN_KEY = 80;
const int ESC_KEY = 27;

int GetPositiveIntFromUser(std::string message);

enum Direction
{
	RIGHT = 0,
	DOWN = 1,
	LEFT = 2,
	UP = 3
};

int main()
{
	while (true)
	{
		int seed = -1;

		const int PRINT_EXPLORATION_SLEEP = 100; // the amount of time in ms to sleep between printing each move the mouse makes
		const int PRINT_EXPLORATION_SLEEP_MOD = 10; // the amount of time in ms that printExplorationSleepAmount is modified each key press
		const int PRINT_ROUTE_SLEEP = 50; // the amount of time in ms to sleep between printing each tile when maze solved
		const int PRINT_MAZE_SLEEP = 4000; // the amount of time in ms to sleep before showing the maze

		bool showExploration = true;
		int printRouteSleepAmount = PRINT_ROUTE_SLEEP;

		{
			// Extract whether player wants to use a seed
			int choice = -1;
			bool isChoiceValid = false;

			do
			{
				choice = GetPositiveIntFromUser("Do you want to use a seed to generate the maze?\n(0) Yes\n(1) No");
				switch (choice) {
				case 0:
					std::cout << std::endl;
					seed = GetPositiveIntFromUser("Enter a seed to create the maze with: ");
					isChoiceValid = true;
					break;

				case 1:
					seed = -1;
					isChoiceValid = true;
					break;

				default:
					std::cout << "Please enter either 0 or 1." << std::endl << std::endl;
					isChoiceValid = false;
					break;
				}
			} while (!isChoiceValid);

			choice = -2;
			isChoiceValid = false;

			std::cout << std::endl;

			do
			{
				choice = GetPositiveIntFromUser("Do you want to see the mouse explore the maze?\n(0) Yes\n(1) No");
				switch (choice) {
				case 0:
					showExploration = true;
					printRouteSleepAmount = PRINT_ROUTE_SLEEP;

					std::cout << std::endl << "Press the UP arrow or DOWN arrow to speed the mouse up or slow the mouse down";
					Sleep(PRINT_MAZE_SLEEP);

					isChoiceValid = true;
					break;

				case 1:
					showExploration = false;
					printRouteSleepAmount = 0;
					isChoiceValid = true;
					break;

				default:
					std::cout << "Please enter either 0 or 1." << std::endl << std::endl;
					isChoiceValid = false;
					break;
				}
			} while (!isChoiceValid);
		}

		// create a maze variable and create a new maze
		CustomMaze	maze;
		maze.createRandomSize(seed);

		// print the maze
		maze.print();

		// Display timing updates
		std::cout << std::endl << std::endl << "Limit =  " << maze.getTimeLimit_ms() << " ms" << std::endl;
		std::cout << std::endl << "Press a key to start the mouse ...";

		std::cin.clear();
		(void)_getch();
		std::cin.clear();

		// create a timer to keep track of elapsed time
		cTimer	timer;

		// create the "mouse"
		Mouse	mouse;

		// ensure the maze was created successfully with a start point
		cVector2 startPos;
		maze.getStart(startPos);

		// Fill the maze grid & convert from mazeRowsStrings[ROW][COLUMN] to maze.grid[COLUMN][ROW]
		{
			const std::vector<std::string> mazeRowsStrings = maze.getStrings(); // access with mazeRowStrings[ROW][COLUMN] or mazeRowsStrings[y pos][x pos]

			maze.grid.reserve(mazeRowsStrings[0].size());
			maze.grid.resize(mazeRowsStrings[0].size());

			for (unsigned int column = 0; column < mazeRowsStrings[0].size(); column++)
			{
				maze.grid[column].reserve(mazeRowsStrings.size());
				maze.grid[column].resize(mazeRowsStrings.size());
				for (unsigned int row = 0; row < mazeRowsStrings.size(); row++)
				{
					char displayChar = mazeRowsStrings[row][column];
					maze.grid[column][row].displayChar = displayChar;
					maze.grid[column][row].pos = cVector2(column, row);

					if (displayChar == maze.getExitChar())
					{
						maze.grid[column][row].isExit = true;
					}
				}
			}
		}

		// Setup the mouse in the maze
		mouse.MoveTo(maze.grid[startPos.x][startPos.y], maze);

		// Setup the branch stack
		std::stack<Branch> branchStack;
		Branch startBranch;
		startBranch.tiles.push(maze.grid[startPos.x][startPos.y]);
		branchStack.push(startBranch);

		int printExplorationSleepAmount = PRINT_EXPLORATION_SLEEP;

		HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO buffer;
		COORD startingOutputPos{ 0, 0 };

		if (GetConsoleScreenBufferInfo(output, &buffer))
		{
			startingOutputPos = buffer.dwCursorPosition;
			startingOutputPos.X = 0;
		}

		do
		{
			// start keeping time
			timer.start();

			// make the mouse move - REPLACE WITH YOUR CODE

			if (mouse.isRetracingSteps || !mouse.ExploreMaze(mouse, maze, branchStack))
			{
				if (!mouse.FoundCheese())
				{
					mouse.RetraceSteps(mouse, maze, branchStack);
				}
			}

			// stop keeping time
			timer.stop();

			if (showExploration)
			{
				// update screen
				if (!mouse.FoundCheese())
				{
					mouse.PrintPosition(maze, branchStack);
				}

				// Display timing updates
				SetConsoleCursorPosition(output, startingOutputPos);
				std::cout << "Elapsed =  " << timer.getElapsed_ms() << " ms                       ";

				// speed up or slow down mouse if user presses up or down arrow key
				if (_kbhit())
				{
					char keyPress = _getch();
					if (DOWN_KEY == keyPress)
					{
						printExplorationSleepAmount += PRINT_EXPLORATION_SLEEP_MOD;
					}
					else if (UP_KEY == keyPress)
					{
						printExplorationSleepAmount -= PRINT_EXPLORATION_SLEEP_MOD;
						printExplorationSleepAmount -= PRINT_EXPLORATION_SLEEP_MOD;
						if (printExplorationSleepAmount < 0)
						{
							printExplorationSleepAmount = 0;
						}
					}
				}

				// short delay between moves to make them visible
				Sleep(printExplorationSleepAmount);

				// Quit if Escape is pressed.
				if (_kbhit())
				{
					if (ESC_KEY == _getch())
						return 0;
				}
			}

		} while (!mouse.FoundCheese());

		maze.PrintRoute(branchStack, printRouteSleepAmount);

		double elapsed = timer.getElapsed_ms();
		std::cout << std::endl << "Total Elapsed =  " << elapsed << " ms";
		std::cout << std::endl << std::endl << "Press ESC key to exit or press any other key to test another maze";

		std::cin.clear();
		
		// Quit if Escape is pressed. Restart if any other key pressed.
		if (ESC_KEY == _getch())
		{
			return 1;
		}
		std::cin.clear();
		system("CLS");
	}
}

//Prints message to console.Returns a positive integer input by user.
int GetPositiveIntFromUser(std::string message)
{
	int result = NULL;
	std::string userInput = "defualt";

	do
	{
		std::cout << message << std::endl;
		std::getline(std::cin, userInput);
		std::cin.clear();
		fflush(stdin);

		if (userInput.find_first_not_of("1234567890") == -1)
		{
			try
			{
				result = stoi(userInput);
				return result;
			}
			catch (const std::invalid_argument & ia)
			{
				std::cout << ia.what() << "is an invalid argument. Please enter an integer value.\n" << std::endl;
			}
			catch (const std::out_of_range & oor)
			{
				std::cout << oor.what() << " is out of range. Please enter a smaller value.\n" << std::endl;

			}
		}
		else
		{
			std::cout << "Please enter a positive number.\n" << std::endl;
		}
	} while (true);
}
