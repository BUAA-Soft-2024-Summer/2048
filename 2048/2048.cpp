// Copyright (C) 2018 - 2023 Tony's Studio. All rights reserved.
// Licensed under the MIT License.

#include <cstdio>
#include <cstdlib>  // rand(), srand()
#include <ctime>    // time()
#include <easyx.h>
#include <Windows.h>
#include "2048.h"

// global font family
const wchar_t FONT_FAMILY[] = L"Comic Sans MS";

// high score save file name
const wchar_t SAVE_FILE[] = L"score.txt";

int score; // current score
int highScore; // high score

// essential game data
const int BOARD_SIZE = 4;
// we add padding around actual board to avoid range check
int board[BOARD_SIZE + 2][BOARD_SIZE + 2]; // current game board
int boardCopy[BOARD_SIZE + 2][BOARD_SIZE + 2]; // copy of game board
bool isMerged[BOARD_SIZE + 2][BOARD_SIZE + 2]; // whether merged once or not

// user input, value is virtual key code
int userInput;

// game status
enum GameStatus
{
    WINNER,
    LOSER,
    ABORT,
    RESTART
};

// tile colors
struct TileStyle
{
    COLORREF backColor;
    COLORREF frontColor;
    int fontSize;
};

const TileStyle TILE_STYLES[] = {
    { 0xb5becc, WHITE,    72 }, // 0
    { 0xdbe6ee, 0x707b83, 72 }, // 2
    { 0xc7e1ed, 0x707b83, 72 }, // 4
    { 0x78b2f4, WHITE,    72 }, // 8
    { 0x538ded, WHITE,    72 }, // 16
    { 0x607df6, WHITE,    72 }, // 32
    { 0x3958e9, WHITE,    72 }, // 64
    { 0x6bd9f5, WHITE,    56 }, // 128
    { 0x4bd0f2, WHITE,    56 }, // 256
    { 0x2ac0e4, WHITE,    56 }, // 512
    { 0x13b8e3, WHITE,    40 }, // 1024
    { 0x00c5eb, WHITE,    40 }, // 2048
    { 0x3958e9, WHITE,    40 }, // 5096
};


void Initialize()
{
    srand((unsigned)time(nullptr));
    LoadHighScore();

    initgraph(400, 480);
    setbkcolor(RGB(250, 248, 239));
    setbkmode(TRANSPARENT); // make text background transparent

    BeginBatchDraw();
}

void Launch()
{
    // whether start new round
    bool startNewRound = true;
    do
    {
        // user's choice of message box
        int choice;
        // 1 for winner, 2 for loser, -1 for aborted, -2 for restart
        GameStatus gameStatus;

        // reset game status to start new round
        ResetGame();

        // main loop, for one round
        for (; ;)
        {
            ProcessInput();
            if (userInput == VK_ESCAPE)
            {
                // confirm exit
                choice = MessageBox(nullptr, L"Quit game?", L"Confirm Quit", MB_OKCANCEL);
                if (choice == IDOK)
                {
                    gameStatus = ABORT;
                    break;
                }
            }
            else if (userInput == 'R')
            {
                // confirm reset game
                choice = MessageBox(nullptr, L"Restart game?", L"Confirm Restart", MB_OKCANCEL);
                if (choice == IDOK)
                {
                    gameStatus = RESTART;
                    break;
                }
            }

            Update();
            DrawInterface();

            // check game status
            if (IsVictorious())
            {
                gameStatus = WINNER;
                break;
            }
            if (IsLost())
            {
                gameStatus = LOSER;
                break;
            }

            // give CPU some break
            Sleep(15);
        }

        if (gameStatus == ABORT)
        {
            break;
        }
        if (gameStatus == RESTART)
        {
            continue;
        }

        // one round over, save high score
        SaveHighScore();

        if (gameStatus == WINNER)
        {
            choice = MessageBox(nullptr, L"You are victorious!\nPlay Again?", L"Winner", MB_OKCANCEL);
        }
        else if (gameStatus == LOSER)
        {
            choice = MessageBox(nullptr, L"You have lost!\nPlay Again?", L"Loser", MB_OKCANCEL);
        }
        else
        {
            // shouldn't reach here
            choice = IDCANCEL;
        }

        if (choice == IDCANCEL)
        {
            // stop game if user selected cancel
            startNewRound = false;
        }
    } while (startNewRound);
}

void ClearUp()
{
    EndBatchDraw();
    closegraph();
}

