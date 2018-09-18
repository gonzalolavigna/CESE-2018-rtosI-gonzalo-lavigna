extern QueueHandle_t queueIRQToDebounceTec;
extern QueueHandle_t queueTasksToLogger;
extern QueueHandle_t queueTasksToLedUpdate;
extern QueueHandle_t queueDebounceTecToEdgeDetector;


typedef enum {BUTTON_DOWN = 0,  BUTTON_UP , BUTTON_FALLING, BUTTON_RAISING } buttonState_t;
typedef enum {TEC1_INDICE = 0,  TEC2_INDICE} teclaIndex_t;

//MENSAJE entre las ISR y la tarea del debouncer de las teclas.
typedef struct {
	TickType_t 		irqTickTime;
	teclaIndex_t	tecla;
	uint8_t 		flancoTecla;
} messageQueueIRQToDebounceTec_t;


void debounceTec(void* taskParmPtr);
void debounceTecInit(void);

