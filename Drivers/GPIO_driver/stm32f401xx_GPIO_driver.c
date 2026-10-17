/**
 * @file stm32f401xx_GPIO_driver.c
 * @brief Implementation of the GPIO driver. The API is documented in the
 * header.
 */

#include "stm32f401xx_GPIO_driver.h"
#include "stm32f401xx_GPIO_regs.h"
#include <stddef.h>
#include <stdint.h>

/* ---- Private data ---------------------------------------------------- */

/* One callback slot per EXTI line (line n = pin n of whichever port owns it).
 * Fields are volatile because an ISR reads them. Lives in .bss, so it is
 * NULL until the startup code has zeroed .bss. */
typedef struct
{
	volatile GPIO_exti_callback_t cb;
	void* volatile ctx;
} gpio_exti_slot_t;

static gpio_exti_slot_t gpio_exti_slots[GPIO_PINS_PER_PORT];

#define GPIO_PORT_COUNT 6U

/* Reset values from RM0368; only ports A and B differ from 0. OTYPER, ODR,
 * AFRL, AFRH and LCKR reset to 0. Order matches gpio_ports[]. */
typedef struct
{
	uint32_t moder;
	uint32_t ospeedr;
	uint32_t pupdr;
} gpio_reset_vals_t;

static const gpio_reset_vals_t gpio_reset_vals[GPIO_PORT_COUNT] = {
    {0xA8000000U, 0x0C000000U, 0x64000000U}, /* A: PA13..PA15 SWD/JTAG */
    {0x00000280U, 0x000000C0U, 0x00000100U}, /* B: PB3, PB4 JTAG       */
    {0U, 0U, 0U},                            /* C */
    {0U, 0U, 0U},                            /* D */
    {0U, 0U, 0U},                            /* E */
    {0U, 0U, 0U},                            /* H */
};

/* Port order matches the AHB1ENR/AHB1RSTR bit positions: A..E = 0..4, H = 7.
 * The same number is the EXTICR port code. */
static GPIO_TypeDef* const gpio_ports[GPIO_PORT_COUNT] = {GPIOA, GPIOB, GPIOC,
                                                          GPIOD, GPIOE, GPIOH};
static const uint8_t gpio_ahb1_bit[GPIO_PORT_COUNT] = {0U, 1U, 2U, 3U, 4U, 7U};

/* ---- Private: port lookup -------------------------------------------- */

/** Index of @p port in gpio_ports[], or -1 if it is not a valid port. */
static int gpio_port_index(const GPIO_TypeDef* port)
{
	for (size_t i = 0U; i < GPIO_PORT_COUNT; i++) {
		if (gpio_ports[i] == port) {
			return (int)i;
		}
	}
	return -1;
}

/* ---- Private: per-pin register fields -------------------------------- */

static inline void gpio_set_mode(GPIO_TypeDef* port, uint32_t n, uint32_t v)
{
	reg_set_field(&port->MODER, GPIO_MODER_MASK(n), v);
}

static inline void gpio_set_speed(GPIO_TypeDef* port, uint32_t n, uint32_t v)
{
	reg_set_field(&port->OSPEEDR, GPIO_OSPEEDR_MASK(n), v);
}

static inline void gpio_set_pull(GPIO_TypeDef* port, uint32_t n, uint32_t v)
{
	reg_set_field(&port->PUPDR, GPIO_PUPDR_MASK(n), v);
}

static inline void gpio_set_otype(GPIO_TypeDef* port, uint32_t n, uint32_t v)
{
	reg_set_field(&port->OTYPER, GPIO_OTYPER_MASK(n), v);
}

static inline void gpio_set_af(GPIO_TypeDef* port, uint32_t n, uint32_t v)
{
	volatile uint32_t* afr = (n < 8U) ? &port->AFRL : &port->AFRH;
	reg_set_field(afr, GPIO_AFR_MASK(n), v);
}

