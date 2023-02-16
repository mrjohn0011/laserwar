#ifndef _BlinkTimer_h
#define _BlinkTimer_h
#include <TimerMs.h>

class BlinkTimer {
    private:
        bool state = false;
        int count = -1;
        int delayBefore = 0;
        void (*handler)(bool state) = nullptr;
        void (*onFinish)() = nullptr;
        TimerMs timer;
        TimerMs delayTimer;

        void startBlinking(){
            this->handler(this->state);
            this->timer.start();
        }

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

            if (this->delayBefore > 0){
                this->delayTimer.start();
            } else {
                this->startBlinking();
            }
        }

        void setDelayBefore(int delayBefore){
            this->delayBefore = delayBefore;
            this->delayTimer.setTimerMode();
            this->delayTimer.setTime(delayBefore);
        }

        void stop(){
            this->timer.stop();
            this->delayTimer.stop();
        }

        void tick(){
            if (this->delayTimer.tick()){
                startBlinking();
            }

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