typedef void (*accelerometer_callback_type)(float, float, float, void*);
void accelerometer_init(int hz, accelerometer_callback_type, void*);