/** True if any of @p pins is locked. */
static inline bool gpio_pins_locked(const GPIO_TypeDef* port, uint32_t pins)
{
	return (FIELD_GET(GPIO_LCKR_LCK_MASK, port->LCKR) & pins) != 0U;
}

/** Return pin @p n to its reset state. The mode is written last. */
static void gpio_reset_pin(GPIO_TypeDef* port, uint32_t n,
                           const gpio_reset_vals_t* rv)
{
	gpio_set_af(port, n, 0U);
	gpio_set_otype(port, n, 0U);
	reg_modify(&port->OSPEEDR, GPIO_OSPEEDR_MASK(n), rv->ospeedr);
	reg_modify(&port->PUPDR, GPIO_PUPDR_MASK(n), rv->pupdr);
	reg_modify(&port->MODER, GPIO_MODER_MASK(n), rv->moder);
}

/* ---- Private: EXTI building blocks ----------------------------------- */

static inline void exti_mask(uint32_t line)
{
	CLEAR_BIT(EXTI->IMR, line);
}
static inline void exti_unmask(uint32_t line)
{
	SET_BIT(EXTI->IMR, line);
}

static inline void exti_clear_pending(uint32_t line)
{
	EXTI->PR = BIT(line); /* write 1 to clear */
}

static inline void exti_disable_edges(uint32_t line)
{
	CLEAR_BIT(EXTI->RTSR, line);
	CLEAR_BIT(EXTI->FTSR, line);
}

static inline void exti_enable_edges(uint32_t line, GPIO_trigger_t trig)
{
	if (trig == GPIO_TRIG_RISING || trig == GPIO_TRIG_BOTH) {
		SET_BIT(EXTI->RTSR, line);
	}
	if (trig == GPIO_TRIG_FALLING || trig == GPIO_TRIG_BOTH) {
		SET_BIT(EXTI->FTSR, line);
	}
}

static inline void exti_route(uint32_t line, uint32_t port_code)
{
	SYSCFG_PCLK_EN();
	reg_set_field(&SYSCFG->EXTICR[line / 4U], EXTICR_MASK(line), port_code);
}

static inline uint32_t exti_routed_port(uint32_t line)
{
	return FIELD_GET(EXTICR_MASK(line), SYSCFG->EXTICR[line / 4U]);
}

/** A line is in use if it is unmasked or has an edge enabled. */
static inline bool exti_line_in_use(uint32_t line)
{
	return ((EXTI->IMR | EXTI->RTSR | EXTI->FTSR) & BIT(line)) != 0U;
}

static inline bool exti_owned_by(uint32_t line, uint32_t port_code)
{
	return exti_line_in_use(line) && exti_routed_port(line) == port_code;
}

static inline bool exti_owned_by_other(uint32_t line, uint32_t port_code)
{
	return exti_line_in_use(line) && exti_routed_port(line) != port_code;
}

/* ---- Private: GPIO + EXTI operations --------------------------------- */

/** Route line to the port, set the edges, clear any stale edge, then unmask. */
static void gpio_exti_arm(uint32_t line, uint32_t port_code,
                          GPIO_trigger_t trig)
{
	exti_mask(line); /* silent while reconfiguring */
	exti_disable_edges(line);
	exti_route(line, port_code);
	exti_enable_edges(line, trig);
	exti_clear_pending(line); /* an edge may have fired during setup */
	exti_unmask(line);
}

static void gpio_exti_disarm(uint32_t line)
{
	exti_mask(line);
	exti_disable_edges(line);
	exti_clear_pending(line);
}

/** Disarm every line in @p pins that is owned by this port. */
static void gpio_exti_release(uint32_t pins, uint32_t port_code)
{
	for (uint32_t n = 0U; n < GPIO_PINS_PER_PORT; n++) {
		if (mask_has_bit(pins, n) && exti_owned_by(n, port_code)) {
			gpio_exti_disarm(n);
		}
	}
}

