
//Esta Cola se encuentra definida e inicializada en el main
extern QueueHandle_t queueIRQToDebounceTec;
extern QueueHandle_t queueTasksToLogger;

#define RISING_EDGE 0
#define FALLING_EDGE 1

void initIRQ(void);


