#include <algorithm>
#include <cstdio>
#include <iostream>
#include "../../shared/SnakeGame.hpp"


Direction AIMove(int player, SnakeGameInfo& state)
{
  Point head, newHead, foodDelta;
  const Direction startMoves[4] = { Up, Down, Left, Right };
  Direction d;
  std::vector<Direction> possibleMoves;

  head = state.snakes[player].bodyParts[0];
  foodDelta.x = state.foodPosition.x - head.x;
  foodDelta.y = state.foodPosition.y - head.y;

  if(foodDelta.x < 0){
    possibleMoves.push_back(Left);
  } else if(foodDelta.x > 0){
    possibleMoves.push_back(Right);
  }
  if(foodDelta.y < 0){
    possibleMoves.push_back(Up);
  } else if(foodDelta.y > 0){
    possibleMoves.push_back(Down);
  }

  /* Try to get closer to the food first */
  if(!possibleMoves.empty()){
    std::random_shuffle(possibleMoves.begin(), possibleMoves.end());
    for(int i=0; i<possibleMoves.size(); ++i){
      newHead = snakeComputeNewHead(head, possibleMoves[i]);
      bool collideWithBorder = snakeIsCellBorder(newHead.x, newHead.y, state.level);
      bool collideWithSnake = snakeIsCellSnake(newHead.x, newHead.y, -1, state.snakes);
      if(!collideWithBorder && !collideWithSnake){
	return possibleMoves[i];
      }
    }
  }
  /* If none of the directions that brings us closer to the food is safe, then try any
     safe direction. If we die anyway (for example, spiral of death), return Up as a last resort */
  possibleMoves.clear();
  for(int i = 0; i < 4; ++i){
    newHead = snakeComputeNewHead(head, startMoves[i]);
    bool collideWithBorder = snakeIsCellBorder(newHead.x, newHead.y, state.level);
    bool collideWithSnake = snakeIsCellSnake(newHead.x, newHead.y, -1, state.snakes);
    /* If cell is free of walls and snakes, it is a possible move */
    if(!collideWithBorder && !collideWithSnake)
      possibleMoves.push_back(startMoves[i]);
  }
  std::random_shuffle(possibleMoves.begin(), possibleMoves.end());
  /* If no moves are possible, go up and die */
  if(possibleMoves.empty()) d = Up;
  else d = possibleMoves[0];

  return d;
}

void readGameState(SnakeGameInfo& state)
{
  std::vector<std::string> strm;
  std::string line;
  char szbuf[512];
  
  while(std::getline(std::cin, line)){
    if(line == "END") break;
    strm.push_back(line);
  };
  
  snakeSerializeStreamToState(state, strm);
}

int main(int argc, char* argv[])
{
  SnakeGameInfo state;
  Direction d;
  readGameState(state);
  while(state.snakes[state.currentPlayer].alive){
    d = AIMove(state.currentPlayer, state);
    switch(d){
    case Up: std::cout << 'u'; break;
    case Down: std::cout << 'd'; break;
    case Left: std::cout << 'l'; break;
    default: std::cout << 'r'; break;
    }
    std::cout.flush();
    readGameState(state);
  }
  return 0;
}
