volatile int digit = 0;

#define SECOND 62500

void setup() {
    // Set PB0-PB3 to output
    DDRB = 15;

    // Enable INT0 on falling edge
    EICRA = (1 << ISC01);
    // Enable interrupt for INT0
    EIMSK = (1 << INT0);
    // PD2 to input
    DDRD &= ~(1 << DDD2);
    PORTD |= (1 << PORTD2);

    // CTC mode, pre-scaler at 256
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12);
    // Compare register A value
    OCR1A = SECOND/10;
    // Enable compare A interrupt for timer 1
    TIMSK1 = (1 << OCIE1A);

    sei();
    Serial.begin(9600);
}

void loop() {
    PORTB = digit;
}

ISR(INT0_vect) {
    digit = 0;
    // Reset timer counter
    TCNT1 = 0;
    // Clear timer interrupt flag
    TIFR1 |= (1 << OCF1A);
}

ISR(TIMER1_COMPA_vect) {
    if (++digit == 10)
        digit = 0;
}
