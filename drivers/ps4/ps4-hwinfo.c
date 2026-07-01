// SPDX-License-Identifier: GPL-2.0-only
#define pr_fmt(fmt) "ps4-hwinfo: " fmt

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/mmc/sdio_func.h>
#include "ps4-hwinfo.h"

#define AMD_VENDOR 0x1002

static const struct {
	u16 device;
	const char *name;
} ps4_gpu_ids[] = {
	{ 0x9920, "Liverpool" },
	{ 0x9922, "Liverpool" },
	{ 0x9923, "Liverpool" },
	{ 0x9924, "Gladius"   },
};

static const struct {
	u16 device;
	const char *name;
} ps4_sb_ids[] = {
	{ PCI_DEVICE_ID_SONY_AEOLIA_PCIE, "Aeolia" },
	{ PCI_DEVICE_ID_SONY_BELIZE_PCIE, "Belize" },
	{ PCI_DEVICE_ID_SONY_BAIKAL_PCIE, "Baikal" },
};

static void stepping_str(u8 rev, char *buf)
{
	buf[0] = 'A' + (rev >> 4);
	buf[1] = '0' + (rev & 0xf);
	buf[2] = '\0';
}

void ps4_hwinfo_print(void)
{
	struct pci_dev *gpu  = NULL;
	struct pci_dev *sb   = NULL;
	struct pci_dev *wlan = NULL;
	const char *gpu_name = "unknown";
	const char *sb_name  = "unknown";
	u16 gpu_dev = 0, sb_dev = 0, wlan_dev = 0;
	u16 wlan_ven = 0;
	u8 sb_rev = 0;
	char stepping[3];
	int i;

	for (i = 0; i < ARRAY_SIZE(ps4_gpu_ids); i++) {
		gpu = pci_get_device(AMD_VENDOR, ps4_gpu_ids[i].device, NULL);
		if (gpu) {
			gpu_name = ps4_gpu_ids[i].name;
			gpu_dev  = gpu->device;
			pci_dev_put(gpu);
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(ps4_sb_ids); i++) {
		sb = pci_get_device(PCI_VENDOR_ID_SONY, ps4_sb_ids[i].device, NULL);
		if (sb) {
			sb_name = ps4_sb_ids[i].name;
			sb_dev  = sb->device;
			sb_rev  = sb->revision;
			pci_dev_put(sb);
			break;
		}
	}

	wlan = pci_get_device(PCI_VENDOR_ID_MEDIATEK, 0x7668, NULL);
	if (wlan) {
		wlan_ven = wlan->vendor;
		wlan_dev = wlan->device;
		pci_dev_put(wlan);
	}

	stepping_str(sb_rev, stepping);
	if (wlan_dev)
		pr_info("GPU: %s [%04x:%04x]  southbridge: %s %s [%04x:%04x]  WLAN: [%04x:%04x]\n",
			gpu_name, AMD_VENDOR, gpu_dev,
			sb_name, stepping, PCI_VENDOR_ID_SONY, sb_dev,
			wlan_ven, wlan_dev);
	else
		pr_info("GPU: %s [%04x:%04x]  southbridge: %s %s [%04x:%04x]\n",
			gpu_name, AMD_VENDOR, gpu_dev,
			sb_name, stepping, PCI_VENDOR_ID_SONY, sb_dev);
}

static int ps4_hwinfo_sdio_iter(struct device *dev, void *data)
{
	struct sdio_func *func = dev_to_sdio_func(dev);

	if (func->num == 1)
		pr_info("WLAN: [%04x:%04x] via SDIO\n", func->vendor, func->device);

	return 0;
}

static int __init ps4_hwinfo_wlan_late(void)
{
	bus_for_each_dev(&sdio_bus_type, NULL, NULL, ps4_hwinfo_sdio_iter);
	return 0;
}
late_initcall(ps4_hwinfo_wlan_late);
