/**
 * \file
 *
 * \brief API driver for KSZ8081MNX PHY component.
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "ethernet_phy.h"

#include "gmac.h"
//#include "gmac_SAM.h"	// HICS

#include "conf_eth.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/**
 * \defgroup ksz8081mnx_ethernet_phy_group PHY component (KSZ8081MNX)
 *
 * Driver for the ksz8081mnx component. This driver provides access to the main
 * features of the PHY.
 *
 * \section dependencies Dependencies
 * This driver depends on the following modules:
 * - \ref gmac_group Ethernet Media Access Controller (GMAC) module.
 *
 * @{
 */

/* Max PHY number */
#define ETH_PHY_MAX_ADDR   31

/* Ethernet PHY operation max retry count */
#define ETH_PHY_RETRY_MAX 1000000

/* Ethernet PHY operation timeout */
#define ETH_PHY_TIMEOUT 10

/**
 * \brief Find a valid PHY Address ( from addrStart to 31 ).
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_phy_addr PHY address.
 * \param uc_start_addr Start address of the PHY to be searched.
 *
 * \return 0xFF when no valid PHY address is found.
 */
int ethernet_phy_addr = 0; // HICS: Para manter compatibilidade com FreeRTOS
static uint8_t ethernet_phy_find_valid(Gmac *p_gmac, uint8_t uc_phy_addr,
		uint8_t uc_start_addr)
{
	uint32_t ul_value = 0;
	uint8_t uc_rc = 0;
	uint8_t uc_cnt;
	uint8_t uc_phy_address = uc_phy_addr;

	gmac_enable_management(p_gmac, true);

	/* Check the current PHY address */
	gmac_phy_read(p_gmac, uc_phy_addr, GMII_PHYID1, &ul_value);

	/* Find another one */
	if (ul_value != GMII_OUI_LSB) {
		uc_rc = 0xFF;
		ethernet_phy_addr = 0xFF; // HICS: Para manter compatibilidade com FreeRTOS

		for (uc_cnt = uc_start_addr; uc_cnt <= ETH_PHY_MAX_ADDR; uc_cnt++) {
			uc_phy_address = (uc_phy_address + 1) & 0x1F;
			gmac_phy_read(p_gmac, uc_phy_address, GMII_PHYID1, &ul_value);
			if (ul_value == GMII_OUI_MSB) {
				uc_rc = uc_phy_address;
				ethernet_phy_addr = uc_phy_address; // HICS: Para manter compatibilidade com FreeRTOS
				break;
			}
		}
	}

	gmac_enable_management(p_gmac, false);

	if (uc_rc != 0xFF) {
		gmac_phy_read(p_gmac, uc_phy_address, GMII_BMSR, &ul_value);
	}
	return uc_rc;
}


/**
 * \brief Perform a HW initialization to the PHY and set up clocks.
 *
 * This should be called only once to initialize the PHY pre-settings.
 * The PHY address is the reset status of CRS, RXD[3:0] (the emacPins' pullups).
 * The COL pin is used to select MII mode on reset (pulled up for Reduced MII).
 * The RXDV pin is used to select test mode on reset (pulled up for test mode).
 * The above pins should be predefined for corresponding settings in resetPins.
 * The GMAC peripheral pins are configured after the reset is done.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_phy_addr PHY address.
 * \param ul_mck GMAC MCK.
 *
 * Return GMAC_OK if successfully, GMAC_TIMEOUT if timeout.
 */
uint8_t ethernet_phy_init(Gmac *p_gmac, uint8_t uc_phy_addr, uint32_t mck)
{
	uint8_t uc_rc;
	uint8_t uc_phy;

	ethernet_phy_reset(GMAC,uc_phy_addr);

	/* Configure GMAC runtime clock */
	uc_rc = gmac_set_mdc_clock(p_gmac, mck);
	if (uc_rc != GMAC_OK) {
		return 0;
	}

	/* Check PHY Address */
	uc_phy = ethernet_phy_find_valid(p_gmac, uc_phy_addr, 0);
	if (uc_phy == 0xFF) {
		return 0;
	}
	if (uc_phy != uc_phy_addr) {
		ethernet_phy_reset(p_gmac, uc_phy_addr);
	}

	return uc_rc;
}


/**
 * \brief Get the Link & speed settings, and automatically set up the GMAC with the
 * settings.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_phy_addr PHY address.
 * \param uc_apply_setting_flag Set to 0 to not apply the PHY configurations, else to apply.
 *
 * Return GMAC_OK if successfully, GMAC_TIMEOUT if timeout.
 */
