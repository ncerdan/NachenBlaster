#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Projectile;
class Alien;
class NachenBlaster;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

//ABSTRACT CLASS
class Actor : public GraphObject
{
public:
	Actor(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
		bool canCollideWNB, bool canCollideWAliens, int startDirection = 0, double size = 1.0, int depth = 0);
	virtual void doSomething() = 0;
	
	//public getter bc used in StudentWorld.move()
	bool getIsActive() const;

	//used for differentiating objects
	bool isAlien() const;			
	bool isNB() const;				
	bool canCollideWNB() const;		
	bool canCollideWAliens() const;	

	//uses Euclidian distance formula
	bool collidesWOtherActor(Actor* a) const;
protected:
	//getters and setters used in derived classes
	void setIsActive(bool b);			
	void setCanCollideWNB(bool b);		
	void setCanCollideWAliens(bool b);	
	StudentWorld* getWorld() const;		
private:
	//world it is in
	StudentWorld * m_world;		
	
	//is alive
	bool m_isActive;

	//aliens
	bool m_isAlien;	

	//NachenBlaster
	bool m_isNB;				

	//turnips, bad FTorps, Aliens, Goodies
	bool m_canCollideWNB;		

	//Cabbage, good FTorps, Aliens
	bool m_canCollideWAliens;	
};

//ABSTRACT CLASS
class Interactor : public Actor		
{
public:
	Interactor(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
		bool canCollideWNB, bool canCollideWAliens, int startDirection = 0, double size = 1.0, int depth = 0);
	
	//ALL Interactors must be able to interact
	virtual void interact(Interactor* i) = 0;	
	
	//ALL Interactros must check for collisions. return true if it found collision (only used by goodie)
	virtual bool checkCollisions() = 0;			
};

class Goodie : public Interactor
{
public:
	//must pass imageID upon creation to distinguish which one it is
	Goodie(int imageID, int startX, int startY, StudentWorld* world);
	virtual void doSomething();
	virtual void interact(Interactor* nb);
	virtual bool checkCollisions();
private:
	//uses m_goodieID to give proper bonus
	void giveBonus(NachenBlaster* nb);	

	//used to determine what type of Goodie it is
	const int ELG = 0;	
	const int RG = 1;	
	const int TG = 2;

	//must be one of the above consts
	int m_goodieID;		
};

//ABSTRACT CLASS
class SpaceShip : public Interactor	
{
public:
	SpaceShip(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
		bool canCollideWNB, bool canCollideWAliens, int startDirection = 0, double size = 1.0, int depth = 0);
	virtual void sufferDamage(double amountToDecrease);

	//only time this will be called is if p is a projectile and a member function calls it. otherwise only other virtual functions w be called
	virtual void interact(Interactor* p);	

	//used for string output
	double getHP() const;					
protected:
	//varies by NB to Alien to Snagglegon
	virtual void shoot() = 0;	

	//setters and getters used in derived classes
	void setHP(double h);			
private:
	//health
	double m_hp;		
};

class NachenBlaster : public SpaceShip
{
public:
	NachenBlaster(StudentWorld* world);

	//checks for input etc
	virtual void doSomething();	

	//checks for Aliens, non-Friendly Projectiles, or Goodie
	virtual bool checkCollisions();	

	//interacts w either Alien, Projectile, or Goodie
	virtual void interact(Interactor* ALorPJorGD);

	//takes number to increase it by. caps at 50
	void increaseHP(int amountToIncreaseItBy);	

	//gives 5 torpedos, no cap
	void recTorpedo();	

	//getters
	int getFTorpedos();
	int getCabbage();
private:
	//for shooting cabbage
	virtual void shoot();

	//for shooting torpedos
	void shoot(int doesntMatter);	
	
	//ammo
	int m_cabbage;		
	int m_fTorpedos;
};

class Alien : public SpaceShip
{
public:
	Alien(int imageID, int startX, int startY, StudentWorld* world);

	//used by Smallgon. Other two call this then might drop goodie
	virtual void sufferDamage(double amountToDecrease);	

	//param is either Projectile or NB
	virtual void interact(Interactor* projectileOrNB);	

	//hurts NB, increases score, kills alien
	void crash(NachenBlaster* n);	

	//checks for NB or Projectile
	virtual bool checkCollisions();		

	//used by all 3 types of aliens (functions handle different behaviors)
	virtual void doSomething();							
protected:
	//getters and setters used in derived classes
	int getFlightPlan() const;				
	int getFlightPlanLength() const;		
	double getTravelSpeed() const;			
	void setFlightPlan(int i);				
	void setFlightPlanLength(int i);		
	void setTravelSpeed(double d);			
	void setPointsForDeath(int i);			
	void setDamageOnCrash(int i);		

	//used by 2 types of aliens, snagglegon defines its own
	virtual void shoot();	

	//used by 2 types of aliens, snagglegon has its own which calls this but then keeps flightPlanLength from changing
	virtual void move();		

	//used by all 3 types of aliens (bc snagglegon's length will never = 0)
	void changeFlightPlanIfNeeded();		

	//used by 1 type: snagglgon defines its own w different odds. smoregon calls this then adds something at end. if true, end tick. if false, continue w doSomething()
	virtual bool maybeShootOrChangePlan();	
	
	//represents 3 possible flight plans
	const int DUE_LEFT = 0;			
	const int UP_AND_LEFT = 1;		
	const int DOWN_AND_LEFT = 2;	
private:
	//always one of the protected consts above
	int m_flightPlan;

	int m_flightPlanLength;
	double m_travelSpeed;

	//points given to player when killed
	int m_pointsForDeath;		

	//hp of NB reduced upon impact
	int m_damageOnNBForCrash;	
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

	//used by all 3 types of projectiles
	virtual void doSomething();		

	//impacts s. used by all three types of projectiles
	void impact(SpaceShip* s);	

	//checks for either isAlien or isNB depending on isFriendly. used by all 3
	virtual bool checkCollisions();	

	//s will only be of type ship. used by all 3
	virtual void interact(Interactor* s);	
protected:
	//getters and setter used in derived classes
	int getVelocity() const;		
	bool getRotates() const;		
	bool getIsFriendly() const;		
	void setVelocity(int v);		
	void setIsFriendly(bool b);		
	void setDamageOnContact(int i);	
private:
	//negative indicates left, number indicates pixels per tick
	int m_velocity;				

	//true if it rotates every tick (turnip/cabbage)
	bool m_rotates;	

	//true if fired by NB and hits alien, false if fired by alien and hits NB
	bool m_isFriendly;			

	//damage it does when hitting a ship
	int m_damageOnContact;		
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
