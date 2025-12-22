#include "sleep.h"


void sleep_ms(int milliseconds){
    sleep(milliseconds / 1000); /* Linux takes seconds, Windows expects milliseconds. Make abstraction (utility) */
}