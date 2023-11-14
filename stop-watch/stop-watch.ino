#include <avr/interrupt.h>

#define TENTH_OF_A_SECOND 6250

enum State {
    IDLE,
    START,
    PRINT_TIME,
    STOP
};

using Time = struct {
    uint8_t minutes;
    uint8_t seconds;
    uint8_t millis;
};

volatile uint64_t timerTicks = 0;
volatile State state = 0;

// Having a simple global state for the time
// where only the ticks are tracked and then
// deriving the desired output from it when
// required is preferrable to tracking minutes,
// seconds and milliseconds separately. It requires
// some more computation, but is much less error prone
// and cleaner.
void makeTime(uint64_t ticks, Time* const time) {
    uint8_t minutes = ticks / 600;
    ticks %= 600;
    uint8_t seconds = ticks / 10;
    ticks %= 10;

    *time = { minutes, seconds, ticks };
}

String timeToString(const Time* const time, bool showMilliseconds) {
    // Helper function to pad time units with zeros in case they're only one digit long
    static auto formatTimeUnit = [](uint8_t timeUnit) {
        return timeUnit < 10 ?
                   String(0) + String(timeUnit) :
                   String(timeUnit);
    };

    String timeString = formatTimeUnit(time->minutes) +
                        ":" +
                        formatTimeUnit(time->seconds);

    if (showMilliseconds)
        timeString += String(".") +
                      String(time->millis) +
                      String("00");

    return timeString;
}

void printTime(uint64_t ticks, bool showMilliseconds = false) {
    static Time time;
    makeTime(ticks, &time);

    Serial.println(timeToString(&time, showMilliseconds));
}

void setup() {
    // Crear TCCR1A for no pin compares
    // WGM10 and WGM11 to 0 for compare mode
    TCCR1A = 0;
    // Set WGM12 for compare mode
    TCCR1B = (1 << WGM12);
    // Enable compare interrupts
    TIMSK1 = (1 << OCIE1A);
    // Compare register to a value
    // that triggers an interrupt every 100ms with pre-scaler at 256
    OCR1A = TENTH_OF_A_SECOND;

    // Falling edge trigger for pin 2 and pin 3
    EICRA = (1 << ISC01) | (1 << ISC11);
    // Enable both interrupts
    EIMSK = (1 << INT0) | (1 << INT1);

    // Set pins 2 and 3 to input
    DDRD &= ~((1 << DDD2) | (1 << DDD3));
    // Enable pull-up resistors
    PORTD = (1 << PORTD2) | (1 << PORTD3);

    sei();
    Serial.begin(9600);
}

void loop() {
    switch (state) {
    case IDLE:
        break;
    case START:
        Serial.println("Timer started.");
        state = IDLE;
        break;
    case PRINT_TIME:
        printTime(timerTicks);
        state = IDLE;
        break;
    case STOP:
        Serial.println("Timer stopped at:");
        printTime(timerTicks, true);
        timerTicks = 0;
        state = IDLE;
        break;
    default:
        ;
    }

}

ISR(TIMER1_COMPA_vect) {
    // Counts up to 10 for every second
    static volatile uint8_t secondCounter = 0;

    // Tick counter was reset, also reset secondCounter
    if (timerTicks == 0)
        secondCounter = 0;

    timerTicks++;

    if (++secondCounter == 10) {
        secondCounter = 0;
        state = PRINT_TIME;
    }
}

// Start timer with pin 2
ISR(INT0_vect) {
    // Exit early if timer is already running
    if (TCCR1B & (1 << CS12))
        return;

    // Start timer
    TCCR1B |= (1 << CS12);
    // Clear pin 3 interrupt flag
    EIFR |= (1 << INTF1);
    // Enable pin 3 interrupts again
    EIMSK |= (1 << INT1);

    state = START;
}

// Stop timer with pin 3
ISR(INT1_vect) {
    // Exit early if timer is not running
    if ( !(TCCR1B & (1 << CS12)) )
        return;

    // Stop timer
    TCCR1B &= ~(1 << CS12);
    // Reset timer counter
    TCNT1 = 0;

    // Disable further interrupts from pin 3
    EIMSK &= ~(1 << INT1);

    state = STOP;
}
