#include "silentdriver.h"

static void driver_free(Driver* driver);
static int driver_init(Driver* driver);
static void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback);
static int driver_open(Driver* driver);
static int driver_close(Driver* driver);
static int driver_dispose(Driver* driver);

Driver* create_silent_driver(void)
{
	Driver* driver = malloc(sizeof(Driver));
	memset(driver, 0, sizeof(Driver));
	driver->open = driver_open;
	driver->free = driver_free;
	driver->init = driver_init;
	driver->connect = driver_connect;
	driver->open = driver_open;
	driver->close = driver_close;
	driver->dispose = driver_dispose;
	return driver;
}

void driver_free(Driver* driver)
{
	free(driver);
}

int driver_init(Driver* driver)
{
	return 0;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback)
{
}

int driver_open(Driver* driver)
{
	return 0;
}

int driver_close(Driver* driver)
{
	return 0;
}

int driver_dispose(Driver* driver)
{
	return 0;
}
