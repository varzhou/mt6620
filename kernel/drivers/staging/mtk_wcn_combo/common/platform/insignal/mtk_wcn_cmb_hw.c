/*******************************************************************************
* Copyright (c) 2013, MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License version 2 as published by the Free
* Software Foundation.
*
* Alternatively, this software may be distributed under the terms of BSD
* license.
********************************************************************************
*/

/*******************************************************************************
* THIS SOFTWARE IS PROVIDED BY MEDIATEK "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT OR FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL MEDIATEK BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
*/

/*! \file
\brief  Declaration of library functions

Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG "[WMT-CMB-HW]"

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "osal_typedef.h"

#include "mtk_wcn_cmb_hw.h"
#include "wmt_exp.h"
#include "wmt_plat.h"
/* #include "wmt_lib.h" */

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define DFT_RTC_STABLE_TIME 100
#define DFT_LDO_STABLE_TIME 100
#define DFT_RST_STABLE_TIME 30
#define DFT_OFF_STABLE_TIME 10
#define DFT_ON_STABLE_TIME 30

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

PWR_SEQ_TIME gPwrSeqTime;

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
INT32 mtk_wcn_cmb_hw_pwr_off(VOID)
{
	INT32 iRet = 0;

	WMT_INFO_FUNC("CMB-HW, hw_pwr_off start\n");

/*1. disable irq --> should be done when do wmt-ic swDeinit period*/
/* TODO:[FixMe][GeorgeKuo] clarify this */

/*2. set bgf eint/all eint to deinit state, namely input low state*/
	iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);
	iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);
/* 2.1 set ALL_EINT pin to correct state even it is not used currently */
/* iRet += wmt_plat_eirq_ctrl(PIN_ALL_EINT, PIN_STA_DEINIT); */
/* iRet += wmt_plat_gpio_ctrl(PIN_ALL_EINT, PIN_STA_DEINIT); */

/* 2.2 deinit gps sync */
/* iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_DEINIT); */

/*3. set audio interface to CMB_STUB_AIF_0, BT PCM OFF, I2S OFF*/
	iRet += wmt_plat_audio_ctrl(CMB_STUB_AIF_0, CMB_STUB_AIF_CTRL_DIS);

/*4. set control gpio into deinit state, namely input low state*/
/* iRet += wmt_plat_gpio_ctrl(PIN_SDIO_GRP, PIN_STA_DEINIT); / * SDIO is configured by default * / */
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_L);
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_L);

/*5. set uart tx/rx into deinit state, namely input low state*/
	iRet += wmt_plat_gpio_ctrl(PIN_UART_GRP, PIN_STA_DEINIT);

#if 0
/*7. deinit gps_lna*/
	iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_DEINIT);
#endif

	WMT_INFO_FUNC("CMB-HW, hw_pwr_off finish\n");
	return iRet;
}

INT32 mtk_wcn_cmb_hw_pwr_on(VOID)
{
	INT32 iRet = 0;

	WMT_INFO_FUNC("CMB-HW, hw_pwr_on start\n");

/*set all control and eint gpio to init state, namely input low mode*/
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_INIT);
/* iRet += wmt_plat_gpio_ctrl(PIN_SDIO_GRP, PIN_STA_INIT); / * SDIO is configured by default * / */
	iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_INIT);
/* iRet += wmt_plat_gpio_ctrl(PIN_ALL_EINT, PIN_STA_INIT); */
/* iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_INIT); */
/* iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_INIT); */

#if 0				/* RTC source is enabled by default */
/* 2. export RTC clock to chip*/
	if (_pwr_first_time) {
/* rtc clock should be output all the time, so no need to enable output again*/
		iRet += wmt_plat_gpio_ctrl(PIN_RTC, PIN_STA_INIT);
		osal_msleep(gPwrSeqTime.rtcStableTime);
		WMT_INFO_FUNC("CMB-HW, rtc clock exported\n");
	}
#endif

/*3. set host platform's UART Tx/Rx to UART mode*/
	iRet += wmt_plat_gpio_ctrl(PIN_UART_GRP, PIN_STA_INIT);

/*4. PMU->output low, RST->output low, sleep off stable time*/
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_L);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_L);
	osal_msleep(gPwrSeqTime.offStableTime);

/*5. PMU->output high, sleep rst stable time*/
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_H);
	osal_msleep(gPwrSeqTime.rstStableTime);

/*6. RST->output high, sleep on stable time*/
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_H);
	osal_msleep(gPwrSeqTime.onStableTime);

/*7. set audio interface to CMB_STUB_AIF_1, BT PCM ON, I2S OFF*/
/* BT PCM bus default mode. Real control is done by audio */
	iRet += wmt_plat_audio_ctrl(CMB_STUB_AIF_1, CMB_STUB_AIF_CTRL_DIS);

