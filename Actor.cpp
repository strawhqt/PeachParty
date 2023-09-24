#include "Actor.h"
#include "StudentWorld.h"
using namespace std;

Actor::Actor(int imageID, int startX, int startY, int startDirection, int depth, double size, bool impactable, bool is_square, StudentWorld* sw)
	:GraphObject(imageID, startX, startY, startDirection, depth, size)
{
	m_world = sw;
	m_active = true;
	m_impactable = impactable;
	m_is_a_square = is_square;
}

StudentWorld* Actor::getWorld() const { return m_world; }

bool Actor::getActive() const { return m_active; }

bool Actor::isImpactable() const { return m_impactable; }

int Actor::getBoardX() const { return getX() / SPRITE_WIDTH; }

int Actor::getBoardY() const { return getY() / SPRITE_HEIGHT; }

void Actor::setInactive() {	m_active = false; }

bool Actor::isASquare() const { return m_is_a_square; }

bool Actor::checkIfAvatarIsOn(Actor* avatar) const
{
	return avatar->getX() == getX() && avatar->getY() == getY();
}

void Actor::hitByVortex() { }


movableEntity::movableEntity(int imageID, int startX, int startY, int startDirection, int depth, double size, bool impactable, int walkDirection, StudentWorld* sw)
	:Actor(imageID, startX, startY, startDirection, depth, size, impactable, false, sw)
{
	m_state = "";
	m_walk_direction = walkDirection;
	m_ticks_to_move = 0;
	m_has_moved_to_new_square = true;
	m_oldX = getBoardX();
	m_oldY = getBoardY();
}

string movableEntity::getState() const { return m_state; }

void movableEntity::setState(string state) { m_state = state; }

int movableEntity::getWalkDirection() const { return m_walk_direction; }

void movableEntity::setWalkDirection(int dir) {	m_walk_direction = dir; }

int movableEntity::getTicks() {	return m_ticks_to_move; }

void movableEntity::setTicks(int ticks_to_move) { m_ticks_to_move = ticks_to_move; }

void movableEntity::setSpriteDirection()
{
	if (getWalkDirection() == 180)
		setDirection(180);
	else
		setDirection(0);
}

bool movableEntity::moveToSquareInFront(int dir)
{
	int x = getX();
	int y = getY();

	switch(dir)
	{
		case 0:
		{
			if (getWorld()->getBoard().getContentsOf((x + SPRITE_WIDTH) / SPRITE_WIDTH, y / SPRITE_HEIGHT) == Board::empty)
				return false;
			break;
		}
		case 90:
		{
			if (getWorld()->getBoard().getContentsOf(x / SPRITE_WIDTH, (y + SPRITE_HEIGHT) / SPRITE_HEIGHT) == Board::empty)
				return false;
			break;
		}
		case 180:
		{
			if (x - 2 < 0) return false;
			if (getWorld()->getBoard().getContentsOf((x - 2) / SPRITE_WIDTH, y / SPRITE_HEIGHT) == Board::empty)
				return false;
			break;
		}
		case 270:
		{
			if (y - 2 < 0) return false;
			if (getWorld()->getBoard().getContentsOf(x / SPRITE_WIDTH, (y - 2) / SPRITE_HEIGHT) == Board::empty)
				return false;
			break;
		}
	}
	return true;
}

void movableEntity::findNewRandomLegalWalkDirection()
{
	bool found_valid_direction = false;
	while (!found_valid_direction)
	{
		int random_dir = randInt(0, 3) * 90;
		if (moveToSquareInFront(random_dir))
		{
			setWalkDirection(random_dir);
			setSpriteDirection();
			found_valid_direction = true;
		}
	}
}


void movableEntity::findNewWalkDirection()
{
	if (getWalkDirection() == 0 || getWalkDirection() == 180)
	{
		if (moveToSquareInFront(90))
			setWalkDirection(90);
		else
			setWalkDirection(270);
	}
	else
	{
		if (moveToSquareInFront(0))
			setWalkDirection(0);
		else
			setWalkDirection(180);
	}
	setSpriteDirection();
}

bool movableEntity::getHasMovedToNewSquare() const { return m_has_moved_to_new_square; }

void movableEntity::setHasMovedToNewSquare(bool b) { m_has_moved_to_new_square = b; }

int movableEntity::getOldX() const { return m_oldX; }

int movableEntity::getOldY() const { return m_oldY; }

