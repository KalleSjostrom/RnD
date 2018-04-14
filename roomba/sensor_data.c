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

typedef struct {
	uint8_t Bumpwheeldrops;
	uint8_t Wall;
	uint8_t CliffLeft;
	uint8_t CliffFrontLeft;
	uint8_t CliffFrontRight;
	uint8_t CliffRight;
	uint8_t VirtualWall;
	uint8_t MotorOvercurrents;
	uint8_t DirtDetectorLeft;
	uint8_t DirtDetectorRight;
	uint8_t RemoteOpcode;
	uint8_t Buttons;
	uint8_t ChargingState;
	int8_t Temperature;

	int16_t distance;
	int16_t angle;
	int16_t voltage;
	int16_t current;
	float battery_level;
	float time;
} SensorData;

typedef struct {
	char *memory;
	uint64_t cursor;
} SensorStream;

void print_sensor_data(FILE *file, SensorData sensor_data) {
	fprintf(file, "Sensor data %g:\n", sensor_data.time);
	fprintf(file, "   Bumpwheeldrops: %d\n", sensor_data.Bumpwheeldrops);
	fprintf(file, "   Wall: %d\n", sensor_data.Wall);
	fprintf(file, "   CliffLeft: %d\n", sensor_data.CliffLeft);
	fprintf(file, "   CliffFrontLeft: %d\n", sensor_data.CliffFrontLeft);
	fprintf(file, "   CliffFrontRight: %d\n", sensor_data.CliffFrontRight);
	fprintf(file, "   CliffRight: %d\n", sensor_data.CliffRight);
	fprintf(file, "   VirtualWall: %d\n", sensor_data.VirtualWall);
	fprintf(file, "   MotorOvercurrents: %d\n", sensor_data.MotorOvercurrents);
	fprintf(file, "   DirtDetectorLeft: %d\n", sensor_data.DirtDetectorLeft);
	fprintf(file, "   DirtDetectorRight: %d\n", sensor_data.DirtDetectorRight);
	fprintf(file, "   RemoteOpcode: %d\n", sensor_data.RemoteOpcode);
	fprintf(file, "   Buttons: %d\n", sensor_data.Buttons);
	fprintf(file, "   ChargingState: %d\n", sensor_data.ChargingState);
	fprintf(file, "   Temperature: %d\n", sensor_data.Temperature);
	fprintf(file, "   distance: %d\n", sensor_data.distance);
	fprintf(file, "   angle: %d\n", sensor_data.angle);
	fprintf(file, "   voltage: %d\n", sensor_data.voltage);
	fprintf(file, "   current: %d\n", sensor_data.current);
	fprintf(file, "   battery_level: %g\n", sensor_data.battery_level);
	fprintf(file, "\n");
}