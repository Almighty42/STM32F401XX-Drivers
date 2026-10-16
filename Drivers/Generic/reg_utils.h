/* Generic/reg_utils.h */
#ifndef REG_UTILS_H
#define REG_UTILS_H

#include <stdint.h>

/* Single-bit helpers: read-modify-write. Do NOT use them on write-1-to-act
 * registers such as NVIC->ICER or EXTI->PR; write those directly. */
#define READ_BIT(reg, bit)    ((reg) & (1UL << (bit)))
#define IS_BIT_SET(reg, bit)  (READ_BIT(reg, bit) != 0UL)
#define SET_BIT(reg, bit)     ((reg) |= (1UL << (bit)))
#define CLEAR_BIT(reg, bit)   ((reg) &= ~(1UL << (bit)))
#define TOGGLE_BIT(reg, bit)  ((reg) ^= (1UL << (bit)))

/* Mask of the lowest `width` bits. Valid for width 1..32. */
#define FIELD_MASK(width)     (UINT32_MAX >> (32U - (width)))

/* Replace the bits in `mask` with the matching bits of `value`. One read,
 * one write. Bits of `value` outside `mask` are ignored. */
static inline void reg_modify(volatile uint32_t* reg, uint32_t mask,
                              uint32_t value)
{
    *reg = (*reg & ~mask) | (value & mask);
}

/* Update a `width`-bit field at bit position `pos` (pos + width <= 32). */
#define MODIFY_FIELD(reg, width, pos, val) \
    reg_modify(&(reg), FIELD_MASK(width) << (pos), (uint32_t)(val) << (pos))

#endif /* REG_UTILS_H */
