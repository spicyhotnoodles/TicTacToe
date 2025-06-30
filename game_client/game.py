
class GameManager():

    def __init__(self):
        """ Initialize the GameManager """
        self.hosted_games = []

    def append_hosted_game(self, game):
        """ Add a game to the game list """
        self.hosted_games.append(game)

    def remove_hosted_game(self, game):
        """ Remove a game from the game list """
        if game in self.hosted_games:
            self.hosted_games.remove(game)
        else:
            print(f"Game '{game}' not found in the list.")

    def list_hosted_games(self):
        """ List all games in the game list """
        if not self.hosted_games:
            print("No games available.")
        else:
            print("Your active games:")
            for i, game in enumerate(self.hosted_games):
                print(f"- {i + 1}. ID: {game} (Waiting for players to join...)")