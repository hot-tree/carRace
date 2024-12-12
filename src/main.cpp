#include "DxLib.h"
#include <stdlib.h>

#include "main.h"
#include "function.h"

/**********************************************************
 * define
***********************************************************/
#define WIDTH 720
#define HEIGHT 640
/**********************************************************
 * グローバル変数定義
***********************************************************/
CAR_IMAGE const carImgs[] = {
    {"resources/image/car_red.png", CAR_TYPE_RED},
    {"resources/image/car_yellow.png", CAR_TYPE_YELLOW},
    {"resources/image/car_blue.png", CAR_TYPE_BLUE},
    {"resources/image/truck.png", CAR_TYPE_TRUCK}
};

AUDIO_FILE const audioFiles[] = {
    {"resources/sound/bgm.mp3", E_AUDIO_MAIN},
    {"resources/sound/gameover.mp3", E_AUDIO_OVER},
    {"resources/sound/fuel.mp3", E_AUDIO_GET_FUEL},
    {"resources/sound/crash.mp3", E_AUDIO_CRASH},
    {"resources/sound/shield_effect.mp3", E_AUDIO_SHIELD}
};

const char *backgroudImage = "resources/image/bg.png";
const char *fuelImage = "resources/image/fuel.png";
const char *defensiveImage = "resources/image/shield_kiteshield_iron.png";

