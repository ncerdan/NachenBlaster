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
	//stats set at start of level
	m_totalAliensNeededToKillForLevel = 6 + (4 * getLevel());	
	m_numAliensKilled = 0;										
	m_totalNumOfAliensOnScreen = 0;								
}

//just calls cleanUp()
StudentWorld::~StudentWorld() { cleanUp(); }	

int StudentWorld::init()
{
	//stats set at start of level
	m_totalAliensNeededToKillForLevel = 6 + (4 * getLevel());	
	m_numAliensKilled = 0;										
	m_totalNumOfAliensOnScreen = 0;								

	//creates a new NachenBlaster object and saves its pointer in vector
	m_nachenBlaster = new NachenBlaster(this);					
	m_actors.push_back(m_nachenBlaster);						

	//creates 30 new stars
	for (int i = 0; i < 30; i++)								
	{															
		m_actors.push_back(new Star(randInt(0, VIEW_WIDTH - 1),	
			randInt(0, VIEW_HEIGHT - 1),	
			this));						
	}															

	//returns continue game value
	return GWSTATUS_CONTINUE_GAME;								
}

int StudentWorld::move()
{
	////////////////////// ACTORS "DO SOMETHING" //////////////////////

	//has every actor doSomething(), including NB
	for (int i = 0; i < m_actors.size(); i++)					
	{															
		m_actors[i]->doSomething();								
	}															

	////////////////////// CHECK NB STATUS /////////////////////

	//checks if player dies
	if (m_nachenBlaster->getIsActive() == false)						
	{																
		decLives();													
		return GWSTATUS_PLAYER_DIED;										
	}																	
	else															
	{	
		//if alive check num of aliens killed
		if (m_numAliensKilled >= m_totalAliensNeededToKillForLevel)	
			return GWSTATUS_FINISHED_LEVEL;
	}

	/////////////////////// ADD NEW ACTORS ///////////////////////

	//checks if needs to add new star to RHS of screen (1/15 chance)
	if (randInt(1, 15) == 1)									
		m_actors.push_back(new Star(VIEW_WIDTH - 1,				
			randInt(0, VIEW_HEIGHT - 1),						
			this));												

	//calculate whether to add a new Alien
	int D = m_numAliensKilled;												
	int T = m_totalAliensNeededToKillForLevel;								
	int R = T - D;															
	double M = 4 + (.5 * getLevel());
	if (m_totalNumOfAliensOnScreen < M && m_totalNumOfAliensOnScreen < R)	
		addOneNewAlien();

	/////////////////// CLEAN UP DEAD ACTORS //////////////////

	//checks/deletes all dead actors, skipping NB
	for (int i = 0; i < m_actors.size();)									
	{																		
		if (m_actors[i] != m_nachenBlaster && !m_actors[i]->getIsActive())	
		{
			//if its an alien update stat
			if (m_actors[i]->isAlien())										
				m_totalNumOfAliensOnScreen--;								
			delete m_actors[i];												
			m_actors.erase(m_actors.begin() + i);							
		}																	
		else																
			i++;															
	}																		

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
	//deletes all actors left in the actor vector
	for (int i = 0; i < m_actors.size();)		
	{											
		delete m_actors[i];						
		m_actors.erase(m_actors.begin());		
	}											
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

	//with S1/S odds add a new Smallgon, with S2/S odds add a new Smoregon, with S3/S odds add a new Snagglegon
	if (i <= S1)																				
		addActorToVector(new Smallgon(VIEW_WIDTH - 1, randInt(0, VIEW_HEIGHT - 1), this));		
	else if (i <= S1 + S2)																		
		addActorToVector(new Smoregon(VIEW_WIDTH - 1, randInt(0, VIEW_HEIGHT - 1), this));		
	else																						
		addActorToVector(new Snagglegon(VIEW_HEIGHT - 1, randInt(0, VIEW_HEIGHT - 1), this));	

	//increment this to keep track
	m_totalNumOfAliensOnScreen++;
}
