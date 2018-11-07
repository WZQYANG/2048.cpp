#include "game.hpp"

namespace {
namespace Keypress {
namespace Code {

enum { CODE_ESC = 27, CODE_LSQUAREBRACKET = '[' };

// Hotkey bindings:
// Style: ANSI (Arrow Keys)
enum {
  CODE_ANSI_TRIGGER_1 = CODE_ESC,
  CODE_ANSI_TRIGGER_2 = CODE_LSQUAREBRACKET
};
enum {
  CODE_ANSI_UP = 'A',
  CODE_ANSI_DOWN = 'B',
  CODE_ANSI_LEFT = 'D',
  CODE_ANSI_RIGHT = 'C'
};

// Style: WASD
enum {
  CODE_WASD_UP = 'W',
  CODE_WASD_DOWN = 'S',
  CODE_WASD_LEFT = 'A',
  CODE_WASD_RIGHT = 'D'
};

// Style: Vim
enum {
  CODE_VIM_UP = 'K',
  CODE_VIM_DOWN = 'J',
  CODE_VIM_LEFT = 'H',
  CODE_VIM_RIGHT = 'L'
};

enum { CODE_HOTKEY_ACTION_SAVE = 'Z', CODE_HOTKEY_ALTERNATE_ACTION_SAVE = 'P' };

} // namespace Code
} // namespace Keypress
} // namespace

Color::Modifier Tile::tileColor(ull value) {
  std::vector<Color::Modifier> colors{red, yellow, magenta, blue, cyan, yellow,
                                      red, yellow, magenta, blue, green};
  int log = log2(value);
  int index = log < 12 ? log - 1 : 10;

  return colors[index];
}

int GetLines() {
  int noOfLines = 0;
  std::string tempLine;
  std::ifstream stateFile("./data/previousGame");
  while (std::getline(stateFile, tempLine, '\n')) {
    noOfLines++;
  }
  stateFile.close();
  return noOfLines;
}

void Game::initialiseContinueBoardArray() {

  std::ifstream stateFile("./data/previousGame");
  if ((bool)stateFile) {
    std::string temp, tempLine, tempBlock;
    const ull savedBoardPlaySize = GetLines();

    std::string tempArr[savedBoardPlaySize][savedBoardPlaySize];
    int i = 0;
    while (std::getline(stateFile, tempLine, '\n') && i < savedBoardPlaySize) {
      std::stringstream line(tempLine);
      int j = 0;
      while (std::getline(line, temp, ',') && j < savedBoardPlaySize) {
        tempArr[j][i] = temp;
        j++;
      }
      i++;
    }

    gamePlayBoard = GameBoard(savedBoardPlaySize);

    for (int i = 0; i < gamePlayBoard.getPlaySize(); i++) {
      for (int j = 0; j < gamePlayBoard.getPlaySize(); j++) {
        std::stringstream blocks(tempArr[j][i]);
        int k = 0;
        while (std::getline(blocks, tempBlock, ':')) {
          const auto current_point = point2D_t{j, i};
          if (k == 0) {
            gamePlayBoard.setTileValue(current_point, std::stoi(tempBlock));
          } else if (k == 1) {
            gamePlayBoard.setTileBlocked(current_point, std::stoi(tempBlock));
          }
          k++;
        }
      }
    }
    stateFile.close();
    std::ifstream stats("./data/previousGameStats");
    while (std::getline(stats, tempLine, '\n')) {
      std::stringstream line(tempLine);
      int k = 0;
      while (std::getline(line, temp, ':')) {
        if (k == 0)
          gamePlayBoard.score = std::stoi(temp);
        else if (k == 1)
          moveCount = std::stoi(temp) - 1;
        k++;
      }
    }

  } else {
    noSave = true;
  }
}

void Game::drawBoard() const {

  clearScreen();
  drawAscii();
  drawScoreBoard(std::cout);
  std::cout << gamePlayBoard;
}

