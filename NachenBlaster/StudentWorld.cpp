#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>

using namespace std;

GameWorld* createStudentWorld(string assetDir) { return new StudentWorld(assetDir); }

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetDir)
	: GameWorld(assetDir)
{
	m_totalAliensNeededToKillForLevel = 6 + (4 * getLevel());	//stats set at start of level
	m_numAliensKilled = 0;										//
	m_totalNumOfAliensOnScreen = 0;								//
}

StudentWorld::~StudentWorld() { cleanUp(); }	//just calls cleanUp()

int StudentWorld::init()
{
	m_totalAliensNeededToKillForLevel = 6 + (4 * getLevel());	//stats set at start of level
	m_numAliensKilled = 0;										//
	m_totalNumOfAliensOnScreen = 0;								//

	m_nachenBlaster = new NachenBlaster(this);					//creates a new NachenBlaster object
	m_actors.push_back(m_nachenBlaster);						//and also saves its pointer in vector

	for (int i = 0; i < 30; i++)								//creates 30 new stars
	{															//
		m_actors.push_back(new Star(randInt(0, VIEW_WIDTH - 1),	//
			randInt(0, VIEW_HEIGHT - 1),	//
			this));						//
	}															//

	return GWSTATUS_CONTINUE_GAME;								//returns continue game value
}

int StudentWorld::move()
{
	////////////////////// ACTORS "DO SOMETHING" //////////////////////

	for (int i = 0; i < m_actors.size(); i++)					//has every actor doSomething()
	{															//	including NB
		m_actors[i]->doSomething();								//
	}															//

																////////////////////// CHECK NB STATUS /////////////////////

	if (m_nachenBlaster->getIsActive() == false)					//checks if player dies	
	{																//
		decLives();													//
		return GWSTATUS_PLAYER_DIED;								//		
	}																//	
	else															//if alive check num of aliens killed
	{																//
		if (m_numAliensKilled >= m_totalAliensNeededToKillForLevel)	//
			return GWSTATUS_FINISHED_LEVEL;
	}

	/////////////////////// ADD NEW ACTORS ///////////////////////

	if (randInt(1, 15) == 1)									//checks if needs to add new star to RHS (1/15 chance)
		m_actors.push_back(new Star(VIEW_WIDTH - 1,				//
			randInt(0, VIEW_HEIGHT - 1),						//
			this));												//

	int D = m_numAliensKilled;												//calculate whether to add a new Alien
	int T = m_totalAliensNeededToKillForLevel;								//
	int R = T - D;															//
	double M = 4 + (.5 * getLevel());										//
	if (m_totalNumOfAliensOnScreen < M && m_totalNumOfAliensOnScreen < R)	//if its less than min(M, R) AKA less than both
		addOneNewAlien();													//add an Alien

																			/////////////////// CLEAN UP DEAD ACTORS //////////////////


	for (int i = 0; i < m_actors.size();)									//checks/deletes all dead actors
	{																		//	skipping NB
		if (m_actors[i] != m_nachenBlaster && !m_actors[i]->getIsActive())	//
		{																	//
			if (m_actors[i]->isAlien())										//if its an alien update stat
				m_totalNumOfAliensOnScreen--;								//
			delete m_actors[i];												//
			m_actors.erase(m_actors.begin() + i);							//
		}																	//
		else																//
			i++;															//
	}																		//

																			//////////////////// PRINT SCORE AT TOP /////////////////////
	ostringstream oss;
	double hpPercent = (m_nachenBlaster->getHP() / 50.0) * 100;
	double cabbagePercent = (m_nachenBlaster->getCabbage() / 30.0) * 100;
	oss.setf(ios::fixed);
	oss.precision(0);
	oss << "Lives: " << getLives() << "  Health: " << hpPercent << "%" << "  Score: " << getScore() << "  Level: " << getLevel()
		<< "  Cabbages: " << cabbagePercent << "%" << "  Torpedoes: " << m_nachenBlaster->getFTorpedos();

	setGameStatText(oss.str());

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (int i = 0; i < m_actors.size();)		//deletes all actors left in the actor vector
	{											//
		delete m_actors[i];						//
		m_actors.erase(m_actors.begin());		//
	}											//
}

void StudentWorld::addActorToVector(Actor* a) { m_actors.push_back(a); }

bool StudentWorld::willSeeNachenBlaster(const Alien& a) const { return (m_nachenBlaster->getX() < a.getX() && abs(m_nachenBlaster->getY() - a.getY()) <= 4); }

void StudentWorld::addOneAlienDeath() { m_numAliensKilled++; }

void StudentWorld::addOneNewAlien()
{
	int S1 = 60;
	int S2 = 20 + getLevel() * 5;
	int S3 = 5 + getLevel() * 10;
	int S = S1 + S2 + S3;
	int i = randInt(1, S);

	if (i <= S1)																				//with S1/S odds add a new Smallgon
		addActorToVector(new Smallgon(VIEW_WIDTH - 1, randInt(0, VIEW_HEIGHT - 1), this));		//
	else if (i <= S1 + S2)																		//with S2/S odds add a new Smoregon
		addActorToVector(new Smoregon(VIEW_WIDTH - 1, randInt(0, VIEW_HEIGHT - 1), this));		//
	else																						//with S3/S odds add a new Snagglegon
		addActorToVector(new Snagglegon(VIEW_HEIGHT - 1, randInt(0, VIEW_HEIGHT - 1), this));	//

	m_totalNumOfAliensOnScreen++;		//increase this to keep track
}
