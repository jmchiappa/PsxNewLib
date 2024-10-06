#include "PsxNewLib.h"
#include <SPI.h>
#include <DigitalIO.h>

/** \brief Attention Delay
 *
 * Time between attention being issued to the controller and the first clock
 * edge (us).
 */
const byte ATTN_DELAY = 50;

// Set up the speed, data order and data mode
static SPISettings spiSettings (250000, LSBFIRST, SPI_MODE3);

template <uint8_t PIN_ATT>
class PsxControllerHwSpi: public PsxController {
private:
	DigitalPin<PIN_ATT> att;
	DigitalPin<MOSI> cmd;
	DigitalPin<MISO> dat;
	DigitalPin<SCK> clk;
	SPIClass *spi_ = &SPI;
protected:
	virtual void attention () override {
		att.low ();

		spi_->beginTransaction (spiSettings);

		delayMicroseconds (ATTN_DELAY);
	}
	
	virtual void noAttention () override {
		//~ delayMicroseconds (5);
		
		spi_->endTransaction ();

		// Make sure CMD and CLK sit high
		cmd.high ();
		clk.high ();
		att.high ();
		delayMicroseconds (ATTN_DELAY);
	}
	
	virtual byte shiftInOut (const byte out) override {
		return spi_->transfer (out);
	}

public:
	virtual boolean begin () override{
		begin(SPI);
	}
	virtual boolean begin (SPIClass &spi) {
		*spi_ = spi;
		att.config (OUTPUT, HIGH);    // HIGH -> Controller not selected

		/* We need to force these at startup, that's why we need to know which
		 * pins are used for HW spi_-> It's a sort of "start condition" the
		 * controller needs.
		 */
		cmd.config (OUTPUT, HIGH);
		clk.config (OUTPUT, HIGH);
		dat.config (INPUT, HIGH);     // Enable pull-up

		spi_->begin ();

		return PsxController::begin ();
	}
};
