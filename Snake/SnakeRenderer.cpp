#include "SnakeGame.hpp"
#include <cstring>
#include <SDL/SDL.h>

static void drawSquare(unsigned int* dst, int squareSize,
		       int x, int y, int pitch, int a, int r, int g, int b)
{
  int xStart, yStart, xEnd, yEnd;
  unsigned int color;
  
  yStart = y;
  yEnd = y+squareSize;
  xStart = x;
  xEnd = x+squareSize;
  color = (a<<24)|(r<<16)|(g<<8)|b;

  for(int j=yStart; j<yEnd; ++j){
    int col = j*pitch;
    for(int i=xStart; i<xEnd; ++i){
      dst[col + i] = color;
    }
  }
}

bool snakeShouldQuit()
{
  SDL_PumpEvents();
  unsigned char* keys = SDL_GetKeyState(NULL);
  if(keys[SDLK_ESCAPE]) return true;
  return false;
}

bool snakeInitGraphics(SnakeGameInfo& state)
{
  const int squareSize = 16;
  int width = squareSize * state.levelWidth;
  int height = squareSize * state.levelHeight;
  SDL_Init(SDL_INIT_VIDEO);
  state.vs = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
  if(!state.vs) return false;
  return true;
}

void snakeDestroyGraphics()
{
  SDL_Quit();
}

void snakeRender(SnakeGameInfo& state)
{
  const int squareSize = 16;
  const int colors[4][4] = {
    {255, 255, 255,   0},
    {255,   0,   0, 255},
    {255, 255,   0, 255},
    {255,   0, 255, 255}
  };

  int width = state.vs->w;
  int height = state.vs->h;
  int pitch = state.vs->pitch / sizeof(int);

  unsigned int* pixels = (unsigned int*)state.vs->pixels;
  /* Clear to red */
  for(int i = 0; i < height*pitch; i+=pitch)
    memset(&pixels[i], 0x0, sizeof(unsigned int) * width);

  /* Draw food */
  int foodX = state.foodPosition.x * squareSize;
  int foodY = state.foodPosition.y * squareSize;

  drawSquare(pixels, squareSize-4, foodX + 2, foodY + 2, pitch, 255, 128, 0, 0);

  /* Draw level borders */
  for(int j=0; j<height; j+=squareSize){
    for(int i=0; i<width; i+=squareSize){
      int mx = i/squareSize;
      int my = j/squareSize;
      bool mw = (state.level[my][mx] == 'x');
      if(mw)
	drawSquare(pixels, squareSize, i, j, pitch, 255, 255, 255, 255);
    }
  }
  /* Draw snakes */
  for(int eachSnake = 0; eachSnake < state.playerCount; ++eachSnake){
    const int* playerColor = &colors[eachSnake][0];
    for(int eachBodyPart = 0; eachBodyPart < state.snakes[eachSnake].bodyParts.size(); ++eachBodyPart){
      Point bp = state.snakes[eachSnake].bodyParts[eachBodyPart];
      bp.x *= squareSize;
      bp.y *= squareSize;
      drawSquare(pixels, squareSize-2, bp.x+1, bp.y+1, pitch,
		   playerColor[0], playerColor[1], playerColor[2], playerColor[3]);
    }
  }
  SDL_Flip(state.vs);
  SDL_Delay(50);
}
