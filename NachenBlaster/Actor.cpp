#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <cmath>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

//////////////////////////////////////////////////
// HELPER FUNCTIONS  /////////////////////////////
//////////////////////////////////////////////////
double euclidianDist(double x1, double y1, double x2, double y2) { return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); }

//////////////////////////////////////////////////
// ACTOR /////////////////////////////////////////
//////////////////////////////////////////////////
Actor::Actor(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB,
	bool canCollideWNB, bool canCollideWAliens, int startDirection /*default 0*/, double size /*default 1.0*/, int depth /*default 0*/)
	: GraphObject(imageID, startX, startY, startDirection, size, depth)
{
	m_world = world;
	m_isActive = true;
	m_isAlien = isAlien;
	m_isNB = isNB;
	m_canCollideWNB = canCollideWNB;
	m_canCollideWAliens = canCollideWAliens;
}

bool Actor::getIsActive() const { return m_isActive; }

void Actor::setIsActive(bool b) { m_isActive = b; }

void Actor::setCanCollideWNB(bool b) { m_canCollideWNB = b; }

void Actor::setCanCollideWAliens(bool b) { m_canCollideWAliens = b; }

bool Actor::isAlien() const { return m_isAlien; }

bool Actor::isNB() const { return m_isNB; }

bool Actor::canCollideWNB() const { return m_canCollideWNB; }

bool Actor::canCollideWAliens() const { return m_canCollideWAliens; }

StudentWorld* Actor::getWorld() const { return m_world; }

bool Actor::collidesWOtherActor(Actor* a) const { return euclidianDist(getX(), getY(), a->getX(), a->getY()) < .75 * (getRadius() + a->getRadius()); }

//////////////////////////////////////////////////
// INTERACTORS ///////////////////////////////////
//////////////////////////////////////////////////
Interactor::Interactor(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB, bool canCollideWNB, bool canCollideWAliens, int startDirection /*default 0*/, double size /*default 1.0*/, int depth /*default 0*/)
	: Actor(imageID, startX, startY, world, isAlien, isNB, canCollideWNB, canCollideWAliens, startDirection, size, depth)
{}

//////////////////////////////////////////////////
// GOODIE ////////////////////////////////////////
//////////////////////////////////////////////////
Goodie::Goodie(int imageID, int startX, int startY, StudentWorld* world)
	: Interactor(imageID, startX, startY, world, false, false, true, false, 0, .5, 1)
{
	// sets ID to proper value based on ID passed in
	if (imageID == IID_LIFE_GOODIE)			
		m_goodieID = ELG;					
	else if (imageID == IID_REPAIR_GOODIE)	
		m_goodieID = RG;					
	else									
		m_goodieID = TG;					
}

void Goodie::giveBonus(NachenBlaster* nb)
{
	//extra life goodie
	if (m_goodieID == ELG)		
		getWorld()->incLives();
	//repair goodie
	else if (m_goodieID == RG)	
		nb->increaseHP(10);
	//torpedo goodie
	else						
		nb->recTorpedo();
}

void Goodie::doSomething()
{
	if (getIsActive() == false)
		return;
	if (getX() < 0 || getY() < 0)
		setIsActive(false);

	if (checkCollisions())
		return;

	moveTo(getX() - .75, getY() - .75);

	checkCollisions();
}

//nb is guarenteed to be NB
void Goodie::interact(Interactor* nb)	
{
	//if its already dead just return
	if (nb->getIsActive() == false)					
		return;

	//increase score, set it to dead, play sound
	getWorld()->increaseScore(100);					
	setIsActive(false);								
	getWorld()->playSound(SOUND_GOODIE);			

	//will give one of three bonuses based on m_goodieID
	giveBonus(dynamic_cast<NachenBlaster*>(nb));	
}

bool Goodie::checkCollisions()
{
	if (getWorld()->checkCollisions_wThingsThat(&Actor::isNB, this))
		return true;
	return false;
}