void LoadHighScore()
{
    FILE* fp;
    if (_wfopen_s(&fp, SAVE_FILE, L"r") == 0)
    {
        // successfully opened the file
        if (fwscanf_s(fp, L"%d", &highScore) != 1)
        {
            highScore = 0;
        }
        fclose(fp);
    }
    else
    {
        highScore = 0;
    }
}

void SaveHighScore()
{
    FILE* fp;
    if (_wfopen_s(&fp, SAVE_FILE, L"w") == 0)
    {
        fwprintf(fp, L"%d", highScore);
        fclose(fp);
    }
    else
    {
        // save failed, do nothing for now
    }
}

void ResetGame()
{
    score = 0;
    memset(board, 0, sizeof(board));

    // random select two starting block
    int x1 = rand() % 4 + 1;
    int y1 = rand() % 4 + 1;
    int x2 = rand() % 4 + 1;
    int y2 = rand() % 4 + 1;
    while ((x1 == x2) && (y1 == y2))
    {
        // if two blocks are the same, select a new one
        x2 = rand() % 4 + 1;
        y2 = rand() % 4 + 1;
    }
    board[x1][y1] = (rand() % 6 == 5) ? 4 : 2;
    board[x2][y2] = (rand() % 6 == 5) ? 4 : 2;
}

void ProcessInput()
{
    ExMessage message;

    userInput = 0;
    // use while to iterate through all messages during two updates
    while (peekmessage(&message, EX_KEY))
    {
        if (message.message == WM_KEYDOWN)
        {
            // prevent holding key
            if (!message.prevdown)
            {
                userInput = message.vkcode;
            }
        }
    }
}

void Update()
{
    if (userInput == 0)
    {
        // direct return if no interactions
        return;
    }

    memcpy(boardCopy, board, sizeof(board));
    memset(isMerged, false, sizeof(isMerged));

    // move board array
    switch (userInput)
    {
    case VK_UP:
    case 'W':
        // column
        for (int j = 1; j <= BOARD_SIZE; j++)
        {
            // row
            for (int i = 2; i <= BOARD_SIZE; i++)
            {
                if (!board[i][j])
                {
                    continue;
                }

                int k = i;
                // move if no tile blocking it
                while (!board[k - 1][j] && k >= 2)
                {
                    board[k - 1][j] = board[k][j];
                    board[k][j] = 0;
                    k--;
                }
                // merge tiles with same number
                if (board[k][j] == board[k - 1][j] && !isMerged[k - 1][j])
                {
                    board[k - 1][j] = 2 * board[k][j];
                    board[k][j] = 0;
                    isMerged[k - 1][j] = true;
                    score += board[k - 1][j];
                }
            }
        }
        break;
    case VK_DOWN:
    case 'S':
        for (int j = 1; j <= BOARD_SIZE; j++)
        {
            for (int i = 3; i >= 1; i--)
            {
                if (!board[i][j])
                {
                    continue;
                }

                int k = i;
                while (!board[k + 1][j] && k <= 3)
                {
                    board[k + 1][j] = board[k][j];
                    board[k][j] = 0;
                    k++;
                }
                if (board[k][j] == board[k + 1][j] && !isMerged[k + 1][j])
                {
                    board[k + 1][j] = 2 * board[k][j];
                    board[k][j] = 0;
                    isMerged[k + 1][j] = true;
                    score += board[k + 1][j];
                }
            }
        }
        break;
    case VK_LEFT:
    case 'A':
        for (int i = 1; i <= BOARD_SIZE; i++)
        {
            for (int j = 2; j <= BOARD_SIZE; j++)
            {
                if (!board[i][j])
                {
                    continue;
                }

                int k = j;
                while (!board[i][k - 1] && k >= 2)
                {
                    board[i][k - 1] = board[i][k];
                    board[i][k] = 0;
                    k--;
                }
                if (board[i][k] == board[i][k - 1] && !isMerged[i][k - 1])
                {
                    board[i][k - 1] = 2 * board[i][k];
                    board[i][k] = 0;
                    isMerged[i][k - 1] = true;
                    score += board[i][k - 1];
                }
            }
        }
        break;
    case VK_RIGHT:
    case 'D':
        for (int i = 1; i <= BOARD_SIZE; i++)
        {
            for (int j = 3; j >= 1; j--)
            {
                if (!board[i][j])
                {
                    continue;
                }

                int k = j;
                while (!board[i][k + 1] && k <= 3)
                {
                    board[i][k + 1] = board[i][k];
                    board[i][k] = 0;
                    k++;
                }
                if (board[i][k] == board[i][k + 1] && !isMerged[i][k + 1])
                {
                    board[i][k + 1] = 2 * board[i][k];
                    board[i][k] = 0;
                    isMerged[i][k + 1] = true;
                    score += board[i][k + 1];
                }
            }
        }
        break;
    default:
        break;
    }

    // whether board changed after move
    bool isChanged = false;
    for (int i = 1; i <= BOARD_SIZE; i++)
    {
        for (int j = 1; j <= BOARD_SIZE; j++)
        {
            if (board[i][j] != boardCopy[i][j])
            {
                isChanged = true;
                break;
            }
        }
    }
    if (!isChanged)
    {
        return;
    }

    // generate a new number
    int x, y;
    do
    {
        x = rand() % 4 + 1;
        y = rand() % 4 + 1;
    } while (board[x][y]);

    // possibility for 2 is 5/6
    // possibility for 4 is 1/6
    board[x][y] = (rand() % 6 == 5) ? 4 : 2;

    highScore = max(highScore, score);
}

