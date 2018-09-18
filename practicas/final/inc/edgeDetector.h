extern QueueHandle_t queueTasksToLedUpdate;
extern QueueHandle_t queueTasksToLogger;
extern QueueHandle_t queueDebounceTecToEdgeDetector;

typedef struct {
	TickType_t 		edgeTickTime;
	uint8_t 		tecla;
	uint8_t 		flancoTecla;
} messageToEdgeDetector_t;


void edgeDetectorUpdate(void* taskParmPtr);


