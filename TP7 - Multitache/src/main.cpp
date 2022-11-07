#include <Arduino.h>

#define LED1_GPIO 25
#define LED2_GPIO 26
#define BUTTON_GPIO 27

static SemaphoreHandle_t hsem; 
bool tache1;
bool tache2;

int valeurBP;
int valeurBP_apres;

void button_task(void *argp){ 
 int start = 0;
 BaseType_t rc;
 for(;;){
 valeurBP = digitalRead(BUTTON_GPIO); 
 if (valeurBP and valeurBP != valeurBP_apres and start == 1) {
 rc = xSemaphoreGive(hsem);
 assert(rc == pdPASS); 
 }
 valeurBP_apres = valeurBP;

 start = 1; 
 vTaskDelay(10);
 }
}
void tache_LED1(void*argp){
 BaseType_t rc;
 for (;;){
    rc= xSemaphoreTake(hsem,portMAX_DELAY);
    assert(rc == pdPASS);
    tache1=true; 
    tache2=false; 
    while (tache1==true){
        digitalWrite(LED1_GPIO,digitalRead(LED1_GPIO)^1);
        vTaskDelay(500);
 }
 digitalWrite(LED1_GPIO,LOW);
 assert(rc == pdPASS);
 }
}
void tache_LED2(void*argp){
 BaseType_t rc;
 for (;;){
 rc= xSemaphoreTake(hsem,portMAX_DELAY);
 assert(rc == pdPASS);
 tache2 = true;
 tache1=false; 
 while(tache2==true){
 digitalWrite(LED2_GPIO,digitalRead(LED2_GPIO)^1);
 vTaskDelay(200);
 }
 digitalWrite(LED2_GPIO,LOW);
 assert(rc == pdPASS);
 }
}
void setup(){

 pinMode(LED1_GPIO,OUTPUT);
 pinMode(LED2_GPIO,OUTPUT);
 digitalWrite(LED1_GPIO,LOW);
 digitalWrite(LED2_GPIO,LOW);
 pinMode(BUTTON_GPIO, INPUT);

 int app_cpu = xPortGetCoreID(); 
 BaseType_t rc; 
 hsem = xSemaphoreCreateBinary();
 assert(hsem);
 rc = xTaskCreatePinnedToCore(tache_LED1, "led1task", 3000, NULL, 1,nullptr, app_cpu);
 assert(rc == pdPASS);
 rc= xTaskCreatePinnedToCore(tache_LED2, "led2task", 3000, NULL, 1, nullptr, app_cpu); 
 xTaskCreatePinnedToCore(button_task,"buttontask",3000, NULL,1, nullptr,app_cpu);
 assert(rc == pdPASS);
 }
// Not used
void loop()
{
vTaskDelete(nullptr);
//Supprimer une tâche de la gestion des noyaux RTOS.
}
