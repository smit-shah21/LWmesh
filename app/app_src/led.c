#include "led.h"
#if defined(__PICC18__)
#include "circular_buffer.h"
#include "mcc.h"
#else
#include "system.h"
#include "pin_manager.h"
#endif
#include "Timers.h"
#define led_event_queue_depth       32
   
extern const uint8_t FirmwareVersionMajor;
extern const uint8_t FirmwareVersionMinor;

struct ledEvent
    {
        uint8_t LEDCondition;
        uint16_t eventDuration;
    };

struct ledEvent ledqueue[led_event_queue_depth]; 
uint16_t ledTimerVar = 0;
uint8_t ledStateVar;
    
CircularBufferContext ledtasksbuf;


void ledInit(void)
{
    
    //Initialize the led task queue
    CircularBufferInit(&ledtasksbuf,&ledqueue,
            sizeof(ledqueue),sizeof(struct ledEvent));
    
    //Now load the version blink activity on the queue
    //Initial off period
    (void)queueLedEvent(0,2000);
    
    //Blink major
    for(uint8_t i = 0; i < FirmwareVersionMajor; i++)
    {
        (void)queueLedEvent(1,250);
        (void)queueLedEvent(0,250);
    }
    //Gap between major and minor
    (void)queueLedEvent(0,1000);
    //Blink minor
    for(uint8_t i = 0; i < FirmwareVersionMinor; i++)
    {
        (void)queueLedEvent(1,250);
        (void)queueLedEvent(0,250);
    }
    //Gap between minor and led always on
    (void)queueLedEvent(0,2000);
    (void)queueLedEvent(1,100);
}

int8_t queueLedEvent(uint8_t LEDState,uint16_t duration)
{   
    int8_t ret_var;
    if(0 != CircularBufferSpace(&ledtasksbuf))
    {
        struct ledEvent temp;
        temp.LEDCondition = LEDState;
        temp.eventDuration = duration;
        CircularBufferPushBack(&ledtasksbuf,&temp);
        ret_var = 0;
    }
    else
    {
        ret_var = -1;
    }
    return ret_var;
}

void handle_led_events(void)
{
    //Call every loop to find out if LED event has to be handled
    if(!get_timer0base(&ledtimer)){
        //No active led event. Look for new event
        if(!CircularBufferEmpty(&ledtasksbuf)){
            //New event to handle. Load the event
            struct ledEvent temp;
            CircularBufferPopFront(&ledtasksbuf,&temp);
            //Check if it was a zero time event
            if(0u == temp.eventDuration){
                //Turn LED on and exit
                LED_SetLow();
            }
            else {
                if (0u == temp.LEDCondition){
                    LED_SetHigh();
                }
                else{
                    LED_SetLow();
                }
                set_timer0base(&ledtimer, temp.eventDuration);
            }
        }
        else{
            LED_SetLow();
        }
    }
}


void queue_serial_led_event(void)
{
    (void)queueLedEvent(0,400);
    for(uint8_t i = 0; i < 2u; i++)
    {
        (void)queueLedEvent(1,100);
        (void)queueLedEvent(0,100);
    }
    (void)queueLedEvent(0,100);
    (void)queueLedEvent(1,100);
}

void queue_tx_led_event(void)
{    
    (void)queueLedEvent(0,400);
    for(int i = 0; i < 2; i++)
    {
        (void)queueLedEvent(1,100);
        (void)queueLedEvent(0,100);
    }
    (void)queueLedEvent(1,300);
    (void)queueLedEvent(0,300);
    (void)queueLedEvent(1,100);
}

void queue_rx_led_event(void)
{
    (void)queueLedEvent(0,400);
    for(int i = 0; i < 2; i++)
    {
        (void)queueLedEvent(1,300);
        (void)queueLedEvent(0,300);
    }
    (void)queueLedEvent(1,100);
    (void)queueLedEvent(0,300);
    (void)queueLedEvent(1,100);
}
