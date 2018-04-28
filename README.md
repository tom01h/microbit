# micro:bit で遊ぶ

## 開発環境
[yotta](http://docs.yottabuild.org/) を使っています。インストールは [ここ](https://www.iot-programmer.com/index.php/books/27-micro-bit-iot-in-c/chapters-micro-bit-iot-in-c/44-offline-c-c-development-with-the-micro-bit)を参考にしました。

インストールが出来たら下のような感じにプロジェクトを作成します。

```
mkdir sd_test
cd sd_test
yotta init
yotta target bbc-microbit-classic-gcc
yotta install lancaster-university/microbit
yotta build
```

init ではいろいろ聞かれるので、

```
At this point you will be asked a set of questions:
    1. Name of module - defaults to name of directory
    2. Initial version - defaults to 0.0.0
    3. Is this an executable? - Yes in this case, defaults to No
    4. short description - whatever you want to put
    5. author - whatever you want to put
    6. licence - your preferred licence you want, defaults to Apache 2.0
```


メモリの節約と radio 機能を使うために BLE 機能は OFF にします。  
yotta_modules/microbit-dal/inc/core/MicroBitConfig.h の中で、

```
#define MICROBIT_BLE_ENABLED                    0
```

## SD カードのテスト
sd_test/ 以下にあります。  
SPI 機能を使って SD カードにアクセスします。

## 無線接続のデータロガー
control/ と sensor/ 以下にあります。  
コントローラ用 micro:bit とセンサー用 micro:bit の間で無線通信を通してセンサーデータを収集し、コントローラ用 micro:bit につないだ SD カード(予定)にログを残します。
