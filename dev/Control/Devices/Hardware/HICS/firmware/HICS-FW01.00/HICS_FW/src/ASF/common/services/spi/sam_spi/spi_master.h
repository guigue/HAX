/**
 * \file
 *
 * \brief SPI master common service for SAM.
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef _SPI_MASTER_H_
#define _SPI_MASTER_H_

#include "compiler.h"
#include "sysclk.h"
#include "status_codes.h"
#include "spi.h"

/*! \name SPI Master Management Configuration */
//! @{
#include "conf_spi_master.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

//! Default Configuration of SPI Master Delay BCS
#ifndef CONFIG_SPI_MASTER_DELAY_BCS
#define CONFIG_SPI_MASTER_DELAY_BCS          0
#endif

//! Default Configuration of SPI Master Bits per Transfer Definition
#ifndef CONFIG_SPI_MASTER_BITS_PER_TRANSFER
#define CONFIG_SPI_MASTER_BITS_PER_TRANSFER  SPI_CSR_BITS_8_BIT
#endif

//! Default Configuration of SPI Master Delay BCT
#ifndef CONFIG_SPI_MASTER_DELAY_BCT
#define CONFIG_SPI_MASTER_DELAY_BCT          0
#endif

//! Default Configuration of SPI Master Delay BS
#ifndef CONFIG_SPI_MASTER_DELAY_BS
#define CONFIG_SPI_MASTER_DELAY_BS           0
#endif

//! Default Configuration of SPI Master Dummy Field
#ifndef CONFIG_SPI_MASTER_DUMMY
//#define CONFIG_SPI_MASTER_DUMMY              0xFF
#define CONFIG_SPI_MASTER_DUMMY              0x80 // HICS: Keeping the SDI pin high during 64 consecutives clocks generates a software reset
#endif
//! @}

/**
 * \brief Clock phase.
 */
#define SPI_CPHA  (1 << 0)

/**
 * \brief Clock polarity.
 */
#define SPI_CPOL  (1 << 1)

/**
 * \brief SPI mode 0.
 */
#define SPI_MODE_0  0
/**
 * \brief SPI mode 1.
 */
#define SPI_MODE_1  (SPI_CPHA)
/**
 * \brief SPI mode 2.
 */
#define SPI_MODE_2  (SPI_CPOL)
/**
 * \brief SPI mode 3.
 */
#define SPI_MODE_3  (SPI_CPOL | SPI_CPHA)

#ifndef SPI_TYPE_DEFS
#define SPI_TYPE_DEFS
//! SPI Flags Definition
typedef uint8_t spi_flags_t;

//! Board SPI Select Id Definition
typedef uint32_t board_spi_select_id_t;
#endif

//! \brief Polled SPI device definition.
struct spi_device {
	//! Board specific select id
	board_spi_select_id_t id;
};

/** \brief Initialize the SPI in master mode.
 *
 * \param p_spi Base address of the SPI instance.
 *
 */
extern void spi_master_init(Spi *p_spi);

/**
 * \brief Set up an SPI device.
 *
 * The returned device descriptor structure must be passed to the driver
 * whenever that device should be used as current slave device.
 *
 * \param p_spi     Base address of the SPI instance.
 * \param device    Pointer to SPI device struct that should be initialized.
 * \param flags     SPI configuration flags. Common flags for all
 *                  implementations are the SPI modes SPI_MODE_0 ...
 *                  SPI_MODE_3.
 * \param baud_rate Baud rate for communication with slave device in Hz.
 * \param sel_id    Board specific select id.
 */
extern void spi_master_setup_device(Spi *p_spi, struct spi_device *device,
		spi_flags_t flags, uint32_t baud_rate, board_spi_select_id_t sel_id);

/**
 * \brief Select the given device on the SPI bus.
 *
 * Set device specific setting and call board chip select.
 *
 * \param p_spi  Base address of the SPI instance.
 * \param device SPI device.
 *
 */
extern void spi_select_device(Spi *p_spi, struct spi_device *device);

/**
 * \brief Deselect the given device on the SPI bus.
 *
 * Call board chip deselect.
 *
 * \param p_spi  Base address of the SPI instance.
 * \param device SPI device.
 *
 * \pre SPI device must be selected with spi_select_device() first.
 */
extern void spi_deselect_device(Spi *p_spi, struct spi_device *device);


/** \brief Write one byte to an SPI device.
 *
 * \param p_spi     Base address of the SPI instance.
 * \param data      Data to write.
 *
 */
static inline void spi_write_single(Spi *p_spi, uint8_t data)
{
	spi_put(p_spi, (uint16_t)data);
}

/**
 * \brief Send a sequence of bytes to an SPI device.
 *
 * Received bytes on the SPI bus are discarded.
 *
 * \param p_spi     Base address of the SPI instance.
 * \param data      Data buffer to write.
 * \param len       Length of data to be written.
 *
 * \pre SPI device must be selected with spi_select_device() first.
 */
extern status_code_t spi_write_packet(Spi *p_spi,
		const uint8_t *data, size_t len);

/** \brief Receive one byte from an SPI device.
 *
 * \param p_spi     Base address of the SPI instance.
 * \param data      Data to read.
 *
 */
static inline void spi_read_single(Spi *p_spi, uint8_t *data)
{
	*data = (uint8_t)spi_get(p_spi);
}

/**
 * \brief Receive a sequence of bytes from an SPI device.
 *
 * All bytes sent out on SPI bus are sent as value 0xff.
 *
 * \param p_spi     Base address of the SPI instance.
 * \param data      Data buffer to read.
 * \param len       Length of data to be read.
 *
 * \pre SPI device must be selected with spi_select_device() first.
 */
extern status_code_t spi_read_packet(Spi *p_spi, uint8_t *data, size_t len);

/**
 * \brief Send and receive a sequence of bytes from an SPI device.
 *
 * \param p_spi     Base address of the SPI instance.
 * \param tx_data   Data buffer to send.
 * \param rx_data   Data buffer to read.
 * \param len       Length of data to be read.
 *
 * \pre SPI device must be selected with spi_select_device() first.
 */
extern status_code_t spi_transceive_packet(Spi *p_spi, uint8_t *tx_data, uint8_t *rx_data, size_t len);

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond

#endif // _SPI_MASTER_H_
