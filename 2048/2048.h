// Copyright (C) 2018 - 2023 Tony's Studio. All rights reserved.
// Licensed under the MIT License.

#ifndef _2048_H_
#define _2048_H_

// initialize whole game
void Initialize();

// launch and run game
void Launch();

// clear up game on exit
void ClearUp();

// load and save high score to file
void LoadHighScore();
void SaveHighScore();

// reset each round
void ResetGame();

// receive user input
void ProcessInput();
// update game
void Update();
// get style for each number
void ApplyTileStyle(int number);
// draw interface
void DrawInterface();

// check if is lost
bool IsLost();
// check if is victorious
bool IsVictorious();

#endif  // _2048_H_