void movableEntity::setOldX(int x) { m_oldX = x; }

void movableEntity::setOldY(int y) { m_oldY = y; }

void movableEntity::teleport()
{
	int x = getBoardX();
	int y = getBoardY();
	bool teleported = false;
	while (!teleported)
	{
		int newX = randInt(0, BOARD_WIDTH - 1);
		int newY = randInt(0, BOARD_HEIGHT - 1);
		if (newX != x || newY != y)
		{
			if (getWorld()->getBoard().getContentsOf(newX, newY) != Board::empty)
			{
				moveTo(newX * SPRITE_WIDTH, newY * SPRITE_HEIGHT);
				teleported = true;
			}
		}
	}
	postTeleportUpdate();
}

void movableEntity::postTeleportUpdate()
{
	setHasMovedToNewSquare(true);
	setOldX(getBoardX());
	setOldY(getBoardY());
}

bool movableEntity::onFork()
{
	int count = 0;
	for (int i = 0; i < 4; i++)
	{
		if (moveToSquareInFront(i * 90))
			count++;
	}
	return count >= 3;
}

bool movableEntity::reachedNewSquareUpdate()
{
	if (getX() % SPRITE_WIDTH == 0 && getY() % SPRITE_HEIGHT == 0) // standing on corner of square
	{
		if (getX() != getOldX() * SPRITE_WIDTH || getY() != getOldY() * SPRITE_HEIGHT)
		{
			setOldX(getBoardX());
			setOldY(getBoardY());
			setHasMovedToNewSquare(true);
			return true;
		}
	}
	else
		setHasMovedToNewSquare(false);
	return false;
}

void movableEntity::move()
{
	moveAtAngle(getWalkDirection(), 2);
	setTicks(getTicks() - 1);
}

Vortex::Vortex(int imageID, int startX, int startY, int dir,  StudentWorld* sw)
	:movableEntity(imageID, startX, startY, 0, 0, 1.0, false, dir, sw)
{
}

void Vortex::doSomething()
{
	if (!getActive())
		return;

	moveAtAngle(getWalkDirection(), 2);

	if (getX() < 0 || getX() > VIEW_WIDTH - 1 || getY() < 0 || getY() > VIEW_HEIGHT - 1)
		setInactive();

	Actor* overlapped_object = getWorld()->overlapWithVortex(getX(), getY());
	if (overlapped_object != nullptr)
	{
		overlapped_object->hitByVortex();
		setInactive();
		getWorld()->playSound(SOUND_HIT_BY_VORTEX);
	}
}

Avatar::Avatar(int imageID, int startX, int startY, int player_num, StudentWorld* sw)
	:movableEntity(imageID, startX, startY, 0, 0, 1.0, false, 0, sw)
{
	m_player = player_num;
	m_coins = 0;
	m_stars = 0;
	m_has_vortex = false;
	m_check_to_activate_event_after_swap = false;
	m_on_directional_square = false;
	setState("waiting to roll");
}

void Avatar::doSomething()
{
	if (getState() == "waiting to roll")
	{
		if (getWalkDirection() == -1)
			findNewRandomLegalWalkDirection();

		switch (getWorld()->getAction(m_player))
		{
			case ACTION_ROLL:
			{
				int die_roll = randInt(1, 10);
				setTicks(die_roll * 8);
				setState("walking");
				break;
			}
			case ACTION_FIRE:
			{
				if(getVortex())
				{
					int x = -1; int y = -1;
					findSquareInFront(x, y, getWalkDirection());
					getWorld()->addActor(new Vortex(IID_VORTEX, x, y, getWalkDirection(), getWorld()));
					getWorld()->playSound(SOUND_PLAYER_FIRE);
					setVortex(false);
				}
				break;
			}
			case ACTION_NONE:
				return;
			default:
				return;
		}
	}

	if (getState() == "walking")
	{
		if (getX() % SPRITE_WIDTH == 0 && getY() % SPRITE_HEIGHT == 0 && onFork())
		{
			int input_direction = -1;
			switch (getWorld()->getAction(m_player))
			{
				case ACTION_RIGHT:
					input_direction = 0;
					break;
				case ACTION_UP:
					input_direction = 90;
					break;
				case ACTION_LEFT:
					input_direction = 180;
					break;
				case ACTION_DOWN:
					input_direction = 270;
					break;
				default:
					break;
			}
			if (input_direction != -1 && abs(getWalkDirection() - input_direction) != 180 && moveToSquareInFront(input_direction))
			{
				setWalkDirection(input_direction);
				setSpriteDirection();
			}
			else
				return;
		}
		else if (!moveToSquareInFront(getWalkDirection()))
			findNewWalkDirection();

		move();

		if(reachedNewSquareUpdate()) // marks moving to new square if it reaches one
		{
			m_check_to_activate_event_after_swap = false; // event squares now activate on avatar
			m_on_directional_square = false; // not on directional square after it leaves
		}

		if (getTicks() == 0)
			setState("waiting to roll");
	}
}

