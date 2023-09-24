#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Board.h"
#include "Actor.h"
#include <vector>

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath);
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	Board& getBoard();
	Avatar* getPeach();
	Avatar* getYoshi();
	int getBank() const;
	void addBank(int coins);
	void resetBank();
	void addActor(Actor* actor);
	Actor* getSquareAtLocation(int x, int y);
	Avatar* getOtherPlayer(Avatar* avatar);
	void droppingSquarePlacedUnderPlayer(Avatar* avatar, int x, int y);
	Actor* overlapWithVortex(int x, int y);
	~StudentWorld();

private:
	std::vector<Actor*> m_actors;
	Avatar* m_peach;
	Avatar* m_yoshi;
	Board m_bd;
	int m_bank;

	bool overlap(int vx, int vy, int ax, int ay);
	bool overlapHelper(int bx1, int by1, int tx1, int ty1, int bx2, int by2, int tx2, int ty2);
	Avatar* findWinner();

	std::string addAvatarStats(Avatar* avatar);
};

#endif // STUDENTWORLD_H_
