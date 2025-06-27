
class GameManager():

    def __init__(self):
        """ Initialize the GameManager """
        self.gamelist = []

    def add_game(self, game):
        """ Add a game to the game list """
        self.gamelist.append(game)

    def remove_game(self, game):
        """ Remove a game from the game list """
        if game in self.gamelist:
            self.gamelist.remove(game)
        else:
            print(f"Game '{game}' not found in the list.")

    def list_games(self):
        """ List all games in the game list """
        if not self.gamelist:
            print("No games available.")
        else:
            print("Your active games:")
            for i, game in enumerate(self.gamelist):
                print(f"- {i + 1}. ID: {game} (Waiting for players to join...)")