uint8_t ethernet_phy_set_link(Gmac *p_gmac, uint8_t uc_phy_addr,
		uint8_t uc_apply_setting_flag)
{
	uint32_t ul_stat1;
	uint32_t ul_stat2;
	uint8_t uc_phy_address, uc_speed, uc_fd;
	uint8_t uc_rc;

	gmac_enable_management(p_gmac, true);

	uc_phy_address = uc_phy_addr;

	uc_rc = gmac_phy_read(p_gmac, uc_phy_address, GMII_BMSR, &ul_stat1);
	if (uc_rc != GMAC_OK) {
		/* Disable PHY management and start the GMAC transfer */
		gmac_enable_management(p_gmac, false);

		return uc_rc;
	}

	if ((ul_stat1 & GMII_LINK_STATUS) == 0) {
		/* Disable PHY management and start the GMAC transfer */
		gmac_enable_management(p_gmac, false);

		return GMAC_INVALID;
	}

	if (uc_apply_setting_flag == 0) {
		/* Disable PHY management and start the GMAC transfer */
		gmac_enable_management(p_gmac, false);

		return uc_rc;
	}

	/* Read advertisement */
	uc_rc = gmac_phy_read(p_gmac, uc_phy_address, GMII_PCR1, &ul_stat2);
	if (uc_rc != GMAC_OK) {
		/* Disable PHY management and start the GMAC transfer */
		gmac_enable_management(p_gmac, false);

		return uc_rc;
	}

	if ((ul_stat1 & GMII_100BASE_TX_FD) && (ul_stat2 & GMII_OMI_100BASE_TX_FD)) {
		/* Set GMAC for 100BaseTX and Full Duplex */
		uc_speed = true;
		uc_fd = true;
	}

	if ((ul_stat1 & GMII_10BASE_T_FD) && (ul_stat2 & GMII_OMI_10BASE_T_FD)) {
		/* Set MII for 10BaseT and Full Duplex */
		uc_speed = false;
		uc_fd = true;
	}

	if ((ul_stat1 & GMII_100BASE_TX_HD) && (ul_stat2 & GMII_OMI_100BASE_TX_HD)) {
		/* Set MII for 100BaseTX and Half Duplex */
		uc_speed = true;
		uc_fd = false;
	}

	if ((ul_stat1 & GMII_10BASE_T_HD) && (ul_stat2 & GMII_OMI_10BASE_T_HD)) {
		/* Set MII for 10BaseT and Half Duplex */
		uc_speed = false;
		uc_fd = false;
	}

	gmac_set_speed(p_gmac, uc_speed);
	gmac_enable_full_duplex(p_gmac, uc_fd);

	/* Start the GMAC transfers */
	gmac_enable_management(p_gmac, false);
	return uc_rc;
}


/**
 * \brief Issue an auto negotiation of the PHY.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_phy_addr PHY address.
 *
 * Return GMAC_OK if successfully, GMAC_TIMEOUT if timeout.
 */
