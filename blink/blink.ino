#include <avr/interrupt.h>
#include <avr/sleep.h>

// Timer counter (TCNT1) increments every 256th clock cycle with prescaler bit CS12
// Clock speed of ATmega328P is 16MHz
// 16MHz / 256 = 62 500Hz
// So the counter is incremented 62 500 times per second
// ISR is set to execute once timer counter reaches the value of OCR1A
// So 62 500 / X is the number of ISR exections per second

#define DEBUG
#define times_per_second(x) 62500/x

void setup() {
    // Output compare pins off, WGM10 and WGM11 to 0 for CTC mode
    TCCR1A = 0;
    // Prescaler 256, WGM12 to 1 and WGM13 to 0 for CTC mode
    TCCR1B = (1 << CS12) | (1 << WGM12);
    // Enable interrupt for Timer1 output compare A
    TIMSK1 = (1 << OCIE1A);
    // Set compare register to execute interrupt x times per second
    OCR1A = times_per_second(1);

    set_sleep_mode(SLEEP_MODE_IDLE);

    // Enable interrupts
    sei();

    // Built-in LED to output
    DDRB |= (1 << DD5);

    #ifdef DEBUG
    Serial.begin(9600);
    #endif
}

void loop() {
    sleep_mode();
}

ISR(TIMER1_COMPA_vect) {
    // Toggle built-in LED
    PORTB ^= (1 << PB5);

    #ifdef DEBUG
    Serial.println(millis());
    #endif
}
