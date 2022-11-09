/*
  ============================================================
  Copyright (c) 2022 Christopher Connolly
  Project:     Halloween 2022
  Author:      Christopher Connolly
  Last update: 28.10.2022
  Description: ESP32 Halloween Fun
  License:     GNU GPL 3, see libraries for respective licenes too

  Dependencies & Kudos:
  https://github.com/pschatzmann/ESP32-A2DP
  https://github.com/arkhipenko/TaskScheduler
  ============================================================
*/

#include <Arduino.h>
#include <TaskScheduler.h>
// #include "BluetoothA2DPSource.h"
// #include "piano8bit.h"

// BluetoothA2DPSource a2dp_source;
// SoundData *data1 = new OneChannel8BitSoundData((int8_t *)piano8bit_raw, piano8bit_raw_len, false);
// SoundData *data2 = new OneChannel8BitSoundData((int8_t *)piano8bit_raw, piano8bit_raw_len, false);

// #define BLUETOOTH_DEVICE_NAME "JBL Clip 3"         // Output device name
// #define Y_BLUETOOTH_DEVICE_NAME "JBL Flip 4"       // Output device name
// #define X_BLUETOOTH_DEVICE_NAME "MetalliX Headset" // Output device name
#define LED 2                          // On-board LED
#define STEPPER_DIR_PIN 22             // Stepper direction pin
#define STEPPER_STEP_PIN 23            // Stepper jog pin
#define STEPPER_STEP_DELAY_MICROS 1000 // Pause between steps
#define DELAY_BEFORE_RISE 4000         // Delay between rising
#define DISTANCE_SENSOR_PIN 4          // Distance sensor interupt pin
#define LIMIT_UP_PIN 10                // Up limit
#define LIMIT_DOWN_PIN 11              // Up limit
#define AIR_DELAY_MICROS 100000        // Air on time (Microseconds)
#define AIR_DELAY_MIILLS 0             // Air on time (Milliseconds) (additive to above)
#define AIR_RELAY_PIN 32               // Air relay pin

const int STEPS_MOVEMENT = 5;  // Number of revolutions
const int STEPS_PER_REV = 200; // Number of steps per revolution
const int STEPS = STEPS_PER_REV * STEPS_MOVEMENT;

volatile boolean up_limit = false;
volatile boolean down_limit = false;
volatile boolean direction_up = true;

// *********************************************************************
// Task Prototypes
// *********************************************************************
// Callback methods prototypes
void performActivation();
void performIndicatorOn();
void performIndicatorOff();
void performSlideDown();
void performSlideUp();
void performMove();
void performAirPulse();
void performAirReset();
void performSoundPlay();
void performSoundReset();

// Task Scheduler
Scheduler runner;

// Tasks
Task taskActivate(TASK_IMMEDIATE, TASK_FOREVER, &performActivation);
Task taskIndicator(TASK_IMMEDIATE, TASK_FOREVER, &performIndicatorOn);
Task taskSlide(TASK_IMMEDIATE, TASK_FOREVER, &performSlideDown);
Task taskMove(TASK_IMMEDIATE, STEPS, &performMove);
Task taskAir(TASK_IMMEDIATE, TASK_FOREVER, &performAirPulse);
// Task taskSound(TASK_IMMEDIATE, TASK_FOREVER, &performSoundPlay);

void IRAM_ATTR distanceInterupt()
{
  // Serial.print("!");
  taskActivate.enableIfNot();
}

void IRAM_ATTR limitUpInterupt()
{
  up_limit = true;
}