//////////////////////////////////////////////////
// SPACESHIP /////////////////////////////////////
//////////////////////////////////////////////////
SpaceShip::SpaceShip(int imageID, int startX, int startY, StudentWorld* world, bool isAlien, bool isNB, bool canCollideWNB, bool canCollideWAliens, int startDirection /*default 0*/, double size /*default 1.0*/, int depth /*default 0*/)
	: Interactor(imageID, startX, startY, world, isAlien, isNB, canCollideWNB, canCollideWAliens, startDirection, size, depth)
{
	//default of spaceship is 50 (the hp of NB). aliens will have a different hp set in their constructor body
	m_hp = 50;	
}

void SpaceShip::sufferDamage(double amountToDecrease)
{
	//decrease hp
	m_hp -= amountToDecrease;	

	//check for death
	if (m_hp <= 0)				
		setIsActive(false);		
}

//must be a projectile bc I call this in member functions and ensure this
void SpaceShip::interact(Interactor* p)		
{
	//if its already dead stop
	if (p->getIsActive() == false)			
		return;					
	
	//call PJ::impact(SS)
	dynamic_cast<Projectile*>(p)->impact(this);
}

void SpaceShip::setHP(double h) { m_hp = h; }

double SpaceShip::getHP() const { return m_hp; }

//////////////////////////////////////////////////
// ALIEN /////////////////////////////////////////
//////////////////////////////////////////////////
Alien::Alien(int imageID, int startX, int startY, StudentWorld* world)
//sets hp to 0 which will change in body of constructor
	: SpaceShip(imageID, startX, startY, world, true, false, true, false, 0, 1.5, 1)	
{
	//stats for smallgon and smoregon. will be overwritten in snagglegon constructor body
	setHP(5 * (1 + (getWorld()->getLevel() - 1) * .1));	
	m_flightPlanLength = 0;								
	m_travelSpeed = 2.0;								
	m_pointsForDeath = 250;								
	m_damageOnNBForCrash = 5;							
}

void Alien::interact(Interactor* projectileOrNB)
{
	//if its already dead stop
	if (projectileOrNB->getIsActive() == false)		
		return;

	//if its a projectile go to correct interact (SS::interact)
	if (dynamic_cast<Projectile*>(projectileOrNB) != nullptr)			
		SpaceShip::interact(dynamic_cast<Projectile*>(projectileOrNB));	
	//if its the NB crash into it
	else																
		crash(dynamic_cast<NachenBlaster*>(projectileOrNB));			
}

void Alien::sufferDamage(double amountToDecrease)
{
	//decreases health and checks for death 
	SpaceShip::sufferDamage(amountToDecrease);	

	//if it dies, increase score, play the correct scound, explode, inc stat
	if (getIsActive() == false)							
	{
		getWorld()->increaseScore(m_pointsForDeath);							
		getWorld()->playSound(SOUND_DEATH);										
		getWorld()->addActorToVector(new Explosion(getX(), getY(), getWorld()));
		getWorld()->addOneAlienDeath();											
	}
	//if it doesn't die play the correct sound. this only happens from projectile bc Aliens always die from crashing
	else														
		getWorld()->playSound(SOUND_BLAST);				
}

void Alien::crash(NachenBlaster* nb)
{
	//hurt the NB
	nb->sufferDamage(m_damageOnNBForCrash);	

	//increase the score
	getWorld()->increaseScore(m_pointsForDeath);	

	//kills alien
	sufferDamage(getHP());							
}

int Alien::getFlightPlan() const { return m_flightPlan; }

int Alien::getFlightPlanLength() const { return m_flightPlanLength; }

double Alien::getTravelSpeed() const { return m_travelSpeed; }

void Alien::setFlightPlan(int i) { m_flightPlan = i; }

void Alien::setFlightPlanLength(int i) { m_flightPlanLength = i; }

void Alien::setTravelSpeed(double d) { m_travelSpeed = d; }

void Alien::setPointsForDeath(int i) { m_pointsForDeath = i; }

