This will eventually become a library of small games for which it is easy to create AIs for.
While not generalized yet, AIs are separate programs that communicates with the game via its
standard streams, stdin and stdout.
Currently Snake is the only game, but more games are planned as soon as the Snake code is cleaned up.
The larger goal is to make a web portal for the games, where people can upload code and have AIs compete.

To build the games (out-of-source build), do:
mkdir Build
cd Build
cmake ..
make

To run Snake: ./Snake/Snake level ai1 ai2 .. aiN
Example:
./Snake/Snake Snake/data/level1.txt Snake/AIs/StupidAI/StupidAI Snake/AIs/SmarterAI/SmarterAI