class DeskControl : public Component, public UARTDevice
{
public:
    DeskControl(UARTComponent *parent) : UARTDevice(parent) {}

    enum State
    {
        WAIT_FOR_BYTE_1 = 0,
        WAIT_FOR_BYTE_2,
        RECEIVE_COMMAND,
        RECEIVE_SIZE,
        RECEIVE_DATA,
        VERIFY_CHECKSUM,
        PROCESS_COMMAND
    };

    int state = WAIT_FOR_BYTE_1;
    int value;
    int command;
    int chksum = 0;
    int bufsize = 0;
    int bufread = 0;
    float valueF;
    uint8_t buffer[10];

    void loop() override
    {
        while (available())
        {
            uint8_t c;
            this->read_byte(&c);

            switch (state)
            {
            case WAIT_FOR_BYTE_1:
                handleWaitForByte1(c);
                break;
            case WAIT_FOR_BYTE_2:
                handleWaitForByte2(c);
                break;
            case RECEIVE_COMMAND:
                handleReceiveCommand(c);
                break;
            case RECEIVE_SIZE:
                handleReceiveSize(c);
                break;
            case RECEIVE_DATA:
                handleReceiveData(c);
                break;
            case VERIFY_CHECKSUM:
                handleVerifyChecksum(c);
                break;
            case PROCESS_COMMAND:
                handleProcessCommand(c);
                break;
            default:
                ESP_LOGD("custom", "🔴 Unexpected state: %i", state);
                state = WAIT_FOR_BYTE_1;
                break;
            }
        }
    }

private:

    void handleWaitForByte1(uint8_t c)
    {
        if (c == 0xf2)
        {
            state = WAIT_FOR_BYTE_2;
        }
    }

    void handleWaitForByte2(uint8_t c)
    {
        if (c == 0xf2)
        {
            state = RECEIVE_COMMAND;
            chksum = 0;
        }
        else
        {
            state = WAIT_FOR_BYTE_1;
        }
    }

    void handleReceiveCommand(uint8_t c)
    {
        command = c;
        state = RECEIVE_SIZE;
        chksum += command;
    }

    void handleReceiveSize(uint8_t c)
    {
        bufsize = c;
        bufread = 0;
        chksum += bufsize;
        state = RECEIVE_DATA;
    }

    void handleReceiveData(uint8_t c)
    {
        if (bufread < bufsize)
        {
            buffer[bufread] = c;
            chksum += c;
            bufread++;
        }
        if (bufread == bufsize)
        {
            state = VERIFY_CHECKSUM;
        }
    }

    void handleVerifyChecksum(uint8_t c)
    {
        chksum = (chksum & 0xff); // Limit to 8 bits
        if (chksum == c)
        {
            state = PROCESS_COMMAND;
        }
        else
        {
            state = WAIT_FOR_BYTE_1; // Checksum failure, reset state
        }
    }

    void handleProcessCommand(uint8_t c)
    {
        if (c == 0x7e)
        {
            processCommand();
            ESP_LOGD("custom", "🔵 Command processed: %i", command);
        } else {
            ESP_LOGD("custom", "🔴 Unexpected byte: %02x", c);
        }
        state = WAIT_FOR_BYTE_1; // Reset to initial state after processing
    }

    void processCommand()
    {
        switch (command)
        {
            case 1:
                valueF = (buffer[0] * 256) + buffer[1];
                id(height_slider).publish_state(valueF);
                break;
            case 0x25:
            case 0x26:
            case 0x27:
            case 0x28:
                valueF = (buffer[0] * 256) + buffer[1];
                publishSensorState(command);
                break;
            default:
                ESP_LOGD("custom", "🔴 Unexpected command: %i", command);
                break;
        }
    }

    void publishSensorState(int command)
    {
        switch (command)
        {
            case 0x25:
                id(sensor_m1).publish_state(valueF);
                break;
            case 0x26:
                id(sensor_m2).publish_state(valueF);
                break;
            case 0x27:
                id(sensor_m3).publish_state(valueF);
                break;
            case 0x28:
                id(sensor_m4).publish_state(valueF);
                break;
            default:
                ESP_LOGD("custom", "🔴 Unexpected command: %i", command);
                break;
        }
    }
};
