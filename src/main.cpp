#include "DxLib.h"
#include <stdlib.h>

typedef enum {
    CAR_TYPE_RED,
    CAR_TYPE_YELLOW,
    CAR_TYPE_BLUE,
    CAR_TYPE_TRUCK,
    CAR_TYPE_NUM // 車のタイプの数
} CAR_TYPE;

typedef struct {
    const char *filePath;
    CAR_TYPE carType;
} MEDIA_FILE;

MEDIA_FILE const carImgs[] = {
    {"resources/image/car_red.png", CAR_TYPE_RED},
    {"resources/image/car_yellow.png", CAR_TYPE_YELLOW},
    {"resources/image/car_blue.png", CAR_TYPE_BLUE},
    {"resources/image/truck.png", CAR_TYPE_TRUCK}
};

int carGraphHandles[CAR_TYPE_NUM]; // 車画像のハンドル

// 車の画像を管理する定数と配列
const int CAR_W[CAR_TYPE_NUM] = { 32, 26, 26,  40 };
const int CAR_H[CAR_TYPE_NUM] = { 48, 48, 48, 100 };

// 車を表示する関数
void drawCar(int x, int y, int type)
{
    DrawGraph(x - CAR_W[type] / 2, y - CAR_H[type] / 2, carGraphHandles[type], TRUE);
}