bool Avatar::onFork()
{
	return movableEntity::onFork() && !OnDirectionalSquare();
}

void Avatar::findSquareInFront(int& x, int& y, int dir)
{
	x = getX();
	y = getY();
	switch (dir)
	{
		case 0:
			x += SPRITE_WIDTH;
			break;
		case 90:
			y += SPRITE_HEIGHT;
			break;
		case 180:
			x -= SPRITE_WIDTH;
			break;
		case 270:
			y -= SPRITE_HEIGHT;
			break;
		default:
			return;
	}
}

int Avatar::getCoins() const { return m_coins; }

void Avatar::adjustCoins(int coins)
{
	if (coins < 0 && m_coins < -coins)
		m_coins = 0;
	else
		m_coins += coins;
}

int Avatar::getStars() const { return m_stars; }

void Avatar::adjustStars(int stars) { m_stars += stars; }

int Avatar::getDieRoll() { return ceil(double(getTicks()) / 8); }

bool Avatar::getVortex() const { return m_has_vortex; }

void Avatar::setVortex(bool has_vortex) { m_has_vortex = has_vortex; }

bool Avatar::getCheckToActivateEventAfterSwap() const { return m_check_to_activate_event_after_swap; }

bool Avatar::OnDirectionalSquare() const { return m_on_directional_square; }

void Avatar::setOnDirectionalSquare(bool on) { m_on_directional_square = on; }

void Avatar::teleport()
{
	movableEntity::teleport();
	setWalkDirection(-1); 
}

void Avatar::swapCoins(Avatar* other)
{
	int temp_coins = m_coins;
	m_coins = other->m_coins;
	other->m_coins = temp_coins;	
}

void Avatar::swapStars(Avatar* other)
{
	int temp_stars = m_stars;
	m_stars = other->m_stars;
	other->m_stars = temp_stars;
}

void Avatar::swapAvatar(Avatar* avatar)
{
	Avatar* other_avatar = getWorld()->getOtherPlayer(avatar);

	int tempX = avatar->getX();
	int tempY = avatar->getY();
	avatar->moveTo(other_avatar->getX(), other_avatar->getY());
	other_avatar->moveTo(tempX, tempY);

	int temp_ticks = avatar->getTicks();
	avatar->setTicks(other_avatar->getTicks());
	other_avatar->setTicks(temp_ticks);

	int temp_walk_dir = avatar->getWalkDirection();
	avatar->setWalkDirection(other_avatar->getWalkDirection());
	other_avatar->setWalkDirection(temp_walk_dir);

	int temp_sprite_dir = avatar->getDirection();
	avatar->setDirection(other_avatar->getDirection());
	other_avatar->setDirection(temp_sprite_dir);

	string temp_state = avatar->getState();
	avatar->setState(other_avatar->getState());
	other_avatar->setState(temp_state);

	postTeleportUpdate();
	other_avatar->postTeleportUpdate();
	other_avatar->m_check_to_activate_event_after_swap = true;
}

Enemy::Enemy(int imageID, int startX, int startY, int numSquareToMove, StudentWorld* sw)
	:movableEntity(imageID, startX, startY, 0, 0, 1.0, true, 0, sw)
{
	m_pause_counter = 180;
	m_num_sq_to_move = numSquareToMove;
	setState("Paused");
}

void Enemy::doSomething()
{
	if (getState() == "Paused")
	{
		enemyAction(getWorld()->getPeach());
		enemyAction(getWorld()->getYoshi());

		m_pause_counter--;

		if (m_pause_counter == 0)
		{
			int squares_to_move = randInt(1, m_num_sq_to_move);
			setTicks(squares_to_move * 8);
			findNewRandomLegalWalkDirection();
			setState("Walking");
		}
	}

	if (getState() == "Walking")
	{
		if (getX() % SPRITE_WIDTH == 0 && getY() % SPRITE_HEIGHT == 0 && onFork())
			findNewRandomLegalWalkDirection();
		else if (!moveToSquareInFront(getWalkDirection()))
			findNewWalkDirection();

		move();
		reachedNewSquareUpdate();

		if (getTicks() == 0)
		{
			setState("Paused");
			setPauseCounter(180);
			actionWhenAtZeroTicks();
		}
	}
}

