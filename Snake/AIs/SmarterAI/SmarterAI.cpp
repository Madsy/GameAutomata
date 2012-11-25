#include <algorithm>
#include <iterator>
#include <stack>
#include <cstdio>
#include <cmath>
#include <iostream>
#include "../../shared/SnakeGame.hpp"


/*
Depth first search for moves.
Return scores based on which direction gets the greatest score.
The score for the 3 directions is calculated based on accumulating
the individual moves when recursing.

What to use as the score heuristic?
  * Area "coverage". Flood fill from the snake head and count the samples.
  Base score on how much space there is. If the food is there, take that into account.
  * Potential positions for enemy snake heads are dangerous, so mark those with a negative score.
  * Walls and other snake bodies are guaranteed death so mark with a negative score.
*/

std::string directionToString(Direction d)
{
  switch(d)
  {
  case Up: return "Up";
  case Down: return "Down";
  case Left: return "Left";
  case Right: return "Right";
  }
}

Point getCurrentHead(const SnakeGameInfo& state)
{
  return state.snakes[state.currentPlayer].bodyParts[0];
}

Point getHead(const SnakeGameInfo& state, int playerID)
{
  return state.snakes[playerID].bodyParts[0];
}

int getCurrentSnakeLength(const SnakeGameInfo& state)
{
  return state.snakes[state.currentPlayer].bodyParts.size();
}

int getSnakeLength(const SnakeGameInfo& state, int playerID)
{
  return state.snakes[playerID].bodyParts.size();
}


int generateCoverageMap(const SnakeGameInfo& state, std::vector<int>& level, std::vector<Point> potentialEnemyHeads)
{
  int width = state.levelWidth;
  int height = state.levelHeight;
  /* Return the total number of empty space on the level */
  int initialCoverage = 0;

  level.resize(width * height);
  for(int y = 0; y < height; ++y){
    for(int x = 0; x < width; ++x){
      bool inPEH = false;
      for(int i = 0; i < potentialEnemyHeads.size(); ++i){
	if(potentialEnemyHeads[i] == Point(x,y)){
	  inPEH = true;
	  break;
	}
      }
      if(snakeIsCellBorder(x, y, state.level) ||
	 snakeIsCellSnake(x, y, -1, state.snakes) ||
	 inPEH)
	level[x + y * width] = 1;
      else {
	level[x + y * width] = 0;
	++initialCoverage;
      }
    }
  }
  return initialCoverage;
}

int getCoverageScore(const SnakeGameInfo& state, std::vector<int> coverageMap, Point head)
{
  int coverageCount = 0;
  std::stack<Point> st;
  
  /* Flood fill to find available "area" if we move to [head.x head.y]*/
  st.push(head);
  while(!st.empty()){
    Point p = st.top();
    st.pop();
    int index = p.x + p.y * state.levelWidth;
    if(coverageMap[index] == 0){
      coverageMap[index] = 1;
      ++coverageCount;
      st.push(Point(p.x + 1, p.y));
      st.push(Point(p.x - 1, p.y));
      st.push(Point(p.x    , p.y - 1));
      st.push(Point(p.x    , p.y + 1));
    }
  }
  return coverageCount;
}

/* The available positions ("potential" heads) for the enemy snakes.
   We want to avoid these positions because we might collide there at the next turn. */
void getPotentialEnemyPositions(const SnakeGameInfo& state, std::vector<Point>& pheads)
{
  for(int eachSnake = 0; eachSnake < state.playerCount; ++eachSnake){
    if(eachSnake == state.currentPlayer) continue;
    Point head = getHead(state, eachSnake);
    /* One of these are redundant if the snake length is greater than 1, but it doesn't matter */
    pheads.push_back(snakeComputeNewHead(head, Up));
    pheads.push_back(snakeComputeNewHead(head, Down));
    pheads.push_back(snakeComputeNewHead(head, Left));
    pheads.push_back(snakeComputeNewHead(head, Right));
  }
}

void RemoveSuicideMoves(const SnakeGameInfo& state, std::vector<Direction>& potentialMoves,
			const std::vector<int>& coverageMap)
{
  std::vector<Direction> suicideMoves;
  std::vector<Direction> result;
  for(int i = 0; i < potentialMoves.size(); ++i){
    Point head = getCurrentHead(state);
    Point newhead = snakeComputeNewHead(head, potentialMoves[i]);
    int sampleCount = getCoverageScore(state, coverageMap, newhead);
    /* If sampleCount is less than the snake length, then there isn't space for the whole snake. */
    bool pathIsEvilSpiralOfDeath = sampleCount < getCurrentSnakeLength(state);
    bool pathCollidesWithBorder = snakeIsCellBorder(newhead.x, newhead.y, state.level);
    bool pathCollidesWithSnake = snakeIsCellSnake(newhead.x, newhead.y, -1, state.snakes);
    if(pathIsEvilSpiralOfDeath || pathCollidesWithBorder || pathCollidesWithSnake)
      suicideMoves.push_back(potentialMoves[i]);      
  }
  std::set_symmetric_difference(potentialMoves.begin(), potentialMoves.end(),
				suicideMoves.begin(), suicideMoves.end(),
				std::back_inserter(result));
  potentialMoves = result;
}

