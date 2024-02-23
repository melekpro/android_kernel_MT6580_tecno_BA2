#if defined(BUILD_LK)
#include <debug.h>
#else
#include <linux/kernel.h>
#include <linux/string.h>
#endif

#if defined(BUILD_LK)
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#else
#ifdef CONFIG_MTK_LEGACY
#include <mach/mt_gpio.h>
#endif
#endif

#ifdef CONFIG_MTK_LEGACY
#include <cust_gpio_usage.h>
#endif
#ifndef CONFIG_FPGA_EARLY_PORTING
#if defined(CONFIG_MTK_LEGACY)
#include <cust_i2c.h>
#endif
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH (480)
#define FRAME_HEIGHT (960)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY 0xFFFC
#define REGFLAG_END_OF_TABLE 0xFFFD

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

struct LCM_setting_table
{
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] =
	{
		{0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}},
		{0xF6, 2, {0x5A, 0x87}},
		{0xC1, 1, {0x3F}},
		{0xC2, 1, {0x0E}},
		{0xC6, 1, {0xF8}},
		{0xCD, 1, {0x25}},
		{0xC9, 1, {0x10}},
		{0xF8, 1, {0x8A}},
		{0xAC, 1, {0x65}},
		{0xA7, 1, {0x47}},
		{0xA0, 1, {0xFF}},
		{0x86, 4, {0x99, 0xA4, 0xA4, 0x61}},
		{0x87, 3, {0x04, 0x03, 0x66}},
		{0xFA, 3, {0x08, 0x08, 0x00}},
		{0xA3, 1, {0x6E}},
		{0xFD, 3, {0x28, 0x3C, 0x00}},
		{0x9A, 1, {0x4E}},
		{0x9B, 1, {0x3C}},
		{0x82, 2, {0x0E, 0x0E}},
		{0xB1, 1, {0x10}},
		{0x7A, 2, {0x0F, 0x13}},
		{0x7B, 2, {0x0F, 0x13}},
		{0x6D, 32, {0x1E, 0x1E, 0x1E, 0x03, 0x01, 0x09, 0x0F, 0x0B, 0x0D, 0x05, 0x07, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x08, 0x06, 0x0E, 0x0C, 0x10, 0x0A, 0x02, 0x04, 0x1E, 0x1E, 0x1E}},
		{0x64, 16, {0x28, 0x05, 0x03, 0xC2, 0x03, 0x03, 0x28, 0x04, 0x03, 0xC3, 0x03, 0x03, 0x01, 0x7A, 0x01, 0x7A}},
		{0x65, 16, {0x28, 0x01, 0x03, 0xC6, 0x03, 0x03, 0x28, 0x00, 0x03, 0xC7, 0x03, 0x03, 0x01, 0x7A, 0x01, 0x7A}},
		{0x66, 16, {0x20, 0x01, 0x03, 0xC8, 0x03, 0x03, 0x20, 0x02, 0x03, 0xC9, 0x03, 0x03, 0x01, 0x7A, 0x01, 0x7A}},
		{0x67, 16, {0x28, 0x03, 0x03, 0xC4, 0x03, 0x03, 0x28, 0x02, 0x03, 0xC5, 0x03, 0x03, 0x01, 0x7A, 0x01, 0x7A}},
		{0x60, 8, {0x38, 0x09, 0x7A, 0x7A, 0x38, 0x08, 0x7A, 0x7A}},
		{0x61, 8, {0x38, 0x07, 0x7A, 0x7A, 0x38, 0x06, 0x7A, 0x7A}},
		{0x62, 8, {0x33, 0xBB, 0x7A, 0x7A, 0x33, 0xBC, 0x7A, 0x7A}},
		{0x63, 8, {0x33, 0xBD, 0x7A, 0x7A, 0x33, 0xBE, 0x7A, 0x7A}},
		{0x69, 7, {0x11, 0x24, 0x11, 0x24, 0x44, 0x22, 0x08}},
		{0x6B, 1, {0x07}},
		{0xD1, 52, {0x00, 0x00, 0x00, 0x12, 0x00, 0x3A, 0x00, 0x5A, 0x00, 0x74, 0x00, 0x9E, 0x00, 0xBF, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x68, 0x01, 0x9E, 0x01, 0xEC, 0x02, 0x29, 0x02, 0x2B, 0x02, 0x66, 0x02, 0xAA, 0x02, 0xD7, 0x03, 0x16, 0x03, 0x3F, 0x03, 0x70, 0x03, 0x91, 0x03, 0xB8, 0x03, 0xCF, 0x03, 0xE9, 0x03, 0xFA, 0x03, 0xFF}},
		{0xD2, 52, {0x00, 0x00, 0x00, 0x12, 0x00, 0x3A, 0x00, 0x5A, 0x00, 0x74, 0x00, 0x9E, 0x00, 0xBF, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x68, 0x01, 0x9E, 0x01, 0xEC, 0x02, 0x29, 0x02, 0x2B, 0x02, 0x66, 0x02, 0xAA, 0x02, 0xD7, 0x03, 0x16, 0x03, 0x3F, 0x03, 0x70, 0x03, 0x91, 0x03, 0xB8, 0x03, 0xCF, 0x03, 0xE9, 0x03, 0xFA, 0x03, 0xFF}},
		{0xD3, 52, {0x00, 0x00, 0x00, 0x12, 0x00, 0x3A, 0x00, 0x5A, 0x00, 0x74, 0x00, 0x9E, 0x00, 0xBF, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x68, 0x01, 0x9E, 0x01, 0xEC, 0x02, 0x29, 0x02, 0x2B, 0x02, 0x66, 0x02, 0xAA, 0x02, 0xD7, 0x03, 0x16, 0x03, 0x3F, 0x03, 0x70, 0x03, 0x91, 0x03, 0xB8, 0x03, 0xCF, 0x03, 0xE9, 0x03, 0xFA, 0x03, 0xFF}},
		{0xD4, 52, {0x00, 0x00, 0x00, 0x12, 0x00, 0x3A, 0x00, 0x5A, 0x00, 0x74, 0x00, 0x9E, 0x00, 0xBF, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x68, 0x01, 0x9E, 0x01, 0xEC, 0x02, 0x29, 0x02, 0x2B, 0x02, 0x66, 0x02, 0xAA, 0x02, 0xD7, 0x03, 0x16, 0x03, 0x3F, 0x03, 0x70, 0x03, 0x91, 0x03, 0xB8, 0x03, 0xCF, 0x03, 0xE9, 0x03, 0xFA, 0x03, 0xFF}},
		{0xD5, 52, {0x00, 0x00, 0x00, 0x12, 0x00, 0x3A, 0x00, 0x5A, 0x00, 0x74, 0x00, 0x9E, 0x00, 0xBF, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x68, 0x01, 0x9E, 0x01, 0xEC, 0x02, 0x29, 0x02, 0x2B, 0x02, 0x66, 0x02, 0xAA, 0x02, 0xD7, 0x03, 0x16, 0x03, 0x3F, 0x03, 0x70, 0x03, 0x91, 0x03, 0xB8, 0x03, 0xCF, 0x03, 0xE9, 0x03, 0xFA, 0x03, 0xFF}},
		{0xD6, 52, {0x00, 0x00, 0x00, 0x12, 0x00, 0x3A, 0x00, 0x5A, 0x00, 0x74, 0x00, 0x9E, 0x00, 0xBF, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x68, 0x01, 0x9E, 0x01, 0xEC, 0x02, 0x29, 0x02, 0x2B, 0x02, 0x66, 0x02, 0xAA, 0x02, 0xD7, 0x03, 0x16, 0x03, 0x3F, 0x03, 0x70, 0x03, 0x91, 0x03, 0xB8, 0x03, 0xCF, 0x03, 0xE9, 0x03, 0xFA, 0x03, 0xFF}},
		{0x11, 0, {0x00}},
		{REGFLAG_DELAY, 120, {}},
		{0x29, 0, {0x00}},
		{REGFLAG_DELAY, 120, {}},
};

