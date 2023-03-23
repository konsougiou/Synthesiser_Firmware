#include <Arduino.h>

class knob{

    public:
        knob(uint8_t number);

        void updateRotation(volatile uint8_t& knobRotation);

        void setLimits(uint8_t upper, uint8_t lower);

    private:
        uint8_t knobNumber;
        uint8_t upperLimit;
        uint8_t lowerLimit;
        uint8_t knobState;
        uint8_t keyArrayIdx;
};