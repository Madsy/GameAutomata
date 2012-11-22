#ifndef SNAKEGAME_HPP_GUARD
#define SNAKEGAME_HPP_GUARD
#include <vector>
#include <string>

struct Point
{
  Point(int _x, int _y) : x(_x), y(_y){}
  Point() : x(0), y(0){}
  bool operator==(const Point& p) const
  {
    return (x == p.x) && (y == p.y);
  }
    /* Manhattan distance */
  bool operator<(const Point& p) const
  {
    return (x+y) < (p.x+p.y);
  }
  int x,y;
};

struct SnakeInfo
{
  std::vector<Point> bodyParts;
  bool alive;
  int growCount;
};

/* Use SDL_Surface as a pimpl */
struct SDL_Surface;

struct SnakeGameInfo
{
  std::vector<std::string> level;
  std::vector<SnakeInfo> snakes;
  Point foodPosition;
  int playerCount;
  int currentPlayer;
  int levelWidth;
  int levelHeight;
  SDL_Surface* vs;
};

enum Direction
{
  Up = 0,
  Down = 1,
  Left = 2,
  Right = 3,
  IllegalDirection = 4
};

/* SnakeMisc.cpp */
int randRange(int min, int max);
Point randPoint(int xmin, int xmax, int ymin, int ymax);

Direction snakeGenerateRandomDirection();
Point snakeComputeNewHead(Point head, Direction direction);
bool snakeIsCellBorder(int x, int y, const std::vector<std::string>& level);
bool snakeIsCellFood(int x, int y, const Point& food);
bool snakeIsCellSnake(int x, int y, int snakeToSkip, const std::vector<SnakeInfo>& snakes);
bool snakeIsCellClear(int x, int y, int snakeToSkip, const SnakeGameInfo& state);
bool snakeIsSnakeGrowing(SnakeInfo& snake);

/* SnakeAI.cpp
Direction AIMove(int player, SnakeGameInfo& state);
*/

/* SnakeRenderer.cpp */
bool snakeInitGraphics(SnakeGameInfo& state);
void snakeDestroyGraphics();
void snakeRender(SnakeGameInfo& state);
bool snakeShouldQuit();

/* SnakeSerialization.cpp */
void snakeSerializeStateToStream(const SnakeGameInfo& state, std::string& strm);
bool snakeSerializeStreamToState(SnakeGameInfo& state, const std::vector<std::string>& strm);

/* SnakeGame.cpp  */
bool snakeInitLevel(const std::string& levelFile, SnakeGameInfo& state);
void snakeInitSnakes(SnakeGameInfo& state, int playerCount);
void snakeInitFood(SnakeGameInfo& state);
void snakeUpdateSnake(SnakeInfo& snake, Direction direction);
void snakeUpdateFood(SnakeGameInfo& state);
int snakeGameTick(SnakeGameInfo& state, const std::vector<Direction>& input);

#endif

