#pragma once
#include "../ECS/Components.h"
#include <random>
#include <string>

class PlayerShip
{
public:

	PlayerShip(std::string texturesheetID, std::string textureName, Group groupID, 
		Vector2Di position, int turn, SDL_Color color)
	{
		Entity& ship(manager.addEntity());
		ship.addComponent<TransformComponent>(position.x, position.y);
		ship.addComponent<SpriteComponent>(texturesheetID,  
			AssetManager::Assets->getSourceRect(textureName), 0.70f);
		ship.addGroup(groupID);
		ship.addComponent<InputComponent>();
		ship.addComponent<ColliderComponent>(textureName);
		//for rolling dice in battles
		ship.addComponent<DiceComponent>();
	
		
		ship.addComponent<TurnComponent>(turn);
		ship.addComponent<GoldComponent>(Vector2Di(SCREEN_WIDTH, 1600), "dice", "dieGold.png",
			Application::groupUIGold);
		ship.addComponent<CannonComponent>("ships", "cannon.png", Application::groupUICannon);
		
		switch (turn)
		{
		case 1:
			ship.addComponent<HealthComponent>(4, Vector2Di(920, 1400), "diceB",
				"dieWhite4.png", Application::groupUIDice);
			break;
		case 2:
			ship.addComponent<HealthComponent>(4, Vector2Di(SCREEN_WIDTH, 1400), "diceY",
				"dieWhite4.png", Application::groupUIDice);
			break;
		case 3:
			ship.addComponent<HealthComponent>(4, Vector2Di(SCREEN_WIDTH, 1400), "diceR",
				"dieWhite4.png", Application::groupUIDice);
			break;
		case 4:
			ship.addComponent<HealthComponent>(4, Vector2Di(SCREEN_WIDTH, 1400), "diceG",
				"dieWhite4.png", Application::groupUIDice);
			break;
		}

		m_transform = &ship.GetComponent<TransformComponent>();
		m_sprite = &ship.GetComponent<SpriteComponent>();
		m_collider = &ship.GetComponent<ColliderComponent>();
		e_ship = &ship;
		m_color = color;
		m_sprite->changeSpriteAngle(180);
	}

	void addFlavorText(std::string text, std::string id)
	{
		e_ship->addComponent<UILabel>(Vector2Di(100, 1400),
			text, "8bit", id, Vector2Di(700, 300), m_color);
	}

	SDL_Color getPlayerColor()
	{
		return m_color;
	}

private:

	SDL_Color m_color;

	Entity *e_ship;
	TransformComponent *m_transform;
	SpriteComponent *m_sprite;
	ColliderComponent *m_collider;
};


std::multimap<Group, int> tilesOnBoard;

class ClickArea 
{
public:

	ClickArea(std::string textureID, Vector2Di position)
	{
		Entity& area(manager.addEntity());

		area.addComponent<TransformComponent>(position.x * scaling, position.y * scaling);
		area.addComponent<SpriteComponent>(textureID, Vector2Di(220, 250), pickRandomTile(), 1 * scaling);
		area.addGroup(m_groupID);
		area.addComponent<DiceComponent>(m_diceAmount, m_revealedTileTextureCoordinates);
		area.addGroup(Application::groupArea);	
		area.addComponent<ColliderComponent>("area");

		area.addGroup(isTileSafe(m_groupID));
	}

	//ocean tile placement
	ClickArea(Vector2Di position, Group groupID)
	{
		Entity& area(manager.addEntity());

		area.addComponent<TransformComponent>(position.x * scaling, position.y * scaling);
		area.addComponent<SpriteComponent>("hex", Vector2Di(220, 250), 
			Vector2Di(220 * 7, 0), 1 * scaling);
		area.addGroup(groupID);
		area.addComponent<DiceComponent>(getTileDiceAmount(groupID), getTextureCoordinates(groupID));
		area.addGroup(Application::groupArea);

		area.addGroup(isTileSafe(groupID));

		area.addComponent<ColliderComponent>("area");

	}



	Vector2Di getTextureCoordinates(Group groupID)
	{

		switch (groupID)
		{
		case Application::groupOcean:
			return Vector2Di(220 * 0, 0);
			break;
		case Application::groupTrees:
			return Vector2Di(220 * 1, 0);
			break;
		case Application::groupWheat:
			return Vector2Di(220 * 2, 0);
			break;
		case Application::groupBrick:
			return Vector2Di(220 * 3, 0);
			break;
		case Application::groupSheep:
			return Vector2Di(220 * 4, 0);
			break;
		case Application::groupDesert:
			return Vector2Di(220 * 5, 0);
			break;
		case Application::groupSteel:
			return Vector2Di(220 * 6, 0);
			break;
		default:
			break;
		}

		return Vector2Di(0, 0);
	}

	int getTileDiceAmount(Group groupID)
	{
		switch (groupID)
		{
		case Application::groupOcean:
			return 0;
			break;
		case Application::groupTrees:
			return 2;
			break;
		case Application::groupWheat:
			return 1;
			break;
		case Application::groupBrick:
			return 3;
			break;
		case Application::groupSheep:
			return 0;
			break;
		case Application::groupDesert:
			return 2;
			break;
		case Application::groupSteel:
			return 0;
			break;
		default:
			break;
		}

		return 0;
	}

