#ifndef FUNCTION_H
#define FUNCTION_H

#include "DxLib.h"

#include "main.h"

/**********************************************************
 * プロトタイプ宣言
***********************************************************/
void drawPlayerCar(PLAYER_CAR playerCar, int graphHandle);
void drawComputerCar(COMPUTER_CAR computerCar, int graphHandle);
void drawText(int x, int y, int col, const char* txt, int val, int siz);

#endif