int Enemy::shouldEnemyAct(Avatar* avatar)
{
	if (checkIfAvatarIsOn(avatar) && (getHasMovedToNewSquare() || avatar->getHasMovedToNewSquare()) && avatar->getState() != "walking")
	{
		setHasMovedToNewSquare(false);
		return randInt(1, 2);
	}
	return -1;
}

void Enemy::actionWhenAtZeroTicks() { }

void Enemy::hitByVortex()
{
	teleport();
	setWalkDirection(0);
	setSpriteDirection();
	setState("Paused");
	setPauseCounter(180);
}

void Enemy::setPauseCounter(int pause) { m_pause_counter = pause; }

Bowser::Bowser(int imageID, int startX, int startY, StudentWorld* sw)
	:Enemy(imageID, startX, startY, 10, sw)
{
}

void Bowser::enemyAction(Avatar* avatar)
{
	if (shouldEnemyAct(avatar) == 1)
	{
		avatar->adjustCoins(-(avatar->getCoins()));
		avatar->adjustStars(-(avatar->getStars()));
		getWorld()->playSound(SOUND_BOWSER_ACTIVATE);
	}
}

void Bowser::actionWhenAtZeroTicks()
{
	int x = randInt(1, 4);
	if (x == 1)
	{
		getWorld()->getSquareAtLocation(getX(), getY())->setInactive();
		getWorld()->addActor(new droppingSquare(IID_DROPPING_SQUARE, getX(), getY(), getWorld()));

		// if dropping squares placed under an avatar do appropriate actions
		getWorld()->droppingSquarePlacedUnderPlayer(getWorld()->getPeach(), getX(), getY());
		getWorld()->droppingSquarePlacedUnderPlayer(getWorld()->getYoshi(), getX(), getY());

		getWorld()->playSound(SOUND_DROPPING_SQUARE_CREATED);
	}
}

Boo::Boo(int imageID, int startX, int startY, StudentWorld* sw)
	:Enemy(imageID, startX, startY, 3, sw)
{
}

void Boo::enemyAction(Avatar* avatar)
{
	Avatar* other_avatar = getWorld()->getOtherPlayer(avatar);
	int action = shouldEnemyAct(avatar);

	if (action == 1)
		avatar->swapCoins(other_avatar);
	else if (action == 2)
		avatar->swapStars(other_avatar);

	if (action == 1 || action == 2)
		getWorld()->playSound(SOUND_BOO_ACTIVATE);
}

Square::Square(int imageID, int startX, int startY, int dir, StudentWorld* sw)
	:Actor(imageID, startX, startY, dir, 1, 1.0, false, true, sw)
{
}

void Square::doSomething()
{
	if (!getActive())
		return;
	
	activateOnPlayerLand(getWorld()->getPeach());
	activateOnPlayerMoveOver(getWorld()->getPeach());
	activateOnPlayerLand(getWorld()->getYoshi());
	activateOnPlayerMoveOver(getWorld()->getYoshi());
}

void Square::activateOnPlayerMoveOver(Avatar* avatar) { }

bool Square::checkIfNewPlayerLandedOnSquare(Avatar* avatar)
{
	return checkIfAvatarIsOn(avatar) && avatar->getHasMovedToNewSquare() && avatar->getState() != "walking"; 
}

bool Square::checkIfNewPlayerMovedOverSquare(Avatar* avatar)
{
	return checkIfAvatarIsOn(avatar) && avatar->getHasMovedToNewSquare() && avatar->getState() == "walking";
}

coinSquare::coinSquare(int imageID, int startX, int startY, int coins_to_give, StudentWorld* sw)
	:Square(imageID, startX, startY, 0, sw)
{
	m_coins_to_give = coins_to_give;
}

void coinSquare::activateOnPlayerLand(Avatar* avatar)
{
	if (checkIfNewPlayerLandedOnSquare(avatar))
	{
		avatar->setHasMovedToNewSquare(false); 
		avatar->adjustCoins(m_coins_to_give);
		if (m_coins_to_give == 3)
			getWorld()->playSound(SOUND_GIVE_COIN);
		else
			getWorld()->playSound(SOUND_TAKE_COIN);
	}
}