/*8. set EINT< -ommited-> move this to WMT-IC module, where common sdio interface will be identified and do proper operation*/
	iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_MUX);
	iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_INIT);
	iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
	WMT_INFO_FUNC("CMB-HW, BGF_EINT IRQ registered and disabled \n");

#if 0
	iRet += wmt_plat_gpio_ctrl(PIN_WIFI_EINT, PIN_STA_MUX);
	iRet += wmt_plat_eirq_ctrl(PIN_WIFI_EINT, PIN_STA_INIT);
	iRet += wmt_plat_eirq_ctrl(PIN_WIFI_EINT, PIN_STA_EINT_DIS);
	WMT_INFO_FUNC("CMB-HW, WIFI_EINT IRQ registered and disabled \n");
#endif

#if 0
/* 8.1 set ALL_EINT pin to correct state even it is not used currently */
	iRet += wmt_plat_gpio_ctrl(PIN_ALL_EINT, PIN_STA_MUX);
	iRet += wmt_plat_eirq_ctrl(PIN_ALL_EINT, PIN_STA_INIT);
	iRet += wmt_plat_eirq_ctrl(PIN_ALL_EINT, PIN_STA_EINT_DIS);
	WMT_INFO_FUNC("CMB-HW, ALL_EINT IRQ registered and disabled \n");
#endif

	WMT_INFO_FUNC("CMB-HW, hw_pwr_on finish (%d)\n", iRet);

	return iRet;
}

INT32 mtk_wcn_cmb_hw_rst(VOID)
{
	INT32 iRet = 0;

	WMT_DBG_FUNC
	    ("CMB-HW, hw_rst start, eirq should be disabled before this step\n");

/*1. PMU->output low, RST->output low, sleep off stable time*/
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_L);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_L);
	osal_msleep(gPwrSeqTime.offStableTime);

/*2. PMU->output high, sleep rst stable time*/
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_H);
	osal_msleep(gPwrSeqTime.rstStableTime);

/*3. RST->output high, sleep on stable time*/
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_H);
	osal_msleep(gPwrSeqTime.onStableTime);
	WMT_DBG_FUNC
	    ("CMB-HW, hw_rst finish, eirq should be enabled after this step\n");
	return 0;
}

static VOID mtk_wcn_cmb_hw_dmp_seq(VOID)
{
	PUINT32 pTimeSlot = (PUINT32) & gPwrSeqTime;

	WMT_INFO_FUNC
	    ("combo chip power on sequence time, RTC (%d), LDO (%d), RST(%d), OFF(%d), ON(%d)\n",
	     pTimeSlot[0],
/**pTimeSlot++,*/
	     pTimeSlot[1], pTimeSlot[2], pTimeSlot[3], pTimeSlot[4]
	    );
	return;
}

INT32 mtk_wcn_cmb_hw_init(P_PWR_SEQ_TIME pPwrSeqTime)
{
	if (NULL != pPwrSeqTime &&
	    pPwrSeqTime->ldoStableTime > 0 &&
	    pPwrSeqTime->rtcStableTime > 0 &&
	    pPwrSeqTime->offStableTime > DFT_OFF_STABLE_TIME &&
	    pPwrSeqTime->onStableTime > DFT_ON_STABLE_TIME &&
	    pPwrSeqTime->rstStableTime > DFT_RST_STABLE_TIME) {
/*memcpy may be more performance*/
		WMT_DBG_FUNC("setting hw init sequence parameters\n");
		osal_memcpy(&gPwrSeqTime, pPwrSeqTime,
			    osal_sizeof(gPwrSeqTime));
	} else {
		WMT_INFO_FUNC("use default hw init sequence parameters\n");
		gPwrSeqTime.ldoStableTime = DFT_LDO_STABLE_TIME;
		gPwrSeqTime.offStableTime = DFT_OFF_STABLE_TIME;
		gPwrSeqTime.onStableTime = DFT_ON_STABLE_TIME;
		gPwrSeqTime.rstStableTime = DFT_RST_STABLE_TIME;
		gPwrSeqTime.rtcStableTime = DFT_RTC_STABLE_TIME;
	}
	mtk_wcn_cmb_hw_dmp_seq();
	return 0;
}

INT32 mtk_wcn_cmb_hw_deinit(VOID)
{
	WMT_WARN_FUNC
	    ("mtk_wcn_cmb_hw_deinit start, set to default hw init sequence parameters\n");
	gPwrSeqTime.ldoStableTime = DFT_LDO_STABLE_TIME;
	gPwrSeqTime.offStableTime = DFT_OFF_STABLE_TIME;
	gPwrSeqTime.onStableTime = DFT_ON_STABLE_TIME;
	gPwrSeqTime.rstStableTime = DFT_RST_STABLE_TIME;
	gPwrSeqTime.rtcStableTime = DFT_RTC_STABLE_TIME;
	WMT_WARN_FUNC("mtk_wcn_cmb_hw_deinit finish\n");
	return 0;
}
