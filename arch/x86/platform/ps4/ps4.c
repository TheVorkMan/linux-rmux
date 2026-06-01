// SPDX-License-Identifier: GPL-2.0-only
/*
 * ps4.c: Sony PS4 platform setup code
 */

#define pr_fmt(fmt) "ps4: " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/scatterlist.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/notifier.h>

#include <asm/setup.h>
#include <asm/mpspec_def.h>
#include <asm/hw_irq.h>
#include <asm/apic.h>
#include <asm/io_apic.h>
#include <asm/io.h>
#include <asm/i8259.h>
#include <asm/reboot.h>
#include <asm/msr.h>
#include <asm/ps4.h>

static bool is_ps4;
bool apcie_initialized;

/*
 * The RTC is part of the Aeolia PCI device and will be implemented there as
 * an RTC class device; stub these out.
 */
static void dummy_get_wallclock(struct timespec64 *now)
{
	now->tv_sec = now->tv_nsec = 0;
}
static int dummy_set_wallclock(const struct timespec64 *now)
{
	return -ENODEV;
}

/*
 * Provide a way for generic drivers to query for the availability of the
 * PS4 apcie driver/device, which is a dependency for them.
 */
int apcie_status(void)
{
	if (!is_ps4)
		return -ENODEV;
	return apcie_initialized;
}
EXPORT_SYMBOL_GPL(apcie_status);

/*
 * Report whether the kernel is running on PS4 hardware.  Valid after
 * x86_ps4_early_setup() and usable by PS4-conditional code in shared
 * subsystems so they stay no-ops on other x86 machines.
 */
bool x86_ps4_present(void)
{
	return is_ps4;
}
EXPORT_SYMBOL_GPL(x86_ps4_present);

void icc_reboot(void);

/*
 * PS4 specific x86_init function overrides and early setup calls.
 */
void __init x86_ps4_early_setup(void)
{
	pr_info("x86_ps4_early_setup: PS4 early setup\n");
	is_ps4 = true;
	x86_platform.calibrate_tsc = ps4_calibrate_tsc;
	x86_platform.get_wallclock = dummy_get_wallclock;
	x86_platform.set_wallclock = dummy_set_wallclock;

	legacy_pic = &null_legacy_pic;
	machine_ops.emergency_restart = icc_reboot;
}
