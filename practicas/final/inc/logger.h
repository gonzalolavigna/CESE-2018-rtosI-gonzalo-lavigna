//MENSAJE de LOG entre las distintas tareas
extern QueueHandle_t queueTasksToLogger;

#define MAX_MESSAGE_LENGTH 256

typedef enum {TECLA_TASK = 0, IRQs, LED_TASK, EDGE_DETECTOR_TASK} taskOrigin_t;

typedef struct {
    taskOrigin_t origin;
    uint8_t messageToPrint[MAX_MESSAGE_LENGTH];
    TickType_t messageTick;
} messageToLogger_t;

void initLogger(void);
void logger(void* taskParmPtr);
void sendMessageToLogger(taskOrigin_t origin, uint8_t * messageToPrint);
