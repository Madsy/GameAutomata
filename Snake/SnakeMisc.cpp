#include <cstdlib>
#include "SnakeGame.hpp"

/* Random integer in a range */
int randRange(int min, int max)
{
  double dmin, dmax, drand;
  drand = (double)rand() / (double)RAND_MAX;
  dmin = (double)min;
  dmax = (double)max;

  return dmin + drand*(dmax - dmin);
}

Point randPoint(int xmin, int xmax, int ymin, int ymax)
{
  Point p;
  p.x = randRange(xmin, xmax);
  p.y = randRange(ymin, ymax);
  return p;
}

Direction snakeGenerateRandomDirection()
{
  int d = randRange(0, 3);
  switch(d){
    case 0: return Up;
    case 1: return Down;
    case 2: return Left;
  }
  return Right;
}

Point snakeComputeNewHead(Point head, Direction direction)
{
  Point p;
  switch(direction)
  {
  case Up:
    p.x = head.x;
    p.y = head.y - 1;
    break;
  case Down:
    p.x = head.x;
    p.y = head.y + 1;
    break;
  case Left:
    p.x = head.x - 1;
    p.y = head.y;
    break;
  case Right:
    p.x = head.x + 1;
    p.y = head.y;
    break;
  }

  return p;
}

/* Is cell [x, y] on the board a solid part of the level ? */
bool snakeIsCellBorder(int x, int y, const std::vector<std::string>& level)
{
  return level[y][x] == 'x';
}

/* Is cell [x, y] on the board a food piece? */
bool snakeIsCellFood(int x, int y, const Point& food)
{
  return (food.x == x) && (food.y == y);
}

/* Is cell [x, y] on the board a snake ? */
bool snakeIsCellSnake(int x, int y, int snakeToSkip, const std::vector<SnakeInfo>& snakes)
{
  int s;
  Point p(x,y);
  for(int eachSnake = 0; eachSnake < (int)snakes.size(); ++eachSnake){
    /* Don't incorrectly compare the snake's head to itself */
    if(eachSnake == snakeToSkip) s = 1;
    else s = 0;
    for(int eachBodyPart = s; eachBodyPart < (int)snakes[eachSnake].bodyParts.size(); ++eachBodyPart){
      if(snakes[eachSnake].bodyParts[eachBodyPart] == p) return true;
    }
  }
  return false;
}

bool snakeIsCellClear(int x, int y, int snakeToSkip, const SnakeGameInfo& state)
{
  return !snakeIsCellBorder(x, y, state.level) && !snakeIsCellSnake(x, y, snakeToSkip, state.snakes);
}

bool snakeIsSnakeGrowing(SnakeInfo& snake)
{
  return snake.growCount > 0;
}
