# TicTacToe
Multiplayer tic-tac-toe game developed with server-client architecture. The server is implemented in C, while the client is a Python application. This was a project developed for educational purposes during my academic career at Università degli Studi di Napoli Federico II.
## How to build the server
+ Build the project locally on your machine:
    > ⚠️ Make sure to have [cJSON](https://github.com/DaveGamble/cJSON) and [glib](https://docs.gtk.org/glib/) libraries installed on your system!
    ```shell
    make && ./build/server
    ```
+ Build with docker:
    ```shell
    docker build . -t game-server  && docker run -it -p 12345:12345 game-server
    ```
## How to run the client
> ⚠️ Make sure the server is up and running **before** launching the client

Python 3.x is required on your system. You can download it from [python.org](https://www.python.org/downloads/).
Once you have Python installed, you can run the client by executing the following command in the terminal:
```shell
python run.py
```
## How to play
<p float="left">
  <img src="https://github.com/user-attachments/assets/26ffb8b1-835f-4249-b966-2508c372fe16" width="32%" />
  <img src="https://github.com/user-attachments/assets/7f89b4c1-0cd6-42f7-8b8e-678a238cc5e6" width="32%" />
  <img src="https://github.com/user-attachments/assets/72ee2b0d-e860-4f0a-a78e-0d81caa754ab" width="32%" />
</p>

This program is a multiplayer tic-tac-toe game so, in order to play either create a new game or join an existing one.
+ To create a new game, simply select the option from the menu and wait for another player to join.
    + Always check "My Games" to see if a fellow guest has requested to join. If so select the game and accept or deny the request.      
+ To join an existing game, select "Join Game" from the menu and choose a game from the list. A join request will be sent to the host for approval.
   + If the host accepts the request, you will be able to play the game.
   + If the host rejects the request, you will be notified and can try to join another game.

When playing, make your move by selecting a position on the board (1-9) when prompted. The game will alternate turns between players until there is a winner or the game ends in a draw. The host of the game always start first and their symbol is X, while the guest's symbol is O.