// 影を付けた文字列を表示する関数
void drawText(int x, int y, int col, const char* txt, int val, int siz)
{
    SetFontSize(siz);
    DrawFormatString(x + 2, y + 2, 0x000000, txt, val);
    DrawFormatString(x, y, col, txt, val);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 定数
    const int WIDTH = 720, HEIGHT = 640; // ウィンドウの幅と高さのピクセル数

    SetWindowText("carRace"); // ウィンドウのタイトル
    SetGraphMode(WIDTH, HEIGHT, 32); // ウィンドウの大きさとカラービット数の指定
    ChangeWindowMode(TRUE); // ウィンドウモードで起動
    if (DxLib_Init() == -1) return -1; // ライブラリ初期化 エラーが起きたら終了
    SetBackgroundColor(0, 0, 0); // 背景色の指定
    SetDrawScreen(DX_SCREEN_BACK); // 描画面を裏画面にする

    int bgY = 0; // 道路をスクロールさせるための変数
    int imgBG = LoadGraph("resources/image/bg.png"); // 背景の画像

    // 車の画像を配列に読み込む
    for (int i = 0; i < CAR_TYPE_NUM; i++)
    {
        carGraphHandles[i] = LoadGraph(carImgs[i].filePath);
    }

    // プレイヤーの車用の変数
    int playerX = WIDTH / 2;
    int playerY = HEIGHT / 2;
    int playerType = CAR_TYPE_RED;

    // コンピューターが動かす車用の配列
    const int COM_MAX = 8;
    int computerX[COM_MAX], computerY[COM_MAX], computerType[COM_MAX], computerFlag[COM_MAX];
    for (int i = 0; i < COM_MAX; i++) // 初期値の代入
    {
        computerX[i] = rand() % 180 + 270;
        computerY[i] = -100;
        computerType[i] = CAR_TYPE_YELLOW + rand() % 3;
        computerFlag[i] = 0;
    }

    // スコアとハイスコアを代入する変数
    int score = 0;
    int highScore = 5000;

    // 燃料アイテム用の変数
    int fuel = 0;
    int fuelX = WIDTH / 2;
    int fuelY = 0;
    int imgFuel = LoadGraph("resources/image/fuel.png");

    // ゲーム進行に関する変数
    enum { TITLE, PLAY, OVER };
    int scene = TITLE;
    int timer = 0;

    // サウンドの読み込みと音量設定
    int bgm = LoadSoundMem("resources/sound/bgm.mp3");
    int jin = LoadSoundMem("resources/sound/gameover.mp3");
    int seFuel = LoadSoundMem("resources/sound/fuel.mp3");
    int seCrash = LoadSoundMem("resources/sound/crash.mp3");
    ChangeVolumeSoundMem(128, bgm);
    ChangeVolumeSoundMem(128, jin);

    while (1) // メインループ
    {
        ClearDrawScreen(); // 画面をクリアする

        // 背景のスクロール処理
        if (scene == PLAY) bgY = (bgY + 10) % HEIGHT; // プレイ中にだけスクロール
        DrawGraph(0, bgY - HEIGHT, imgBG, FALSE);
        DrawGraph(0, bgY, imgBG, FALSE);

        // プレイヤーの車を動かす処理
        if (scene == PLAY) // プレイ中にだけ動かす
        {
            GetMousePoint(&playerX, &playerY);
            if (playerX < 260) playerX = 260;
            if (playerX > 460) playerX = 460;
            if (playerY < 40) playerY = 40;
            if (playerY > 600) playerY = 600;
        }
        drawCar(playerX, playerY, playerType);

        // コンピューターの車を動かす処理
        for (int i = 0; i < COM_MAX; i++)
        {
            if (scene == PLAY) // プレイ中の車の処理
            {
                computerY[i] = computerY[i] + 1 + i;
                //画面の下から外に出たかを判定
                if (computerY[i] > HEIGHT + 100)
                {
                    computerX[i] = rand() % 180 + 270;
                    computerY[i] = -100;
                    computerType[i] = CAR_TYPE_YELLOW + rand() % 3;
                    computerFlag[i] = 0;
                }
                // ヒットチェック
                int dx = abs(computerX[i] - playerX); // x軸方向のピクセル数
                int dy = abs(computerY[i] - playerY); // y軸方向のピクセル数
                int wid = CAR_W[playerType] / 2 + CAR_W[computerType[i]] / 2 - 4;
                int hei = CAR_H[playerType] / 2 + CAR_H[computerType[i]] / 2 - 4;
                if (dx < wid && dy < hei) // 接触しているか
                {
                    int col = GetColor(rand() % 256, rand() % 256, rand() % 256); // 重ねる色
                    SetDrawBlendMode(DX_BLENDMODE_ADD, 255); // 色を加算する設定
                    DrawBox(playerX - CAR_W[playerType] / 2, playerY - CAR_H[playerType] / 2, playerX + CAR_W[playerType] / 2, playerY + CAR_H[playerType] / 2, col, TRUE);
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // 通常の描画に戻す
                    PlaySoundMem(seCrash, DX_PLAYTYPE_BACK); // 効果音
                    fuel -= 10;
                }
                // 追い抜いたかを判定
                if (computerY[i] > playerY && computerFlag[i] == 0)
                {
                    computerFlag[i] = 1;
                    score += 100;
                    if (score > highScore) highScore = score;
                }
            }
            else // タイトル画面とゲームオーバー画面での車の動き
            {
                computerY[i] = computerY[i] - 1 - i;
                if (computerY[i] < -100) computerY[i] = HEIGHT + 100;
            }
            drawCar(computerX[i], computerY[i], computerType[i]);
        }

        // 燃料アイテムの処理
        if (scene == PLAY) // ゲーム中だけ出現
        {
            fuelY += 4;
            if (fuelY > HEIGHT) fuelY = -100;
            if (abs(fuelX - playerX) < CAR_W[playerType] / 2 + 12 && abs(fuelY - playerY) < CAR_H[playerType] / 2 + 12)
            {
                fuelX = rand() % 180 + 270;
                fuelY = -500;
                fuel += 200;
                PlaySoundMem(seFuel, DX_PLAYTYPE_BACK); // 効果音
            }
            DrawGraph(fuelX - 12, fuelY - 12, imgFuel, TRUE);
        }

        timer++; // タイマーをカウント
        switch (scene) // タイトル、ゲームプレイ、ゲームオーバーの分岐
        {
        case TITLE: // タイトル画面の処理
            drawText(160, 160, 0xffffff, "Car Race", 0, 100);
            if (timer % 60 < 30) drawText(210, 400, 0x00ff00, "Click to start.", 0, 40);
            if (GetMouseInput() & MOUSE_INPUT_LEFT)
            {
                playerX = WIDTH / 2;
                playerY = HEIGHT / 2;
                for (int i = 0; i < COM_MAX; i++)
                {
                    computerY[i] = HEIGHT + 100;
                    computerFlag[i] = 0;
                }
                fuelX = WIDTH / 2;
                fuelY = -100;
                score = 0;
                fuel = 1000;
                scene = PLAY;
                PlaySoundMem(bgm, DX_PLAYTYPE_LOOP); // ＢＧＭをループ再生
            }
            break;

        case PLAY: // ゲームをプレイする処理
            fuel -= 1;
            if (fuel < 0)
            {
                fuel = 0;
                scene = OVER;
                timer = 0;
                StopSoundMem(bgm); // ＢＧＭを停止
                PlaySoundMem(jin, DX_PLAYTYPE_BACK); // ジングルを出力
            }
            break;

        case OVER: // ゲームオーバーの処理
            drawText(180, 240, 0xff0000, "GAME OVER", 0, 80);
            if (timer > 60 * 5) scene = TITLE;
            break;
        }

        // スコアなどの表示
        drawText(10, 10, 0x00ffff, "SCORE %d", score, 30);
        drawText(WIDTH - 200, 10, 0xffff00, "HI-SC %d", highScore, 30);
        int col = 0x00ff00; // 燃料の値を表示する色
        if (fuel < 400) col = 0xffc000;
        if (fuel < 200) col = 0xff0000;
        drawText(10, HEIGHT - 40, col, "FUEL %d", fuel, 30);

        ScreenFlip(); // 裏画面の内容を表画面に反映させる
        WaitTimer(16); // 一定時間待つ
        if (ProcessMessage() == -1) break; // Windowsから情報を受け取りエラーが起きたら終了
        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break; // ESCキーが押されたら終了
    }

    DxLib_End(); // ＤＸライブラリ使用の終了処理
    return 0; // ソフトの終了
}