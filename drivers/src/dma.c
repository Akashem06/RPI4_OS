#include "dma.h"

DmaChannel dma_channels[15];

static u16 channel_map = 0x1F35;  // Maps which channels are available on the RPI

static u16 allocate_channel(u32 channel) {
  if (!(channel & ~0x0F)) {
    if (channel_map & (1 << channel)) {
      channel_map &= ~(1 << channel);  // If channel is available, mark it
      return channel;
    }
    return -1;  // Channel is not available, return -1 (error)
  }

  int i = (channel == CT_NORMAL) ? 6 : 12;  // Choose a starting index based on 'channel' type

  for (; i >= 0; i--) {
    if (channel_map & (1 << i)) {
      channel_map &= ~(1 << i);  // If channel is available, mark it
      return i;
    }
  }

  return CT_NONE;  // No available channels
}

DmaChannel *dma_open_channel(u32 channel) {
  u32 allocated_channel = allocate_channel(channel);

  if (allocated_channel == CT_NONE) {  // Checks if we were able to allocate a channel
    log("INVALID CHANNEL! %d\r\n", channel);
    return 0;
  }

  // Initializes the DMA channel
  // RPI including other SoCs use control blocks to define source/dest addr, byte number, etc.
  DmaChannel *dma = (DmaChannel *)&dma_channels[allocated_channel];
  dma->channel = allocated_channel;
  dma->block = (DmaControlBlock *)((LOW_MEMORY + 31) & ~31);
  dma->block->reserved[0] = 0;
  dma->block->reserved[1] = 0;

  // Gives 3ms for channel to initalize
  DMA_REGS_ENABLE |= (1 << dma->channel);
  timer_sleep(3);

  // Waits for channel to reset so all errors/transactions are cleared
  DMA_REGS(dma->channel)->control |= CS_RESET;
  while (DMA_REGS(dma->channel)->control & CS_RESET)
    ;

  return dma;
}

void dma_close_channel(DmaChannel *channel) {
  channel_map |= (1 << channel->channel);  // Marks the specified channel, disabling all access
}

void dma_setup_mem_copy(DmaChannel *channel, void *dest, void *src, u32 length, u32 burst_length) {
  channel->block->transfer_info = (burst_length << TI_BURST_LENGTH_SHIFT)  // How many data words shuold be sent in 1 burst,
                                                                           // important for speed
                                  | TI_SRC_WIDTH                           // Data width (the word size) for source address
                                  | TI_SRC_INC                             // src memory will increment with each word write
                                  | TI_DEST_WIDTH                          // Data width (the word size) for dest address
                                  | TI_DEST_INC;                           // dest memory will increment with each word write

  channel->block->src_addr = (u64)src;
  channel->block->dest_addr = (u64)dest;
  channel->block->transfer_length = length;
  channel->block->mode_2d_stride =
      0;  // 2d stride refers to how memory is copied. 1 means we copy memory in rows and columns
  channel->block->next_block_addr = 0;  // There is no next block to chain to (once one is complete the next will not run)
}

void dma_start(DmaChannel *channel) {
  // Sets DMA control block address (source, destination, transfer info, etc.) using the physical
  // bus address
  DMA_REGS(channel->channel)->control_block_addr = BUS_ADDRESS((u64)channel->block);

  // DMA waits for any outstanding writes to memory to complete
  // Uses default priority for when the system becomes overwhelmed
  // Marks the channel as active
  DMA_REGS(channel->channel)->control = CS_WAIT_FOR_OUTSTANDING_WRITES |
                                        (DEFAULT_PANIC_PRIORITY << CS_PANIC_PRIORITY_SHIFT) |
                                        (DEFAULT_PRIORITY << CS_PRIORITY_SHIFT) | CS_ACTIVE;
}

bool dma_wait(DmaChannel *channel) {
  // Waits for the channel to no longer be active
  while (DMA_REGS(channel->channel)->control & CS_ACTIVE)
    ;

  // Determines the channel's status by checking the control error register
  channel->status = DMA_REGS(channel->channel)->control & CS_ERROR ? false : true;

  return channel->status;
}