void Game::drawScoreBoard(std::ostream &out_stream) const {
  constexpr auto score_text_label = "SCORE:";
  constexpr auto bestscore_text_label = "BEST SCORE:";
  constexpr auto moves_text_label = "MOVES:";

  // * border padding: vvv
  // | l-outer: 2, r-outer: 0
  // | l-inner: 1, r-inner: 1
  // * top border / bottom border: vvv
  // | tl_corner + horizontal_sep + tr_corner = length: 1 + 27 + 1
  // | bl_corner + horizontal_sep + br_corner = length: 1 + 27 + 1
  enum {
    UI_SCOREBOARD_SIZE = 27,
    UI_BORDER_OUTER_PADDING = 2,
    UI_BORDER_INNER_PADDING = 1
  }; // length of horizontal board - (corners + border padding)
  constexpr auto border_padding_char = ' ';
  constexpr auto vertical_border_pattern = "│";
  constexpr auto top_board =
      "┌───────────────────────────┐"; // Multibyte character set
  constexpr auto bottom_board =
      "└───────────────────────────┘"; // Multibyte character set
  const auto outer_border_padding =
      std::string(UI_BORDER_OUTER_PADDING, border_padding_char);
  const auto inner_border_padding =
      std::string(UI_BORDER_INNER_PADDING, border_padding_char);
  const auto inner_padding_length =
      UI_SCOREBOARD_SIZE - (std::string{inner_border_padding}.length() * 2);
  out_stream << outer_border_padding << top_board << "\n";
  out_stream << outer_border_padding << vertical_border_pattern
             << inner_border_padding << bold_on << score_text_label << bold_off
             << std::string(inner_padding_length -
                                std::string{score_text_label}.length() -
                                std::to_string(gamePlayBoard.score).length(),
                            border_padding_char)
             << gamePlayBoard.score << inner_border_padding
             << vertical_border_pattern << "\n";
  if (gamePlayBoard.getPlaySize() == COMPETITION_GAME_BOARD_PLAY_SIZE) {
    const auto tempBestScore =
        (bestScore < gamePlayBoard.score ? gamePlayBoard.score : bestScore);
    out_stream << outer_border_padding << vertical_border_pattern
               << inner_border_padding << bold_on << bestscore_text_label
               << bold_off
               << std::string(inner_padding_length -
                                  std::string{bestscore_text_label}.length() -
                                  std::to_string(tempBestScore).length(),
                              border_padding_char)
               << tempBestScore << inner_border_padding
               << vertical_border_pattern << "\n";
  }
  out_stream << outer_border_padding << vertical_border_pattern
             << inner_border_padding << bold_on << moves_text_label << bold_off
             << std::string(inner_padding_length -
                                std::string{moves_text_label}.length() -
                                std::to_string(moveCount).length(),
                            border_padding_char)
             << moveCount << inner_border_padding << vertical_border_pattern
             << "\n";
  out_stream << outer_border_padding << bottom_board << "\n \n";
}

void Game::input(KeyInputErrorStatus err) {

  using namespace Keypress::Code;
  char c;

  std::cout << "  W or K or \u2191 => Up";
  newline();
  std::cout << "  A or H or \u2190 => Left";
  newline();
  std::cout << "  S or J or \u2193 => Down";
  newline();
  std::cout << "  D or L or \u2192 => Right";
  newline();
  std::cout << "  Z or P => Save";
  newline(2);
  std::cout << "  Press the keys to start and continue.";
  newline();

  if (err == KeyInputErrorStatus::STATUS_INPUT_ERROR) {
    std::cout << red << "  Invalid input. Please try again." << def;
    newline(2);
  }

  getInput(c);

  if (c == CODE_ANSI_TRIGGER_1) {
    getInput(c);
    if (c == CODE_ANSI_TRIGGER_2) {
      getInput(c);
      newline(4);
      switch (c) {
      case CODE_ANSI_UP:
        decideMove(UP);
        return;
      case CODE_ANSI_DOWN:
        decideMove(DOWN);
        return;
      case CODE_ANSI_RIGHT:
        decideMove(RIGHT);
        return;
      case CODE_ANSI_LEFT:
        decideMove(LEFT);
        return;
      }
    } else {
      newline(4);
    }
  }

  newline(4);

  switch (toupper(c)) {

  case CODE_WASD_UP:
  case CODE_VIM_UP:
    decideMove(UP);
    break;
  case CODE_WASD_LEFT:
  case CODE_VIM_LEFT:
    decideMove(LEFT);
    break;
  case CODE_WASD_DOWN:
  case CODE_VIM_DOWN:
    decideMove(DOWN);
    break;
  case CODE_WASD_RIGHT:
  case CODE_VIM_RIGHT:
    decideMove(RIGHT);
    break;
  case CODE_HOTKEY_ACTION_SAVE:
  case CODE_HOTKEY_ALTERNATE_ACTION_SAVE:
    saveState();
    stateSaved = true;
    break;
  default:
    drawBoard();
    input(KeyInputErrorStatus::STATUS_INPUT_ERROR);
    break;
  }
}

void Game::decideMove(Directions d) {

  switch (d) {
  case UP:
    gamePlayBoard.tumbleTilesUp();
    break;

  case DOWN:
    gamePlayBoard.tumbleTilesDown();
    break;

  case LEFT:
    gamePlayBoard.tumbleTilesLeft();
    break;

  case RIGHT:
    gamePlayBoard.tumbleTilesRight();
    break;
  }
}

void Game::statistics() const {

  std::cout << yellow << "  STATISTICS" << def;
  newline();
  std::cout << yellow << "  ──────────" << def;
  newline();
  std::cout << "  Final score:       " << bold_on << gamePlayBoard.score
            << bold_off;
  newline();
  std::cout << "  Largest Tile:      " << bold_on << gamePlayBoard.largestTile
            << bold_off;
  newline();
  std::cout << "  Number of moves:   " << bold_on << moveCount << bold_off;
  newline();
  std::cout << "  Time taken:        " << bold_on << secondsFormat(duration)
            << bold_off;
  newline();
}

