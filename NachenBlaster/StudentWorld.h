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

	virtual int init();			//control game flow. called by their files
	virtual int move();			//
	virtual void cleanUp();		//

	void addActorToVector(Actor* a);					//adds Actor to game
	bool willSeeNachenBlaster(const Alien& a) const;	//check it Alien will 'see' NB and try and shoot/change plan
	void addOneAlienDeath();

	template <typename Func>
	bool checkCollisions_wThingsThat(Func f, Interactor* i);	//template for checking for collisions based on type/predicate f
private:
	void addOneNewAlien();			//does odds and create one alien of proper type

	vector<Actor*> m_actors;		//holds all actors
	NachenBlaster* m_nachenBlaster;	//points to nachenblaster which is also in m_actors

	int m_totalAliensNeededToKillForLevel;	//defined at construction of level
	int m_numAliensKilled;					//kept track of during of the game			
	int m_totalNumOfAliensOnScreen;			// 
};

template <typename Func>
bool StudentWorld::checkCollisions_wThingsThat(Func f, Interactor* i)	//will be passed a predicate and a matching Interactor to check for proper collisions
{
	for (int j = 0; j < m_actors.size(); j++)
	{
		if (m_actors[j]->getIsActive() && (m_actors[j]->*f)() && m_actors[j]->collidesWOtherActor(i))
		{
			i->interact(dynamic_cast<Interactor*>(m_actors[j]));//m_actors[j] is gaurenteed to be Interactor bc f will only return true for specific interactors
			return true;					//return true if collision made to indicate and to stop looking for other collisions
		}
	}
	return false;
}
#endif // STUDENTWORLD_H_
