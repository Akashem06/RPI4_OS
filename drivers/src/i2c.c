#include "i2c.h"
#include "gpio.h"

static void i2c_setup(u8 address, u32 size, u32 control_flags) {
    I2C_REGS->slave_address = address;
    I2C_REGS->control = C_CLEAR;
    I2C_REGS->status = S_CLKT | S_ERR | S_DONE;
    I2C_REGS->data_length = size;
    I2C_REGS->control = C_I2CEN | C_ST | control_flags;
}

static I2CStatus i2c_check_status(reg32 status, int count, u32 size) {
    if (status & S_ERR) {
        return I2CS_ACK_ERROR;
    } else if (status & S_CLKT) {
        return I2CS_CLOCK_TIMEOUT;
    } else if (count < size) {
        return I2CS_DATA_LOSS;
    }
    return I2CS_SUCCESS;
}

void i2c_init(I2CClockSpeed clock_speed) {
    if (clock_speed > CORE_CLOCK_SPEED) {
        return;
    }
    gpio_set_function(2, GF_ALT0);
    gpio_set_function(3, GF_ALT0);
    gpio_enable(2);
    gpio_enable(3);

    I2C_REGS->div = CORE_CLOCK_SPEED / clock_speed;
}

I2CStatus i2c_recv(u8 address, u8 *buffer, u32 size) {
    int count = 0;
    int timeout = I2C_TIMEOUT;

    i2c_setup(address, size, C_READ);

    while (!(I2C_REGS->status & S_DONE) && timeout > 0) {
        if (I2C_REGS->status & (S_ERR | S_CLKT)) {
            break;
        }

        while (count < size && I2C_REGS->status & S_RXD) {
            *buffer++ = I2C_REGS->fifo & 0xFF;
            count++;
        }

        timeout--;
    }

    reg32 status = I2C_REGS->status;
    I2C_REGS->status = S_DONE;
    
    return i2c_check_status(status, count, size);
}

I2CStatus i2c_send(u8 address, u8 *buffer, u32 size) {
    int count = 0;
    int timeout = I2C_TIMEOUT;

    i2c_setup(address, size, 0);

    while (!(I2C_REGS->status & S_DONE) && timeout > 0) {
        if (I2C_REGS->status & (S_ERR | S_CLKT)) {
            break;
        }

        while (count < size && I2C_REGS->status & S_TXD) {
            I2C_REGS->fifo = *buffer++;
            count++;
        }

        timeout--;
    }

    reg32 status = I2C_REGS->status;
    I2C_REGS->status = S_DONE;
    
    return i2c_check_status(status, count, size);
}
