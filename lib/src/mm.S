.globl memzero
memzero:
    str xzr, [x0], #8    // Store the value of register xzr (which is always 0) at the address pointed to by x0 and post-increment the pointer by 8 bytes
    subs x1, x1, #8      // Subtract 8 from x1 (the remaining size of the memory to zero) and update the flags
    b.gt memzero         // If x1 is still greater than 0, branch back to the 'memzero' label and repeat
    ret                  // Return from the function