static struct LCM_setting_table lcm_sleep_mode_in_setting[] =
	{
		// {0xF0, 5,{0x55,0xaa,0x52,0x08,0x00}},
		// {0xc1, 1,{0x3f}},

		{0x6C, 1, {0x60}},
		{REGFLAG_DELAY, 20, {}},
		{0xB1, 1, {0x00}},
		{0xFA, 4, {0x7F, 0x00, 0x00, 0x00}},
		{REGFLAG_DELAY, 20, {}},

		{0x6C, 1, {0x50}},
		{REGFLAG_DELAY, 10, {}},
		{0x28, 0, {}},
		{REGFLAG_DELAY, 50, {}},
		{0x10, 0, {}},
		{REGFLAG_DELAY, 20, {}},

		{0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}},
		{0xC2, 1, {0xCE}},
		{0xC3, 1, {0xCD}},
		{0xC6, 1, {0xFC}},
		{0xC5, 1, {0x03}},
		{0xCD, 1, {0x64}},
		{0xC4, 1, {0xFF}},
		{0xC9, 1, {0xCD}},
		{0xF6, 2, {0x5A, 0x87}},
		{0xFD, 3, {0xAA, 0xAA, 0x0A}},
		{0xFE, 2, {0x6A, 0x0A}},
		{0x78, 2, {0x2A, 0xAA}},
		{0x92, 2, {0x17, 0x08}},
		{0x77, 2, {0xAA, 0x2A}},
		{0x76, 2, {0xAA, 0xAA}},
		{0x84, 1, {0x00}},
		{0x78, 2, {0x2B, 0xBA}},
		{0x89, 1, {0x73}},
		{0x88, 1, {0x3A}},
		{0x85, 1, {0xB0}},
		{0x76, 2, {0xEB, 0xAA}},
		{0x94, 1, {0x80}},
		{0x87, 3, {0x04, 0x07, 0x30}},
		{0x93, 1, {0x27}},
		{0xAF, 1, {0x02}},

		{REGFLAG_END_OF_TABLE, 0x00, {}},
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++)
	{
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd)
		{
		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
			MDELAY(2);
		}
	}
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;
	params->virtual_width = VIRTUAL_WIDTH;
	params->virtual_height = VIRTUAL_HEIGHT;
	params->density            = LCM_DENSITY;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
	lcm_dsi_mode = CMD_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
	lcm_dsi_mode = SYNC_PULSE_VDO_MODE;
