#ifndef IO_PROVIDER_HPP_
#define IO_PROVIDER_HPP_



#include <cstdint>
namespace grs_io{

class IOProvider{
    public:
        virtual ~IOProvider() = default;
        virtual bool readDigitalInput(uint8_t index) = 0;
        virtual void writeDigitalOutput(uint8_t index, bool value) = 0;
        virtual bool readDigitalOutput(uint8_t index) = 0;
};


//For testing without hardware
class LocalIOProvider : public IOProvider{
    
    public:
        LocalIOProvider();

        bool readDigitalInput(uint8_t index) override;
        void writeDigitalOutput(uint8_t index, bool value) override;
        bool readDigitalOutput(uint8_t index) override;
    
        //input setting as external for testing
        void setDigitalInput(uint8_t index, bool value);
        

        //take all of state for debugging
        uint32_t getInputWord() const {return input_state_;}
        uint32_t getOutputWord() const{return output_state_;}


    private:
        uint32_t input_state_;
        uint32_t output_state_;
};







}

#endif //IO_PROVIDER_HPP_
