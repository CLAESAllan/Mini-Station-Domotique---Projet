//Library
#include <Arduino.h>

//Definition of variables
#define LED1_GPIO 25
#define LED2_GPIO 26
#define BUTTON_GPIO 27

static SemaphoreHandle_t hsem; //Waits parameter to be read

//Variables used
bool tache1;
bool tache2;

//Variables used for Toggle button
int valeurBP;
int valeurBP_apres;

//Task Button
void button_task(void *argp){ 
 int start = 0; //used so that nothing lights up when the program starts
 BaseType_t rc;
 for(;;){
 valeurBP = digitalRead(BUTTON_GPIO); //Read the status of the button
 if (valeurBP and valeurBP != valeurBP_apres and start == 1) {
 rc = xSemaphoreGive(hsem); //Give the semaphore to the next task
 assert(rc == pdPASS); //Allow tache_LED1 to start first
 }
 valeurBP_apres = valeurBP; //Updating the value of the button

 start = 1; //Exit from the initial state
 vTaskDelay(10); //Time limit for the task
 }
}

void tache_LED1(void*argp){
 BaseType_t rc;
 for (;;){
    // First gain control of hsem
    rc= xSemaphoreTake(hsem,portMAX_DELAY);
    assert(rc == pdPASS);
    tache1=true; // Execute tache1
    tache2=false; // Stop tache2
    while (tache1==true){ //If tache1 is true then we turn on LED1_GPIO with a delay of 500 ms
        digitalWrite(LED1_GPIO,digitalRead(LED1_GPIO)^1);
        vTaskDelay(500);
 }
 digitalWrite(LED1_GPIO,LOW); //Reset to LOW LED1_GPIO on task exit
 assert(rc == pdPASS);
 }
}

void tache_LED2(void*argp){
 BaseType_t rc;
 for (;;){
 // First gain control of hsem
 rc= xSemaphoreTake(hsem,portMAX_DELAY);
 assert(rc == pdPASS);
 tache2 = true; // Execute tache2
 tache1=false; // Stop tache2
 while(tache2==true){ //If tache2 is true then we turn on LED2_GPIO with a delay of 200 ms
 digitalWrite(LED2_GPIO,digitalRead(LED2_GPIO)^1);
 vTaskDelay(200);
 }
 digitalWrite(LED2_GPIO,LOW); //Reset to LOW LED1_GPIO on task exit
 assert(rc == pdPASS);
 }
}
void setup(){

 //Declares I/O
 pinMode(LED1_GPIO,OUTPUT);
 pinMode(LED2_GPIO,OUTPUT);
 digitalWrite(LED1_GPIO,LOW);
 digitalWrite(LED2_GPIO,LOW);
 pinMode(BUTTON_GPIO, INPUT);

 int app_cpu = xPortGetCoreID(); //Retrieve core ID
 BaseType_t rc; 
 hsem = xSemaphoreCreateBinary(); //Creates a new binary semaphore instance, handle by which the new semaphore can be referenced.
 assert(hsem);
 rc = xTaskCreatePinnedToCore(
    tache_LED1, // Function
    "led1task", //Task name
    3000, // Stack size
    NULL, // arg
    1, // Priority
    nullptr, // No handle returned
    app_cpu); // CPU
 assert(rc == pdPASS); // Allow tache1 to start first
 rc= xTaskCreatePinnedToCore(
    tache_LED2, // Function
    "led2task", //Task name
    3000, // Stack size
    NULL, // arg
    1, // Priority
    nullptr, // No handle returned
    app_cpu); // CPU
 xTaskCreatePinnedToCore(
    button_task, // Function
    "buttontask", //Task name
    3000, // Stack size
    NULL, // arg
    1, // Priority
    nullptr, // No handle returned
    app_cpu); // CPU
 assert(rc == pdPASS);
 }
// Not used
void loop(){
vTaskDelete(nullptr);
//Supprimer une tâche de la gestion des noyaux RTOS.
}