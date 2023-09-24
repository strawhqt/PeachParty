#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    m_peach = nullptr;
    m_yoshi = nullptr;
    m_bank = 0;
}

int StudentWorld::init()
{
    m_bank = 0;
    
    int board_num = getBoardNumber();
    string board_file = assetPath() + "board0" + to_string(board_num) + ".txt";
    Board::LoadResult result = m_bd.loadBoard(board_file);

    if (result == Board::load_fail_file_not_found || result == Board::load_fail_bad_format)
        return GWSTATUS_BOARD_ERROR;

    if (result == Board::load_success)
    {
        for (int i = 0; i < BOARD_WIDTH; i++)
        {
            for (int j = 0; j < BOARD_HEIGHT; j++)
            {
                Board::GridEntry ge = m_bd.getContentsOf(i, j);
                switch (ge)
                    {
                    case Board::empty:
                        break;
                    case Board::player:
                    {
                        m_peach = new Avatar(IID_PEACH, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 1, this);
                        m_yoshi = new Avatar(IID_YOSHI, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 2, this);
                        m_actors.push_back(new coinSquare(IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 3, this));
                        break;
                    }
                    case Board::blue_coin_square:
                        m_actors.push_back(new coinSquare(IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 3, this));
                        break;
                    case Board::red_coin_square:
                        m_actors.push_back(new coinSquare(IID_RED_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, -3, this));
                        break;
                    case Board::star_square:
                        m_actors.push_back(new starSquare(IID_STAR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, this));
                        break;
                    case Board::right_dir_square:
                        m_actors.push_back(new directionalSquare(IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 0, this));
                        break;
                    case Board::up_dir_square:
                        m_actors.push_back(new directionalSquare(IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 90, this));
                        break;
                    case Board::left_dir_square:
                        m_actors.push_back(new directionalSquare(IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 180, this));
                        break;
                    case Board::down_dir_square:
                        m_actors.push_back(new directionalSquare(IID_DIR_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 270, this));
                        break;
                    case Board::bank_square:
                        m_actors.push_back(new bankSquare(IID_BANK_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, this));
                        break;
                    case Board::event_square:
                        m_actors.push_back(new eventSquare(IID_EVENT_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, this));
                        break;
                    case Board::bowser:
                    {
                        m_actors.push_back(new Bowser(IID_BOWSER, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, this));
                        m_actors.push_back(new coinSquare(IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 3, this));
                        break;
                    }
                    case Board::boo:
                    {
                        m_actors.push_back(new Boo(IID_BOO, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, this));
                        m_actors.push_back(new coinSquare(IID_BLUE_COIN_SQUARE, i * SPRITE_WIDTH, j * SPRITE_HEIGHT, 3, this));
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    startCountdownTimer(99);  
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    ostringstream game_text;
    game_text << "P1 " << addAvatarStats(m_peach);
    game_text << " | Time: " << timeRemaining() << " | Bank: " << getBank() << " | ";
    game_text << "P2 " << addAvatarStats(m_yoshi);
    setGameStatText(game_text.str());
    
    m_peach->doSomething();
    m_yoshi->doSomething();

    vector <Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if((*it)->getActive())
            (*it)->doSomething();
    }

    if (!m_peach->getActive())
    {
        delete m_peach;
        m_peach = nullptr;
    }
    if (!m_yoshi->getActive())
    {
        delete m_yoshi;
        m_yoshi = nullptr;
    }

    it = m_actors.begin();
    while(it != m_actors.end())
    {
        if (!(*it)->getActive())
        {
            delete (*it);
            it = m_actors.erase(it);
        }
        else
        {
            it++;
        }
    }

    if (timeRemaining() <= 0)
    {
        playSound(SOUND_GAME_FINISHED);
        Avatar* winner = findWinner();
        setFinalScore(winner->getStars(), winner->getCoins());
        if (winner == m_peach)
            return GWSTATUS_PEACH_WON;
        else
            return GWSTATUS_YOSHI_WON;
    }
    
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete m_peach;
    m_peach = nullptr;
    delete m_yoshi;
    m_yoshi = nullptr;
    vector<Actor*>::iterator it;
    it = m_actors.begin();
    while (it != m_actors.end())
    {
        delete (*it);
        it = m_actors.erase(it);
    }
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

Board& StudentWorld::getBoard() { return m_bd; }

Avatar* StudentWorld::getPeach() { return m_peach; }

Avatar* StudentWorld::getYoshi() { return m_yoshi; }

int StudentWorld::getBank() const { return m_bank; }

void StudentWorld::addBank(int coins) { m_bank += coins; }

void StudentWorld::resetBank() { m_bank = 0; }

void StudentWorld::addActor(Actor* actor) { m_actors.push_back(actor); }

Actor* StudentWorld::getSquareAtLocation(int x, int y)
{
    vector <Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->getX() == x && (*it)->getY() == y && (*it)->isASquare())
            return (*it);
    }
    return nullptr;
}

void StudentWorld::droppingSquarePlacedUnderPlayer(Avatar* avatar, int x, int y)
{
    if (avatar->getX() == x && avatar->getY() == y)
    {
        avatar->setHasMovedToNewSquare(true);
        avatar->setOnDirectionalSquare(false);
    }
}

Avatar* StudentWorld::getOtherPlayer(Avatar* avatar)
{
    if (avatar == m_peach)
        return m_yoshi;
    return m_peach;
}

Actor* StudentWorld::overlapWithVortex(int x, int y)
{
    vector <Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ((*it)->isImpactable() && overlap(x, y, (*it)->getX(), (*it)->getY()))
        {
            return (*it);
        }
    }
    return nullptr;
}

bool StudentWorld::overlap(int vx, int vy, int ax, int ay)
{
    return overlapHelper(vx, vy, vx + SPRITE_WIDTH - 1, vy + SPRITE_HEIGHT - 1, ax, ay, ax + SPRITE_WIDTH - 1, ay + SPRITE_HEIGHT - 1);
}

bool StudentWorld::overlapHelper(int bx1, int by1, int tx1, int ty1, int bx2, int by2, int tx2, int ty2)
{
    if (bx1 > tx2 || bx2 > tx1) // one square to the left of another
        return false;
    
    if (by1 > ty2 || by2 > ty1) // one square above another
        return false;

    return true;
}

string StudentWorld::addAvatarStats(Avatar* avatar)
{
    ostringstream s;
    s << "Roll: " << avatar->getDieRoll() << " Stars: " << avatar->getStars() << " $$: " << avatar->getCoins();
    if (avatar->getVortex())
        s << " VOR";
    return s.str();
}

Avatar* StudentWorld::findWinner()
{
    Avatar* winner = m_peach;
    if (m_yoshi->getStars() > m_peach->getStars())
        winner = m_yoshi;
    else if (m_yoshi->getStars() == m_peach->getStars() && m_yoshi->getCoins() > m_peach->getCoins())
        winner = m_yoshi;
    else if (m_yoshi->getStars() == m_peach->getStars() && m_yoshi->getCoins() == m_peach->getCoins())
    {
        int x = randInt(1, 2);
        if (x == 1)
            winner = m_yoshi;
    }
    return winner;
}