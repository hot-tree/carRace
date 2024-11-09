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
    int carType;
} MEDIA_FILE;

typedef struct {
    int carType;
    int x;
    int y;
    int width;
    int height;
    int fuel;
} PLAYER_CAR;

typedef struct {
    int carType;
    int x;
    int y;
    int width;
    int height;
    int flag; // プレイヤーカーに追い越されたかどうか
} COMPUTER_CAR;

MEDIA_FILE const carImgs[] = {
    {"resources/image/car_red.png", CAR_TYPE_RED},
    {"resources/image/car_yellow.png", CAR_TYPE_YELLOW},
    {"resources/image/car_blue.png", CAR_TYPE_BLUE},
    {"resources/image/truck.png", CAR_TYPE_TRUCK}
};

int carGraphHandles[CAR_TYPE_NUM]; // 車画像のハンドル

// 車を表示する関数
void drawPlayerCar(PLAYER_CAR playerCar)
{
    DrawGraph(playerCar.x - playerCar.width / 2, playerCar.y - playerCar.height / 2, carGraphHandles[playerCar.carType], TRUE);
}

void drawComputerCar(COMPUTER_CAR computerCar)
{
    DrawGraph(computerCar.x - computerCar.width / 2, computerCar.y - computerCar.height / 2, carGraphHandles[computerCar.carType], TRUE);
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
    PLAYER_CAR playerCar;
    playerCar.carType = CAR_TYPE_RED;
    playerCar.x = WIDTH / 2;
    playerCar.y = HEIGHT / 2;
    GetGraphSize(carGraphHandles[playerCar.carType], &playerCar.width, &playerCar.height);

    // コンピューターが動かす車用の配列
    const int COM_MAX = 8;
    COMPUTER_CAR computerCars[COM_MAX];
    for (int i = 0; i < COM_MAX; i++) // 初期値の代入
    {
        computerCars[i].x = rand() % 180 + 270;
        computerCars[i].y = -100;
        computerCars[i].carType = CAR_TYPE_YELLOW + rand() % 3;
        GetGraphSize(carGraphHandles[computerCars[i].carType], &computerCars[i].width, &computerCars[i].height);
        computerCars[i].flag = 0;
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
            GetMousePoint(&playerCar.x, &playerCar.y);
            if (playerCar.x < 260) playerCar.x = 260;
            if (playerCar.x > 460) playerCar.x = 460;
            if (playerCar.y < 40) playerCar.y = 40;
            if (playerCar.y > 600) playerCar.y = 600;
        }
        drawPlayerCar(playerCar);

        // コンピューターの車を動かす処理
        for (int i = 0; i < COM_MAX; i++)
        {
            if (scene == PLAY) // プレイ中の車の処理
            {
                computerCars[i].y = computerCars[i].y + 1 + i;
                //画面の下から外に出たかを判定
                if (computerCars[i].y > HEIGHT + 100)
                {
                    computerCars[i].x = rand() % 180 + 270;
                    computerCars[i].y = -100;
                    computerCars[i].carType = CAR_TYPE_YELLOW + rand() % 3;
                    computerCars[i].flag = 0;
                }
                // ヒットチェック
                int dx = abs(computerCars[i].x - playerCar.x); // x軸方向のピクセル数
                int dy = abs(computerCars[i].y - playerCar.y); // y軸方向のピクセル数
                int wid = playerCar.width / 2 + computerCars[i].width / 2 - 4;
                int hei = playerCar.height / 2 + computerCars[i].height / 2 - 4;
                if (dx < wid && dy < hei) // 接触しているか
                {
                    int col = GetColor(rand() % 256, rand() % 256, rand() % 256); // 重ねる色
                    SetDrawBlendMode(DX_BLENDMODE_ADD, 255); // 色を加算する設定
                    DrawBox(playerCar.x - playerCar.width / 2, playerCar.y - playerCar.height / 2, playerCar.x + playerCar.width / 2, playerCar.y + playerCar.height / 2, col, TRUE);
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // 通常の描画に戻す
                    PlaySoundMem(seCrash, DX_PLAYTYPE_BACK); // 効果音
                    fuel -= 10;
                }
                // 追い抜いたかを判定
                if (computerCars[i].y > playerCar.y && computerCars[i].flag == 0)
                {
                    computerCars[i].flag = 1;
                    score += 100;
                    if (score > highScore) highScore = score;
                }
            }
            else // タイトル画面とゲームオーバー画面での車の動き
            {
                computerCars[i].y = computerCars[i].y - 1 - i;
                if (computerCars[i].y < -100) computerCars[i].y = HEIGHT + 100;
            }
            drawComputerCar(computerCars[i]);
        }

        // 燃料アイテムの処理
        if (scene == PLAY) // ゲーム中だけ出現
        {
            fuelY += 4;
            if (fuelY > HEIGHT) fuelY = -100;
            if (abs(fuelX - playerCar.x) < playerCar.width / 2 + 12 && abs(fuelY - playerCar.y) < playerCar.height / 2 + 12)
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
                playerCar.x = WIDTH / 2;
                playerCar.y = HEIGHT / 2;
                for (int i = 0; i < COM_MAX; i++)
                {
                    computerCars[i].y = HEIGHT + 100;
                    computerCars[i].flag = 0;
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