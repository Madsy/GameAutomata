#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <SDL/SDL.h>
#include "SnakeGame.hpp"

int main(int argc, char* argv[])
{
  SnakeGameInfo state;
  std::vector<Direction> playerInputs;
  srand(time(NULL));
  if(argc < 2){
    printf("Usage: %s <levelFile>\n", argv[0]);
    return 0;
  }
  if(!snakeInitLevel(std::string(argv[1]), state)){
    printf("Couldn't open level \"%s\"\n", argv[1]);
    return 0;
  }
  snakeInitSnakes(state, 2);
  snakeInitFood(state);
  /* Important. Call snakeInitSnakes before setting up the graphics, to set the number of players.
     Maybe merge snakeInitLevel, snakeInitSnakes and snakeInitFood into a single snakeInit function? */
  if(!snakeInitGraphics(state)){
    printf("Unable to set video mode.\n");
    return 0;
  }
  
  int winner;

  do {
    snakeRender(state);
    playerInputs.clear();
    playerInputs.push_back(AIMove(0, state));
    playerInputs.push_back(AIMove(1, state));
  } while((winner = snakeGameTick(state, playerInputs)) < 0 && !snakeShouldQuit());

  if(!winner)
    printf("Game ended in a draw.\n");
  else {
    printf("Player %d wins!\n", winner);
  }

  while(!snakeShouldQuit()){}
  snakeDestroyGraphics();
  return 0;
}
