extern QueueHandle_t queueTasksToLedUpdate;
extern QueueHandle_t queueTasksToLogger;



#define DELAY_TASK_LED_UPDATE 10/portTICK_RATE_MS
#define NUMBER_OF_LEDS 3

typedef enum {LED1_INDICE = 0,  LED2_INDICE, LED3_INDICE} ledIndex_t;
typedef enum {LED_SPARKING = 0 , LED_NOT_SPARKING } ledState_t;


typedef struct {
	ledIndex_t ledToSpark;
    TickType_t ledSparkTime;
} messageToLedUpdate_t;


void initLed(void);
void ledUpdate(void* taskParmPtr);