/* ---- Private: validation --------------------------------------------- */

static inline bool gpio_in_range(uint32_t value, uint32_t max)
{
	return value <= max;
}

/** Check every field of @p cfg. Touches no register. */
static GPIO_status_t gpio_validate_config(const GPIO_config_t* cfg)
{
	if (cfg->pins == 0U) {
		return GPIO_ERR_INVALID_PINS;
	}
	if (!gpio_in_range((uint32_t)cfg->mode, GPIO_MODE_ANALOG)) {
		return GPIO_ERR_INVALID_MODE;
	}
	if (!gpio_in_range((uint32_t)cfg->otype, GPIO_OTYPE_OPEN_DRAIN)) {
		return GPIO_ERR_INVALID_OTYPE;
	}
	if (!gpio_in_range((uint32_t)cfg->speed, GPIO_SPEED_HIGH)) {
		return GPIO_ERR_INVALID_SPEED;
	}
	if (!gpio_in_range((uint32_t)cfg->pull, GPIO_PULL_DOWN)) {
		return GPIO_ERR_INVALID_PULL;
	}
	if (cfg->mode == GPIO_MODE_AF &&
	    !gpio_in_range((uint32_t)cfg->af, 15U)) {
		return GPIO_ERR_INVALID_AF; /* also rejects GPIO_AF_NONE */
	}
	if (!gpio_in_range((uint32_t)cfg->trigger, GPIO_TRIG_BOTH)) {
		return GPIO_ERR_INVALID_TRIGGER;
	}
	return GPIO_OK;
}

/** Check the EXTI-specific rules of @p cfg. Touches no register. */
static GPIO_status_t gpio_validate_exti(const GPIO_config_t* cfg,
                                        uint32_t port_code)
{
	if (cfg->trigger == GPIO_TRIG_NONE) {
		return GPIO_OK;
	}
	if (cfg->mode != GPIO_MODE_INPUT) {
		return GPIO_ERR_INVALID_TRIGGER;
	}
	if (!mask_is_single_bit(cfg->pins)) {
		return GPIO_ERR_EXTI_NOT_SINGLE;
	}
	if (exti_owned_by_other(mask_lowest_bit_index(cfg->pins), port_code)) {
		return GPIO_ERR_EXTI_LINE_BUSY;
	}
	return GPIO_OK;
}

/** Apply @p cfg to pin @p n. The mode is written last. */
static void gpio_apply_pin(GPIO_TypeDef* port, uint32_t n, uint32_t port_code,
                           const GPIO_config_t* cfg)
{
	if (cfg->mode == GPIO_MODE_OUTPUT || cfg->mode == GPIO_MODE_AF) {
		gpio_set_otype(port, n, (uint32_t)cfg->otype);
		gpio_set_speed(port, n, (uint32_t)cfg->speed);
	}
	gpio_set_pull(port, n, (uint32_t)cfg->pull);

	if (cfg->mode == GPIO_MODE_AF) {
		gpio_set_af(port, n, (uint32_t)cfg->af);
	}
	gpio_set_mode(port, n, (uint32_t)cfg->mode);

	if (cfg->trigger != GPIO_TRIG_NONE) {
		gpio_exti_arm(n, port_code, cfg->trigger);
	}
	else if (exti_owned_by(n, port_code)) {
		gpio_exti_disarm(n);
	}
}

/* ---- Clock and initialisation ---------------------------------------- */

/* Reading AHB1ENR back after enabling gives the clock time to start. Check
 * the STM32F401 errata sheet for the exact wording of that requirement. */
GPIO_status_t GPIO_peri_clk_control(GPIO_TypeDef* port, bool enable)
{
	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}
	if (enable) {
		SET_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]);
		(void)RCC->AHB1ENR; /* read-back: clock start-up delay */
	}
	else {
		CLEAR_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]);
	}
	return GPIO_OK;
}

