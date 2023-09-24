#ifndef ACTOR_H_
#define ACTOR_H_

#include <string>
#include "GraphObject.h"

class StudentWorld;

class Actor : public GraphObject 
{
public:
	Actor(int imageID, int startX, int startY, int startDirection, int depth, double size, bool impactable, bool issquare, StudentWorld* sw);
	virtual void doSomething() = 0;
	virtual void hitByVortex();
	StudentWorld* getWorld() const;
	bool getActive() const;
	void setInactive();
	bool checkIfAvatarIsOn(Actor* avatar) const;
	bool isImpactable() const;
	bool isASquare() const;
	int getBoardX() const;
	int getBoardY() const;
private:
	bool m_impactable;
	StudentWorld* m_world;
	bool m_active;
	bool m_is_a_square;
};

class movableEntity : public Actor
{
public:
	movableEntity(int imageID, int startX, int startY, int startDirection, int depth, double size, bool impactable, int walkDirection, StudentWorld* sw);
	std::string getState() const;
	void setState(std::string state);
	int getWalkDirection() const;
	void setWalkDirection(int dir);
	int getTicks();
	void setTicks(int ticks_to_move);
	bool moveToSquareInFront(int dir);
	void findNewWalkDirection(); // both the find walk directions will automatically update sprite direction
	void findNewRandomLegalWalkDirection();
	void setSpriteDirection();
	bool getHasMovedToNewSquare() const; // check if player/enemy moved to new square
	void setHasMovedToNewSquare(bool b);
	virtual void teleport();
	void postTeleportUpdate();
	virtual bool onFork();
	void move();	
	bool reachedNewSquareUpdate(); // mark movable entities as having reached a new square
private:
	std::string m_state;
	int m_walk_direction;
	int m_ticks_to_move;
	bool m_has_moved_to_new_square;
	int m_oldX;
	int m_oldY;


	int getOldX() const; // checking for old positions to see if player/enemy moved to new square
	int getOldY() const;
	void setOldX(int x);
	void setOldY(int y);
};

class Vortex : public movableEntity
{
public:
	Vortex(int imageID, int startX, int startY, int dir, StudentWorld* sw);
	virtual void doSomething();
};

class Avatar : public movableEntity
{
public:
	Avatar(int imageID, int startX, int startY, int player_num, StudentWorld* sw);
	virtual void doSomething();
	int getCoins() const;
	void adjustCoins(int coins);
	int getStars() const;
	void adjustStars(int stars);
	int getDieRoll();
	bool getVortex() const;
	void setVortex(bool has_vortex);
	virtual void teleport();
	void swapAvatar(Avatar* avatar);
	bool getCheckToActivateEventAfterSwap() const; // can't activate event after it's been swapped
	void setOnDirectionalSquare(bool on);
	void swapCoins(Avatar* other);
	void swapStars(Avatar* other);
private:
	int m_player;
	int m_coins;
	int m_stars;
	bool m_has_vortex;
	bool m_check_to_activate_event_after_swap; // only for player that activated event square
	bool m_on_directional_square; // check for a directional square since a directional square isn't a fork

	bool OnDirectionalSquare() const;
	void findSquareInFront(int& x, int& y, int dir);
	virtual bool onFork();
};

class Enemy : public movableEntity 
{
public:
	Enemy(int imageID, int startX, int startY, int numSqToMove, StudentWorld* sw);
	virtual void doSomething();
	int shouldEnemyAct(Avatar* avatar);
	virtual void hitByVortex();
private:
	int m_pause_counter;
	int m_num_sq_to_move;

	virtual void enemyAction(Avatar* avatar) = 0;
	virtual void actionWhenAtZeroTicks();
	void setPauseCounter(int pause);
};

class Bowser : public Enemy
{
public:
	Bowser(int imageID, int startX, int startY, StudentWorld* sw);
private:
	virtual void enemyAction(Avatar* avatar);
	virtual void actionWhenAtZeroTicks();
};

class Boo : public Enemy
{
public:
	Boo(int imageID, int startX, int startY, StudentWorld* sw);
private:
	virtual void enemyAction(Avatar* avatar);
};

class Square : public Actor
{
public:
	Square(int imageID, int startX, int startY, int dir, StudentWorld* sw);
	void doSomething();
	bool checkIfNewPlayerLandedOnSquare(Avatar* avatar);
	bool Square::checkIfNewPlayerMovedOverSquare(Avatar* avatar);
private:
	virtual void activateOnPlayerLand(Avatar* avatar) = 0;
	virtual void activateOnPlayerMoveOver(Avatar* avatar);
};

class coinSquare : public Square
{
public:
	coinSquare(int imageID, int startX, int startY, int coins_to_give, StudentWorld* sw);
private:
	int m_coins_to_give;

	virtual void activateOnPlayerLand(Avatar* avatar);
};

class starSquare : public Square
{
public:
	starSquare(int imageID, int startX, int startY, StudentWorld* sw);
private:
	virtual void activateOnPlayerLand(Avatar* avatar);
	virtual void activateOnPlayerMoveOver(Avatar* avatar);
	void grantStars(Avatar* avatar);
};

class directionalSquare : public Square
{
public:
	directionalSquare(int imageID, int startX, int startY, int sprite_dir, StudentWorld* sw);
private:
	int m_forcing_direction;

	virtual void activateOnPlayerLand(Avatar* avatar);
};

class bankSquare : public Square
{
public:
	bankSquare(int imageID, int startX, int startY, StudentWorld* sw);
private:
	virtual void activateOnPlayerLand(Avatar* avatar);
	virtual void activateOnPlayerMoveOver(Avatar* avatar);
};

class eventSquare : public Square
{
public: 
	eventSquare(int imageID, int startX, int startY, StudentWorld* sw);
private:
	virtual void activateOnPlayerLand(Avatar* avatar);
};

class droppingSquare : public Square
{
public: 
	droppingSquare(int imageID, int startX, int startY, StudentWorld* sw);
private:
	virtual void activateOnPlayerLand(Avatar* avatar);
};

#endif // ACTOR_H_