starSquare::starSquare(int imageID, int startX, int startY, StudentWorld* sw)
	:Square(imageID, startX, startY, 0, sw)
{
}

void starSquare::grantStars(Avatar* avatar)
{
	avatar->setHasMovedToNewSquare(false);
	if (avatar->getCoins() < 20)
		return;
	avatar->adjustCoins(-20);
	avatar->adjustStars(1);
	getWorld()->playSound(SOUND_GIVE_STAR);
}

void starSquare::activateOnPlayerLand(Avatar* avatar)
{
	if (checkIfNewPlayerLandedOnSquare(avatar))
	{
		grantStars(avatar);
	}
}

void starSquare::activateOnPlayerMoveOver(Avatar* avatar)
{
	if (checkIfNewPlayerMovedOverSquare(avatar))
	{
		grantStars(avatar);
	}
}

directionalSquare::directionalSquare(int imageID, int startX, int startY, int sprite_dir, StudentWorld* sw)
	:Square(imageID, startX, startY, sprite_dir, sw)
{
	m_forcing_direction = sprite_dir;
}

void directionalSquare::activateOnPlayerLand(Avatar* avatar)
{
	if (checkIfAvatarIsOn(avatar) && avatar->getHasMovedToNewSquare()) // both waiting and walking states allowed
	{
		avatar->setHasMovedToNewSquare(false);
		avatar->setOnDirectionalSquare(true);
		avatar->setWalkDirection(m_forcing_direction);
		avatar->setSpriteDirection();
	}
}

bankSquare::bankSquare(int imageID, int startX, int startY, StudentWorld* sw)
	:Square(imageID, startX, startY, 0, sw)
{
}

void bankSquare::activateOnPlayerLand(Avatar* avatar)
{
	if (checkIfNewPlayerLandedOnSquare(avatar))
	{
		avatar->setHasMovedToNewSquare(false);
		avatar->adjustCoins(getWorld()->getBank());
		getWorld()->resetBank();
		getWorld()->playSound(SOUND_WITHDRAW_BANK);
	}
}

void bankSquare::activateOnPlayerMoveOver(Avatar* avatar)
{
	if (checkIfNewPlayerMovedOverSquare(avatar))
	{
		avatar->setHasMovedToNewSquare(false);
		int add_to_bank = 5;
		if (avatar->getCoins() < 5)
			add_to_bank = avatar->getCoins();
		avatar->adjustCoins(-5);
		getWorld()->addBank(add_to_bank);
		getWorld()->playSound(SOUND_DEPOSIT_BANK);
	}
}

eventSquare::eventSquare(int imageID, int startX, int startY, StudentWorld* sw)
	:Square(imageID, startX, startY, 0, sw)
{	
}

void eventSquare::activateOnPlayerLand(Avatar* avatar)
{
	if (checkIfNewPlayerLandedOnSquare(avatar) && !avatar->getCheckToActivateEventAfterSwap())
	{
		avatar->setHasMovedToNewSquare(false); 
		int x = randInt(1,3);
		switch (x)
		{
			case 1:
			{
				avatar->teleport();
				getWorld()->playSound(SOUND_PLAYER_TELEPORT);
				break;
			}
			case 2:
			{
				avatar->swapAvatar(avatar);
				getWorld()->playSound(SOUND_PLAYER_TELEPORT);
				break;
			}
			case 3:
			{
				avatar->setVortex(true);
				getWorld()->playSound(SOUND_GIVE_VORTEX);
				break;
			}
		}
	}
	if (avatar->getCheckToActivateEventAfterSwap())
	{
		avatar->setHasMovedToNewSquare(false);
	}
}

droppingSquare::droppingSquare(int imageID, int startX, int startY, StudentWorld* sw)
	:Square(imageID, startX, startY, 0, sw)
{
}

void droppingSquare::activateOnPlayerLand(Avatar* avatar)
{
	if (checkIfNewPlayerLandedOnSquare(avatar))
	{
		avatar->setHasMovedToNewSquare(false);
		int x = randInt(1, 2);
		if (x == 1)
			avatar->adjustCoins(-10);
		else
		{
			if (avatar->getStars() >= 1)
				avatar->adjustStars(-1);
		}
		getWorld()->playSound(SOUND_DROPPING_SQUARE_ACTIVATE);
	}
}