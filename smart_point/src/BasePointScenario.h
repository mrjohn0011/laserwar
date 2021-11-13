#ifndef BasePointScenario_h
#define BasePointScenario_h

#include "PointScenario.h"

class BasePointScenario: public PointScenario {
    public:
        BasePointScenario();
    protected:
        void setup() override;
        void loop() override;
};

#endif