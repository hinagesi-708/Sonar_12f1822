#define SDA(x)		x ## A2
#define SCL(x)		x ## A1

#define I2C_ADDR_DEV1    13

signed char reseaved_data[8] = {0};
unsigned char send_data[8] = {0};

void I2C_init();
void set_send_string(signed char Str[8]);
