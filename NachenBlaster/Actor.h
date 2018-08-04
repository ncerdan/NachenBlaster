#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Projectile;
class Alien;
class NachenBlaster;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class Actor : public GraphObject	//ABSTRACT CLASS
{
public:
	Actor(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
		bool canCollideWNB, bool canCollideWAliens, int startDirection = 0, double size = 1.0, int depth = 0);
	virtual void doSomething() = 0;
	bool getIsActive() const;		//public getter bc used in StudentWorld.move()

	bool isAlien() const;			//used for differentiating objects
	bool isNB() const;				//
	bool canCollideWNB() const;		//
	bool canCollideWAliens() const;	//

	bool collidesWOtherActor(Actor* a) const;	//used Euclidian distance formula
protected:
	void setIsActive(bool b);			//getters and setters used in derived classes
	void setCanCollideWNB(bool b);		//
	void setCanCollideWAliens(bool b);	//
	StudentWorld* getWorld() const;		//
private:
	StudentWorld * m_world;		//is world it is in
	bool m_isActive;			//is alive
	bool m_isAlien;				//aliens
	bool m_isNB;				//NachenBlaster
	bool m_canCollideWNB;		//turnips, bad FTorps, Aliens, Goodies
	bool m_canCollideWAliens;	//Cabbage, good FTorps, Aliens
};

class Interactor : public Actor		//ABSTRACT CLASS
{
public:
	Interactor(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
		bool canCollideWNB, bool canCollideWAliens, int startDirection = 0, double size = 1.0, int depth = 0);
	virtual void interact(Interactor* i) = 0;	//ALL Interactors must be able to interact
	virtual bool checkCollisions() = 0;			// by checking for collisions. return true if it found collision (only used by goodie)
};

class Goodie : public Interactor
{
public:
	Goodie(int imageID, int startX, int startY, StudentWorld* world);		//must pass imageID upon creation to distinguish which one it is
	virtual void doSomething();
	virtual void interact(Interactor* nb);
	virtual bool checkCollisions();
private:
	void giveBonus(NachenBlaster* nb);	//uses m_goodieID to give proper bonus

	const int ELG = 0;	//used to determine what type of Goodie it is
	const int RG = 1;	//
	const int TG = 2;	//
	int m_goodieID;		//must be one of the above consts
};

class SpaceShip : public Interactor	//ABSTRACT CLASS
{
public:
	SpaceShip(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
		bool canCollideWNB, bool canCollideWAliens, int startDirection = 0, double size = 1.0, int depth = 0);
	virtual void sufferDamage(double amountToDecrease);
	virtual void interact(Interactor* p);	//only time this will be called is if p is a projectile and I call it. otherwise only other virtual functions w be called
	double getHP() const;					//used for string output
protected:
	virtual void shoot() = 0;	//varies by NB to Alien to Snagglegon
	void setHP(double h);		//setters and getters used in derived classes	
private:
	double m_hp;		//health
};

class NachenBlaster : public SpaceShip
{
public:
	NachenBlaster(StudentWorld* world);
	virtual void doSomething();						//checks for input etc
	virtual bool checkCollisions();					//checks for Aliens, non-Friendly Projectiles, or Goodie
	virtual void interact(Interactor* ALorPJorGD);	//interacts w either Alien, Projectile, or Goodie
	void increaseHP(int amountToIncreaseItBy);		//takes number to increase it by. caps at 50
	void recTorpedo();								//gives 5 torpedos, no cap
	int getFTorpedos();
	int getCabbage();
private:
	virtual void shoot();			//for cabbage
	void shoot(int doesntMatter);	//for torpedos
	int m_cabbage;		//ammo
	int m_fTorpedos;	//
};

