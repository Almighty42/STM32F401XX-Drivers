/**
 * @file reg_utils.h
 * @brief Generic register and bit-mask helpers, shared by all drivers.
 *
 * Naming follows the Linux kernel and Zephyr: BIT, GENMASK, FIELD_PREP,
 * FIELD_GET. Nothing here knows about a specific peripheral.
 *
 * Conventions:
 *  - A "mask" is a contiguous run of bits, built with BIT() or GENMASK().
 *  - A "field" is the value stored under a mask, shifted down to bit 0.
 *  - Macros may evaluate their arguments more than once. Never pass an
 *    expression with side effects (n++, function calls).
 *
 * Requires C11 (_Static_assert) and GCC or Clang (__builtin_ctz).
 */

#ifndef REG_UTILS_H
#define REG_UTILS_H

#include <stdbool.h>
#include <stdint.h>

/* TODO: */

/**
 * @name Deprecated helpers
 * Used by drivers not yet migrated. Delete when unused.
 * @{
 */

/**
 * @brief OR @p val into @p reg.
 * @param reg  Register (lvalue).
 * @param val  Bits to set, already in register position.
 * @deprecated Use SET_BIT() or reg_modify().
 */
#define SET_BITS_BY_VAR(reg, val) ((reg) |= (val))

/**
 * @brief Clear the 8-bit field that starts at bit @p pos.
 * @param reg  Register (lvalue).
 * @param pos  Bit offset of the field (not a byte index).
 * @deprecated Use reg_modify() with GENMASK().
 */
#define CLEAR_BYTE(reg, pos) ((reg) &= ~((uint32_t)0xFFU << (pos)))

/**
 * @brief Clear the 2-bit field that starts at bit @p pos.
 * @param reg  Register (lvalue).
 * @param pos  Bit offset of the field, for example 2 * pin for GPIO MODER.
 * @deprecated Use reg_set_field() with GENMASK().
 */
#define CLEAR_FIELD_2BIT(reg, pos) ((reg) &= ~((uint32_t)0x3U << (pos)))

/**
 * @brief Clear the 4-bit field that starts at bit @p pos.
 * @param reg  Register (lvalue).
 * @param pos  Bit offset of the field, for example 4 * (pin % 8) for GPIO AFR.
 * @deprecated Use reg_set_field() with GENMASK().
 */
#define CLEAR_FIELD_4BIT(reg, pos) ((reg) &= ~((uint32_t)0xFU << (pos)))

/** @} */

/* ---- Masks ----------------------------------------------------------- */

/** Mask with only bit @p n set. Uses uint32_t so 64-bit host tests match the target. */
#define BIT(n) ((uint32_t)1U << (n))

/** Contiguous mask from bit @p l up to and including bit @p h (0 <= l <= h <= 31). */
#define GENMASK(h, l) ((~(uint32_t)0U << (l)) & (~(uint32_t)0U >> (31U - (h))))

/** Lowest set bit of @p mask. @p mask must be non-zero. */
#define LSB_GET(mask) ((mask) & (~(mask) + 1U))

/* ---- Fields ---------------------------------------------------------- */

/** Move an unshifted @p val into the position of @p mask, dropping excess bits. */
#define FIELD_PREP(mask, val) (((uint32_t)(val) * LSB_GET(mask)) & (mask))

/** Extract the field under @p mask from @p reg, shifted down to bit 0. */
#define FIELD_GET(mask, reg) (((uint32_t)(reg) & (mask)) / LSB_GET(mask))

/* ---- Register access ------------------------------------------------- */

/**
 * @brief Replace the bits under @p mask with the same bits of @p value.
 *
 * One read and one write. @p value must already be in register position
 * (for example a hardware reset constant); bits outside @p mask are ignored.
 */
static inline void reg_modify(volatile uint32_t* reg, uint32_t mask,
                              uint32_t value)
{
	*reg = (*reg & ~mask) | (value & mask);
}

/**
 * @brief Set the field under @p mask to @p value.
 *
 * @p value is the unshifted field value, for example 2 for "alternate
 * function" in a 2-bit MODER field. Bits of @p value that do not fit the
 * mask are dropped, so a bad value cannot corrupt a neighbouring field.
 */
static inline void reg_set_field(volatile uint32_t* reg, uint32_t mask,
                                 uint32_t value)
{
	reg_modify(reg, mask, FIELD_PREP(mask, value));
}

#ifdef REG_POLL_HOOK
/**
 * @brief Hook called before every read in ::reg_poll (tests only).
 *
 * Unit tests define REG_POLL_HOOK as the name of a function that emulates
 * hardware changing registers by itself (ready flags, status bits).
 */
