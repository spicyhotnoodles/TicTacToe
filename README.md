# TicTacToe
TicTacToe Game
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