void ApplyTileStyle(int number)
{
    int index = 0;

    while (number)
    {
        number >>= 1;
        index++;
    }

    setfillcolor(TILE_STYLES[index].backColor);
    settextcolor(TILE_STYLES[index].frontColor);
    settextstyle(TILE_STYLES[index].fontSize, 0, FONT_FAMILY);
}

void DrawInterface()
{
    wchar_t buffer[16];

    // since all drawing operations are here, we can
    // place 'cleardevice()' inside this function.
    cleardevice();

    // draw title
    settextcolor(RGB(119, 110, 101));
    settextstyle(50, 0, FONT_FAMILY);
    outtextxy(10, 10, L"2048");
    settextstyle(20, 0, FONT_FAMILY);
    outtextxy(10, 65, L"Join the numbers and get to the 2048 tile!");


    // draw current score
    setfillcolor(RGB(187, 173, 160));
    solidroundrect(200, 15, 290, 60, 5, 5);
    settextcolor(RGB(230, 220, 210));
    // settextstyle with font-weight
    settextstyle(15, 0, FONT_FAMILY, 0, 0, 600, false, false, false);
    outtextxy(230, 20, L"SCORE");
    swprintf_s(buffer, L"%d", score);
    settextcolor(WHITE);
    settextstyle(25, 0, FONT_FAMILY, 0, 0, 600, false, false, false);
    RECT textArea = { 200, 30, 290, 60 };
    drawtext(buffer, &textArea, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // draw high score
    solidroundrect(295, 15, 385, 60, 5, 5);
    settextcolor(RGB(230, 220, 210));
    settextstyle(15, 0, FONT_FAMILY, 0, 0, 600, false, false, false);
    outtextxy(330, 20, L"BEST");
    swprintf_s(buffer, L"%d", highScore);
    settextcolor(WHITE);
    settextstyle(25, 0, FONT_FAMILY, 0, 0, 600, false, false, false);
    textArea = { 295, 30, 385, 60 };
    drawtext(buffer, &textArea, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // draw board
    solidroundrect(10, 90, 390, 470, 5, 5);
    settextstyle(36, 0, FONT_FAMILY);
    settextcolor(WHITE);
    for (int i = 1; i <= BOARD_SIZE; i++)
    {
        for (int j = 1; j <= BOARD_SIZE; j++)
        {
            // calculate color for each tile
            ApplyTileStyle(board[i][j]);
            solidroundrect(94 * j - 80, 94 * i, 94 * j + 10, 94 * i + 90, 5, 5);
            if (board[i][j])
            {
                swprintf_s(buffer, L"%d", board[i][j]);
                textArea = { 94 * j - 80, 94 * i, 94 * j + 10, 94 * i + 90 };
                drawtext(buffer, &textArea, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }

    // draw to screen buffer
    FlushBatchDraw();
}

bool IsLost()
{
    // if there exists a tile that has any neighboring tile with the same number,
    // or is empty, then the game is not over yet
    for (int i = 1; i <= BOARD_SIZE; i++)
    {
        for (int j = 1; j <= BOARD_SIZE; j++)
        {
            if (!board[i][j] ||
                board[i][j] == board[i + 1][j] ||
                board[i][j] == board[i - 1][j] ||
                board[i][j] == board[i][j + 1] ||
                board[i][j] == board[i][j - 1])
            {
                return false;
            }
        }
    }

    return true;
}

bool IsVictorious()
{
    // wins if any tile is 2048
    for (int i = 1; i <= BOARD_SIZE; i++)
    {
        for (int j = 1; j <= BOARD_SIZE; j++)
        {
            if (board[i][j] == 2048)
            {
                return true;
            }
        }
    }

    return false;
}
