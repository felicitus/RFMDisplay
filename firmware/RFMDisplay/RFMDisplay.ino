#include <avr/interrupt.h> 
#define QUARTER_SECOND_TICKS 32
#include <rfm12lab/src/rfm12.h>

#define MASK1(b1)                         ( (1<<b1) )
#define MASK2(b1,b2)                      ( (1<<b1) | (1<<b2) )
#define MASK3(b1,b2,b3)                   ( (1<<b1) | (1<<b2) | (1<<b3) )
#define MASK4(b1,b2,b3,b4)                ( (1<<b1) | (1<<b2) | (1<<b3) | (1<<b4) )

volatile uint8_t i = 0;


#define OE PB2
#define MR PD6
#define LATCH PD5
#define CYCLE PD0
#define DATA PB0
#define CLOCK PB4

volatile uint8_t cycle;
volatile unsigned long numberToDisplay;

void setup() {
  //set pins to output so you can control the shift register
  
  DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD2) | (1 << PD0);
  DDRB |= (1 << PB4) | (1 << PB0);
  
  PORTB &= ~(1 << OE);
  PORTD |= (1 << MR);
  
  cycle = 0;
  
  cli(); // Disable global interrupts
  
 
  // Turn off Input Capture Noise Canceler, Input Capture Edge Select on Falling, stop the clock
  TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
  // Disconnect the timer from the output pins, Set Waveform Generation Mode to Normal
  TCCR1A = (0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
  // Reset the count to zero
  TCNT1 = 0;
  // Set the output compare registers to zero
  OCR1A = 0;
  OCR1B = 0;
  // Clear the input capture?
  // ICR1 = 0;
  // Disable all Timer1 interrupts
  TIMSK &= ~(1 << TOIE1 | 1 << OCIE1A | 1 << OCIE1B | 1 << ICIE1);
  // Clear the Timer1 interrupt flags
  TIFR |= MASK4(TOV1,OCF1A,OCF1B,ICF1);

  OCR1A = QUARTER_SECOND_TICKS;
  
  TCCR1A = (TCCR1A & ~MASK2(WGM11,WGM10)) | (((B0100 & B0011) >> 0) << WGM10);
  TCCR1B = (TCCR1B & ~MASK2(WGM13,WGM12)) | (((B0100 & B1100) >> 2) << WGM12);
  TCCR1B = (TCCR1B & ~MASK3(CS12,CS11,CS10)) | (B100 << CS10);
  TIMSK |= (1<<OCIE1A);
  sei(); // Enable global interrupts
  
  numberToDisplay = 0;
}

ISR(TIMER1_COMPA_vect)
{
  byte cnt;
  
  if (cycle) {

    // take the latchPin low so 
    // the LEDs don't change while you're sending in bits:
    PORTD &= ~(1 << LATCH);

    for (cnt=0;cnt<32;cnt++) {
      
       if (!!(numberToDisplay & (1UL << (31 - cnt)))) {
         PORTB |= (1 << DATA);
       } else {
          PORTB &= ~(1<<DATA);         
       }
       
       PORTB |= (1 << CLOCK);
       PORTB &= ~(1 << CLOCK);
       

    }

    //take the latch pin high so the LEDs will light up:
    PORTD |= (1 << LATCH);
    PORTD &= ~(1 << CYCLE);
  } else {
     // take the latchPin low so 
    // the LEDs don't change while you're sending in bits:
    PORTD &= ~(1 << LATCH);
    
    for (cnt=0;cnt<32;cnt++) {
    if (!!(~numberToDisplay & (1UL << (31 - cnt)))) {
         PORTB |= (1 << DATA);
       } else {
        PORTB &= ~(1<<DATA);         
       }
       PORTB |= (1 << CLOCK);
       PORTB &= ~(1 << CLOCK);
    }
    
    PORTD |= (1 << LATCH) | (1 << CYCLE);
  }
  
  cycle = !cycle;
}

void loop() {
  
  
}



