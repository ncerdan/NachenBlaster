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
	if (imageID == IID_LIFE_GOODIE)			// sets ID to proper value based on ID passed in
		m_goodieID = ELG;					//
	else if (imageID == IID_REPAIR_GOODIE)	//
		m_goodieID = RG;					//
	else									//
		m_goodieID = TG;					//
}

void Goodie::giveBonus(NachenBlaster* nb)
{
	if (m_goodieID == ELG)		//extra life goodie
		getWorld()->incLives();
	else if (m_goodieID == RG)	//repair goodie
		nb->increaseHP(10);
	else						//torpedo goodie
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

void Goodie::interact(Interactor* nb)	//this is guarenteed to be a NB
{
	if (nb->getIsActive() == false)					//if its already dead just return
		return;
	getWorld()->increaseScore(100);					//increase score, set it to dead, play sound
	setIsActive(false);								//
	getWorld()->playSound(SOUND_GOODIE);			//
	giveBonus(dynamic_cast<NachenBlaster*>(nb));	//will give one of three bonuses based on m_goodieID
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
	m_hp = 50;	//default of spaceship is 50 (the hp of NB). aliens will have a different hp set in their constructor body
}

void SpaceShip::sufferDamage(double amountToDecrease)
{
	m_hp -= amountToDecrease;	//decrease hp
	if (m_hp <= 0)				//check for death
		setIsActive(false);		//
}

void SpaceShip::interact(Interactor* p)		//must be a projectile bc I call this myself and ensure this
{
	if (p->getIsActive() == false)			//if its already dead stop
		return;								//
	dynamic_cast<Projectile*>(p)->impact(this);	//call PJ::impact(SS)
}

void SpaceShip::setHP(double h) { m_hp = h; }

double SpaceShip::getHP() const { return m_hp; }

//////////////////////////////////////////////////
// ALIEN /////////////////////////////////////////
//////////////////////////////////////////////////
Alien::Alien(int imageID, int startX, int startY, StudentWorld* world)
	: SpaceShip(imageID, startX, startY, world, true, false, true, false, 0, 1.5, 1)	//sets hp to 0 which will change in body of constructor
{
	setHP(5 * (1 + (getWorld()->getLevel() - 1) * .1));	//stats for smallgon and smoregon. will be overwritten in snagglegon constructor body
	m_flightPlanLength = 0;								//
	m_travelSpeed = 2.0;								//
	m_pointsForDeath = 250;								//
	m_damageOnNBForCrash = 5;							//
}

void Alien::interact(Interactor* projectileOrNB)
{
	if (projectileOrNB->getIsActive() == false)		//if its already dead stop
		return;
	if (dynamic_cast<Projectile*>(projectileOrNB) != nullptr)			//if its a projectile go to correct interact (SS::interact)
		SpaceShip::interact(dynamic_cast<Projectile*>(projectileOrNB));	//
	else																//if its the NB crash into it
		crash(dynamic_cast<NachenBlaster*>(projectileOrNB));			//
}

void Alien::sufferDamage(double amountToDecrease)
{
	SpaceShip::sufferDamage(amountToDecrease);			//decreases health and checks for death 
	if (getIsActive() == false)							//if it dies
	{
		getWorld()->increaseScore(m_pointsForDeath);							//increase score, play the correct scound, explode, inc stat
		getWorld()->playSound(SOUND_DEATH);										//
		getWorld()->addActorToVector(new Explosion(getX(), getY(), getWorld()));//
		getWorld()->addOneAlienDeath();											//
	}
	else												//if it doesn't die play the correct sound. this only happens from projectile bc Aliens always die from crashing		
		getWorld()->playSound(SOUND_BLAST);				//
}

void Alien::crash(NachenBlaster* nb)
{
	nb->sufferDamage(m_damageOnNBForCrash);			//hurt the NB
	getWorld()->increaseScore(m_pointsForDeath);	//increase the score
	sufferDamage(getHP());							//kills alien
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
	if (getWorld()->checkCollisions_wThingsThat(&Actor::canCollideWAliens, this))	//check collisions w right func
		return true;
	return false;
}

void Alien::shoot()
{
	getWorld()->addActorToVector(new Turnip(getX() - 14, getY(), getWorld()));	//creates turnip and plays sound
	getWorld()->playSound(SOUND_ALIEN_SHOOT);									//
}

void Alien::move()
{
	m_flightPlanLength--;										//dec flightPlan
	if (m_flightPlan == DUE_LEFT)								//move in direction specified by m_flightPlan
		moveTo(getX() - m_travelSpeed, getY());					//
	else if (m_flightPlan == UP_AND_LEFT)						//
		moveTo(getX() - m_travelSpeed, getY() + m_travelSpeed);	//
	else if (m_flightPlan == DOWN_AND_LEFT)						//
		moveTo(getX() - m_travelSpeed, getY() - m_travelSpeed);	//
}

void Alien::changeFlightPlanIfNeeded()
{
	if (m_flightPlanLength != 0 && getY() < VIEW_HEIGHT - 1 && getY() > 0)	//if theres no reason to change course, dont
		return;																//
	if (getY() >= VIEW_HEIGHT - 1)			//if you're at the top
		setFlightPlan(DOWN_AND_LEFT);		//
	else if (getY() <= 0)					//if you're at the bottom
		setFlightPlan(UP_AND_LEFT);			//
	else if (m_flightPlanLength == 0)		//if you're length is 0
	{										//randomly choose a new direction
		int i = randInt(1, 3);				//	-this will never happen for 
		switch (i)							//	 snagglgons bc their length 
		{									//	 will never = 0
		case 1:								//
			setFlightPlan(DUE_LEFT);		//
			break;							//
		case 2:								//
			setFlightPlan(UP_AND_LEFT);		//
			break;							//
		case 3:								//
			setFlightPlan(DOWN_AND_LEFT);	//
			break;							//
		}									//
	}										//

	m_flightPlanLength = randInt(1, 32);	//set a new flight plan length (OK for snagglegons to do this bc they will call this then setLength back up)
}

bool Alien::maybeShootOrChangePlan()
{
	int i = randInt(1, (20 / getWorld()->getLevel()) + 5);	//odds to shoot if a Smallgon of Smoregon
	if (i == 1)
	{
		shoot();
		return true;	//returns true if it shoots
	}
	return false;		//returns false if it doesn't
}

void Alien::doSomething()
{
	if (!getIsActive())		//if its dead, don't do anything
		return;
	if (getX() < 0)			//set its dead if its off screen
	{
		setIsActive(false);
		return;
	}

	checkCollisions();		//check for collisions

	changeFlightPlanIfNeeded();	//check if it needs to change flight plan

	if (getWorld()->willSeeNachenBlaster(*this))	//if NB is in front
		if (maybeShootOrChangePlan())				//it might shoot/change plan
			return;									//   if it does shoot, end current doSomething()

	move();					//move according to possibly-new flight plan

	checkCollisions();		//check for collisions again
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
	Alien::sufferDamage(amountToDecrease);	//calls Alien's sufferDmg
	if (getIsActive() == false)				//if it dies might drop a goodie
	{
		if (randInt(1, 3) == 1)				//1/3 chance to drop a goodie
		{
			if (randInt(1, 2) == 1)			//50/50 between repair and torpedo
				getWorld()->addActorToVector(new Goodie(IID_REPAIR_GOODIE, getX(), getY(), getWorld()));
			else
				getWorld()->addActorToVector(new Goodie(IID_TORPEDO_GOODIE, getX(), getY(), getWorld()));
		}
	}
}

bool Smoregon::maybeShootOrChangePlan()
{
	if (Alien::maybeShootOrChangePlan())	//if it shoots
		return true;		//returns true to end doSomething() if it shot

	int i = randInt(1, (20 / getWorld()->getLevel()) + 5);	//does charging odds
	if (i == 1)
	{
		setFlightPlan(DUE_LEFT);
		setFlightPlanLength(VIEW_WIDTH);
		setTravelSpeed(5);
	}
	return false;		//returns false to continue w doSomething()
}

//////////////////////////////////////////////////
// SNAGGLEGON ////////////////////////////////////
//////////////////////////////////////////////////
Snagglegon::Snagglegon(int startX, int startY, StudentWorld* world)
	: Alien(IID_SNAGGLEGON, startX, startY, world)
{
	setHP(10 * (1 + (getWorld()->getLevel() - 1) * .1));	//sets stats to correct values since they differ from other two types of Aliens
	setFlightPlan(DOWN_AND_LEFT);							//
	setFlightPlanLength(10);								//
	setTravelSpeed(1.75);									//
	setPointsForDeath(1000);								//
	setDamageOnCrash(15);									//
}

void Snagglegon::sufferDamage(double amountToDecrease)
{
	Alien::sufferDamage(amountToDecrease);	//suffers damage
	if (getIsActive() == false)				//if it dies might drop a goodie
	{
		if (randInt(1, 6) == 1)					//1/6 chance to drop a extra life goodie
			getWorld()->addActorToVector(new Goodie(IID_LIFE_GOODIE, getX(), getY(), getWorld()));
	}
}

void Snagglegon::shoot()
{
	getWorld()->addActorToVector(new FlatulenceTorpedo(getX() - 14, getY(), getWorld(), 180));	//overrides Alien::shoot() bc FTorps
	getWorld()->playSound(SOUND_TORPEDO);
}

void Snagglegon::move()
{
	Alien::move();				//calls alien's move which decrements flightplan length
	setFlightPlanLength(10);	//so this makes up for that bc snagglegons dont use length
}

bool Snagglegon::maybeShootOrChangePlan()
{
	int i = randInt(1, (15 / getWorld()->getLevel()) + 10);	//has different odds to shoot so doesn't call Alien::maybeShootOrChangePlan()
	if (i == 1)
	{
		shoot();
		return true;	//returns true if shot to end current doSomething()
	}
	return false;		//returns false if it doesn't shoot to continue w actions
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
	if (ALorPJorGD->getIsActive() == false)		//if its dead just stop
		return;									//
	if (ALorPJorGD->isAlien())									//if its an alien, crash
		dynamic_cast<Alien*>(ALorPJorGD)->crash(this);			//
	else if (dynamic_cast<Projectile*>(ALorPJorGD) != nullptr)	//if its a projectile, SS::interact() will handle it
		SpaceShip::interact(ALorPJorGD);						//
	else														//if its a Goodie, Goodie::interact() will handle it
		dynamic_cast<Goodie*>(ALorPJorGD)->interact(this);		//
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
	if (!getIsActive())		//check its still active
		return;				//

	int ch;																					//check for input and move if possible
	int moveDistance = 6;																	//
	if (getWorld()->getKey(ch))																//
	{																						//
		switch (ch)																			//
		{																					//
		case KEY_PRESS_SPACE:																//
			shoot();																		//
			break;																			//
		case KEY_PRESS_TAB:																	//
			shoot(1);																		//
			break;																			//
		case KEY_PRESS_LEFT:																//
			if (getX() >= moveDistance)														//
				moveTo(getX() - moveDistance, getY());										//
			break;																			//
		case KEY_PRESS_RIGHT:																//
			if (getX() < VIEW_WIDTH - moveDistance)											//
				moveTo(getX() + moveDistance, getY());										//	
			break;																			//
		case KEY_PRESS_UP:																	//
			if (getY() < VIEW_HEIGHT - moveDistance)										//	
				moveTo(getX(), getY() + moveDistance);										//
			break;																			//
		case KEY_PRESS_DOWN:																//
			if (getY() >= moveDistance)														//
				moveTo(getX(), getY() - moveDistance);										//
			break;																			//
		}																					//
	}																						//

	checkCollisions();	//check for collisions

	if (m_cabbage < 30)				//gets 1 cabbage every tick up to 30 max
		m_cabbage++;				//	
}

void NachenBlaster::shoot()	//used for cabbage
{
	if (m_cabbage >= 5)			//if you have enough ammo													
	{
		m_cabbage -= 5;																//create a cabbage and 'fire' it
		getWorld()->addActorToVector(new Cabbage(getX() + 12, getY(), getWorld()));	//
		getWorld()->playSound(SOUND_PLAYER_SHOOT);									//
	}
}

void NachenBlaster::shoot(int doesntMatter)	//used for torpedos
{
	if (m_fTorpedos > 0)	//if you have any torpedos														
	{
		m_fTorpedos--;															//create a torpedo and 'fire' it	
		getWorld()->addActorToVector(new FlatulenceTorpedo(getX() + 12, getY(),	//	
			getWorld(), 0));	//
		getWorld()->playSound(SOUND_TORPEDO);									//	
	}
}

//////////////////////////////////////////////////
// STAR  /////////////////////////////////////////
//////////////////////////////////////////////////
Star::Star(int startX, int startY, StudentWorld* world)
	: Actor(IID_STAR, startX, startY, world, false, false, false, false, 0, 0, 3)	//sets size to 0 here but sets it to actual value in body of constructor
{
	int s = randInt(5, 50);		//set proper size
	double ns = s;				//
	setSize(ns / 100);			//
}

void Star::doSomething()
{
	moveTo(getX() - 1, getY());		//move left
	if (getX() < 0)					//if it leaves screen set it to be deleted
		setIsActive(false);			//
}

//////////////////////////////////////////////////
// EXPLOSION  ////////////////////////////////////
//////////////////////////////////////////////////
Explosion::Explosion(int startX, int startY, StudentWorld* world)
	: Actor(IID_EXPLOSION, startX, startY, world, false, false, false, false /*direction = 0, size = 1, depth = 0*/)
{
	m_age = 0;	//starting age of 0
}

void Explosion::doSomething()
{
	setSize(getSize() * 1.5);	//every tick increase size by factor of 1.5
	m_age++;					//increment age
	if (m_age == 4)				//if its gone 4 ticks, set to dead
		setIsActive(false);		//
}

//////////////////////////////////////////////////
// PROJECTILE  ///////////////////////////////////
//////////////////////////////////////////////////
Projectile::Projectile(int imageID, int startX, int startY, StudentWorld* world, bool canCollideWNB, bool canCollideWAliens, int startDirection, int velocity, bool rotates, bool isFriendly)
	: Interactor(imageID, startX, startY, world, false, false, canCollideWNB, canCollideWAliens, startDirection, .5, 1)
{
	m_velocity = velocity;		//set member variables
	m_rotates = rotates;		//
	m_isFriendly = isFriendly;	//
	m_damageOnContact = 2;		//for cabbage and turnips. torpedo gets 8 in its own constructor body
}

void Projectile::interact(Interactor* s)
{
	if (s->getIsActive() == false)		//if its dead stop
		return;
	impact(dynamic_cast<SpaceShip*>(s));	//s will only be of type spaceship
}

bool Projectile::checkCollisions()
{
	if (getIsFriendly())					//if its friendly call function looking for Aliens
	{
		if (getWorld()->checkCollisions_wThingsThat(&Actor::isAlien, this))
			return true;
		return false;
	}
	else									//if its not friendly call func looking for NB
	{
		if (getWorld()->checkCollisions_wThingsThat(&Actor::isNB, this))
			return true;
		return false;
	}
}

void Projectile::impact(SpaceShip* s)
{
	s->sufferDamage(m_damageOnContact);	//hurt ship based on projectile dmg
	setIsActive(false);					//set projectile to dead
}

void Projectile::doSomething()
{
	if (!getIsActive())			//if its dead stop
		return;					//
	if (getX() < 0 || getX() >= VIEW_WIDTH)		//if its off the screen set to dead and stop
	{											//
		setIsActive(false);						//
		return;									//
	}											//

	if (checkCollisions())			//check collisions, if it does, return
		return;						//

	moveTo(getX() + getVelocity(), getY());		//move by velocity
	if (getRotates())							//if it rotates, rotate it
		setDirection(getDirection() + 20);		//

	checkCollisions();			//check collisions again
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
FlatulenceTorpedo::FlatulenceTorpedo(int startX, int startY, StudentWorld* world, int direction)	//set direction to 0 if by nachenb, 180 if by alien!
	: Projectile(IID_TORPEDO, startX, startY, world, false, true, direction, 8, false, true)		//defaults velocity to positive, friendly to true,
{																					// and can only collide w aliens (all for nachenblaster firing)
	if (getDirection() == 180)				//if it was shot by alien: 
	{										//	change velocity to negative
		setVelocity(-1 * getVelocity());	// AND 
		setIsFriendly(false);				//  set friendly to false
		setCanCollideWNB(true);				// AND
		setCanCollideWAliens(false);		//  set it so it hits NB, not aliens
	}

	setDamageOnContact(8);		//correct this variable for torpedos
}