void Alien::setDamageOnCrash(int i) { m_damageOnNBForCrash = i; }

bool Alien::checkCollisions()
{
	//check collisions w right func
	if (getWorld()->checkCollisions_wThingsThat(&Actor::canCollideWAliens, this))	
		return true;
	return false;
}

void Alien::shoot()
{
	//creates turnip and plays sound
	getWorld()->addActorToVector(new Turnip(getX() - 14, getY(), getWorld()));	
	getWorld()->playSound(SOUND_ALIEN_SHOOT);									
}

void Alien::move()
{
	//dec flightPlan
	m_flightPlanLength--;		

	//move in direction specified by m_flightPlan
	if (m_flightPlan == DUE_LEFT)								
		moveTo(getX() - m_travelSpeed, getY());					
	else if (m_flightPlan == UP_AND_LEFT)						
		moveTo(getX() - m_travelSpeed, getY() + m_travelSpeed);	
	else if (m_flightPlan == DOWN_AND_LEFT)						
		moveTo(getX() - m_travelSpeed, getY() - m_travelSpeed);	
}

void Alien::changeFlightPlanIfNeeded()
{
	//if theres no reason to change course, dont
	if (m_flightPlanLength != 0 && getY() < VIEW_HEIGHT - 1 && getY() > 0)	
		return;					

	//if you're at the top, if you're at the bottom, if you're length is 0 respond correctly
	if (getY() >= VIEW_HEIGHT - 1)			
		setFlightPlan(DOWN_AND_LEFT);		
	else if (getY() <= 0)					
		setFlightPlan(UP_AND_LEFT);			
	else if (m_flightPlanLength == 0)		
	{										
		int i = randInt(1, 3);				 
		switch (i)							 
		{									
		case 1:								
			setFlightPlan(DUE_LEFT);		
			break;							
		case 2:								
			setFlightPlan(UP_AND_LEFT);		
			break;							
		case 3:								
			setFlightPlan(DOWN_AND_LEFT);	
			break;							
		}									
	}										

	//set a new flight plan length (OK for snagglegons to do this bc they will call this then setLength back up)
	m_flightPlanLength = randInt(1, 32);	
}

bool Alien::maybeShootOrChangePlan()
{
	//odds to shoot if a Smallgon of Smoregon
	int i = randInt(1, (20 / getWorld()->getLevel()) + 5);	
	if (i == 1)
	{
		shoot();

		//returns true if it shoots
		return true;	
	}

	//returns false if it doesn't
	return false;		
}

void Alien::doSomething()
{
	//if its dead, don't do anything
	if (!getIsActive())		
		return;

	//set it dead if its off screen
	if (getX() < 0)			
	{
		setIsActive(false);
		return;
	}

	//check for collisions
	checkCollisions();		

	//check if it needs to change flight plan
	changeFlightPlanIfNeeded();	

	//if NB is in front, it might shoot/change plan and if it does shoot, end current doSomething()
	if (getWorld()->willSeeNachenBlaster(*this))	
		if (maybeShootOrChangePlan())				
			return;									   

	//move according to possibly-new flight plan
	move();					

	//check for collisions again
	checkCollisions();		
}

//////////////////////////////////////////////////
// SMALLGON //////////////////////////////////////
//////////////////////////////////////////////////
Smallgon::Smallgon(int startX, int startY, StudentWorld* world)
	: Alien(IID_SMALLGON, startX, startY, world)
{}

//////////////////////////////////////////////////
// SMOREGON //////////////////////////////////////
//////////////////////////////////////////////////
Smoregon::Smoregon(int startX, int startY, StudentWorld* world)
	: Alien(IID_SMOREGON, startX, startY, world)
{}

void Smoregon::sufferDamage(double amountToDecrease)
{
	//calls Alien's sufferDmg, if it dies it might drop a goodie
	Alien::sufferDamage(amountToDecrease);	
	if (getIsActive() == false)				
	{
		if (randInt(1, 3) == 1)				
		{
			if (randInt(1, 2) == 1)			
				getWorld()->addActorToVector(new Goodie(IID_REPAIR_GOODIE, getX(), getY(), getWorld()));
			else
				getWorld()->addActorToVector(new Goodie(IID_TORPEDO_GOODIE, getX(), getY(), getWorld()));
		}
	}
}