void IRAM_ATTR limitDownInterupt()
{
  down_limit = true;
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nStarting ESP\n");
  Serial.println("Configuring IO...");

  // Configure IO - OUTPUT
  pinMode(LED, OUTPUT);
  pinMode(STEPPER_DIR_PIN, OUTPUT);
  pinMode(STEPPER_STEP_PIN, OUTPUT);
  pinMode(AIR_RELAY_PIN, OUTPUT);

  // Configure IO - INPUT
  pinMode(DISTANCE_SENSOR_PIN, INPUT);
  pinMode(LIMIT_UP_PIN, INPUT);
  pinMode(LIMIT_DOWN_PIN, INPUT);

  // Configure interupts
  Serial.println("Attaching interupts...");
  attachInterrupt(DISTANCE_SENSOR_PIN, distanceInterupt, CHANGE);
  attachInterrupt(LIMIT_UP_PIN, limitUpInterupt, HIGH);
  attachInterrupt(LIMIT_DOWN_PIN, limitDownInterupt, HIGH);

  Serial.println("Adding tasks to schduler...");
  runner.addTask(taskActivate);
  runner.addTask(taskIndicator);
  runner.addTask(taskSlide);
  runner.addTask(taskMove);
  runner.addTask(taskAir);
  // runner.addTask(taskSound);

  // Connect Bluetooth Audio
  // Serial.println("Configuring Bluetooth...");
  // a2dp_source.set_auto_reconnect(true);
  // a2dp_source.start(BLUETOOTH_DEVICE_NAME);
  // a2dp_source.set_volume(100);
  // a2dp_source.write_data(data2);
}

void loop()
{
  // delay(10);
  runner.execute(); // this function must called here, do not delete it
}

void performActivation()
{
  Serial.println("Activating...");

  // Enable tasks
  taskIndicator.enableIfNot();
  taskSlide.enableIfNot();
  taskAir.enableIfNot();
  // taskSound.enableIfNot();

  // Disable self
  taskActivate.disable();
}

void performIndicatorOn()
{
  Serial.println("Turning LED on");
  digitalWrite(LED, HIGH);
  taskIndicator.delay(2000);
  taskIndicator.setCallback(&performIndicatorOff);
}

void performIndicatorOff()
{
  Serial.println("Turning LED off");
  digitalWrite(LED, LOW);
  taskIndicator.disable();
  taskIndicator.setCallback(&performIndicatorOn);
}

void performSlideDown()
{
  Serial.println("Stepper slide down");
  direction_up = false;
  taskMove.setIterations(STEPS);
  taskMove.enable();
  taskSlide.delay(DELAY_BEFORE_RISE);
  taskSlide.setCallback(&performSlideUp);
}

void performSlideUp()
{
  Serial.println("Stepper slide up");
  direction_up = true;
  taskMove.setIterations(STEPS);
  taskMove.enable();
  taskSlide.setCallback(&performSlideDown);
  taskSlide.disable();
}

void performMove()
{
  if (taskMove.isFirstIteration())
  {
    Serial.println("Starting moving task");
  }

  // if (up_limit == true || down_limit == true)
  //   return;
  // Serial.print("*");
  digitalWrite(STEPPER_DIR_PIN, direction_up);
  digitalWrite(STEPPER_STEP_PIN, HIGH);
  delayMicroseconds(STEPPER_STEP_DELAY_MICROS);
  digitalWrite(STEPPER_STEP_PIN, LOW);
  delayMicroseconds(STEPPER_STEP_DELAY_MICROS);

  if (taskMove.isLastIteration())
  {
    Serial.println("Finished moving task");
  }
}

void performAirPulse()
{
  Serial.println("Triggering AIR");
  digitalWrite(AIR_RELAY_PIN, HIGH);
  delayMicroseconds(AIR_DELAY_MICROS);
  delay(AIR_DELAY_MIILLS);
  digitalWrite(AIR_RELAY_PIN, LOW);
  taskAir.delay(2000);
  taskAir.setCallback(&performAirReset);
}

void performAirReset()
{
  Serial.println("Reset AIR task");
  digitalWrite(AIR_RELAY_PIN, LOW);
  taskAir.disable();
  taskAir.setCallback(&performAirPulse);
}

// void performSoundPlay()
// {
//   Serial.println("Triggering Sound");
//   a2dp_source.write_data(data2);
//   taskSound.delay(6000);
//   taskSound.setCallback(&performSoundReset);
// }

// void performSoundReset()
// {
//   Serial.println("Sound complete");
//   taskSound.disable();
//   taskSound.setCallback(&performSoundPlay);
// }