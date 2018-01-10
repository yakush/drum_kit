#ifndef AUDIO_OUT_H_
#define AUDIO_OUT_H_
// -------------------------------------
/**
   circuit setup:

   PIN HI ∙───<   1R >─┐
   PIN LO ∙───< 256R >─┼───∙ OUTPUT
                          │
                          ╪ C
                          │
                         GND

   R can be any value
   equivelent resistance ≈ 1R
   optional : LOW PASS FILTER using cap to groung.
   cutt off freq ≈ 1/2πRC
*/

#define DELTA_SIGM //use delta sigma dac

#define DAC_PIN_HI 26
#define DAC_PIN_LO 25

void audioOut_init() {

#ifdef DELTA_SIGM
  sigmaDeltaSetup(0, 312500 );
  sigmaDeltaSetup(1, 312500 );
  sigmaDeltaAttachPin(DAC_PIN_HI, 0);
  sigmaDeltaAttachPin(DAC_PIN_LO, 1);

#else
  pinMode(DAC_PIN_HI, OUTPUT);
  pinMode(DAC_PIN_LO, OUTPUT);
#endif

}

void audioOut_playSample(uint32_t val) {

#ifdef DELTA_SIGM
  sigmaDeltaWrite(0, (uint8_t) (val >> 8));
  sigmaDeltaWrite(1, (uint8_t) (val & 0xff));

#else
  dacWrite(DAC_PIN_HI, (uint8_t) (val >> 8));
  dacWrite(DAC_PIN_LO, (uint8_t) (val & 0xff));
#endif

}

#endif
