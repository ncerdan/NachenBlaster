#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <vector>

class Actor;
class Alien;
class NachenBlaster;
class Interactor;

using namespace std;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);
	~StudentWorld();

	//control game flow. called by their files
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	//adds Actor to game
	void addActorToVector(Actor* a);

	//check it Alien will 'see' NB and try and shoot/change plan
	bool willSeeNachenBlaster(const Alien& a) const;
	void addOneAlienDeath();

	//template for checking for collisions based on type/predicate f
	template <typename Func>
	bool checkCollisions_wThingsThat(Func f, Interactor* i);
private:
	//calculates odds and create one alien of proper type
	void addOneNewAlien();

	//holds all actors
	vector<Actor*> m_actors;

	//points to nachenblaster which is also in m_actors
	NachenBlaster* m_nachenBlaster;

	//defined at construction of level
	int m_totalAliensNeededToKillForLevel;

	//kept track of during of the course of the game
	int m_numAliensKilled;								
	int m_totalNumOfAliensOnScreen; 
};

//will be passed a predicate and a matching Interactor to check for proper collisions
template <typename Func>
bool StudentWorld::checkCollisions_wThingsThat(Func f, Interactor* i)
{
	for (int j = 0; j < m_actors.size(); j++)
	{
		if (m_actors[j]->getIsActive() && (m_actors[j]->*f)() && m_actors[j]->collidesWOtherActor(i))
		{
			//m_actors[j] is gaurenteed to be Interactor bc f will only return true for specific interactors
			i->interact(dynamic_cast<Interactor*>(m_actors[j]));
			
			//return true to indicate if a collision is made and to stop looking for other collisions
			return true;					
		}
	}
	return false;
}
#endif // STUDENTWORLD_H_