	Group isTileSafe(Group groupID)
	{
		switch (groupID)
		{
		case Application::groupOcean:
			return Application::groupSafeSpace;
			break;
		case Application::groupTrees:
			return Application::groupSafeSpace;
			break;
		case Application::groupSheep:
			return Application::groupSafeSpace;
			break;
		case Application::groupSteel:
			return Application::groupSafeSpace;
			break;
		default:
			break;
		}

		return Application::groupTrapSpace;
	}

	Group getTile(int id)
	{
		static const Application::groupLabels AllTiles[] = {

			Application::groupOcean,
			Application::groupTrees,
			Application::groupWheat,
			Application::groupBrick,
			Application::groupSheep,
			Application::groupDesert,
			Application::groupSteel

		};

		return AllTiles[id];
	}



	Vector2Di pickRandomTile()
	{
		//Group ID, int amount of tiles legal to place
		bool numberPicking = true;

		while (numberPicking)
		{		
			int tile = pickRandomNumber(0, 6);

			m_groupID = getTile(tile);

			it = tilesOnBoard.find(m_groupID);
			if (it != tilesOnBoard.end())
			{
				if (it->second != 0)
				{
					it->second--;
					m_revealedTileTextureCoordinates = getTextureCoordinates(m_groupID);
					m_diceAmount = getTileDiceAmount(m_groupID);

					
					return Vector2Di(220 * 7, 0);
				}
			}
		}

		//if an error occurs, just place an ocean tile
		m_revealedTileTextureCoordinates = Vector2Di(220 * 0, 0);
		return Vector2Di(220 * 7, 0);
	}

private:

	Vector2Di m_revealedTileTextureCoordinates;
	std::multimap<Group, int>::iterator it;
	int m_diceAmount;
	float scaling = 0.85f;
	Group m_groupID;

	int previousTile;

};





class HexBoard
{
public:

	/*******************************************************
	The board is formed out of

		 X X X X
		X X X X X

	Shapes, so the starting row will be X in length with
	X + 1 second row, and this will iterate Y times.
	
	If you want a pattern of 

		 X X X X X
		  X X X X

		  for (int z = 1; z > -1; z--) //used to make the second row			//!!!
			{
				for (int x = 0; x < boardDimension.x + z; x++)					
				{	
					position.x = StartingPoint.x + tileSize.x * x + spacer.x;
					position.y = StartingPoint.y + tileSize.y * y + spacer.y;

					ClickArea *hex = new ClickArea("hex", position)
				}
				spacer.add(Vector2Di(tileSize.x/2, tileSize.y * 0.74));			//!!!
			}
			spacer.add(Vector2Di(-tileSize.x, -tileSize.y));					//!!!

	*/
	void addTilesToBoard()
	{
		tilesOnBoard.erase(tilesOnBoard.begin(), tilesOnBoard.end());
		tilesOnBoard.emplace(Application::groupOcean, 25);	//neutral
		tilesOnBoard.emplace(Application::groupTrees, 5);	//health
		tilesOnBoard.emplace(Application::groupWheat, 7);	//1 damage
		tilesOnBoard.emplace(Application::groupBrick, 8);	//3 damage
		tilesOnBoard.emplace(Application::groupSheep, 4);	//gold
		tilesOnBoard.emplace(Application::groupDesert, 8);	//2 damage
		tilesOnBoard.emplace(Application::groupSteel, 4);	//weapons
	}



	HexBoard(Vector2Di boardDimension, Vector2Di tileSize, Vector2Di StartingPoint)
	{
		
		addTilesToBoard();

		//There are extra tiles just in case

		Vector2Di spacer(0, 0);
		//const Vector2Di StartingPoint(200, 100);
		
		Vector2Di position(0,0);

		for (int y = 0; y < boardDimension.y; y++)
		{
			for (int z = 0; z < 2; z++) //used to make the second row
			{
				for (int x = 0; x < boardDimension.x + z; x++)
				{
					position.x = StartingPoint.x + tileSize.x * x + spacer.x;
					position.y = StartingPoint.y + tileSize.y * y + spacer.y;

					if ((y == 0 && z == 0))
						ClickArea *hex = new ClickArea(position, Application::groupTrees);
					else
						ClickArea *hex = new ClickArea("hex", position);

				}
				spacer.add(Vector2Di(-tileSize.x/2, tileSize.y * 0.74f));
			}
			spacer.add(Vector2Di(tileSize.x, -tileSize.y));
		}
	}
};

class UIButton
{
public:

	UIButton(std::string textureID, std::string textureName,
		Group groupID, Vector2Di position)
	{
		Entity& button(manager.addEntity());

		button.addComponent<TransformComponent>(position.x, position.y);
		maxPosition = position;

		button.addComponent<SpriteComponent>(textureID,
			AssetManager::Assets->getSourceRect(textureName), 1, SDL_FLIP_NONE, 0, false);


		if (groupID != Application::groupUIButton)
		{
			button.addGroup(groupID);
		}
		button.addGroup(Application::groupUIButton);

		button.addComponent<ColliderComponent>("button");

		e_button = &button;
		transform = &button.GetComponent<TransformComponent>();
		sprite = &button.GetComponent<SpriteComponent>();
		collider = &button.GetComponent<ColliderComponent>();
	}

	void sliderPositionUpdate()
	{

		
		if (transform->position.y < maxPosition.y)
			transform->position.y = maxPosition.y;
		if (transform->position.y > maxPosition.y*2)
			transform->position.y = maxPosition.y*2;
		
	}

	Vector2Di minPosition;
	Vector2Di maxPosition;

	Entity *e_button;
	TransformComponent *transform;
	ColliderComponent *collider;
	SpriteComponent *sprite;

private:
};
