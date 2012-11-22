#include <boost/lexical_cast.hpp>
#include <string>
#include "SnakeGame.hpp"

/*
Specification:

[mapwidth]
[mapheight]
[playerCount]
[foodPosition]
[mapdata]
[snake alive 1]
[snake growcount 1]
[snake length 1]
[snake body[0].x 1]
[snake body[0].y 1]
..
[snake body[N].x 1]
[snake body[N].y 1]
...
[snake alive N]
[snake growcount N]
[snake length N]
[snake body[0].x N]
[snake body[0].y N]
..
[snake body[N].x N]
[snake body[N].y N]
*/

void snakeSerializeStateToStream(const SnakeGameInfo& state, std::string& strm)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast;
  using std::string;

  std::string out;

  strm = lexical_cast<string>(state.currentPlayer) + '\n';
  strm += lexical_cast<string>(state.levelWidth) + '\n';
  strm += lexical_cast<string>(state.levelHeight) + '\n';
  strm += lexical_cast<string>(state.playerCount) + '\n';
  strm += lexical_cast<string>(state.foodPosition.x) + '\n';
  strm += lexical_cast<string>(state.foodPosition.y) + '\n';
  for(int y=0; y<state.levelHeight; ++y)
    strm += state.level[y] + '\n';
  for(int i=0; i<state.playerCount; ++i){
    strm += lexical_cast<string>(state.snakes[i].alive) + '\n';
    strm += lexical_cast<string>(state.snakes[i].growCount) + '\n';
    strm += lexical_cast<string>(state.snakes[i].bodyParts.size()) + '\n';
    for(int j=0; j<(int)state.snakes[i].bodyParts.size(); ++j){
      strm += lexical_cast<string>(state.snakes[i].bodyParts[j].x) + '\n';
      strm += lexical_cast<string>(state.snakes[i].bodyParts[j].y) + '\n';
    }
  }
}

bool snakeSerializeStreamToState(SnakeGameInfo& state, const std::vector<std::string>& strm)
{
  using boost::lexical_cast;
  using boost::bad_lexical_cast;
  int snakeLength, pos = 0;
  bool ret = true;
  try {
    state.currentPlayer = lexical_cast<int>(strm[pos++]);
    state.levelWidth = lexical_cast<int>(strm[pos++]);
    state.levelHeight = lexical_cast<int>(strm[pos++]);
    state.playerCount = lexical_cast<int>(strm[pos++]);
    state.foodPosition.x = lexical_cast<int>(strm[pos++]);
    state.foodPosition.y = lexical_cast<int>(strm[pos++]);
    for(int y=0; y<state.levelHeight; ++y)
      state.level.push_back(strm[pos++]);
    state.snakes.resize(state.playerCount);
    for(int i=0; i<state.playerCount; ++i){
      SnakeInfo snake;
      snake.alive = lexical_cast<bool>(strm[pos++]);
      snake.growCount = lexical_cast<int>(strm[pos++]);
      snakeLength = lexical_cast<int>(strm[pos++]);
      snake.bodyParts.resize(snakeLength);
      for(int j = 0; j < snakeLength; ++j){
	Point p;
	p.x = lexical_cast<int>(strm[pos++]);
	p.y = lexical_cast<int>(strm[pos++]);
	snake.bodyParts[j] = p;
      }
      state.snakes[i] = snake;
    }
  } catch(bad_lexical_cast& ex){
    ret = false;
  }

  return ret;
}
