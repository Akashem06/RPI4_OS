#include "mailbox.h"
#include "base.h"
#include "log.h"
#include "mem.h"

// Stores property data sent/received from the videocore. Aligned at 16 byes as required
static u32 property_data[8192] __attribute__((aligned(16)));

/** @brief Returns the mailbox registers as defined in the datasheet */
MailboxRegisters *MBX() {
    return (MailboxRegisters *)(PBASE + 0xB880);
}

/** @brief Polls mailbox write. Will block until mailbox is not full.
The first 4 bits are reserved for channel number, the remainder are data */
static void mailbox_write(u8 channel, u32 data) {
    while (MBX()->status & MAIL_FULL);

    MBX()->write = (data & 0xFFFFFFF0) | (channel & 0xF);
}

/** @brief Polls mailbox read. Will block until mailbox has data. We check the last 4 bits to verify the channel */
static u32 mailbox_read(u8 channel) {
    while (true) {
        while (MBX()->status & MAIL_EMPTY);
        u32 data = MBX()->read;
        u8 read_channel = (u8)(data & 0xF);

        if (read_channel == channel) {
            return (data & 0xFFFFFFF0);
        }
    }
}

bool mailbox_process(MailboxTag *tag, u32 tag_size) {
    // Buffer_size includes the size of the tag, 8 bytes for size and code, and 4 bytes for the property end tag
    int buffer_size = tag_size + 8 + 4;

    // Save the tag at an offset of two becase [0] and [1] is status/size 
    memcpy(&property_data[2], tag, tag_size);
    PropertyBuffer *buff = (PropertyBuffer *) property_data;

    buff->size = buffer_size;
    buff->code = RPI_FIRMWARE_STATUS_REQUEST; // Defines that this is a request
    // The end of the buffer is defined by RPI_FIRMWARE_PROPERTY_END (12 for size/code/end tag)
    property_data[(tag_size + 12) / 4 - 1] = RPI_FIRMWARE_PROPERTY_END;

    // Write the property_data buffer to Tags channel (ARM to VideoCore)
    // This initiates a write to videocore
    mailbox_write(MAIL_TAGS, (u32)(void *)property_data);

    // Read back the value received from VideoCore
    int result = mailbox_read(MAIL_TAGS);

    // Updates the tag that we saved earlier
    memcpy(tag, property_data + 2, tag_size);

    return true;
}

bool mailbox_generic_command(u32 tag_id, u32 id, u32 *value) {
    MailboxGeneric mbx;
    mbx.tag.id = tag_id;
    mbx.tag.value_length = 0;
    mbx.tag.buffer_size = sizeof(MailboxGeneric) - sizeof(MailboxTag);
    mbx.id = id;
    mbx.value = *value;

    if (!mailbox_process((MailboxTag *)&mbx, sizeof(mbx))) {
        log("FAILED TO PROCESS\n");
        return false;
    }

    *value = mbx.value;
    return true;
}

u32 mailbox_clock_rate(ClockType type) {
    MailboxClock c;
    c.tag.id = RPI_FIRMWARE_GET_CLOCK_RATE;
    c.tag.value_length = 0;
    c.tag.buffer_size = sizeof(c) - sizeof(c.tag);
    c.id = type;

    mailbox_process((MailboxTag *)&c, sizeof(c));

    return c.rate;
}

u32 mailbox_power_check(u32 type) {
    MailboxPower p;
    p.tag.id = RPI_FIRMWARE_GET_DOMAIN_STATE;
    p.tag.value_length = 0;
    p.tag.buffer_size = sizeof(p) - sizeof(p.tag);
    p.id = type;
    p.state = ~0;

    mailbox_process((MailboxTag *)&p, sizeof(p));

    return p.state && p.state != ~0;
}

