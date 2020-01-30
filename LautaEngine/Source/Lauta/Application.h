#pragma once
#include <vector>
#include "ECS/ECS.h"
#include "SDL.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/features2d.hpp>

class Application {

public:

	Application() {};
	~Application() {};

	bool initEngine();	//initialize everything regarding SDL

	void InitApplication(); //initialize everything needed for the application

	bool Running() { return m_IsRunning; } //check if the application is running

	void HandleEvents(); //handles all input events from mouse, keyboard etc.

	void Update(); //Handles data updates

	void Render(); //handles all rendering that happens on screen

	void Cleanup(); //cleans up any resource loaded	

	enum groupLabels : Group
	{
		groupShip,

		groupUI,
		groupUIButton,
		groupUIDice,
		groupUIGold,
		groupUICannon,

		groupArea,
		groupOcean,
		groupTrees,
		groupSheep,
		groupWheat,
		groupBrick,
		groupSteel,
		groupDesert,

		groupSafeSpace,
		groupTrapSpace,

		groupEffect
	};
	
	void ChangeTurn();
	void startMenu();
	void checkVictory(Entity* s);
	void resetGame();

	void sortPlayerTurnOrder();

	void handleUIButtons();

	void changeAmountText(int amount);
	



	void checkPlayerDeath();
	void ShipBattle(Entity* shipLeft, Entity* shipRight);

	cv::Mat changeImage(cv::Mat image);
	int countPips(cv::Mat dice);
	std::vector<int> DiceCameraCaptureCalculation(int requiredDice);

private:


	bool mMinimized = false;
	bool windowFull = false;
	bool m_IsRunning = false;	
	bool debugMode = false;


	
	bool PlayerSelect = true;

};