void Game::saveStats() const {
  Stats stats;
  stats.collectStatistics();
  stats.bestScore = stats.bestScore < gamePlayBoard.score ?
                        gamePlayBoard.score :
                        stats.bestScore;
  stats.gameCount++;
  stats.winCount = gamePlayBoard.win ? stats.winCount + 1 : stats.winCount;
  stats.totalMoveCount += moveCount;
  stats.totalDuration += duration;

  std::fstream statistics("../data/statistics.txt");
  statistics << stats.bestScore << std::endl
             << stats.gameCount << std::endl
             << stats.winCount << std::endl
             << stats.totalMoveCount << std::endl
             << stats.totalDuration;

  statistics.close();
}

void Game::saveScore() const {
  Scoreboard s;
  s.score = gamePlayBoard.score;
  s.win = gamePlayBoard.win;
  s.moveCount = moveCount;
  s.largestTile = gamePlayBoard.largestTile;
  s.duration = duration;
  s.save();
}

void Game::saveState() const {
  std::remove("../data/previousGame");
  std::remove("../data/previousGameStats");
  std::fstream stats("../data/previousGameStats", std::ios_base::app);
  std::fstream stateFile("../data/previousGame", std::ios_base::app);
  stateFile << gamePlayBoard.printState();
  stateFile.close();
  stats << gamePlayBoard.score << ":" << moveCount;
  stats.close();
}

void Game::playGame(ContinueStatus cont) {

  auto startTime = std::chrono::high_resolution_clock::now();

  while (true) {
    if (gamePlayBoard.moved) {
      gamePlayBoard.addTile();
      moveCount++;
      gamePlayBoard.moved = false;
    }

    drawBoard();

    if (gamePlayBoard.win || !gamePlayBoard.canMove()) {
      break;
    }

    if (stateSaved) {
      std::cout << green << bold_on
                << "  The game has been saved. Feel free to take a break."
                << def << bold_off;
      newline(2);
      stateSaved = false;
    }
    input();
    gamePlayBoard.unblockTiles();
  }

  auto finishTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finishTime - startTime;
  duration = elapsed.count();

  const auto msg = gamePlayBoard.win ? "  You win! Congratulations! " :
                                       "  Game over! You lose.";

  if (gamePlayBoard.win) {
    std::cout << green << bold_on << msg << def << bold_off;
    newline(3);
  } else {
    std::cout << red << bold_on << msg << def << bold_off;
    newline(3);
  }

  if (gamePlayBoard.getPlaySize() == COMPETITION_GAME_BOARD_PLAY_SIZE &&
      cont == ContinueStatus::STATUS_END_GAME) {
    statistics();
    saveStats();
    newline(2);
    saveScore();
  }

}

ull Game::setBoardSize() {

  enum { MIN_GAME_BOARD_PLAY_SIZE = 3, MAX_GAME_BOARD_PLAY_SIZE = 10 };
  bool err = false;
  ull userInput_PlaySize{0};
  while ((userInput_PlaySize < MIN_GAME_BOARD_PLAY_SIZE ||
          userInput_PlaySize > MAX_GAME_BOARD_PLAY_SIZE)) {
    clearScreen();
    drawAscii();

    if (err) {
      std::cout << red << "  Invalid input. Gameboard size should range from "
                << MIN_GAME_BOARD_PLAY_SIZE << " to "
                << MAX_GAME_BOARD_PLAY_SIZE << "." << def;
      newline(2);
    } else if (noSave) {
      std::cout << red << bold_on
                << "  No saved game found. Starting a new game." << def
                << bold_off;
      newline(2);
      noSave = false;
    }

    std::cout << bold_on
              << "  Enter gameboard size (NOTE: Scores and statistics will be "
                 "saved only for the 4x4 gameboard): "
              << bold_off;

    std::cin >> userInput_PlaySize;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::int32_t>::max(), '\n');
    err = true;
  }
  return userInput_PlaySize;
}

void Game::startGame() {

  Stats stats;
  if (stats.collectStatistics()) {
    bestScore = stats.bestScore;
  }

  ull userInput_PlaySize = setBoardSize();

  gamePlayBoard = GameBoard(userInput_PlaySize);
  gamePlayBoard.addTile();

  playGame(ContinueStatus::STATUS_END_GAME);
}

void Game::continueGame() {

  Stats stats;
  if (stats.collectStatistics()) {
    bestScore = stats.bestScore;
  }

  initialiseContinueBoardArray();

  if (noSave) {
    startGame();
  } else {
    playGame(ContinueStatus::STATUS_CONTINUE);
  }
}
