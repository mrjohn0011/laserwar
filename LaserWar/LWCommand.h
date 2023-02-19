#ifndef LWCommand_h
#define LWCommand_h
#include <Arduino.h>
#include <Constants.h>

class LWCommand: public Printable {
private:
    unsigned char cmd;
    unsigned char data;
public:
    bool load(unsigned long command){
        if (command & 0xff != LW_CMD_END) return false;
        this->data = (command >> 8) & 0xff;
        this->cmd = (command >> 16) & 0xff;
        return true;
    }
    unsigned char getGroup(){ return this->cmd; }
    unsigned char getData(){ return this->data; }
    void setGroup(unsigned char group){ this->cmd = group; }
    void setData(unsigned char data){ this->data = data; }
    unsigned long getCommand(){
        return (((this->cmd << 8) + this->data) << 8) + LW_CMD_END;
    }

    size_t printTo(Print& p) const {
        size_t r = 0;
        
        if (this->cmd == 0x83){
            for (unsigned char i = 0; i < 26; i++){
                if (commands83[i].code == this->data){
                    r += p.print(commands83[i].description);
                    return r;
                }
            }
            return r;
        }

        for (unsigned char i = 0; i < 5; i++){
            if (colorCommands[i].code == this->cmd){
                r += p.print(colorCommands[i].description);
                r += p.print(color_list[this->data]);
                return r;
            }
        }

        for (unsigned char i = 0; i < 11; i++){
            if (dataCommands[i].code == this->cmd){
                r += p.print(dataCommands[i].description);
                r += p.print(this->data, DEC);
                return r;
            }
        }

        return r;
    }
};

#endif