void computePreferredFoodMoves(SnakeGameInfo& state, std::vector<Direction>& preferredFoodMoves)
{
  Point head, foodDelta;
  head = getCurrentHead(state);
  foodDelta.x = state.foodPosition.x - head.x;
  foodDelta.y = state.foodPosition.y - head.y;

  if(foodDelta.x < 0){
    preferredFoodMoves.push_back(Left);
  } else if(foodDelta.x > 0){
    preferredFoodMoves.push_back(Right);
  }
  if(foodDelta.y < 0){
    preferredFoodMoves.push_back(Up);
  } else if(foodDelta.y > 0){
    preferredFoodMoves.push_back(Down);
  }
}

struct MoveWithScore
{
  MoveWithScore(Direction d) : direction(d), score(0){}
  MoveWithScore() : direction(Up), score(0){}
  bool operator<(const MoveWithScore& mws) const { return score < mws.score; }
  Direction direction;
  int score;
};

Direction AIMove(int player, SnakeGameInfo& state)
{
  Direction d;
  int totalCoverage = 0;
  
  std::vector<Direction> startMoves;
  std::vector<Direction> preferredFoodMoves;
  std::vector<Direction> potentialMoves;
  std::vector<MoveWithScore> potentialMovesWithScore;
  std::vector<Point> potentialEnemyHeads;
  std::vector<int> coverageMap;
  std::vector<int> potentialMoveScores;
  startMoves.push_back(Up);
  startMoves.push_back(Down);
  startMoves.push_back(Left);
  startMoves.push_back(Right);
  potentialMoves = startMoves;

  /* An array of moves that leads us closer to the food */
  computePreferredFoodMoves(state, preferredFoodMoves);
  /* An array of points which represents all the possible enemy positions we should avoid */
  getPotentialEnemyPositions(state, potentialEnemyHeads);
  /* Compute coverage map and the total count of free level space */
  totalCoverage = generateCoverageMap(state, coverageMap, potentialEnemyHeads);
  /* Remove all moves that leads to suicide */
  RemoveSuicideMoves(state, potentialMoves, coverageMap);
  
  /* potentialMoves now contains moves that doesn't 100% surely kill us.
     What to do next? Based on the remainding moves, compute a score based on:
     If the move leads to a position in potentialEnemyHeads, give a very low score. (Potential death)
     Check move against coverage and give points based on coverage / totalCoverage.
     If the move leads us to food or closer to the food, give a small positive score.
  */
  /* Potential moves, with initial score of 0 */
  potentialMovesWithScore.resize(potentialMoves.size());
  for(int i = 0; i < (int)potentialMoves.size(); ++i)
    potentialMovesWithScore[i].direction = potentialMoves[i];

  for(int i = 0; i < (int)potentialMovesWithScore.size(); ++i){
    Point head, newhead;
    head = getCurrentHead(state);
    newhead = snakeComputeNewHead(head, potentialMovesWithScore[i].direction);
    for(int j = 0; j < potentialEnemyHeads.size(); ++j){
      if(potentialEnemyHeads[j] == newhead)
	potentialMovesWithScore[i].score -= 20;
    }
    int coverage = getCoverageScore(state, coverageMap, newhead);
    int coverageScore = std::floor((float)coverage / (float)totalCoverage * 18.0f);
    if(coverage < totalCoverage){
      fprintf(stderr, "[Player %d] direction %s coverage %d / %d\n",
	    state.currentPlayer, directionToString(potentialMovesWithScore[i].direction).c_str(),
	    coverage, totalCoverage);
      fflush(stderr);
    }

    potentialMovesWithScore[i].score += coverageScore;
    for(int j = 0; j < preferredFoodMoves.size(); ++j){
      if(preferredFoodMoves[j] == potentialMovesWithScore[i].direction)
	potentialMovesWithScore[i].score += 1;
    }
  }
  std::vector<MoveWithScore>::iterator it;
  it = std::max_element(potentialMovesWithScore.begin(), potentialMovesWithScore.end());

  /* Debug info */
  //fprintf(stderr, "[Player %d] Potential directions / scores:\n", state.currentPlayer);
  for(int i = 0; i < potentialMovesWithScore.size(); ++i){
    fprintf(stderr, "[Player %d] %s : %d\n",
	    state.currentPlayer,
	    directionToString(potentialMovesWithScore[i].direction).c_str(),
	    potentialMovesWithScore[i].score);
    fflush(stderr);
  }
  if(!potentialMovesWithScore.empty())
    return it->direction;
  else return Up;
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