GPIO_status_t GPIO_init(GPIO_TypeDef* port, const GPIO_config_t* cfg)
{
	if (port == NULL || cfg == NULL) {
		return GPIO_ERR_NULL_PTR;
	}
	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}

	GPIO_status_t status = gpio_validate_config(cfg);
	if (status != GPIO_OK) {
		return status;
	}
	const uint32_t port_code = gpio_ahb1_bit[idx];
	status = gpio_validate_exti(cfg, port_code);
	if (status != GPIO_OK) {
		return status;
	}

	(void)GPIO_peri_clk_control(port, true); /* registers need the clock */
	if (gpio_pins_locked(port, cfg->pins)) {
		return GPIO_ERR_PIN_LOCKED;
	}

	for (uint32_t n = 0U; n < GPIO_PINS_PER_PORT; n++) {
		if (mask_has_bit(cfg->pins, n)) {
			gpio_apply_pin(port, n, port_code, cfg);
		}
	}
	return GPIO_OK;
}

/* ---- De-initialisation ----------------------------------------------- */

/* Order: release EXTI, restore each pin (mode last), clear the output latch.
 * Locked pins are rejected up front, so a failed call changes nothing. */
GPIO_status_t GPIO_pin_deinit(GPIO_TypeDef* port, uint16_t pins)
{
	if (port == NULL) {
		return GPIO_ERR_NULL_PTR;
	}
	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}
	if (pins == 0U) {
		return GPIO_ERR_INVALID_PINS;
	}

	(void)GPIO_peri_clk_control(port, true);
	if (gpio_pins_locked(port, pins)) {
		return GPIO_ERR_PIN_LOCKED;
	}

	gpio_exti_release(pins, gpio_ahb1_bit[idx]);
	for (uint32_t n = 0U; n < GPIO_PINS_PER_PORT; n++) {
		if (mask_has_bit(pins, n)) {
			gpio_reset_pin(port, n, &gpio_reset_vals[idx]);
		}
	}
	port->BSRR = GPIO_BSRR_RESET(pins);
	return GPIO_OK;
}

GPIO_status_t GPIO_port_deinit(GPIO_TypeDef* port)
{
	if (port == NULL) {
		return GPIO_ERR_NULL_PTR;
	}
	const int idx = gpio_port_index(port);
	if (idx < 0) {
		return GPIO_ERR_INVALID_PORT;
	}

	gpio_exti_release(GPIO_PIN_ALL, gpio_ahb1_bit[idx]);
	SET_BIT(RCC->AHB1RSTR, gpio_ahb1_bit[idx]); /* pulse the port reset */
	CLEAR_BIT(RCC->AHB1RSTR, gpio_ahb1_bit[idx]);
	CLEAR_BIT(RCC->AHB1ENR, gpio_ahb1_bit[idx]); /* gate the clock */
	return GPIO_OK;
}

/* ---- Data access ----------------------------------------------------- */

void GPIO_set(GPIO_TypeDef* port, uint16_t pins)
{
	port->BSRR = GPIO_BSRR_SET(pins);
}

void GPIO_reset(GPIO_TypeDef* port, uint16_t pins)
{
	port->BSRR = GPIO_BSRR_RESET(pins);
}

void GPIO_toggle(GPIO_TypeDef* port, uint16_t pins)
{
	const uint32_t p = pins;
	const uint32_t odr = port->ODR;

	/* currently high -> reset, currently low -> set */
	port->BSRR = GPIO_BSRR_RESET(odr & p) | GPIO_BSRR_SET(~odr & p);
}

void GPIO_write_pins(GPIO_TypeDef* port, uint16_t mask, uint16_t value)
{
	const uint32_t m = mask;
	const uint32_t v = (uint32_t)value & m;

	port->BSRR = GPIO_BSRR_SET(v) | GPIO_BSRR_RESET(~v & m);
}

uint16_t GPIO_read_pins(GPIO_TypeDef* port, uint16_t pins)
{
	return (uint16_t)(port->IDR & pins);
}

