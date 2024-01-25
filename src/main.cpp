/*
   -- New project --

   This source code of graphical user interface
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.11 or later version
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/
     - for ANDROID 4.11.4 or later version;
     - for iOS 1.9.1 or later version;

   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// определение режима соединения и подключение библиотеки RemoteXY
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <WiFi.h>

// Battery sense lib
#include <Battery.h>

// Battery level reading - IO34
Battery battery(5400, 9000, 34);

#include <RemoteXY.h>

#define LED 16
#define MOTOR_A_FW 18
#define MOTOR_A_RW 19
#define BUZZER 4

// PWM channels
#define MOTOR_A_FW_CH 0
#define MOTOR_A_RW_CH 1
#define BUZZER_CH 3

// ESP32 PWM config
// Motors:
#define PWM_FREQ 1000 // PWM frequency of 1 KHz
#define PWM_RES  8    // 8-bit resolution, 256 possible values
// Buzzer:
#define BUZZER_FREQ 500
#define BUZZER_RES 8

// настройки соединения
#define REMOTEXY_BLUETOOTH_NAME ble_device()

// конфигурация интерфейса
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = // 101 bytes
    {255, 4, 0, 1, 0, 94, 0, 16, 8, 1, 4, 0, 48, 4, 10, 89, 2, 16, 10, 121,
     6, 60, 14, 14, 4, 26, 31, 76, 0, 31, 1, 9, 28, 60, 15, 14, 2, 31, 72, 0,
     2, 0, 12, 82, 22, 11, 2, 26, 31, 31, 70, 87, 0, 82, 0, 71, 56, 6, 12, 37,
     37, 9, 2, 30, 135, 0, 0, 0, 0, 0, 0, 200, 66, 0, 0, 160, 65, 0, 0, 32,
     65, 0, 0, 0, 64, 31, 66, 65, 84, 32, 37, 0, 49, 0, 0, 0, 0, 0, 0, 160,
     65};

// структура определяет все переменные и события вашего интерфейса управления
struct
{
    // input variables
    int8_t speed;      // =0..100 положение слайдера
    uint8_t light;     // =1 если включено, иначе =0
    uint8_t horn;      // =1 если кнопка нажата, иначе =0
    uint8_t direction; // =1 если переключатель включен и =0 если отключен

    // output variables
    int8_t bat; // oт 0 до 100

    // other variable
    uint8_t connect_flag; // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

// Device state, to detect changes
struct {
    int8_t speed;
    uint8_t direction;
} state;

static char ble_device_name[16];

// Return device name as "Train-XX:YY", where XX:YY is last digits of MAC
const char *ble_device()
{
    snprintf(ble_device_name, 16, "Train-%s", WiFi.macAddress().c_str() + 12);
    return ble_device_name;
}

void reset_device()
{
    RemoteXY.light = 0;
    RemoteXY.horn = 0;
    RemoteXY.speed = 0;
    RemoteXY.direction = 1;

    state.speed = 0;
    state.direction = 1;

    digitalWrite(LED, LOW);
}

void setup()
{
    RemoteXY_Init();

    Serial.begin(115200);

    analogReadResolution(10);
    analogSetWidth(10);

    // Battery level
    float ratio = (16.2 + 9.3) / 9.3;
    battery.begin(3300, ratio);

    // TODO you setup code
    pinMode(LED, OUTPUT);

    // Prepare PWM output
    pinMode(MOTOR_A_FW, OUTPUT);
    pinMode(MOTOR_A_RW, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    // ESP32 API: Setup PWM mode
    ledcSetup(MOTOR_A_FW_CH, PWM_FREQ, PWM_RES);
    ledcSetup(MOTOR_A_RW_CH, PWM_FREQ, PWM_RES);
    ledcSetup(BUZZER_CH, BUZZER_FREQ, BUZZER_RES);
    // ESP32 API: Attach PWM to pins
    ledcAttachPin(MOTOR_A_FW, MOTOR_A_FW_CH);
    ledcAttachPin(MOTOR_A_RW, MOTOR_A_RW_CH);
    ledcAttachPin(BUZZER, BUZZER_CH);
    // Reset settings to defauls
    reset_device();
}

void loop()
{
    RemoteXY_Handler();

    // TODO you loop code
    // используйте структуру RemoteXY для передачи данных
    // не используйте функцию delay(), вместо нее используйте RemoteXY_delay()

    RemoteXY.bat = battery.level();
    digitalWrite(LED, RemoteXY.light);
    ledcWrite(BUZZER_CH, RemoteXY.horn ? 127 : 0);

    if (state.direction != RemoteXY.direction || state.speed != RemoteXY.speed) {
        long speed = map(RemoteXY.speed, 0, 100, 0, 255);
        if (RemoteXY.direction) {
            ledcWrite(MOTOR_A_FW_CH, speed);
            ledcWrite(MOTOR_A_RW_CH, 0);
        } else {
            ledcWrite(MOTOR_A_FW_CH, 0);
            ledcWrite(MOTOR_A_RW_CH, speed);
        }
        state.direction = RemoteXY.direction;
        state.speed = RemoteXY.speed;
    }

    Serial.print("Battery voltage is ");
    Serial.print(battery.voltage());
    Serial.print("mV (");
    Serial.print(battery.level());
    Serial.println("%)");
}
