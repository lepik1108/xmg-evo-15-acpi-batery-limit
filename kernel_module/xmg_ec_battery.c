#include <linux/module.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/power_supply.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("XMG EVO 15 ACPI Community");
MODULE_DESCRIPTION("XMG EVO 15 (E25) Battery Charge Control Threshold Driver");
MODULE_VERSION("1.1");

#define ACPI_WMBC_PATH "\\_SB.AMW0.WMBC"
#define EC_LIMIT_REGISTER 0x07B9

static acpi_handle wmbc_handle;
static struct power_supply *bat_psy;
static u8 cached_charge_limit = 80; /* Safe default */

/* Low-level ACPI execution helper for WMBC */
static int call_wmbc(u32 arg0, u32 arg1, u32 arg2, u64 *result)
{
	struct acpi_object_list arg_list;
	union acpi_object args[3];
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	acpi_status status;

	arg_list.count = 3;
	arg_list.pointer = args;

	args[0].type = ACPI_TYPE_INTEGER;
	args[0].integer.value = arg0;
	args[1].type = ACPI_TYPE_INTEGER;
	args[1].integer.value = arg1;
	args[2].type = ACPI_TYPE_INTEGER;
	args[2].integer.value = arg2;

	status = acpi_evaluate_object(wmbc_handle, NULL, &arg_list, &buffer);
	if (ACPI_FAILURE(status))
		return -EIO;

	obj = (union acpi_object *)buffer.pointer;
	if (obj && obj->type == ACPI_TYPE_INTEGER && result)
		*result = obj->integer.value;

	kfree(buffer.pointer);
	return 0;
}

/* Set Charge Limit Value via 3-step EC register sequence */
static int set_charge_limit(u8 val)
{
	u32 payload;
	u64 res = 0;
	int ret;

	if (val < 1 || val > 100)
		return -EINVAL;

	pr_info("xmg_battery: Setting charge limit to %u%%\n", val);

	/* Step 1: Unlock/Initialize EC write */
	ret = call_wmbc(1, 2, 0x0001, &res);
	if (ret)
		return ret;

	/* Step 2: Write percentage payload to register 0x07B9 */
	payload = ((u32)val << 16) | EC_LIMIT_REGISTER;
	ret = call_wmbc(0, 2, payload, &res);
	if (ret)
		return ret;

	/* Step 3: Commit/Sync to EC */
	ret = call_wmbc(0, 1, 0x0000, &res);
	if (ret)
		return ret;

	cached_charge_limit = val;
	pr_info("xmg_battery: Charge threshold %u%% successfully committed to EC register 0x07B9\n", val);
	return 0;
}

static int get_charge_limit(u8 *val)
{
	*val = cached_charge_limit;
	return 0;
}

/* Sysfs Attributes for Power Supply Subsystem */
static ssize_t charge_control_end_threshold_show(struct device *dev,
						  struct device_attribute *attr,
						  char *buf)
{
	u8 val = 80;
	get_charge_limit(&val);
	return sysfs_emit(buf, "%d\n", val);
}

static ssize_t charge_control_end_threshold_store(struct device *dev,
						   struct device_attribute *attr,
						   const char *buf,
						   size_t count)
{
	unsigned int val;
	int ret;

	if (kstrtouint(buf, 10, &val) || val < 1 || val > 100)
		return -EINVAL;

	ret = set_charge_limit((u8)val);
	if (ret)
		return ret;

	return count;
}

static DEVICE_ATTR_RW(charge_control_end_threshold);

static struct attribute *xmg_battery_attrs[] = {
	&dev_attr_charge_control_end_threshold.attr,
	NULL,
};

static const struct attribute_group xmg_battery_group = {
	.attrs = xmg_battery_attrs,
};

static int __init xmg_battery_init(void)
{
	acpi_status status;

	status = acpi_get_handle(NULL, ACPI_WMBC_PATH, &wmbc_handle);
	if (ACPI_FAILURE(status)) {
		pr_err("xmg_battery: WMBC ACPI method not found\n");
		return -ENODEV;
	}

	bat_psy = power_supply_get_by_name("BAT0");
	if (!bat_psy) {
		pr_err("xmg_battery: BAT0 power supply device not found\n");
		return -ENODEV;
	}

	if (sysfs_create_group(&bat_psy->dev.kobj, &xmg_battery_group)) {
		pr_err("xmg_battery: Failed to create sysfs threshold attribute\n");
		power_supply_put(bat_psy);
		return -ENOMEM;
	}

	/* Initialize hardware to default cached limit */
	set_charge_limit(cached_charge_limit);

	pr_info("xmg_battery: Driver loaded successfully\n");
	return 0;
}

static void __exit xmg_battery_exit(void)
{
	if (bat_psy) {
		sysfs_remove_group(&bat_psy->dev.kobj, &xmg_battery_group);
		power_supply_put(bat_psy);
	}
	pr_info("xmg_battery: Driver unloaded\n");
}

module_init(xmg_battery_init);
module_exit(xmg_battery_exit);

