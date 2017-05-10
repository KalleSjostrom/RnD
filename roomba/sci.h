#pragma once

#define DEFAULT_PORT 12347

enum Sensor {
	Sensor_Bumpwheeldrops = 0,
	Sensor_Wall,
	Sensor_CliffLeft,
	Sensor_CliffFrontLeft,
	Sensor_CliffFrontRight,
	Sensor_CliffRight,
	Sensor_VirtualWall,
	Sensor_MotorOvercurrents,
	Sensor_DirtDetectorLeft,
	Sensor_DirtDetectorRight,
	Sensor_RemoteOpcode,
	Sensor_Buttons,
	Sensor_DistanceMSB,
	Sensor_DistanceLSB,
	Sensor_AngleMSB,
	Sensor_AngleLSB,
	Sensor_ChargingState,
	Sensor_VoltageMSB,
	Sensor_VoltageLSB,
	Sensor_CurrentMSB,
	Sensor_CurrentLSB,
	Sensor_Temperature,
	Sensor_ChargeMSB,
	Sensor_ChargeLSB,
	Sensor_CapacityMSB,
	Sensor_CapacityLSB,

	Sensor_Count
};

//Battery Charging States
#define CHARGING_STATE_NO_CHARGING          0
#define CHARGING_STATE_CHARGING_RECOVERY    1
#define CHARGING_STATE_CHARGING             2
#define CHARGING_STATE_TRICKLE_CHAGING      3
#define CHARGING_STATE_WAITING              4
#define CHARGING_STATE_CHARGING_ERROR       5

//Commands
#define COMMAND_SAFE    131
#define COMMAND_FULL    132
#define COMMAND_POWER   133
#define COMMAND_SPOT    134
#define COMMAND_CLEAN   135
#define COMMAND_MAX     136
#define COMMAND_DRIVE   137
#define COMMAND_MOTORS  138
#define COMMAND_LEDS    139
#define COMMAND_SONG    140
#define COMMAND_PLAY    141
#define COMMAND_SENSORS 142
#define COMMAND_DOCK    143

//Number of parameters of Led commands
#define LEDS_NUM_PARAMETERS         3

//Song Notes

//Note duration
#define NOTE_DURATION_SIXTEENTH_NOTE    16
#define NOTE_DURATION_EIGHTH_NOTE       32
#define NOTE_DURATION_QUARTER_NOTE      64

//Led Control MASKS
#define LED_CLEAN_ON                0x04
#define LED_CLEAN_OFF               0xFB
#define LED_SPOT_ON                 0x08
#define LED_SPOT_OFF                0xF7
#define LED_DIRT_ON                 0x01
#define LED_DIRT_OFF                0xFE
#define LED_MAX_ON                  0x02
#define LED_MAX_OFF                 0xFD
#define LED_STATUS_OFF              0x0F
#define LED_STATUS_AMBAR            0x30
#define LED_STATUS_RED              0x10
#define LED_STATUS_GREEN            0x20

//Cleaning Motors Control MASKS
#define SIDE_BRUSH_ON               0x01
#define SIDE_BRUSH_OFF              0xFE
#define VACUUM_ON                   0x02
#define VACUUM_OFF                  0xFD
#define MAIN_BRUSH_ON               0x04
#define MAIN_BRUSH_OFF              0xFB
#define ALL_CLEANING_MOTORS_ON      0xFF
#define ALL_CLEANING_MOTORS_OFF     0x00
