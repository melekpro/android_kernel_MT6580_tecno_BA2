/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/kernel.h>

#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
static struct regulator *disp_bias_pos;
static struct regulator *disp_bias_neg;
static int regulator_inited;

int display_bias_regulator_init(void)
{
	int ret = 0;

	if (regulator_inited)
		return ret;

	/* please only get regulator once in a driver */
	disp_bias_pos = regulator_get(NULL, "dsv_pos");
	if (IS_ERR(disp_bias_pos)) { /* handle return value */
		ret = PTR_ERR(disp_bias_pos);
		pr_err("get dsv_pos fail, error: %d\n", ret);
		return ret;
	}

	disp_bias_neg = regulator_get(NULL, "dsv_neg");
	if (IS_ERR(disp_bias_neg)) { /* handle return value */
		ret = PTR_ERR(disp_bias_neg);
		pr_err("get dsv_neg fail, error: %d\n", ret);
		return ret;
	}

	regulator_inited = 1;
	return ret; /* must be 0 */

}
EXPORT_SYMBOL(display_bias_regulator_init);

int display_bias_enable(void)
{
	int ret = 0;
	int retval = 0;

	display_bias_regulator_init();

	/* set voltage with min & max*/
	ret = regulator_set_voltage(disp_bias_pos, 5400000, 5400000);
	if (ret < 0)
		pr_err("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_set_voltage(disp_bias_neg, 5400000, 5400000);
	if (ret < 0)
		pr_err("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

#if 0
	/* get voltage */
	ret = mtk_regulator_get_voltage(&disp_bias_pos);
	if (ret < 0)
		pr_err("get voltage disp_bias_pos fail\n");
	pr_debug("pos voltage = %d\n", ret);

	ret = mtk_regulator_get_voltage(&disp_bias_neg);
	if (ret < 0)
		pr_err("get voltage disp_bias_neg fail\n");
	pr_debug("neg voltage = %d\n", ret);
#endif
	/* enable regulator */
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_err("enable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_err("enable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	return retval;
}
EXPORT_SYMBOL(display_bias_enable);

/*add by zhangkun for set avdd && avee voltage value --begin--*/
#ifdef CONFIG_TRAN_LCM_SET_VOLTAGE
int tran_display_bias_enable(int vol)
{
	int ret = 0;
	int retval = 0;
#if defined(CONFIG_TRAN_LCM_TIME_OPT_ENABLE)
	printk("%s winston_tran_display_bias_enable start\n",__func__);
#endif
	display_bias_regulator_init();

	/* set voltage with min & max*/
	ret = regulator_set_voltage(disp_bias_pos, vol, vol);
	if (ret < 0)
		pr_err("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_set_voltage(disp_bias_neg, vol, vol);
	if (ret < 0)
		pr_err("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	/* enable regulator */
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_err("enable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_err("enable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;
#if defined(CONFIG_TRAN_LCM_TIME_OPT_ENABLE)
	printk("%s winston_tran_display_bias_enable end\n",__func__);
#endif
	return retval;
}
EXPORT_SYMBOL(tran_display_bias_enable);
#endif
/*add by zhangkun for set avdd && avee voltage value --end--*/

int display_bias_disable(void)
{
	int ret = 0;
	int retval = 0;
#if defined(CONFIG_TRAN_LCM_TIME_OPT_ENABLE)
	printk("%s winston_display_bias_disable start\n",__func__);
#endif
	display_bias_regulator_init();

	ret = regulator_disable(disp_bias_neg);
	if (ret < 0)
		pr_err("disable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_disable(disp_bias_pos);
	if (ret < 0)
		pr_err("disable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
#if defined(CONFIG_TRAN_LCM_TIME_OPT_ENABLE)
	printk("%s winston_display_bias_disable end\n",__func__);
#endif
	return retval;
}
EXPORT_SYMBOL(display_bias_disable);

#else
int display_bias_regulator_init(void)
{
	return 0;
}
EXPORT_SYMBOL(display_bias_regulator_init);

int display_bias_enable(void)
{
	return 0;
}
EXPORT_SYMBOL(display_bias_enable);

/*add by zhangkun for set avdd && avee voltage value --begin--*/
#ifdef CONFIG_TRAN_LCM_SET_VOLTAGE
int tran_display_bias_enable(int vol)
{
	return 0;
}
EXPORT_SYMBOL(tran_display_bias_enable);
#endif
/*add by zhangkun for set avdd && avee voltage value --end--*/

int display_bias_disable(void)
{
	return 0;
}
EXPORT_SYMBOL(display_bias_disable);
#endif
