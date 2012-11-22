#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <boost/array.hpp>
#include <SDL/SDL.h>
#include "shared/SnakeGame.hpp"

/*
static bool serialTest(SnakeGameInfo& state, std::string fileName)
{
  std::string strm, line;
  SnakeGameInfo state2;
  std::vector<std::string> sdata;
  std::ofstream out;
  std::ifstream in;

  snakeSerializeStateToStream(state, strm);
  out.open(fileName.c_str());
  out << strm;
  out.close();
  in.open(fileName.c_str());
  while(std::getline(in, line))
    sdata.push_back(line);
  if(!snakeSerializeStreamToState(state2, sdata)) return false;
  return state.level.size() == state2.level.size() &&
    state.snakes.size() == state2.snakes.size() &&
    state.foodPosition.x == state2.foodPosition.x &&
    state.foodPosition.y == state2.foodPosition.y &&
    state.playerCount == state2.playerCount &&
    state.levelWidth == state2.levelWidth &&
    state.levelHeight == state2.levelHeight;
}
*/

typedef boost::array<FILE*, 2> pipearr_t;
struct childproc_t
{
  pid_t pid;
  std::string path;
};

static bool init_ipc(std::vector<childproc_t>& procList, std::vector<pipearr_t>& strms, int numProcesses)
{
  /*
    parent reads strms[i][0], child writes strms[i][1]
    child reads pipeChild[0], parent writes pipeChild[1];
  */
  strms.resize(numProcesses);
  int pipeParent[2];
  int pipeChild[2];

  for(int i=0; i < numProcesses; ++i){
    (void)pipe(pipeParent);
    (void)pipe(pipeChild);

    pid_t p = fork();
    switch(p){
      case -1:
      {
	/* Close previous */
	for(int j=0; j < i; ++j){
	  fclose(strms[j][0]);
	  fclose(strms[j][1]);
	}
	/* Close current */
	close(pipeParent[0]);
	close(pipeParent[1]);
	close(pipeChild[0]);
	close(pipeChild[1]);
	/* Kill'em all! */
	for(int j=0; j < i; ++j)
	  kill(procList[i].pid, 2);
	return false;
      }
      /* Inside the child process.
	 We start off by closing all the pipes we don't need,
	 then we duplicate the pipe we're going to use to represent
	 stdin and stdout. Finally we start the inferior process with execve. */
      case 0:
      {
	for(int j=0; j < i; ++j){
	  fclose(strms[j][0]);
	  fclose(strms[j][1]);
	}
	/* Close write-end */
	close(pipeChild[1]);
	/* close read-end */
	close(pipeParent[0]);

	dup2(pipeChild[0], STDIN_FILENO);
	dup2(pipeParent[1], STDOUT_FILENO);
	close(pipeChild[0]);
	close(pipeParent[1]);
	char* argv[1] = {NULL};
	if(execve(procList[i].path.c_str(), argv, NULL) < 0) exit(1);
      }
      default:
      {
	procList[i].pid = p;
	/* close read-end */
	close(pipeChild[0]);	  
	/* Close write-end */
	close(pipeParent[1]);
	/* Convert to FILE handles */
	strms[i][0] = fdopen(pipeParent[0], "r");
	strms[i][1] = fdopen(pipeChild[1], "w");
      }
    }
  }
  return true;
}

static void destroy_ipc(std::vector<childproc_t>& procList, std::vector<pipearr_t>& strms, int numProcesses)
{
  for(int i=0; i < numProcesses; ++i){
    fclose(strms[i][0]);
    fclose(strms[i][1]);
    kill(procList[i].pid, 2);
  }
}

static void send_ipc(SnakeGameInfo& state, std::vector<pipearr_t>& strms)
{
  std::string strm_state;
  for(int i=0; i < (int)strms.size(); ++i){
    state.currentPlayer = i;
    snakeSerializeStateToStream(state, strm_state); 
    if(!state.snakes[i].alive) continue;
    fprintf(strms[i][1], "%s", strm_state.c_str());
    fprintf(strms[i][1], "END\n");
    fflush(strms[i][1]);
  }
}

static void recv_ipc(const SnakeGameInfo& state, std::vector<Direction>& inputs, std::vector<pipearr_t>& strms)
{
  char ch;
  Direction d;
  for(int i=0; i < (int)strms.size(); ++i){
    if(!state.snakes[i].alive) continue;
    (void)fscanf(strms[i][0], "%c", &ch);
    switch(ch){
      case 'u' : d = Up; break;
      case 'd' : d = Down; break;
      case 'l' : d = Left; break;
      case 'r' : d = Right; break;
      default: d = IllegalDirection;
    }
    inputs[i] = d;
  }
}

int main(int argc, char* argv[])
{
  int numPlayers;
  SnakeGameInfo state;
  std::vector<Direction> playerInputs;
  std::vector<childproc_t> procList;
  std::vector<pipearr_t> strms;

  srand(time(NULL));
  

  if(argc < 4){
    printf("Usage: %s <levelFile> <AIprog1> ... <AIprogN>\n", argv[0]);
    return 0;
  }
  numPlayers = argc - 2;
  /* You can really add as many players as you want, but first
     you have to increase the size of the "color" array in void snakeRender(SnakeGameInfo& state)
     which is in SnakeRenderer.cpp */
  if(numPlayers > 4){
    printf("Max 4 players supported.\n");
  }
  procList.resize(numPlayers);
  playerInputs.resize(numPlayers);
  for(int i = 0; i < numPlayers; ++i)
    procList[i].path = std::string(argv[i+2]);
  if(!init_ipc(procList, strms, numPlayers)){
    printf("Error spawning processes.\n");
    return 0;
  }

  if(!snakeInitLevel(std::string(argv[1]), state)){
    printf("Couldn't open level \"%s\"\n", argv[1]);
    return 0;
  }
  snakeInitSnakes(state, numPlayers);
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
    printf("[Server] Sending data.. \n");
    send_ipc(state, strms);
    printf("[Server] Receving data.. \n");
    recv_ipc(state, playerInputs, strms);
    printf("[Server] Running mainloop.. \n");
  } while((winner = snakeGameTick(state, playerInputs)) < 0 && !snakeShouldQuit());  
  destroy_ipc(procList, strms, numPlayers);
  if(!winner)
    printf("Game ended in a draw.\n");
  else {
    printf("Player %d wins!\n", winner);
  }

  while(!snakeShouldQuit()){}
  snakeDestroyGraphics();
  return 0;
}