void REG_POLL_HOOK(void);
#define REG_POLL_STEP() REG_POLL_HOOK()
#else
#define REG_POLL_STEP() ((void)0)
#endif

/**
 * @brief Poll a register until the bits under @p mask equal @p expected.
 * @param[in] reg       Register to read.
 * @param[in] mask      Bits to compare.
 * @param[in] expected  Wanted value of those bits, in register position.
 * @param[in] loops     Maximum number of reads.
 * @retval true   The condition became true.
 * @retval false  It did not within @p loops reads.
 * @note The wait is counted in loops, not time, so its length depends on
 *       the core clock.
 */
static inline bool reg_poll(const volatile uint32_t* reg, uint32_t mask,
                            uint32_t expected, uint32_t loops)
{
	REG_POLL_STEP();
	for (uint32_t i = 0U; i < loops; i++) {
		if ((*reg & mask) == expected) {
			return true;
		}
	}
	return false;
}

/**
 * @brief Return @p reg masked with bit @p bit.
 * @param reg  Register or value.
 * @param bit  Bit index, 0..31.
 * @return Non-zero if the bit is set. This is the masked value, not 0 or 1.
 */
#define READ_BIT(reg, bit) ((reg) & BIT(bit))

/**
 * @brief Test whether bit @p bit of @p reg is set.
 * @param reg  Register or value.
 * @param bit  Bit index, 0..31.
 * @return true if the bit is set.
 */
#define IS_BIT_SET(reg, bit) (READ_BIT(reg, bit) != 0U)

/**
 * @brief Set bit @p bit of @p reg (read-modify-write).
 * @param reg  Register (lvalue).
 * @param bit  Bit index, 0..31.
 * @warning Do not use on write-1-to-act registers (EXTI->PR, GPIO->BSRR).
 */
#define SET_BIT(reg, bit) ((reg) |= BIT(bit))

/**
 * @brief Clear bit @p bit of @p reg (read-modify-write).
 * @param reg  Register (lvalue).
 * @param bit  Bit index, 0..31.
 * @warning Do not use on write-1-to-act registers (EXTI->PR, GPIO->BSRR).
 */
#define CLEAR_BIT(reg, bit) ((reg) &= ~BIT(bit))

/**
 * @brief Invert bit @p bit of @p reg (read-modify-write).
 * @param reg  Register (lvalue).
 * @param bit  Bit index, 0..31.
 * @warning Not atomic. Do not use on write-1-to-act registers.
 */
#define TOGGLE_BIT(reg, bit) ((reg) ^= BIT(bit))

/* ---- Bit masks used as sets (for example pin masks) ------------------ */

/** True if bit @p n of @p mask is set. */
static inline bool mask_has_bit(uint32_t mask, uint32_t n)
{
	return (mask & BIT(n)) != 0U;
}

/** True if exactly one bit of @p mask is set. */
static inline bool mask_is_single_bit(uint32_t mask)
{
	return mask != 0U && (mask & (mask - 1U)) == 0U;
}

/** Index of the lowest set bit of @p mask. @p mask must be non-zero. */
static inline uint32_t mask_lowest_bit_index(uint32_t mask)
{
	return (uint32_t)__builtin_ctz(mask);
}

/* ---- Compile-time self-checks ---------------------------------------- */

_Static_assert(BIT(0) == 0x1U && BIT(31) == 0x80000000U, "BIT");
_Static_assert(GENMASK(3, 0) == 0x0000000FU, "GENMASK low nibble");
_Static_assert(GENMASK(29, 28) == 0x30000000U, "GENMASK 2-bit field");
_Static_assert(GENMASK(31, 0) == 0xFFFFFFFFU, "GENMASK full word");
_Static_assert(LSB_GET(0x30U) == 0x10U, "LSB_GET");
_Static_assert(FIELD_PREP(GENMASK(5, 4), 3U) == 0x30U, "FIELD_PREP");
_Static_assert(FIELD_PREP(GENMASK(5, 4), 7U) == 0x30U,
               "FIELD_PREP drops excess bits");
_Static_assert(FIELD_GET(GENMASK(5, 4), 0x30U) == 3U, "FIELD_GET");

/**
 * @brief Set a field given by width and position.
 * @param reg    Register (lvalue, not a pointer).
 * @param width  Field width in bits.
 * @param pos    Bit offset of the field.
 * @param val    Unshifted field value; excess bits are dropped.
 * @deprecated Use reg_set_field() with GENMASK().
 */
#define MODIFY_FIELD(reg, width, pos, val)                                     \
	reg_set_field(&(reg), GENMASK((pos) + (width) - 1U, (pos)), (val))

#endif /* REG_UTILS_H */
