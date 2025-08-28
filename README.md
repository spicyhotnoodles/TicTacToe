# TicTacToe
Multiplayer tic-tac-toe game developed with server-client architecture. The server is implemented in C, while the client is a Python application. This was a project developed for educational purposes during my academic career at Università degli Studi di Napoli Federico II.
## How to build the server
+ Build the project locally on your machine:
    ```shell
    # make sure to have cJSON library installed on your system!
    make && ./build/server
    ```
+ Build with docker:
    ```shell
    docker build . -t game-server  && docker run -it -p 12345:12345 game-server
    ```
## How to run the client
Make sure Python 3.x is installed on your system. You can download it from [python.org](https://www.python.org/downloads/).
Once you have Python installed, you can run the client by executing the following command in the terminal:
```shell
python run.py
```
## How to play
This program is a multiplayer tic-tac-toe game so, in order to play either create a new game or join an existing one.
+ To create a new game, simply select the option from the menu and wait for another player to join.
    + Always check "My Games" to see if a fellow guest has requested to join. If so select the game and accept or deny the request.      
+ To join an existing game, select "Join Game" from the menu and choose a game from the list. A join request will be sent to the host for approval.
   + If the host accepts the request, you will be able to play the game.
   + If the host rejects the request, you will be notified and can try to join another game.
When playing, make your move by selecting a position on the board (1-9) when prompted. The game will alternate turns between players until there is a winner or the game ends in a draw. The host of the game always start first and their symbol is X, while the guest's symbol is O.