bool Smoregon::maybeShootOrChangePlan()
{
	//if it shoots, returns true to end doSomething()
	if (Alien::maybeShootOrChangePlan())	
		return true;		

	//does charging odds
	int i = randInt(1, (20 / getWorld()->getLevel()) + 5);	
	if (i == 1)
	{
		setFlightPlan(DUE_LEFT);
		setFlightPlanLength(VIEW_WIDTH);
		setTravelSpeed(5);
	}

	//returns false to continue w doSomething()
	return false;		
}

//////////////////////////////////////////////////
// SNAGGLEGON ////////////////////////////////////
//////////////////////////////////////////////////
Snagglegon::Snagglegon(int startX, int startY, StudentWorld* world)
	: Alien(IID_SNAGGLEGON, startX, startY, world)
{
	//sets stats to correct values since they differ from other two types of Aliens
	setHP(10 * (1 + (getWorld()->getLevel() - 1) * .1));	
	setFlightPlan(DOWN_AND_LEFT);							
	setFlightPlanLength(10);								
	setTravelSpeed(1.75);									
	setPointsForDeath(1000);								
	setDamageOnCrash(15);									
}

void Snagglegon::sufferDamage(double amountToDecrease)
{
	//suffers damage, if it dies might drop a goodie
	Alien::sufferDamage(amountToDecrease);	
	if (getIsActive() == false)				
	{
		if (randInt(1, 6) == 1)
			getWorld()->addActorToVector(new Goodie(IID_LIFE_GOODIE, getX(), getY(), getWorld()));
	}
}

//overrides Alien::shoot() bc FTorps
void Snagglegon::shoot()
{
	getWorld()->addActorToVector(new FlatulenceTorpedo(getX() - 14, getY(), getWorld(), 180));	
	getWorld()->playSound(SOUND_TORPEDO);
}

void Snagglegon::move()
{
	//calls alien's move which decrements flightplan length
	Alien::move();				

	//this makes up for above bc snagglegons dont use length
	setFlightPlanLength(10);	
}

//overrides Alien::maybeShootOrChangePlan() bc it has different odds to shoot  
bool Snagglegon::maybeShootOrChangePlan()
{
	int i = randInt(1, (15 / getWorld()->getLevel()) + 10);	
	if (i == 1)
	{
		shoot();

		//returns true if shot to end current doSomething()
		return true;	
	}

	//returns false if it doesn't shoot to continue w actions
	return false;		
}

//////////////////////////////////////////////////
// NACHENBLASTER /////////////////////////////////
//////////////////////////////////////////////////
NachenBlaster::NachenBlaster(StudentWorld* world)
	: SpaceShip(IID_NACHENBLASTER, 0, 128, world, false, true, false, true /*default direction = 0, size = 1, and depth = 0*/)
{
	m_cabbage = 30;
	m_fTorpedos = 0;
}

int NachenBlaster::getCabbage() { return m_cabbage; }

int NachenBlaster::getFTorpedos() { return m_fTorpedos; }

bool NachenBlaster::checkCollisions()
{
	if (getWorld()->checkCollisions_wThingsThat(&Actor::canCollideWNB, this))	//calls it w correct func
		return true;
	return false;
}

void NachenBlaster::interact(Interactor* ALorPJorGD)
{
	//if its dead just stop
	if (ALorPJorGD->getIsActive() == false)		
		return;							

	//if its an alien, crash
	if (ALorPJorGD->isAlien())									
		dynamic_cast<Alien*>(ALorPJorGD)->crash(this);			

	//if its a projectile, SS::interact() will handle it
	else if (dynamic_cast<Projectile*>(ALorPJorGD) != nullptr)	
		SpaceShip::interact(ALorPJorGD);
	
	//if its a Goodie, Goodie::interact() will handle it
	else														
		dynamic_cast<Goodie*>(ALorPJorGD)->interact(this);		
}