class Alien : public SpaceShip
{
public:
	Alien(int imageID, int startX, int startY, StudentWorld* world);
	virtual void sufferDamage(double amountToDecrease);	//used by Smallgon. Other two cal this then might drop goodie
	virtual void interact(Interactor* projectileOrNB);	//is either Projectile or NB
	void crash(NachenBlaster* n);						//hurts NB, increases score, kills alien
	virtual bool checkCollisions();						//checks for NB or Projectile
	virtual void doSomething();							//used by all 3 types of aliens (functions handle different behaviors)
protected:
	int getFlightPlan() const;				//getters and setters used in derived classes
	int getFlightPlanLength() const;		//
	double getTravelSpeed() const;			//
	void setFlightPlan(int i);				//
	void setFlightPlanLength(int i);		//
	void setTravelSpeed(double d);			//
	void setPointsForDeath(int i);			//
	void setDamageOnCrash(int i);			//
	virtual void shoot();					//used by 2 types of aliens, snagglegon defines its own
	virtual void move();					//used by 2 types of aliens, snagglegon has its own which calls this but then keeps flightPlanLength from changing
	void changeFlightPlanIfNeeded();		//used by all 3 types of aliens (bc snagglegon's length will never = 0)
	virtual bool maybeShootOrChangePlan();	//used by 1 type: snagglgon defines its own w different odds. smoregon calls this then adds something at end
											//    for changing plan to charge. if returns true, end current tick. if false, continue w doSomething()
	const int DUE_LEFT = 0;			//represents 3 possible flight plans
	const int UP_AND_LEFT = 1;		//
	const int DOWN_AND_LEFT = 2;	//
private:
	int m_flightPlan;			//always one of the protected consts above
	int m_flightPlanLength;
	double m_travelSpeed;
	int m_pointsForDeath;		//points given to player when killed
	int m_damageOnNBForCrash;	//hp of NB reduced upon impact
};

class Smallgon : public Alien
{
public:
	Smallgon(int startX, int startY, StudentWorld* world);
};

class Smoregon : public Alien
{
public:
	Smoregon(int startX, int startY, StudentWorld* world);
	virtual void sufferDamage(double amountToDecrease);		//calls Alien::sufferDamage() but then might drop goodie
protected:
	virtual bool maybeShootOrChangePlan();					//calls Alian::mSOCP() but then might change plan
};

class Snagglegon : public Alien
{
public:
	Snagglegon(int startX, int startY, StudentWorld* world);
	virtual void sufferDamage(double amountToDecrease);		//calls Alen::sufferDamage() but then might drop goodie
protected:
	virtual void shoot();		//overrides Alien::shoot() bc FTorps
	virtual void move();		//calls Alien::move() but then sets PlanLength back to 10 to avoid hitting 0
	virtual bool maybeShootOrChangePlan();	//different odds to shoot
};

class Star : public Actor
{
public:
	Star(int startX, int startY, StudentWorld* world);
	virtual void doSomething();	//moves left and checks for death
};

class Explosion : public Actor
{
public:
	Explosion(int startX, int startY, StudentWorld* world);
	virtual void doSomething();
private:
	int m_age;
};

class Projectile : public Interactor
{
public:
	Projectile(int imageID, int startX, int startY, StudentWorld* world, bool canCollideWNB,
		bool canCollideWAliens, int startDirection, int velocity, bool rotates, bool isFriendly);
	virtual void doSomething();				//used by all 3 types of projectiles
	void impact(SpaceShip* s);				//impacts s. used by all three types of projectiles
	virtual bool checkCollisions();			//checks for either isAlien or isNB depending on isFriendly. used by all 3
	virtual void interact(Interactor* s);	//s will only be of type ship. used by all 3
protected:
	int getVelocity() const;		//getters and setter used in derived classes
	bool getRotates() const;		//
	bool getIsFriendly() const;		//
	void setVelocity(int v);		//
	void setIsFriendly(bool b);		//
	void setDamageOnContact(int i);	//
private:
	int m_velocity;				//negative indicates left, number indicates pixels per tick
	bool m_rotates;				//true if it rotates every tick (turnip/cabbage)
	bool m_isFriendly;			//true if fired by NB and hits alien, false if fired by alien and hits NB
	int m_damageOnContact;		//damage it does when hitting a ship
};

class Cabbage : public Projectile
{
public:
	Cabbage(int startX, int startY, StudentWorld* world);
};

class Turnip : public Projectile
{
public:
	Turnip(int startX, int startY, StudentWorld* world);
};

class FlatulenceTorpedo : public Projectile
{
public:
	FlatulenceTorpedo(int startX, int startY, StudentWorld* world, int direction);	//0 for nb, 180 for aliens
};

#endif // ACTOR_H_