int carGraphHandles[CAR_TYPE_NUM]; // 車画像のハンドル
int soundHandles[E_AUDIO_NUM]; // オーディオファイルのハンドル

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    SetWindowText("carRace"); // ウィンドウのタイトル
    SetGraphMode(WIDTH, HEIGHT, 32); // ウィンドウの大きさとカラービット数の指定
    ChangeWindowMode(TRUE); // ウィンドウモードで起動
    if (DxLib_Init() == -1) return -1; // ライブラリ初期化 エラーが起きたら終了
    SetBackgroundColor(0, 0, 0); // 背景色の指定
    SetDrawScreen(DX_SCREEN_BACK); // 描画面を裏画面にする

    int bgY = 0; // 道路をスクロールさせるための変数
    int imgBG = LoadGraph(backgroudImage); // 背景の画像

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
    playerCar.fuel = 0;
    playerCar.shieldFlag = 0;

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
    FUEL_ITEM fuelItem;
    fuelItem.x = rand() % 180 + 270;
    fuelItem.y = -100;
    fuelItem.graphHandle = LoadGraph(fuelImage);

    // 防御アイテム用の変数
    DEFENSIVE_ITEM defensiveItem;
    defensiveItem.x = rand() % 180 + 270;
    defensiveItem.y = -100;
    defensiveItem.graphHandle = LoadGraph(defensiveImage);

    // ゲーム進行に関する変数
    int scene = E_GAME_SCENE_TITLE;
    int timer = 0;

    // サウンドの読み込みと音量設定
    for (int i = 0; i < E_AUDIO_NUM; i++)
    {
        soundHandles[i] = LoadSoundMem(audioFiles[i].filePath);
    }
    ChangeVolumeSoundMem(128, soundHandles[E_AUDIO_MAIN]);
    ChangeVolumeSoundMem(128, soundHandles[E_AUDIO_OVER]);

    while (1) // メインループ
    {
        ClearDrawScreen(); // 画面をクリアする

        // 背景のスクロール処理
        if (scene == E_GAME_SCENE_PLAY) bgY = (bgY + 10) % HEIGHT; // プレイ中にだけスクロール
        DrawGraph(0, bgY - HEIGHT, imgBG, FALSE);
        DrawGraph(0, bgY, imgBG, FALSE);

        // プレイヤーの車を動かす処理
        if (scene == E_GAME_SCENE_PLAY) // プレイ中にだけ動かす
        {
            GetMousePoint(&playerCar.x, &playerCar.y);
            if (playerCar.x < 260) playerCar.x = 260;
            if (playerCar.x > 460) playerCar.x = 460;
            if (playerCar.y < 40) playerCar.y = 40;
            if (playerCar.y > 600) playerCar.y = 600;
        }
        drawPlayerCar(playerCar, carGraphHandles[playerCar.carType]);

        // コンピューターの車を動かす処理
        for (int i = 0; i < COM_MAX; i++)
        {
            if (scene == E_GAME_SCENE_PLAY) // プレイ中の車の処理
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
                    if (playerCar.shieldFlag == 0) { // シールドがない場合
                        int col = GetColor(rand() % 256, rand() % 256, rand() % 256); // 重ねる色
                        SetDrawBlendMode(DX_BLENDMODE_ADD, 255); // 色を加算する設定
                        DrawBox(playerCar.x - playerCar.width / 2, playerCar.y - playerCar.height / 2, playerCar.x + playerCar.width / 2, playerCar.y + playerCar.height / 2, col, TRUE);
                        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // 通常の描画に戻す
                        PlaySoundMem(soundHandles[E_AUDIO_CRASH], DX_PLAYTYPE_BACK); // 効果音
                        playerCar.fuel -= 10;
                    } else { // シールドがある場合
                        PlaySoundMem(soundHandles[E_AUDIO_SHIELD], DX_PLAYTYPE_BACK); // 効果音
                        playerCar.shieldFlag = 0;
                    }
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
            drawComputerCar(computerCars[i], carGraphHandles[computerCars[i].carType]);
        }

        // 燃料アイテムの処理
        if (scene == E_GAME_SCENE_PLAY) // ゲーム中だけ出現
        {
            fuelItem.y += 6;
            if (fuelItem.y > HEIGHT) fuelItem.y = -100;
            if (abs(fuelItem.x - playerCar.x) < playerCar.width / 2 + 12 && abs(fuelItem.y - playerCar.y) < playerCar.height / 2 + 12)
            {
                fuelItem.x = rand() % 180 + 270;
                fuelItem.y = -500;
                playerCar.fuel += 200;
                PlaySoundMem(soundHandles[E_AUDIO_GET_FUEL], DX_PLAYTYPE_BACK); // 効果音
            }
            DrawGraph(fuelItem.x - 12, fuelItem.y - 12, fuelItem.graphHandle, TRUE);
        }

        // 防御アイテムの処理
        if (scene == E_GAME_SCENE_PLAY) // ゲーム中だけ出現
        {
            defensiveItem.y += 6;
            if (defensiveItem.y > HEIGHT) defensiveItem.y = -100;
            if (abs(defensiveItem.x - playerCar.x) < playerCar.width / 2 + 12 && abs(defensiveItem.y - playerCar.y) < playerCar.height / 2 + 12)
            {
                defensiveItem.x = rand() % 180 + 270;
                defensiveItem.y = -500;
                playerCar.shieldFlag = 1;
                PlaySoundMem(soundHandles[E_AUDIO_GET_FUEL], DX_PLAYTYPE_BACK); // 効果音
            }
            DrawGraph(defensiveItem.x - 12, defensiveItem.y - 12, defensiveItem.graphHandle, TRUE);
        }

        timer++; // タイマーをカウント
        switch (scene) // タイトル、ゲームプレイ、ゲームオーバーの分岐
        {
        case E_GAME_SCENE_TITLE: // タイトル画面の処理
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
                fuelItem.x = WIDTH / 2;
                fuelItem.y = -100;
                score = 0;
                playerCar.fuel = 1000;
                scene = E_GAME_SCENE_PLAY;
                PlaySoundMem(soundHandles[E_AUDIO_MAIN], DX_PLAYTYPE_LOOP); // ＢＧＭをループ再生
            }
            break;

        case E_GAME_SCENE_PLAY: // ゲームをプレイする処理
            playerCar.fuel -= 1;
            if (playerCar.fuel < 0)
            {
                playerCar.fuel = 0;
                scene = E_GAME_SCENE_OVER;
                timer = 0;
                StopSoundMem(soundHandles[E_AUDIO_MAIN]); // ＢＧＭを停止
                PlaySoundMem(soundHandles[E_AUDIO_OVER], DX_PLAYTYPE_BACK); // ジングルを出力
            }
            break;

        case E_GAME_SCENE_OVER: // ゲームオーバーの処理
            drawText(180, 240, 0xff0000, "GAME OVER", 0, 80);
            if (timer > 60 * 5) scene = E_GAME_SCENE_TITLE;
            break;
        }

        // スコアなどの表示
        drawText(10, 10, 0x00ffff, "SCORE %d", score, 30);
        drawText(WIDTH - 200, 10, 0xffff00, "HI-SC %d", highScore, 30);
        int col = 0x00ff00; // 燃料の値を表示する色
        if (playerCar.fuel < 400) col = 0xffc000;
        if (playerCar.fuel < 200) col = 0xff0000;
        drawText(10, HEIGHT - 40, col, "FUEL %d", playerCar.fuel, 30);

        ScreenFlip(); // 裏画面の内容を表画面に反映させる
        WaitTimer(16); // 一定時間待つ
        if (ProcessMessage() == -1) break; // Windowsから情報を受け取りエラーが起きたら終了
        if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break; // ESCキーが押されたら終了
    }

    DxLib_End(); // ＤＸライブラリ使用の終了処理
    return 0; // ソフトの終了
}