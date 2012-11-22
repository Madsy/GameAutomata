#include <fstream>
#include <iostream>
#include "shared/SnakeGame.hpp"

bool snakeInitLevel(const std::string& levelFile, SnakeGameInfo& state)
{
  std::ifstream strm(levelFile.c_str());
  if(!strm.is_open()) return false;
  std::string line;
  while(std::getline(strm, line)){
    state.level.push_back(line);
  }
  state.levelWidth = state.level[0].length();
  state.levelHeight = state.level.size();
  return true;
}

void snakeInitSnakes(SnakeGameInfo& state, int playerCount)
{
    /* IsCellClear checks whether the rpart position collides with
     other snakes, so init state.snakes[i].parts[j] to be outside the map bounds to avoid this. */
  Point point_outside_map(-1, -1);
  SnakeInfo tmpSnake;
  std::vector<Point> tmpParts;

  state.playerCount = playerCount;
  state.currentPlayer = 0;
  /* TODO: Fix so that we can have longer snakes at the beginning of the level.
     Currently we start with snakes of 1 in length. We rather want three-celled snakes. */
  tmpParts.push_back(point_outside_map);
  //tmpParts.push(point_outside_map);
  //tmpParts.push(point_outside_map);
  tmpSnake.bodyParts = tmpParts;
  tmpSnake.alive = true;
  /* Whenever growCount > 0, we move the snake body without updating the tail,
     and decrement growCount accordingly. */
  tmpSnake.growCount = 0;

  state.snakes.resize(playerCount);
  /* Initialize all snakes to be outside the map */
  for(int eachSnake = 0; eachSnake < playerCount; ++eachSnake)
    state.snakes[eachSnake] = tmpSnake;

  for(int eachSnake = 0; eachSnake < playerCount; ++eachSnake){
    Point rpart;
    do {
      rpart = randPoint(0, state.levelWidth - 1, 0, state.levelHeight - 1);
    } while(!snakeIsCellClear(rpart.x, rpart.y, -1, state));
    state.snakes[eachSnake].bodyParts[0] = rpart;
  }
}

void snakeInitFood(SnakeGameInfo& state)
{
  Point point_outside_map(-1, -1);
  Point rfood;
  /* SnakeIsCellClear checks if the rfood position collides with
     state.foodPosition, so init state.foodPosition outside the map bounds to avoid that. */
  state.foodPosition = point_outside_map;
  do {
    rfood = randPoint(0, state.levelWidth, 0, state.levelHeight);
  } while(!snakeIsCellClear(rfood.x, rfood.y, -1, state));
  state.foodPosition = rfood;
}


/* Called only if the snake doesn't collide with anything */
void snakeUpdateSnake(SnakeInfo& snake, Direction direction)
{
  Point head = snake.bodyParts[0];
  if(snakeIsSnakeGrowing(snake)){
    snake.bodyParts.push_back(Point());
    --snake.growCount;
  }

  /* body[i] = body[i-1], i.e the previous head becomes a part of the body */
  for(int eachBody = snake.bodyParts.size() - 1; eachBody > 0; --eachBody){
    /* <1>23456 => <1>12345 => <0>12345*/
    snake.bodyParts[eachBody] = snake.bodyParts[eachBody - 1];  
  }
  snake.bodyParts[0] = snakeComputeNewHead(head, direction);
}

void snakeUpdateFood(SnakeGameInfo& state)
{
  Point point_outside_map(-1, -1);
  Point rfood;
  state.foodPosition = point_outside_map;
  do {
    rfood = randPoint(0, state.levelWidth, 0, state.levelHeight);
  } while(!snakeIsCellClear(rfood.x, rfood.y, -1, state));
  state.foodPosition = rfood;
}

/* Returns the winning player id */
int snakeGameTick(SnakeGameInfo& state, const std::vector<Direction>& input)
{
  int aliveCount = 0;
  int winnerSnake = 0;

  /* Kill snakes with illegal input */
  for(int eachSnake = 0; eachSnake < (int)state.snakes.size(); ++eachSnake){
    if(input[eachSnake] == IllegalDirection)
      state.snakes[eachSnake].alive = false;
  }
  /* Update to new positions */
  for(int eachSnake = 0; eachSnake < (int)state.snakes.size(); ++eachSnake){
    if(state.snakes[eachSnake].alive){
      snakeUpdateSnake(state.snakes[eachSnake], input[eachSnake]);
    }
  }
  /* With the new positions, cull out any dead snakes that collided */
  for(int eachSnake = 0; eachSnake < (int)state.snakes.size(); ++eachSnake){
    if(state.snakes[eachSnake].alive){
      Point head = state.snakes[eachSnake].bodyParts[0];
      state.snakes[eachSnake].alive = snakeIsCellClear(head.x, head.y, eachSnake, state);
      ++aliveCount;
      winnerSnake = eachSnake;
    }
  }

  /* Now only the alive updated snakes are left. Check for winners, and if someone grabbed the food. */
  if(aliveCount == 1) return winnerSnake + 1;
  else if(aliveCount == 0) return 0; /* 0 == draw */

  for(int eachSnake = 0; eachSnake < (int)state.snakes.size(); ++eachSnake){
    if(state.snakes[eachSnake].alive){
      Point head = state.snakes[eachSnake].bodyParts[0];
      /* Food makes the snake tail grow for 'growCount' turns */
      if(snakeIsCellFood(head.x, head.y, state.foodPosition)){
	state.snakes[eachSnake].growCount += 3;
	snakeUpdateFood(state);
      }
    }
  }
  return -1; /* -1 = continue */
}

