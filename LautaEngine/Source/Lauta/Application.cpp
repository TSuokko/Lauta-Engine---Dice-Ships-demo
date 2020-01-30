#include "Application.h"
#include "Globals.h"
#include "ECS/EventManager.h"
#include "Gameplay/GameObjects.h"
#include "Collision.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/features2d.hpp>


//Global SDL structs 
SDL_Event Globals::Global_Event;
SDL_Window* Globals::Global_Window = nullptr;
SDL_Renderer* Globals::Global_Renderer = nullptr;
SDL_Rect Globals::Camera;

//All managers needed for the engine
Manager manager;	
AssetManager* AssetManager::Assets = new AssetManager(&manager);
EventManager *myEventManager = EventManager::Instance();

//Game objects
PlayerShip *blue = nullptr;
PlayerShip *yellow = nullptr;
PlayerShip *red = nullptr;
PlayerShip *green = nullptr;

UIButton *TurnButton = nullptr;
UIButton *DiceButton = nullptr;
UIButton *MapSlider = nullptr;
UIButton *ResetButton = nullptr;
UIButton *EndGameButton = nullptr;


Entity* battlePlayer1 = nullptr;
Entity* battlePlayer2 = nullptr;
Entity* battleSpace = nullptr;
Entity* clickedArea = nullptr;
Entity* activePlayer = nullptr;

//UI entity
auto& UI(manager.addEntity());
auto& Camera(manager.addEntity());
auto& Explosion(manager.addEntity());
auto& DiceTray(manager.addEntity());
//adding groups into the manager
auto& ships(manager.getGroup(Application::groupShip));
auto& areas(manager.getGroup(Application::groupArea));
auto& ui(manager.getGroup(Application::groupUI));
auto& uiDice(manager.getGroup(Application::groupUIDice));
auto& uiGold(manager.getGroup(Application::groupUIGold));
auto& uiCannon(manager.getGroup(Application::groupUICannon));
auto& buttons(manager.getGroup(Application::groupUIButton));
auto& effect(manager.getGroup(Application::groupEffect));

//global variables
int maxPlayerNumber = 4;
int playerTurn = maxPlayerNumber;
const int healthDiceAmount = 3;
const int areaBorder = 250;
int diceResult = 0;
int clickAmount = 0;
int battleDice1 = 0;
int battleDice2 = 0;
int turnNumber = 0;
Vector2D clickPoint = Vector2D(0, 0);
int alarm = 0;

bool gameWon = false;				//game has been won
bool onSpace = false;				//when the ship has moved to a new space
bool alarmSet = false;				
bool clickState = false;			//state of the mouse button
bool cursorState = false;			//show cursor on screen
bool sliderState = false;			//if the slider is clicked
bool goldClicked = false;			//if the gold is clicked
bool playerMoved = false;			//if the player has moved to a new space
bool secondAttack = false;			//if it's the second battling player's turn to distribute damage
bool battleActive = false;			//if two players are occupying the same space
bool tutorialState = false;			//if the tutorial text should be shown
bool healthDecrease = true;			//if the current tile is decresing health
bool damageCalculated = false;		//if damage has been calculated before changing the turn
bool diceButtonClicked = false;		//if a dice button is clicked
bool BattleCalculation = false;		//if battle damage has been calculated
bool battleButtonClicked = false;	//if the battle button is clicked 

bool firstDiceThrownCheck = false;
bool secondDiceRoll = false;

bool debugShowWebcamRecording = false;

bool bothRolled = false;

bool RandomDiceMode = false;

bool diceRead = false;

bool textDrawn = false;

bool changeAmountFlag = false;

SDL_Color colorRed = { 220,50,0,255 };
SDL_Color colorBlue = { 0,191,255, 255 };
SDL_Color colorGreen = { 50,205,50,255 };
SDL_Color colorWhite = { 255,255,255,255 };
SDL_Color colorYellow = { 225,220,0,255 };

//List of all tiles in game, this is used ot iterate through all the game tiles
static const Application::groupLabels AllTiles[] = {

		Application::groupOcean,
		Application::groupTrees,
		Application::groupWheat,
		Application::groupBrick,
		Application::groupSheep,
		Application::groupDesert,	
		Application::groupSteel

};