uint16_t GPIO_read_output_pins(GPIO_TypeDef* port, uint16_t pins)
{
	return (uint16_t)(port->ODR & pins);
}

/* ---- EXTI ------------------------------------------------------------ */

GPIO_status_t GPIO_exti_set_callback(uint16_t pin, GPIO_exti_callback_t cb,
                                     void* ctx)
{
	if (!mask_is_single_bit(pin)) {
		return GPIO_ERR_EXTI_NOT_SINGLE;
	}

	gpio_exti_slot_t* slot = &gpio_exti_slots[mask_lowest_bit_index(pin)];

	slot->cb = NULL;
	slot->ctx = ctx;
	slot->cb = cb;
	return GPIO_OK;
}

GPIO_status_t GPIO_exti_enable(uint16_t pin)
{
	if (!mask_is_single_bit(pin)) {
		return GPIO_ERR_EXTI_NOT_SINGLE;
	}

	const uint32_t line = mask_lowest_bit_index(pin);

	exti_clear_pending(line);
	exti_unmask(line);
	return GPIO_OK;
}

GPIO_status_t GPIO_exti_disable(uint16_t pin)
{
	if (!mask_is_single_bit(pin)) {
		return GPIO_ERR_EXTI_NOT_SINGLE;
	}

	const uint32_t line = mask_lowest_bit_index(pin);

	exti_mask(line);
	exti_clear_pending(line);
	return GPIO_OK;
}

void GPIO_exti_irq_handler(uint8_t first, uint8_t last)
{
	if (first > last || last >= GPIO_PINS_PER_PORT) {
		return;
	}

	uint32_t pending = EXTI->PR & EXTI->IMR & GENMASK(last, first);
	if (pending == 0U) {
		return;
	}

	EXTI->PR = pending;

	while (pending != 0U) {
		const uint32_t line = mask_lowest_bit_index(pending);
		pending &= pending - 1U;

		const gpio_exti_slot_t* slot = &gpio_exti_slots[line];
		const GPIO_exti_callback_t cb = slot->cb;
		void* const ctx = slot->ctx;
		if (cb != NULL) {
			cb((uint16_t)BIT(line), ctx);
		}
	}
}

/* ---- GPIO lock ------------------------------------------------------- */

GPIO_status_t GPIO_lock_pins(GPIO_TypeDef* port, uint16_t pins)
{
	if (port == NULL) {
		return GPIO_ERR_NULL_PTR;
	}
	if (gpio_port_index(port) < 0) {
		return GPIO_ERR_INVALID_PORT;
	}
	if (pins == 0U) {
		return GPIO_ERR_INVALID_PINS;
	}

	(void)GPIO_peri_clk_control(port, true);

	/* Once the key is active, LCKR is frozen until a reset. */
	const uint32_t lckr = port->LCKR;
	if ((lckr & GPIO_LCKR_LCKK_MASK) != 0U) {
		const uint32_t already_locked =
		    FIELD_GET(GPIO_LCKR_LCK_MASK, lckr);
		return ((already_locked & pins) == pins) ? GPIO_OK
		                                         : GPIO_ERR_LOCK_FAILED;
	}

	/* Lock key write sequence. Always 32-bit writes, and the pin bits
	 * (LCKR[15:0]) must be identical in all three writes. */
	const uint32_t lck = pins;
	port->LCKR = GPIO_LCKR_LCKK_MASK | lck; /* 1. LCKK = 1 */
	port->LCKR = lck;                       /* 2. LCKK = 0 */
	port->LCKR = GPIO_LCKR_LCKK_MASK | lck; /* 3. LCKK = 1 */
	(void)port->LCKR;                       /* read */

	if ((port->LCKR & GPIO_LCKR_LCKK_MASK) == 0U) {
		return GPIO_ERR_LOCK_FAILED;
	}

	return GPIO_OK;
}