uint8_t ethernet_phy_auto_negotiate(Gmac *p_gmac, uint8_t uc_phy_addr)
{
	uint32_t ul_retry_max = ETH_PHY_RETRY_MAX;
	uint32_t ul_value;
	uint32_t ul_phy_anar;
	uint32_t ul_phy_analpar;
	uint32_t ul_retry_count = 0;
	uint8_t uc_speed = 0;
	uint8_t uc_fd=0;
	uint8_t uc_rc;

	gmac_enable_management(p_gmac, true);

	/* Set up control register */
	uc_rc = gmac_phy_read(p_gmac, uc_phy_addr, GMII_BMCR, &ul_value);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}

	ul_value &= ~(uint32_t)GMII_AUTONEG; /* Remove auto-negotiation enable */
	ul_value &= ~(uint32_t)(GMII_LOOPBACK | GMII_POWER_DOWN);
	ul_value |= (uint32_t)GMII_ISOLATE; /* Electrically isolate PHY */
	uc_rc = gmac_phy_write(p_gmac, uc_phy_addr, GMII_BMCR, ul_value);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}

	/* 
	 * Set the Auto_negotiation Advertisement Register.
	 * MII advertising for Next page.
	 * 100BaseTxFD and HD, 10BaseTFD and HD, IEEE 802.3.
	 */
	ul_phy_anar = GMII_100TX_FDX | GMII_100TX_HDX | GMII_10_FDX | GMII_10_HDX | 
			GMII_AN_IEEE_802_3;
	uc_rc = gmac_phy_write(p_gmac, uc_phy_addr, GMII_ANAR, ul_phy_anar);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}

	/* Read & modify control register */
	uc_rc = gmac_phy_read(p_gmac, uc_phy_addr, GMII_BMCR, &ul_value);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}

	ul_value |= GMII_SPEED_SELECT | GMII_AUTONEG | GMII_DUPLEX_MODE;
	uc_rc = gmac_phy_write(p_gmac, uc_phy_addr, GMII_BMCR, ul_value);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}

	/* Restart auto negotiation */
	ul_value |= (uint32_t)GMII_RESTART_AUTONEG;
	ul_value &= ~(uint32_t)GMII_ISOLATE;
	uc_rc = gmac_phy_write(p_gmac, uc_phy_addr, GMII_BMCR, ul_value);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}

	/* Check if auto negotiation is completed */
	while (1) {
		uc_rc = gmac_phy_read(p_gmac, uc_phy_addr, GMII_BMSR, &ul_value);
		if (uc_rc != GMAC_OK) {
			gmac_enable_management(p_gmac, false);
			return uc_rc;
		}
		/* Done successfully */
		if (ul_value & GMII_AUTONEG_COMP) {
			break;
		}

		/* Timeout check */
		if (ul_retry_max) {
			if (++ul_retry_count >= ul_retry_max) {
				gmac_enable_management(p_gmac, false);
				return GMAC_TIMEOUT;
			}
		}
	}

	/* Get the auto negotiate link partner base page */
	uc_rc = gmac_phy_read(p_gmac, uc_phy_addr, GMII_ANLPAR, &ul_phy_analpar);
	if (uc_rc != GMAC_OK) {
		gmac_enable_management(p_gmac, false);
		return uc_rc;
	}


	/* Set up the GMAC link speed */
	if ((ul_phy_anar & ul_phy_analpar) & GMII_100TX_FDX) {
		/* Set MII for 100BaseTX and Full Duplex */
		uc_speed = true;
		uc_fd = true;
	} else if ((ul_phy_anar & ul_phy_analpar) & GMII_10_FDX) {
		/* Set MII for 10BaseT and Full Duplex */
		uc_speed = false;
		uc_fd = true;
	} else if ((ul_phy_anar & ul_phy_analpar) & GMII_100TX_HDX) {
		/* Set MII for 100BaseTX and half Duplex */
		uc_speed = true;
		uc_fd = false;
	} else if ((ul_phy_anar & ul_phy_analpar) & GMII_10_HDX) {
		/* Set MII for 10BaseT and half Duplex */
		uc_speed = false;
		uc_fd = false;
	}

	gmac_set_speed(p_gmac, uc_speed);
	gmac_enable_full_duplex(p_gmac, uc_fd);

	/* Select Media Independent Interface type */
	gmac_select_mii_mode(p_gmac, ETH_PHY_MODE);

	gmac_enable_transmit(GMAC, true);
	gmac_enable_receive(GMAC, true);

	gmac_enable_management(p_gmac, false);
	return uc_rc;
}

/**
 * \brief Issue a SW reset to reset all registers of the PHY.
 *
 * \param p_gmac   Pointer to the GMAC instance.
 * \param uc_phy_addr PHY address.
 *
 * \Return GMAC_OK if successfully, GMAC_TIMEOUT if timeout.
 */
uint8_t ethernet_phy_reset(Gmac *p_gmac, uint8_t uc_phy_addr)
{
	uint32_t ul_bmcr;
	uint8_t uc_phy_address = uc_phy_addr;
	uint32_t ul_timeout = ETH_PHY_TIMEOUT;
	uint8_t uc_rc = GMAC_TIMEOUT;

	gmac_enable_management(p_gmac, true);

	ul_bmcr = GMII_RESET;
	gmac_phy_write(p_gmac, uc_phy_address, GMII_BMCR, ul_bmcr);

	do {
		gmac_phy_read(p_gmac, uc_phy_address, GMII_BMCR, &ul_bmcr);
		ul_timeout--;
	} while ((ul_bmcr & GMII_RESET) && ul_timeout);

	gmac_enable_management(p_gmac, false);

	if (!ul_timeout) {
		uc_rc = GMAC_OK;
	}

	return (uc_rc);
}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond

/**
 * \}
 */
