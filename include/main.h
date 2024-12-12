#ifndef MAIN_H
#define MAIN_H

/**********************************************************
 * 型定義
***********************************************************/
typedef enum {
    CAR_TYPE_RED,
    CAR_TYPE_YELLOW,
    CAR_TYPE_BLUE,
    CAR_TYPE_TRUCK,
    CAR_TYPE_NUM // 車のタイプの数
} CAR_TYPE;

typedef enum {
    E_AUDIO_MAIN,
    E_AUDIO_OVER,
    E_AUDIO_GET_FUEL,
    E_AUDIO_CRASH,
    E_AUDIO_NUM // オーディオファイルの数
} E_AUDIO;

typedef enum { 
    E_GAME_SCENE_TITLE,
    E_GAME_SCENE_PLAY,
    E_GAME_SCENE_OVER 
} E_GAME_SCENE;

typedef struct {
    const char *filePath;
    int carType;
} CAR_IMAGE;

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

typedef struct {
    int x;
    int y;
    int graphHandle;
} FUEL_ITEM;

typedef struct {
    const char *filePath;
    int e_audio;
} AUDIO_FILE;

#endif
