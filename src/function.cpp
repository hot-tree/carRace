#include "DxLib.h"

#include "main.h"
#include "function.h"

/**********************************************************
 * 関数定義
***********************************************************/
// プレイヤーカーを表示する
void drawPlayerCar(PLAYER_CAR playerCar, int graphHandle)
{
    DrawGraph(playerCar.x - playerCar.width / 2, playerCar.y - playerCar.height / 2, graphHandle, TRUE);
}

// コンピューターカーを表示する
void drawComputerCar(COMPUTER_CAR computerCar, int graphHandle)
{
    DrawGraph(computerCar.x - computerCar.width / 2, computerCar.y - computerCar.height / 2, graphHandle, TRUE);
}

// 影を付けた文字列を表示する関数
void drawText(int x, int y, int col, const char* txt, int val, int siz)
{
    SetFontSize(siz);
    DrawFormatString(x + 2, y + 2, 0x000000, txt, val);
    DrawFormatString(x, y, col, txt, val);
}
