#include <vm.h>
#include <avr/interrupt.h>

#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

static volatile uint32_t timer0_millis;
static volatile uint8_t timer0_fract;

#if defined(TIM0_OVF_vect)
ISR(TIM0_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
	uint32_t millis = timer0_millis;
	uint8_t fract = timer0_fract;

	millis += MILLIS_INC;
	fract += FRACT_INC;
	if (fract >= FRACT_MAX) {
		fract -= FRACT_MAX;
		millis += 1;
	}

	timer0_fract = fract;
	timer0_millis = millis;
}

uint32_t VM_MilliSeconds(void) {
	uint32_t result;
	uint8_t oldSREG = SREG;

	cli();
	result = timer0_millis;
	SREG = oldSREG;

    return result;
}

void VM_MilliSeconds_Init(void) {
#if defined(TCCR0A) && defined(WGM01)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);
#endif

#if defined(__AVR_ATmega128__)
	sbi(TCCR0, CS02);
#elif defined(TCCR0) && defined(CS01) && defined(CS00)
	sbi(TCCR0, CS01);
	sbi(TCCR0, CS00);
#elif defined(TCCR0B) && defined(CS01) && defined(CS00)
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);
#elif defined(TCCR0A) && defined(CS01) && defined(CS00)
	sbi(TCCR0A, CS01);
	sbi(TCCR0A, CS00);
#else
	#error Timer 0 prescale factor 64 not set correctly
#endif

#if defined(TIMSK) && defined(TOIE0)
	sbi(TIMSK, TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
	sbi(TIMSK0, TOIE0);
#else
	#error	Timer 0 overflow interrupt not set correctly
#endif
}