#endif
	LCM_LOGI("lcm_get_params lcm_dsi_mode %d\n", lcm_dsi_mode);
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 8;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_frontporch_for_low_power = 620;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 10;
	params->dsi.horizontal_backporch = 20;
	params->dsi.horizontal_frontporch = 40;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/*params->dsi.ssc_disable = 1;*/
#ifndef CONFIG_FPGA_EARLY_PORTING
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 420;	/* this value must be in MTK suggested table */
#else
	params->dsi.PLL_CLOCK = 440;	/* this value must be in MTK suggested table */
#endif
	params->dsi.PLL_CK_CMD = 420;
	params->dsi.PLL_CK_VDO = 440;
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	params->dsi.CLK_HS_POST = 36;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x53;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x24;

	params->dsi.lane_swap_en = 1;

	params->dsi.lane_swap[MIPITX_PHY_PORT_0][MIPITX_PHY_LANE_0] = MIPITX_PHY_LANE_CK;
	params->dsi.lane_swap[MIPITX_PHY_PORT_0][MIPITX_PHY_LANE_1] = MIPITX_PHY_LANE_2;
	params->dsi.lane_swap[MIPITX_PHY_PORT_0][MIPITX_PHY_LANE_2] = MIPITX_PHY_LANE_3;
	params->dsi.lane_swap[MIPITX_PHY_PORT_0][MIPITX_PHY_LANE_3] = MIPITX_PHY_LANE_0;
	params->dsi.lane_swap[MIPITX_PHY_PORT_0][MIPITX_PHY_LANE_CK] = MIPITX_PHY_LANE_1;
	params->dsi.lane_swap[MIPITX_PHY_PORT_0][MIPITX_PHY_LANE_RX] = MIPITX_PHY_LANE_1;
#endif
}
static void lcm_init_power(void)
{
	display_bias_enable();
}

static void lcm_suspend_power(void)
{
	display_bias_disable();
}

static void lcm_resume_power(void)
{
	SET_RESET_PIN(0);
	display_bias_enable();

static void lcm_init(void)
{
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);
#ifdef BUILD_LK
	printf("cjx:%s\n", __func__);
#else
	printk("cjx:%s\n", __func__);
#endif

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	push_table(lcm_sleep_mode_in_setting, sizeof(lcm_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
}

static unsigned int lcm_compare_id(void)
{
	return 1;
}

static void lcm_resume(void)
{
	lcm_init();
}

LCM_DRIVER gc9503p_fwp_dsi_vdo_jt_ivo_ba2_lcm_drv = {
	.name = "gc9503p_fwp_dsi_vdo_jt_ivo_ba2",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
        .resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
};
