#ifndef _BlinkTimer_h
#define _BlinkTimer_h
#include <TimerMs.h>

class BlinkTimer {
    private:
        bool state = false;
        int count = -1;
        void (*handler)(bool state) = nullptr;
        void (*onFinish)() = nullptr;
        TimerMs timer;
    public:
        BlinkTimer(void (*handler)(bool state)){
            this->handler = *handler;
        }

        void setHandler(void (*handler)(bool state)){
            this->handler = *handler;
        }

        void setOnFinish(void (*onFinish)()){
            this->onFinish = *onFinish;
        }

        void blink(uint32_t duration, int count) {
            if (this->timer.active()){
                return;
            }
            this->count = count;
            this->timer.setPeriodMode();
            this->timer.setTime(duration);
            this->state = false;
            this->handler(this->state);
            this->timer.start();
        }

        void tick(){
            if (this->timer.tick()){
                if (!this->state){
                    this->count--;
                }

                if (this->count == 0){
                    this->timer.stop();
                }

                this->state = !this->state;
                this->handler(this->state);
                
                if (*this->onFinish && this->count == 0){
                    this->onFinish();
                }
            }
        }
};
#endif