extern QueueHandle_t queueIRQToDebounceTec;
extern QueueHandle_t queueTasksToLogger;
extern QueueHandle_t queueTasksToLedUpdate;


//MENSAJE entre las ISR y la tarea de las teclas.


typedef enum {BUTTON_DOWN = 0,  BUTTON_UP , BUTTON_FALLING, BUTTON_RAISING } buttonState_t;
typedef enum {TEC1_INDICE = 0,  TEC2_INDICE} teclaIndex_t;

typedef struct {
	TickType_t 		irqTickTime;
	teclaIndex_t	tecla;
	uint8_t 		flancoTecla;
} messageQueueIRQToDebounceTec_t;


void debounceTec(void* taskParmPtr);
void debounceTecInit(void);

