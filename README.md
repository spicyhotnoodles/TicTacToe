# TicTacToe
TicTacToe Game
## How to build
+ Build the project locally on your machine:
    ```shell
    make && ./build/server
    ```
+ Build with docker:
    ```shell
    docker build . -t game-server  && docker run -it -p 12345:12345 game-server
    ```