//Initialize everything regarding the app, that needs to be done once. 
void Application::InitApplication()
{
	//adding assets to the engine
	AssetManager::Assets->AddTextureXML("ships","Assets/Spritesheets/ships.xml");
	AssetManager::Assets->AddTextureXML("dice", "Assets/Spritesheets/diceWhite.xml");
	AssetManager::Assets->AddTextureXML("diceR", "Assets/Spritesheets/diceRed.xml");
	AssetManager::Assets->AddTextureXML("diceG", "Assets/Spritesheets/diceGreen.xml");
	AssetManager::Assets->AddTextureXML("diceB", "Assets/Spritesheets/diceBlue.xml");
	AssetManager::Assets->AddTextureXML("diceY", "Assets/Spritesheets/diceYellow.xml");
	AssetManager::Assets->AddTexture("hex", "Assets/Spritesheets/HexTiles.png");
	AssetManager::Assets->AddTexture("explosion", "Assets/Spritesheets/Explosion.png");
	AssetManager::Assets->AddTexture("DiceTray",  "Assets/Spritesheets/DiceTray.png");
	AssetManager::Assets->AddFont("8bit", "Assets/Fonts/PrStart.ttf", 24 * GLOBAL_SCALE);
	AssetManager::Assets->AddTextureXML("uiBlack", "Assets/Spritesheets/uiBlack.xml");
	AssetManager::Assets->AddTextureXML("uiWhite", "Assets/Spritesheets/uiWhite.xml");



	//Player Game Objects
	blue = new PlayerShip("ships", "shipBlue.png", groupShip, Vector2Di(SCREEN_WIDTH + 200, 1600), 1 , colorBlue);
	blue->addFlavorText("Player Blue", "name");
	yellow = new PlayerShip("ships", "shipYellow.png", groupShip, Vector2Di(SCREEN_WIDTH + 400, 1600), 2, colorYellow);
	yellow->addFlavorText("Player Yellow", "name");
	red = new PlayerShip("ships", "shipRed.png", groupShip, Vector2Di(SCREEN_WIDTH + 600, 1600), 3, colorRed);
	red->addFlavorText("Player Red", "name");
	green = new PlayerShip("ships", "shipGreen.png", groupShip, Vector2Di(SCREEN_WIDTH + 800, 1600), 4, colorGreen);
	green->addFlavorText("Player Green", "name");

	Camera.addComponent<CameraComponent>(Vector2Di(SCREEN_WIDTH, SCREEN_HEIGHT / 2),
		Vector2Di(SCREEN_WIDTH, SCREEN_HEIGHT), Vector2Di(0, 0));

	Explosion.addComponent<TransformComponent>();
	Explosion.addComponent<SpriteComponent>("explosion", Vector2Di(74,75), Vector2Di(0, 0), 0.70f);
	Explosion.addComponent<AnimationComponent>(5, 140, false);
	Explosion.addGroup(Application::groupEffect);

	DiceTray.addComponent<TransformComponent>();
	DiceTray.addComponent<SpriteComponent>("DiceTray", Vector2Di(640, 360), Vector2Di(0,0), 1);


	//UI initialization and adding flavor text
	UI.addComponent<UIComponent>("Assets/Maps/UI.tmx", Application::groupUI, 4);
	UI.addComponent<UILabel>(Vector2Di(100, 1500), 
		"All other players have been defeated!", "8bit", "victory", Vector2Di(650, 600), colorWhite);


	UI.GetComponent<UILabel>().AddTextToScreenplay("ocean", 
		"You are sailing on smooth seas", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("trees", 
		"Welcome to Repair Island! You can fix your ship up to 2 HP!", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("sheep",
		"We've found treasure! Now you have gold in your inventory."
		"\n\nWith gold you can exchange it for double effetcs in positive spaces."
		"\n\nYou can also give it to an opponent to negate an attack from the ambushing player!", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("wheat", 
		"A leak was found in your ship! Throw 1 dice.", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("desert", 
		"Someone set up a bomb in the diner, throw 2 dice.", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("brick", 
		"Enemy bandits are shooting you from a far, throw 3 dice.", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("steel", 
		"Welcome to the Weapons black market. Have yourself a cannon!"
		"\nYou will now do more damage to other players, and you can protect yourself from traps.", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("intro",
		"Welcome to Dice Ships! \n\nHow many players will be playing? "
		"\n\nYou can click on the ships to have fewer players."
		"\n\nWhen you are ready, start the game by clicking the white button", "8bit");
	UI.GetComponent<UILabel>().AddTextToScreenplay("tutorial",
		"Players move by clicking one of the two tiles in front of their ship.\n\n"
		"Players will encounter traps and other players for ship battles.\n"
		"Damage is calculated by clicking your ships dice.\n\n"
		"When your turn is done, click the white button.","8bit");


	UI.GetComponent<UILabel>().AddTextToScreenplayWithNewPosition("Amount", "Throw this many dice", "8bit", Vector2Di(400, 575));
	
	//buttons for the UI
	TurnButton = new UIButton("uiWhite", "buttonStart.png", groupUIButton, Vector2Di(900, 1310));
	TurnButton->transform->scale = 2;
	DiceButton = new UIButton("uiBlack", "fightFist_circle.png", groupUIButton, Vector2Di(805, 1340));
	MapSlider = new UIButton("uiWhite", "scrollVertical.png", groupUIButton, Vector2Di(8, SCREEN_HEIGHT/4));
	MapSlider->transform->height *= 4;

	EndGameButton = new UIButton("uiWhite", "cross.png", groupUIButton, Vector2Di(0, 0));
	EndGameButton->transform->scale = 2;
	ResetButton = new UIButton("uiWhite", "return.png", groupUIButton, Vector2Di(SCREEN_WIDTH - 75, 0));
	ResetButton->transform->scale = 2;

}

void Application::resetGame()
{
	Globals::Camera.x = SCREEN_WIDTH;
	Globals::Camera.y = SCREEN_HEIGHT;

	int i = 1;
	for (auto s : ships)
	{
		s->GetComponent<TransformComponent>().position = Vector2D(SCREEN_WIDTH + 200 * i, 1600);
		s->GetComponent<HealthComponent>().resetHealth();
		s->GetComponent<TurnComponent>().changeTurnID(i);
		s->GetComponent<GoldComponent>().resetGold();
		s->GetComponent<CannonComponent>().resetCannons();
		s->GetComponent<SpriteComponent>().changeSpriteAngle(180);

		for (int j = 0; j < 3; j++)
		{
			s->GetComponent<HealthComponent>().getHealthBarSprite(j)->
				changeSourceTexture("dieWhite4.png");
		}

		i++;
	}

	for (auto a : areas)
	{
		a->deleteGroup(Application::groupArea);
	}

	//global variables
	maxPlayerNumber = 4;
	playerTurn = maxPlayerNumber;
	diceResult = 0;
	clickAmount = 0;
	battleDice1 = 0;
	battleDice2 = 0;
	turnNumber = 0;
	clickPoint = Vector2D(0, 0);
	alarm = 0;

	gameWon = false;				//game has been won
	onSpace = false;				//when the ship has moved to a new space
	alarmSet = false;
	clickState = false;				//state of the mouse button
	cursorState = false;			//show cursor on screen
	sliderState = false;			//if the slider is clicked
	goldClicked = false;			//if the gold is clicked
	playerMoved = false;			//if the player has moved to a new space
	secondAttack = false;			//if it's the second battling player's turn to distribute damage
	battleActive = false;			//if two players are occupying the same space
	tutorialState = false;			//if the tutorial text should be shown
	healthDecrease = true;			//if the current tile is decresing health
	damageCalculated = false;		//if damage has been calculated before changing the turn
	diceButtonClicked = false;		//if a dice button is clicked
	BattleCalculation = false;		//if battle damage has been calculated
	battleButtonClicked = false;	//if the battle button is clicked 

	changeAmountFlag = false;
	firstDiceThrownCheck = false;
	secondDiceRoll = false;
	textDrawn = false;
	bothRolled = false;
	PlayerSelect = true;
}


void Application::startMenu()
{
	//toggle active players
	for (auto& s : ships)
	{
		if (s->GetComponent<ColliderComponent>().click(Globals::Global_Event.button)
			== SDL_MOUSEBUTTONDOWN)
		{
			//toggle playing state
			s->GetComponent<TurnComponent>().togglePlayingState();

			if (s->GetComponent<TurnComponent>().stillPlaying())
			{
				switch (s->GetComponent<TurnComponent>().getTurnNumber())
				{
				case 1:
					s->GetComponent<SpriteComponent>().changeSourceTexture(
						"shipBlue.png");
					break;
				case 2:
					s->GetComponent<SpriteComponent>().changeSourceTexture(
						"shipYellow.png");
					break;
				case 3:
					s->GetComponent<SpriteComponent>().changeSourceTexture(
						"shipRed.png");
					break;
				case 4:
					s->GetComponent<SpriteComponent>().changeSourceTexture(
						"shipGreen.png");
					break;
				default:
					break;
				}	
			}
			else {

				s->GetComponent<SpriteComponent>().changeSourceTexture(
					"shipBlack.png");
			}
		}
	}
	//activate the game
	switch (TurnButton->collider->click(Globals::Global_Event.button))
	{
	case SDL_MOUSEBUTTONDOWN:


		TurnButton->transform->scale = 1.8f;
		//once the tile has been clicked, the player can then change the turn
		if (PlayerSelect)
		{

			int turnNumber = 1;
			for (auto& s : ships)
			{
				if (!s->GetComponent<TurnComponent>().stillPlaying())
				{
					s->GetComponent<HealthComponent>().dropAllHealthToZero();
					s->GetComponent<TurnComponent>().changeTurnID(0);
					//go through all active players and give them new turn numbers
					maxPlayerNumber--;
					playerTurn--;
					if (playerTurn == 0)
						playerTurn = 1;
				}
				else
				{
					s->GetComponent<TurnComponent>().changeTurnID(turnNumber);
					turnNumber++;
				}
			}

			Globals::Camera.x = 0;
			Globals::Camera.y = SCREEN_HEIGHT;

			Vector2Di hexStartPos(200 + 110 * (4 - maxPlayerNumber), 100);

			std::cout << maxPlayerNumber << std::endl;

			HexBoard(Vector2Di(maxPlayerNumber, 6), Vector2Di(220, 250), hexStartPos);
			playerMoved = true;


			int i = 0;

			for (auto& s : ships)
			{
				if (s->GetComponent<TurnComponent>().stillPlaying())
				{
					s->GetComponent<TransformComponent>().position =
						Vector2D((hexStartPos.x) + (220 * i), 2100);
					i++;
				}
			}

			ChangeTurn();

			PlayerSelect = false;
			tutorialState = true;
		}
		break;
	}

}

void Application::checkVictory(Entity* s)
{
	if ((s->GetComponent<TransformComponent>().position.y < 300 || maxPlayerNumber == 1) )
	{
		if (!gameWon && s->GetComponent<TurnComponent>().stillPlaying()) 
		{
			s->GetComponent<UILabel>().RemoveTextFromScreenPlay("dice");
			s->GetComponent<UILabel>().replaceTextWithID("name", "Congratulations, you are the winner!");
			s->GetComponent<UILabel>().setPositionOfTextWithID("name", Vector2Di(100, 1400));
			if(maxPlayerNumber != 1)
				UI.GetComponent<UILabel>().replaceTextWithID("trees", "You have arrived safely to a brave new world!");
			gameWon = true;
		}
	}
}

void checkOnePlayerAlive(Entity* s)
{
	if (maxPlayerNumber == 1)
	{
		if (!gameWon && s->GetComponent<TurnComponent>().stillPlaying())
		{
			s->GetComponent<UILabel>().RemoveTextFromScreenPlay("dice");
			s->GetComponent<UILabel>().replaceTextWithID("name", "Congratulations, you are the winner!");
			s->GetComponent<UILabel>().setPositionOfTextWithID("name", Vector2Di(100, 1400));
			gameWon = true;
			onSpace = true;
		}
	}
}



void Application::sortPlayerTurnOrder()
{
	//health, turn number
	std::multimap<int, int, std::greater<int>> turnArray;
	//order the players into decending health order
	for (auto& s : ships)
	{
		turnArray.emplace(s->GetComponent<HealthComponent>().getTotalHealth(),
			s->GetComponent<TurnComponent>().getTurnNumber());
	}

	std::multimap<int, int>::iterator it;

	int i = 1;
	for (it = turnArray.begin(); it != turnArray.end(); it++)
	{	
		//order the player turn order based on the order of the map
		for (auto& s : ships)
		{
			if (s->GetComponent<HealthComponent>().getTotalHealth() == (*it).first &&
				s->GetComponent<TurnComponent>().getTurnNumber()	== (*it).second)
			{
				if ((*it).first == 0)
				{
					s->GetComponent<TurnComponent>().changeTurnID(0);
				}
				else
				{
					s->GetComponent<TurnComponent>().changeTurnID(i);
					i++;	
					break;
				}
			}
		}
	}
}

void Application::checkPlayerDeath()
{
	for (auto& s : ships)
	{
		//check all active players and check if the player lost last turn
		if (s->GetComponent<TurnComponent>().stillPlaying()
			&& s->GetComponent<HealthComponent>().checkLosingCondition())
		{
			s->GetComponent<TurnComponent>().playerLost();
			int i = 1;
			//go through all active players and give them new turn numbers
			for (auto& os : ships)
			{
				//give the losing player turn number 0 and change the sprite
				if (!os->GetComponent<TurnComponent>().stillPlaying())
				{
					os->GetComponent<TurnComponent>().changeTurnID(0);
					os->GetComponent<SpriteComponent>().changeSourceTexture(
						"shipBlack.png");
				}
				//and give the rest turn numbers in order
				else
				{
					os->GetComponent<TurnComponent>().changeTurnID(i);
					i++;
				}
			}
			maxPlayerNumber--;
			playerTurn--;
			if (playerTurn == 0)
				playerTurn = 1;
		}
	}
}

void Application::ChangeTurn()
{
	//if the player has moved to a new space
	if (playerMoved)
	{
		//add to turn number
		playerTurn++;
		turnNumber++;
		//if turn number goes over the max amount of players
		//go back to player 1's turn
		
		if (playerTurn > maxPlayerNumber)
		{
			sortPlayerTurnOrder();
			playerTurn = 1;

			for (auto& a : areas)
			{
				a->GetComponent<ColliderComponent>().setColliderState(true);
			}
		}

		checkPlayerDeath();

		for (auto& s : ships)
		{
			//check who's turn it's going to be next by comparing it to the increased turn number
			if (s->GetComponent<TurnComponent>().getTurnNumber() == playerTurn)
			{		
				//the player who's turn it's going to be next, turn their turnstate to true
				s->GetComponent<UILabel>().setPositionOfTextWithID("name", Vector2Di(100, 1400));
				s->GetComponent<TurnComponent>().changeTurnStateTo(true);

				

				//s->GetComponent<InputComponent>().changeWindowMovementState(true);
				s->GetComponent<CannonComponent>().MoveCannons(880);
				if (s->GetComponent<GoldComponent>().getGoldState())
				{
					s->GetComponent<GoldComponent>().movePosition(Vector2D(920, 1600));
				}
				//move the next player's health dice on to the UI
				for (int i = 0; i < healthDiceAmount; i++)
				{
					s->GetComponent<HealthComponent>().getHealthBarCollider(i)->
						setColliderState(true);
					s->GetComponent<HealthComponent>().getHealthBarEntity(i)->
						GetComponent<TransformComponent>().position.x = 920;
				}
			}
			else
			{
				//other players turn state is changed to false
				s->GetComponent<TurnComponent>().changeTurnStateTo(false);
				s->GetComponent<InputComponent>().changeWindowMovementState(false);
				s->GetComponent<GoldComponent>().movePosition(Vector2D(SCREEN_WIDTH, 1600));

				for (int i = 0; i < healthDiceAmount; i++)
				{
					//move their turn dice out of the way to the edge of the screen
					s->GetComponent<HealthComponent>().getHealthBarCollider(i)->
						setColliderState(true);
					s->GetComponent<HealthComponent>().getHealthBarEntity(i)->
						GetComponent<TransformComponent>().position.x = SCREEN_WIDTH;
				}
			}	
		}
		//reset to base state
		clickAmount = 0;
		battleDice1 = 0;
		battleDice2 = 0;
		onSpace = false;
		playerMoved = false;
		goldClicked = false;
		battleActive = false;
		secondAttack = false;
		damageCalculated = false;
		BattleCalculation = false;
		battleButtonClicked = false;
		alarmSet = false;
		diceRead = false;
		bothRolled = false;
		firstDiceThrownCheck = false;
		secondDiceRoll = false;
		textDrawn = false;
		changeAmountFlag = false;

	}

}





void Application::handleUIButtons()
{
	switch (TurnButton->collider->click(Globals::Global_Event.button))
	{
	case SDL_MOUSEMOTION:
		TurnButton->transform->scale = 2.1f;
		break;
	case SDL_MOUSEBUTTONDOWN:
		TurnButton->transform->scale = 1.8f;
		//once the tile has been clicked, the player can then change the turn
		if (damageCalculated)
		{
			ChangeTurn();
		}
		break;
	default:
		TurnButton->transform->scale = 2.0f;
		break;
	}


	switch (DiceButton->collider->click(Globals::Global_Event.button))
	{
	case SDL_MOUSEMOTION:
		DiceButton->transform->scale = 1.1f;
		break;
	case SDL_MOUSEBUTTONDOWN:
		DiceButton->transform->scale = 0.8f;
		if(onSpace)
			battleButtonClicked = true;
		if (firstDiceThrownCheck)
			secondDiceRoll = true;
		break;
	default:
		DiceButton->transform->scale = 1.0f;
		break;
	}


	switch (MapSlider->collider->click(Globals::Global_Event.button))
	{
	case SDL_MOUSEBUTTONDOWN:
		sliderState = true;
		break;
	case SDL_MOUSEMOTION:
		if (sliderState)
		{
			MapSlider->transform->position.y += Globals::Global_Event.motion.yrel;
			Globals::Camera.y -= Globals::Global_Event.motion.yrel * 2;
			MapSlider->sliderPositionUpdate();
		}
		break;
	default:
		sliderState = false;
		break;
	}

	switch (ResetButton->collider->click(Globals::Global_Event.button))
	{
	case SDL_MOUSEMOTION:
		ResetButton->transform->scale = 2.1f;
		break;
	case SDL_MOUSEBUTTONDOWN:
		ResetButton->transform->scale = 1.8f;
		resetGame();
		break;
	default:
		ResetButton->transform->scale = 2.0f;
		break;
	}

	switch (EndGameButton->collider->click(Globals::Global_Event.button))
	{
	case SDL_MOUSEMOTION:
		EndGameButton->transform->scale = 2.1f;
		break;
	case SDL_MOUSEBUTTONDOWN:
		Globals::Global_Event.type = SDL_QUIT;
		m_IsRunning = false;
		break;
	default:
		EndGameButton->transform->scale = 2.0f;
		break;
	}

	
}




void Application::HandleEvents() 
{
	//Wait for events and give them to the Global_Event
	SDL_PollEvent(&Globals::Global_Event);
	//check what event is happening
	switch (Globals::Global_Event.type) {
	case SDL_QUIT:
		m_IsRunning = false;
		break;
	case SDL_WINDOWEVENT:
		switch (Globals::Global_Event.window.event)
		{
		case SDL_WINDOWEVENT_MINIMIZED:
			mMinimized = true;
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			mMinimized = false;
			break;
		}
	case SDL_MOUSEBUTTONDOWN:
		SDL_Delay(150);
		clickState = true;
		break;
	case SDL_MOUSEBUTTONUP:
		clickState = false;
		break;
	case SDL_KEYDOWN:
		switch (Globals::Global_Event.key.keysym.sym)
		{
		case SDLK_RETURN:
			if (!windowFull)
			{
				SDL_SetWindowFullscreen(Globals::Global_Window, SDL_TRUE);
				windowFull = true;
			}
			else {
				SDL_SetWindowFullscreen(Globals::Global_Window, SDL_FALSE);
				windowFull = false;
			}
			break;
		case SDLK_ESCAPE:
			Globals::Global_Event.type = SDL_QUIT;
			break;
		//toggle tile visibility
		case SDLK_1:
			for (auto& a : areas)
			{
				if (!debugMode)
				{
					a->GetComponent<SpriteComponent>().changeTextureSourcePosition
					(a->GetComponent<DiceComponent>().getTileTextureCoordinates());
				}
				else
				{
					if (!a->GetComponent<DiceComponent>().revealed)
						a->GetComponent<SpriteComponent>().changeTextureSourcePosition(Vector2Di(220 * 7, 0));
				}
			}
			debugMode = !debugMode;
			break;
		//toggle cursor
		case SDLK_2:
			cursorState = !cursorState;
			SDL_ShowCursor(cursorState);
			break;
			//reset game;
		case SDLK_3:
			resetGame();
			break;
			//toggle webcam
		case SDLK_4:
			debugShowWebcamRecording = true;
			printf("meme");
			break;
		case SDLK_5:
			RandomDiceMode = true;
			break;
		default:
			break;
		}
	default:
		break;
	}
	if (alarmSet)
	{
		if (SDL_GetTicks() > alarm)
		{
			ChangeTurn();
		}
	}
}


void Application::Update()
{
	manager.refresh();
	manager.update();
	
	handleUIButtons();

	if (PlayerSelect)
	{
		startMenu();
	}
	else
	{
		DiceTray.GetComponent<TransformComponent>().changePosition(Vector2D((SCREEN_WIDTH / 5), (SCREEN_HEIGHT / 8) + Globals::Camera.y));

		//handle area clicking events
		for (auto& s : ships)
		{
			
			checkOnePlayerAlive(s);

			if (!gameWon) {
				//graphical effect to indicate if it's the players turn
				if (s->GetComponent<ColliderComponent>().click(Globals::Global_Event.button)
					== SDL_MOUSEMOTION
					&& s->GetComponent<TurnComponent>().getTurnState())
					s->GetComponent<TransformComponent>().scale = 0.75f;
				else
					s->GetComponent<TransformComponent>().scale = 0.70f;

				for (auto& a : areas)
				{

					if (Collision::AABB(
						a->GetComponent<ColliderComponent>().colliderArea,
						s->GetComponent<ColliderComponent>().colliderArea)
						&& s->GetComponent<TurnComponent>().getTurnState()
						&& !onSpace)
					{
						
						a->GetComponent<ColliderComponent>().setColliderState(false);
					}
					//skip cutscene
					if (a->GetComponent<ColliderComponent>().click(Globals::Global_Event.button)
						== SDL_MOUSEBUTTONDOWN
						&& s->GetComponent<TurnComponent>().getTurnState()
						&& playerMoved && !onSpace)
					{
						s->GetComponent<TransformComponent>().position = clickPoint;
					}


					//middle point in a hex tile
					if (!playerMoved)
					{
						s->GetComponent<UILabel>().RemoveTextFromScreenPlay("dice");
						s->GetComponent<UILabel>().RemoveTextFromScreenPlay("dice2");
						
						clickPoint = Vector2D(
							a->GetComponent<TransformComponent>().position.x
							+ a->GetComponent<TransformComponent>().width / 3,
							a->GetComponent<TransformComponent>().position.y
							+ a->GetComponent<TransformComponent>().height / 4);
					}

					if (s->GetComponent<TurnComponent>().getTurnState()
						&& playerMoved
						&& !onSpace
						&& floor(s->GetComponent<TransformComponent>().position.y) == floor(clickPoint.y)
						&& ((int)(s->GetComponent<TransformComponent>().position.x - clickPoint.x)) == 0
						)
					{
						

						clickedArea->GetComponent<SpriteComponent>().changeTextureSourcePosition
						(clickedArea->GetComponent<DiceComponent>().getTileTextureCoordinates());
						clickedArea->GetComponent<DiceComponent>().revealed = true;


						clickedArea->GetComponent<ColliderComponent>().setColliderState(true);


						onSpace = true;

					
						s->GetComponent<TransformComponent>().velocity.Zero();


						if (clickedArea->hasGroup(Application::groupOcean))
						{
							diceResult = 0;
						}
						if (clickedArea->hasGroup(Application::groupSteel))
						{
							s->GetComponent<CannonComponent>().IncreaseCannonAmount();
						}
						if (clickedArea->hasGroup(Application::groupSheep))
						{
							s->GetComponent<GoldComponent>().setGold(true);
							s->GetComponent<GoldComponent>().movePosition(Vector2D(920, 1600));
						}
						if (clickedArea->hasGroup(Application::groupTrees))
						{
							//player gains 2 Health
							healthDecrease = false;
							diceResult = 2;
						}
						else
						{
							//calculate damage
							healthDecrease = true;
						}
					}

					
					

					//if a hex is clicked, and it's the ships turn and the player has not moved
					//and lastly check if the area is a legal space to click
					//i.e. (top 2 diagonal hexagons)
					if (a->GetComponent<ColliderComponent>().click(Globals::Global_Event.button)
						== SDL_MOUSEBUTTONDOWN
						&& s->GetComponent<TurnComponent>().getTurnState()
						&& a != battleSpace
						&& !playerMoved
						&& !Collision::AABB(
							a->GetComponent<ColliderComponent>().colliderArea,
							s->GetComponent<ColliderComponent>().colliderArea)
						//check the range of the player movement from the players coordinate
						&& isPointInRange(
							Vector2D(clickPoint.x - areaBorder, clickPoint.y - (areaBorder / 2)),
							s->GetComponent<TransformComponent>().getGlobalPosition(),
							Vector2D(clickPoint.x + areaBorder, clickPoint.y + areaBorder)))
					{
					


						tutorialState = false;

						//move the player to the middle of the hex and mark that they have moved. 
						Vector2D dir = getUnitVector(s->GetComponent<TransformComponent>().position, clickPoint);

						s->GetComponent<TransformComponent>().velocity = dir;
							
						s->GetComponent<SpriteComponent>().changeSpriteAngle(getAngleOfVector(dir) - 90);

						clickedArea = a;
						activePlayer = s;

						changeAmountText(clickedArea->GetComponent<DiceComponent>().getDiceAmount());
						
						playerMoved = true;
					}
				}
			}
		}
	}
}


cv::Mat Application::changeImage(cv::Mat image)
{
	image.convertTo(image, -1, 1, 140);			//brighten up the original image
	cvtColor(image, image, cv::COLOR_BGR2GRAY);		//create a grayscale version of the image
	blur(image, image, cv::Size(10, 10));		//blur the grayscale to reduce noise
	threshold(image, image, 255, 255, cv::THRESH_OTSU);
	//Canny(image, image, 2, 4, 3, false);

	return image;
}

int Application::countPips(cv::Mat dice)
{
	// resize
	cv::resize(dice, dice, cv::Size(150, 150));

	dice = changeImage(dice);

	//cv::imshow("Dice Capture", dice);

	// floodfill
	cv::floodFill(dice, cv::Point(0, 0), cv::Scalar(255));
	cv::floodFill(dice, cv::Point(0, 149), cv::Scalar(255));
	cv::floodFill(dice, cv::Point(149, 0), cv::Scalar(255));
	cv::floodFill(dice, cv::Point(149, 149), cv::Scalar(255));

	// search for blobs
	cv::SimpleBlobDetector::Params params;

	// filter by interia defines how elongated a shape is.
	params.filterByInertia = true;
	params.minInertiaRatio = 0.5;

	// will hold our keyponts
	std::vector<cv::KeyPoint> keypoints;

	// create new blob detector with our parameters
	cv::Ptr<cv::SimpleBlobDetector> blobDetector = cv::SimpleBlobDetector::create(params);

	// detect blobs
	blobDetector->detect(dice, keypoints);


	// return number of pips
	return (int)keypoints.size();

}

cv::VideoCapture cap(0);

std::vector<int> Application::DiceCameraCaptureCalculation(int requiredDice)
{

	cv::Mat image;
	std::vector<int> DiceResults;
	bool firstCheck = true;
	int currentAmount = 0;
	int previousAmount = 0;

	while (true) {

		cap >> image;
		if (image.empty())
		{
			printf("Capture could not be performed");
		}
		//imshow("Image", image);

		cv::Mat unprocessFrame = image.clone();
		image = changeImage(image);
		std::vector<std::vector<cv::Point>> diceContours;
		std::vector<cv::Vec4i> diceHierarchy;
		cv::findContours(image.clone(), diceContours, diceHierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		std::vector<cv::RotatedRect> diceRects;

		for (int i = 0; i < diceContours.size(); i++)
		{
			cv::RotatedRect rect = minAreaRect(diceContours[i]);
			double aspect = fabs(rect.size.aspectRatio() - 1);
			if ((aspect < 0.5) && (rect.size.area() > 100) && (rect.size.area() < 7000))
			{
				bool process = true;
				for (int j = 0; j < diceRects.size(); j++)
				{
					double dist = norm(rect.center - diceRects[j].center);
					if (dist < 10) {
						process = false;
						break;
					}
				}

				if (process)
				{
					diceRects.push_back(rect);

					cv::Rect diceBoundsRect = cv::boundingRect(cv::Mat(diceContours[i]));

					cv::Mat diceROI = unprocessFrame(diceBoundsRect);

					int numberOfPips = countPips(diceROI);

					if (numberOfPips > 0) {

						// ouput debug info
						std::ostringstream diceText;
						diceText << "val: " << numberOfPips;

						// draw value
						cv::putText(unprocessFrame, diceText.str(),
							cv::Point(diceBoundsRect.x, diceBoundsRect.y + diceBoundsRect.height + 20),
							cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar::all(255), 1, 8
						);

						// draw bounding rect

						rectangle(unprocessFrame, diceBoundsRect.tl(), diceBoundsRect.br(), cv::Scalar(0, 153, 255), 2, 8, 0);

						if (firstCheck)
						{
							currentAmount += numberOfPips;
							DiceResults.push_back(numberOfPips);
							//std::cout << "Dice: #"<<DiceResults.size() << " --- "<< numberOfPips << std::endl;
						}
						else
						{
							previousAmount -= numberOfPips;
							DiceResults.clear();
							//std::cout << "Dice REMOVED, current size: "<< DiceResults.size() << std::endl;
						}
					}
				}
			}
		}


		std::ostringstream totalText;
		totalText << "Total: " << currentAmount;

		putText(unprocessFrame, totalText.str(),
			cv::Point(0, 50), cv::FONT_HERSHEY_TRIPLEX, 1.8, cv::Scalar(0, 153, 255), 3, 8);

		//total point counting update
		if (!firstCheck && previousAmount != 0)
		{
			currentAmount = 0;
			firstCheck = true;
		}
		else
		{
			firstCheck = false;
		}

		previousAmount = currentAmount;


		imshow("Dice Image", unprocessFrame);
		


		if (DiceResults.size() == requiredDice )
		{
			//std::cout <<"\n\n\nDice amount: " <<currentAmount << "\n\n\n";

			diceRead = true;

			return DiceResults;
		}
	}

	return DiceResults;
}




void Application::ShipBattle(Entity* shipLeft, Entity* shipRight)
{
	Explosion.GetComponent<TransformComponent>().position =
		shipLeft->GetComponent<TransformComponent>().position;

	alarmSet = false;

	for (auto& e : effect)
	{
		e->draw();
	}

	

	//show both player dice and their owned gold and cannons
	for (int i = 0; i < healthDiceAmount; i++)
	{
		shipLeft->GetComponent<HealthComponent>().getHealthBarEntity(i)->
			GetComponent<TransformComponent>().position.x = 800;
		shipRight->GetComponent<HealthComponent>().getHealthBarEntity(i)->
			GetComponent<TransformComponent>().position.x = 920;
	}
	if (shipLeft->GetComponent<GoldComponent>().getGoldState())
	{
		shipLeft->GetComponent<GoldComponent>().movePosition(Vector2D(800, 1600));
		shipLeft->GetComponent<GoldComponent>().getSprite()->draw();
	}
	shipLeft->GetComponent<CannonComponent>().MoveCannons(760);
	shipLeft->GetComponent<CannonComponent>().drawCannons();

	if (battleButtonClicked && !goldClicked)
	{
		//if the flag for the second attack has been activated
		if (secondAttack)
		{
			Explosion.GetComponent<TransformComponent>().position =
				shipRight->GetComponent<TransformComponent>().position;
			for (int i = 0; i < 3; i++)
			{
				//player 2 can distribute their damage
				//we're setting colliders to false, because we don't want them to be clicked
				shipLeft->GetComponent<HealthComponent>().getHealthBarCollider(i)->setColliderState(false);
				shipRight->GetComponent<HealthComponent>().getHealthBarCollider(i)->setColliderState(true);
			}
		}
		if (!BattleCalculation)
		{	
			if (!firstDiceThrownCheck)
			{
				Explosion.GetComponent<TransformComponent>().position =
					shipLeft->GetComponent<TransformComponent>().position;

				shipRight->GetComponent<UILabel>().RemoveTextFromScreenPlay("battle");
			
				battleDice1 = shipLeft->GetComponent<DiceComponent>().calculateDice
				(shipLeft->GetComponent<CannonComponent>().getCannonCount(),
					true,
					DiceCameraCaptureCalculation
					(shipRight->GetComponent<HealthComponent>().getHealthbarAmount()));

				shipLeft->GetComponent<UILabel>().AddTextToScreenplayWithNewPosition("dice",
					shipLeft->GetComponent<DiceComponent>().getTextForUI(), "8bit",
					Vector2Di(100, 1450));

				shipLeft->GetComponent<UILabel>().setPositionOfTextWithID("name", Vector2Di(100, 1400));
				shipRight->GetComponent<UILabel>().setPositionOfTextWithID("name", Vector2Di(100, 1650));

				firstDiceThrownCheck = true;
			}
			if (secondDiceRoll)
			{
		
				battleDice2 = shipRight->GetComponent<DiceComponent>().calculateDice
				(shipRight->GetComponent<CannonComponent>().getCannonCount(),
					true,
					DiceCameraCaptureCalculation
					(shipLeft->GetComponent<HealthComponent>().getHealthbarAmount()));

				shipRight->GetComponent<UILabel>().AddTextToScreenplayWithNewPosition("dice2",
					shipRight->GetComponent<DiceComponent>().getTextForUI(), "8bit",
					Vector2Di(100, 1700));

				bothRolled = true;
			}
			
			if (bothRolled && !BattleCalculation)
			{
				for (int i = 0; i < 3; i++)
				{
					shipLeft->GetComponent<HealthComponent>().getHealthBarCollider(i)->setColliderState(true);
					shipRight->GetComponent<HealthComponent>().getHealthBarCollider(i)->setColliderState(false);
				}

				//used in dice button calculation
				diceResult = battleDice1 + battleDice2;
				healthDecrease = true;
				if (battleDice1 == 0)
					secondAttack = true;

				//printf("fuck");
				//this current if statement is only activated once
				BattleCalculation = true;
			}
		}
		if(firstDiceThrownCheck)
		{
			//draw damage results onto the UI
			shipLeft->GetComponent<UILabel>().drawTextWithID("name");
			shipLeft->GetComponent<UILabel>().drawTextWithID("dice");
		}
	}
	if (firstDiceThrownCheck && !changeAmountFlag)
	{
		changeAmountText(shipLeft->GetComponent<HealthComponent>().getHealthbarAmount());
		changeAmountFlag = true;
	}

	//player may give their gold to the other player to skip battle
	if (goldClicked)
	{
		shipLeft->GetComponent<UILabel>().drawTextWithID("dice");
		diceResult = 0;
		damageCalculated = true;
		alarmSet = true;
	}
}

void Application::changeAmountText(int amount)
{
	std::stringstream ss;

	ss << "Throw " << amount <<" dice";

	UI.GetComponent<UILabel>().replaceTextWithID("Amount", ss.str());

}


//render everything under manager
//NOTE: render order matters
void Application::Render() 
{
	if (!mMinimized)
	SDL_RenderClear(Globals::Global_Renderer);
	////////////////////////////////////////////////

	for (auto& a : areas)
	{
		//draw all tiles
		a->draw();
	}

	for (auto& s : ships)
	{
		s->draw();
	}

	

	for (auto& u : ui)
	{
		u->draw();

		if (tutorialState)
		{
			UI.GetComponent<UILabel>().drawTextWithID("tutorial");
		}

		for (auto& s : ships)
		{
			//print out player name
			if (s->GetComponent<TurnComponent>().getTurnState() && !PlayerSelect)
				s->GetComponent<UILabel>().drawTextWithID("name");
			if (PlayerSelect)
				UI.GetComponent<UILabel>().drawTextWithID("intro");


			for (auto& a : areas)
			{
				if(Collision::AABB(
					a->GetComponent<ColliderComponent>().colliderArea,
					s->GetComponent<ColliderComponent>().colliderArea))
					//std::cout <<"true"<<std::endl;
				

				//check a collision between the areas & ships, and if it's the players turn
				if (Collision::AABB(
					a->GetComponent<ColliderComponent>().colliderArea,
					s->GetComponent<ColliderComponent>().colliderArea)
					&& s->GetComponent<TurnComponent>().getTurnState()
					&& onSpace)
				{
					checkVictory(s);

				

					//check if two players occupy the same space
					//s = ships, os = other ships
					for (auto& os : ships)
					{
						//check a collision between two different players
						if (Collision::AABB(
							s->GetComponent<ColliderComponent>().colliderArea,
							os->GetComponent<ColliderComponent>().colliderArea)
							&& s != os)
						{
							//used to reserve the space so no more than 2 player can be placed upon
							battleSpace = a;
							if (a->hasGroup(Application::groupSheep))
							{
								s->GetComponent<GoldComponent>().setGold(false);
								s->GetComponent<GoldComponent>().movePosition(Vector2D(SCREEN_WIDTH, 1600));
							}
							if (a->hasGroup(Application::groupSteel))
							{
								s->GetComponent<CannonComponent>().DecreaseCannonAmount();
							}
							//place players side by side on the area
							Vector2Di sideBySidePlacementPoints
							(a->GetComponent<TransformComponent>().position.x +
								a->GetComponent<TransformComponent>().width / 2,
								a->GetComponent<TransformComponent>().position.x +
								a->GetComponent<TransformComponent>().width / 6);

							s->GetComponent<TransformComponent>().position.x =
								sideBySidePlacementPoints.x;
							os->GetComponent<TransformComponent>().position.x =
								sideBySidePlacementPoints.y;


							if (!battleActive)
							{
								s->GetComponent<UILabel>().AddTextToScreenplayWithNewPosition
								("Battle", "Click the battle button!\nRoll dice equal to your own health dice!", "8bit", Vector2Di(100, 1600));

								diceResult = -1;
								battlePlayer1 = os;
								battlePlayer2 = s;
								battleActive = true;

								
								changeAmountText(battlePlayer2->GetComponent<HealthComponent>().getHealthbarAmount());

								
									

								for (int i = 0; i < 3; i++)
								{
									os->GetComponent<HealthComponent>().
										getHealthBarCollider(i)->setColliderState(false);
									s->GetComponent<HealthComponent>().
										getHealthBarCollider(i)->setColliderState(false);
								}
							}
						}
						//gold giving check//////////////////////////////////////////////////////////
						if (os->GetComponent<GoldComponent>().getCollider()->
							click(Globals::Global_Event.button)
							== SDL_MOUSEBUTTONDOWN
							&& !goldClicked
							&& s != os)
						{
							goldClicked = true;
							os->GetComponent<GoldComponent>().setGold(false);
							s->GetComponent<GoldComponent>().setGold(true);
							s->GetComponent<GoldComponent>().movePosition(Vector2D(920, 1600));
							s->GetComponent<UILabel>().replaceTextWithID("dice",
								"Gold was given, no battle will happen.");
						}
						/////////////////////////////////////////////////////////////////////////////
					}

					if (battleActive)
					{
						if (!firstDiceThrownCheck)
						{
							s->GetComponent<UILabel>().drawTextWithID("Battle");
						}
						

						ShipBattle(battlePlayer1, battlePlayer2);
					}

					//draw dice roll results
					if (diceRead)
					{
						if (bothRolled)
							s->GetComponent<UILabel>().drawTextWithID("dice2");
						//printf("meme");
						else
							s->GetComponent<UILabel>().drawTextWithID("dice");
					}


					
					if (onSpace)
					{
						//go through all tiles
						for (auto e : AllTiles)
						{
							//check if area is a certain tile
							if (a->hasGroup(e))
							{
								//print the specific tile text and perform actions with gold
								switch (e)
								{
								case Application::groupOcean:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("ocean");
									break;
									//use gold for more health
								case Application::groupTrees:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("trees");
									if (s->GetComponent<GoldComponent>().getCollider()->click
									(Globals::Global_Event.button) == SDL_MOUSEBUTTONDOWN
										&& clickState)
									{
										diceResult = 4;
										s->GetComponent<GoldComponent>().setGold(false);
										clickState = false;
									}
									break;
								case Application::groupWheat:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("wheat");
									break;
								case Application::groupDesert:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("desert");
									break;
								case Application::groupBrick:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("brick");
									break;
									//use gold for extra cannon
								case Application::groupSteel:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("steel");
									if (s->GetComponent<GoldComponent>().getCollider()->click
									(Globals::Global_Event.button) == SDL_MOUSEBUTTONDOWN
										&& clickState)
									{
										s->GetComponent<CannonComponent>().
											IncreaseCannonAmount();
										s->GetComponent<GoldComponent>().setGold(false);
										clickState = false;
									}
									break;
								case Application::groupSheep:
									if (!battleActive)
										UI.GetComponent<UILabel>().drawTextWithID("sheep");
									break;
								}

						
								textDrawn = true;

							}
						}
					}
					if (maxPlayerNumber == 1)
					{
						UI.GetComponent<UILabel>().drawTextWithID("victory");
					}
				}
			}



			//show player dice on their turn
			//the for loop is used to specify which die is being called upon with i

			for (int i = 0; i < healthDiceAmount; i++)
			{

				//Get the current ships healthbars and their colliders
				//and check if one of them is clicked
				if (s->GetComponent<HealthComponent>().getHealthBarEntity(i)->
					GetComponent<ColliderComponent>().click(Globals::Global_Event.button)
					== SDL_MOUSEBUTTONDOWN)
				{
					//if the mouse button is clicked, 
					//and click amount has not reached it's max cap
					if (clickState
						&& clickAmount != diceResult)
					{
						//decrease the ships health
						if (healthDecrease)
							s->GetComponent<HealthComponent>().decreaseHealth(i);
						else
						{
							//if the player is repairing a broken ship piece
							if (s->GetComponent<HealthComponent>().getHealth(i) == 0
								&& clickAmount == 0)
								clickAmount++;
							s->GetComponent<HealthComponent>().increaseHealth(i);
						}

						//change the state to false, so no extra clicks are registered
						clickState = false;

						//and change the sprite accordingly with the right amount of health
						switch (s->GetComponent<HealthComponent>().getHealth(i))
						{
						case 0:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieCross.png");
							break;
						case 1:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieWhite1.png");
							break;
						case 2:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieWhite2.png");
							break;
						case 3:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieWhite3.png");
							break;
						case 4:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieWhite4.png");
							break;
						case 5:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieWhite5.png");
							break;
						case 6:
							s->GetComponent<HealthComponent>().getHealthBarSprite(i)->
								changeSourceTexture("dieWhite6.png");
							break;
						default:
							break;
						}
						//increase clickAmount for the if statement check
						if (s->GetComponent<HealthComponent>().checkLosingCondition())
						{
							clickAmount = diceResult;
						}
						else
							clickAmount++;
					}
					if (clickAmount == battleDice1)
					{
						secondAttack = true;
					}
				}
				//check if damage has been calculated
				if (clickAmount != diceResult)
					damageCalculated = false;
				else
				{
					damageCalculated = true;
				}
				if (!alarmSet
					&& !battleActive
					&& onSpace
					&& damageCalculated
					&& diceRead)
				{
					alarm = SDL_GetTicks() + 3000;
					alarmSet = true;
				}

				//draw the dice on screen
				s->GetComponent<HealthComponent>().getHealthBarEntity(i)->
					GetComponent<SpriteComponent>().draw();
				//draw player gold
				if (s->GetComponent<GoldComponent>().getGoldState()
					&& s->GetComponent<TurnComponent>().getTurnState())
					s->GetComponent<GoldComponent>().getSprite()->draw();
			}
			//draw player cannons
			if (s->GetComponent<TurnComponent>().getTurnState())
				s->GetComponent<CannonComponent>().drawCannons();


			
		}
	}
		
	

	if (textDrawn && !clickedArea->hasGroup(Application::groupSafeSpace)
		|| firstDiceThrownCheck || battleActive)
	{
		DiceTray.GetComponent<SpriteComponent>().draw();
		UI.GetComponent<UILabel>().drawTextWithID("Amount");
	}


	for (auto& b : buttons)
	{
		//draw all buttons
		b->draw();
	}
	
	

	////////////////////////////////////////////////
	SDL_RenderPresent(Globals::Global_Renderer);

	if (textDrawn && onSpace && !battleActive && !diceRead && !clickedArea->hasGroup(Application::groupSafeSpace))
	{


		diceResult = clickedArea->GetComponent<DiceComponent>().calculateDice(activePlayer->GetComponent<CannonComponent>().getCannonCount(), false,
			DiceCameraCaptureCalculation(clickedArea->GetComponent<DiceComponent>().getDiceAmount()));

		//add damage result text to the Screenplay
		activePlayer->GetComponent<UILabel>().AddTextToScreenplayWithNewPosition("dice",
			clickedArea->GetComponent<DiceComponent>().getTextForUI(), "8bit", Vector2Di(100, 1600));

	}
	
}

void Application::Cleanup() 
{
	SDL_DestroyWindow(Globals::Global_Window);
	SDL_DestroyRenderer(Globals::Global_Renderer);
	SDL_Quit();
	printf("Cleanup finished\n");
}

bool Application::initEngine()
{
	//initialize everything regarding SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		//printf("Systems have initialized!\n");
		Globals::Global_Window = SDL_CreateWindow("Lauta Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (!Globals::Global_Window)
			printf("Window could not be created\n");
		Globals::Global_Renderer = SDL_CreateRenderer(Globals::Global_Window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!Globals::Global_Renderer)
			printf("Renderer could not be created\n");
		m_IsRunning = true;
	}
	else
	{
		printf("SDL_init_everything failed...\n");
		return m_IsRunning;
	}
	if (TTF_Init() == -1)
		printf("ERROR! - Failed to init SDL_TTF");
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_ShowCursor(cursorState);
	/*SDL_RenderSetLogicalSize(Globals::Global_Renderer, 
		SCREEN_WIDTH , 
		SCREEN_HEIGHT);*/
	return m_IsRunning;
}