void NachenBlaster::increaseHP(int amountToIncreaseItBy)
{
	setHP(getHP() + amountToIncreaseItBy);
	if (getHP() > 50)
		setHP(50);
}

void NachenBlaster::recTorpedo()
{
	m_fTorpedos += 5;
}

void NachenBlaster::doSomething()
{
	//check its still active
	if (!getIsActive())		
		return;				

	//check for input and move if possible
	int ch;																					
	int moveDistance = 6;																	
	if (getWorld()->getKey(ch))																
	{																						
		switch (ch)																			
		{																					
		case KEY_PRESS_SPACE:																
			shoot();																		
			break;																			
		case KEY_PRESS_TAB:																	
			shoot(1);																		
			break;																			
		case KEY_PRESS_LEFT:																
			if (getX() >= moveDistance)														
				moveTo(getX() - moveDistance, getY());										
			break;																			
		case KEY_PRESS_RIGHT:																
			if (getX() < VIEW_WIDTH - moveDistance)											
				moveTo(getX() + moveDistance, getY());											
			break;																			
		case KEY_PRESS_UP:																	
			if (getY() < VIEW_HEIGHT - moveDistance)											
				moveTo(getX(), getY() + moveDistance);										
			break;																			
		case KEY_PRESS_DOWN:																
			if (getY() >= moveDistance)														
				moveTo(getX(), getY() - moveDistance);										
			break;																			
		}																					
	}																						

	//check for collisions
	checkCollisions();	

	//gets 1 cabbage every tick up to 30 max
	if (m_cabbage < 30)				
		m_cabbage++;					
}

//used for cabbage
void NachenBlaster::shoot()	
{
	//if you have enough ammo, create a cabbage and 'fire' it
	if (m_cabbage >= 5)																
	{
		m_cabbage -= 5;																
		getWorld()->addActorToVector(new Cabbage(getX() + 12, getY(), getWorld()));	
		getWorld()->playSound(SOUND_PLAYER_SHOOT);									
	}
}

//used for torpedos
void NachenBlaster::shoot(int doesntMatter)	
{
	//if you have any torpedos, create a torpedo and 'fire' it
	if (m_fTorpedos > 0)															
	{
		m_fTorpedos--;																
		getWorld()->addActorToVector(new FlatulenceTorpedo(getX() + 12, getY(),		
			getWorld(), 0));	
		getWorld()->playSound(SOUND_TORPEDO);										
	}
}

//////////////////////////////////////////////////
// STAR  /////////////////////////////////////////
//////////////////////////////////////////////////
Star::Star(int startX, int startY, StudentWorld* world)
//sets size to 0 here but sets it to actual value in body of constructor
	: Actor(IID_STAR, startX, startY, world, false, false, false, false, 0, 0, 3)	
{
	//set proper size
	int s = randInt(5, 50);		
	double ns = s;				
	setSize(ns / 100);			
}

void Star::doSomething()
{
	//move left
	moveTo(getX() - 1, getY());		

	//if it leaves screen set it to be deleted
	if (getX() < 0)					
		setIsActive(false);			
}

//////////////////////////////////////////////////
// EXPLOSION  ////////////////////////////////////
//////////////////////////////////////////////////
Explosion::Explosion(int startX, int startY, StudentWorld* world)
	: Actor(IID_EXPLOSION, startX, startY, world, false, false, false, false /*direction = 0, size = 1, depth = 0*/)
{
	//starting age of 0
	m_age = 0;	
}

void Explosion::doSomething()
{
	//every tick increase size by factor of 1.5
	setSize(getSize() * 1.5);	

	//increment age
	m_age++;			

	//if its gone 4 ticks, set to dead
	if (m_age == 4)				
		setIsActive(false);		
}

