#include <Windows.h>
#include "Game.h"
#include <Eigen/Eigen>
#include <Eigen/Core>
#include <Eigen/LU>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR cmdLine, int cmdShow)
{
	//Creates an instance of game
	Game game;
	
	//Initialise the game if this fails then return to quit the program
	if(!game.Initialise(hInstance, pInstance, cmdLine, cmdShow))
	{
		return -1;
	}
	//Run the game, this contains the game loop
	game.Run();
	
	//After the loop exits call the shutdown to deallocate everything
	game.Shutdown();

	return static_cast<int>(game.GetSystem().GetMsg().wParam);
}