//////////////////////////////////////////////////
// PROJECTILE  ///////////////////////////////////
//////////////////////////////////////////////////
Projectile::Projectile(int imageID, int startX, int startY, StudentWorld* world, bool canCollideWNB, bool canCollideWAliens, int startDirection, int velocity, bool rotates, bool isFriendly)
	: Interactor(imageID, startX, startY, world, false, false, canCollideWNB, canCollideWAliens, startDirection, .5, 1)
{
	//set member variables
	m_velocity = velocity;		
	m_rotates = rotates;		
	m_isFriendly = isFriendly;	

	//for cabbage and turnips. torpedo gets 8 in its own constructor body
	m_damageOnContact = 2;		
}

void Projectile::interact(Interactor* s)
{
	//if its dead stop
	if (s->getIsActive() == false)		
		return;

	//s will only be of type spaceship
	impact(dynamic_cast<SpaceShip*>(s));	
}

bool Projectile::checkCollisions()
{
	//if its friendly call function looking for Aliens
	if (getIsFriendly())					
	{
		if (getWorld()->checkCollisions_wThingsThat(&Actor::isAlien, this))
			return true;
		return false;
	}

	//if its not friendly call func looking for NB
	else									
	{
		if (getWorld()->checkCollisions_wThingsThat(&Actor::isNB, this))
			return true;
		return false;
	}
}

void Projectile::impact(SpaceShip* s)
{
	//hurt ship based on projectile dmg
	s->sufferDamage(m_damageOnContact);	

	//set projectile to dead
	setIsActive(false);					
}

void Projectile::doSomething()
{
	//if its dead, stop
	if (!getIsActive())			
		return;					

	//if its off the screen set to dead and stop
	if (getX() < 0 || getX() >= VIEW_WIDTH)		
	{											
		setIsActive(false);						
		return;									
	}											

	//check collisions, if it does, return
	if (checkCollisions())			
		return;						

	//move by velocity
	moveTo(getX() + getVelocity(), getY());

	//if it rotates, rotate it
	if (getRotates())							
		setDirection(getDirection() + 20);		

	//check collisions again
	checkCollisions();			
}

int Projectile::getVelocity() const { return m_velocity; }

void Projectile::setVelocity(int v) { m_velocity = v; }

bool Projectile::getRotates() const { return m_rotates; }

bool Projectile::getIsFriendly() const { return m_isFriendly; }

void Projectile::setIsFriendly(bool b) { m_isFriendly = b; }

void Projectile::setDamageOnContact(int i) { m_damageOnContact = i; }

//////////////////////////////////////////////////
// CABBAGE  //////////////////////////////////////
//////////////////////////////////////////////////
Cabbage::Cabbage(int startX, int startY, StudentWorld* world)
	: Projectile(IID_CABBAGE, startX, startY, world, false, true, 0, 8, true, true)
{}

//////////////////////////////////////////////////
// TURNIP  ///////////////////////////////////////
//////////////////////////////////////////////////
Turnip::Turnip(int startX, int startY, StudentWorld* world)
	: Projectile(IID_TURNIP, startX, startY, world, true, false, 0, -6, true, false)
{}

//////////////////////////////////////////////////
// F-TORPEDO  ////////////////////////////////////
//////////////////////////////////////////////////
FlatulenceTorpedo::FlatulenceTorpedo(int startX, int startY, StudentWorld* world, int direction)	
//set direction to 0 if by nachenb, 180 if by alien. defaults velocity to positive, friendly to true, and can only collide w aliens (all for nachenblaster firing)
	: Projectile(IID_TORPEDO, startX, startY, world, false, true, direction, 8, false, true)		
{										
	//if it was shot by alien: change velocity to negative, set friendly to false, set it so it hits NB, not aliens
	if (getDirection() == 180)				
	{											
		setVelocity(-1 * getVelocity());	 
		setIsFriendly(false);				  
		setCanCollideWNB(true);				
		setCanCollideWAliens(false);		  
	}

	//correct this variable for torpedos
	setDamageOnContact(8);		
}