#include "MFRC522.h"

static SPI_HandleTypeDef *_MFRC522_SPI;



void MFRC522_SPI_CS_low() {
	HAL_GPIO_WritePin(MFRC522_SPI_CS_GPIO_Port, MFRC522_SPI_CS_Pin,
			GPIO_PIN_RESET);
}
void MFRC522_SPI_CS_high() {
	HAL_GPIO_WritePin(MFRC522_SPI_CS_GPIO_Port, MFRC522_SPI_CS_Pin,
			GPIO_PIN_SET);
}

void MFRC522_writeRegister(uint8_t addr, uint8_t value) {
	MFRC522_SPI_CS_low();
	HAL_SPI_Transmit(_MFRC522_SPI, &addr, 1, 100);
	HAL_SPI_Transmit(_MFRC522_SPI, &value, 1, 100);
	MFRC522_SPI_CS_high();
}

uint8_t MFRC522_readRegister(uint8_t addr) {
	uint8_t rx_bits;
	uint8_t addr_bits = (((addr << 1) & 0x7E) | 0x80);

	MFRC522_SPI_CS_low();

	HAL_SPI_Transmit(_MFRC522_SPI, &addr_bits, 1, 500);

	HAL_SPI_Receive(_MFRC522_SPI, &rx_bits, 1, 500);
	MFRC522_SPI_CS_high();

	return (uint8_t) rx_bits;
}


void Write_MFRC522(uint8_t addr, uint8_t val) {
	uint8_t addr_bits = (((addr << 1) & 0x7E));

	MFRC522_writeRegister(addr_bits, val);
	return;

}

void SetBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp;
	tmp = MFRC522_readRegister(reg);
	Write_MFRC522(reg, tmp | mask);
}

void ClearBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp;
	tmp = MFRC522_readRegister(reg);
	Write_MFRC522(reg, tmp & (~mask));
}


void AntennaOn(void) {
	SetBitMask(TxControlReg, 0x03);
}


void AntennaOff(void) {
	ClearBitMask(TxControlReg, 0x03);
}


void MFRC522_Reset(void) {
	Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

void MFRC522_Init(SPI_HandleTypeDef *spi) {
	_MFRC522_SPI = spi;
	MFRC522_SPI_CS_high();
	MFRC522_Reset();

	Write_MFRC522(TModeReg, 0x80);
	Write_MFRC522(TPrescalerReg, 0xA9);
	Write_MFRC522(TReloadRegL, 0x03);
	Write_MFRC522(TReloadRegH, 0xE8);
	Write_MFRC522(TxAutoReg, 0x40);
	Write_MFRC522(ModeReg, 0x3D);

	AntennaOn();
}


uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType) {
	uint8_t status;
	uint8_t backBits;

	Write_MFRC522(BitFramingReg, 0x07);

	TagType[0] = reqMode;

	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
	if ((status != MI_OK) || (backBits != 0x10)) {
		status = MI_ERR;
	}

	return status;
}


uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen,
		uint8_t *backData, uint32_t *backLen) {
	uint8_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint32_t i;

	switch (command) {
	case PCD_AUTHENT:
	{
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	}
	case PCD_TRANSCEIVE:
	{
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}
	default:
		break;
	}

	Write_MFRC522(CommIEnReg, irqEn | 0x80);
	ClearBitMask(CommIrqReg, 0x80);
	SetBitMask(FIFOLevelReg, 0x80);

	Write_MFRC522(CommandReg, PCD_IDLE);

	for (i = 0; i < sendLen; i++) {
		Write_MFRC522(FIFODataReg, sendData[i]);
	}

	Write_MFRC522(CommandReg, command);
	if (command == PCD_TRANSCEIVE) {
		SetBitMask(BitFramingReg, 0x80);
	}
	i = 2000;
	do {
		n = MFRC522_readRegister(CommIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	ClearBitMask(BitFramingReg, 0x80);

	if (i != 0) {
		if (!(MFRC522_readRegister(ErrorReg) & 0x1B))
		{
			status = MI_OK;
			if (n & irqEn & 0x01) {
				status = MI_NOTAGERR;
			}

			if (command == PCD_TRANSCEIVE) {
				n = MFRC522_readRegister(FIFOLevelReg);
				lastBits = MFRC522_readRegister(ControlReg) & 0x07;
				if (lastBits) {
					*backLen = (n - 1) * 8 + lastBits;
				} else {
					*backLen = n * 8;
				}

				if (n == 0) {
					n = 1;
				}
				if (n > MAX_LEN) {
					n = MAX_LEN;
				}

				for (i = 0; i < n; i++) {
					backData[i] = MFRC522_readRegister(FIFODataReg);
				}
			}
		} else {

			status = MI_ERR;
		}
	} else {

	}

	return status;
}


uint8_t MFRC522_Anticoll(uint8_t *serNum) {
	uint8_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint32_t unLen;

	Write_MFRC522(BitFramingReg, 0x00);

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		for (i = 0; i < 4; i++) {
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {
			status = MI_ERR;
		}
	}



	return status;
}


uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData) {
	uint8_t status;
	uint32_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	CalulateCRC(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}

	return status;
}


uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData) {
	uint8_t status;
	uint32_t recvBits;
	uint8_t i;
	uint8_t buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	CalulateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK))
	{
		status = MI_ERR;
	}

	if (status == MI_OK) {
		for (i = 0; i < 16; i++)
				{
			buff[i] = *(writeData + i);
		}
		CalulateCRC(buff, 16, &buff[16]);
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK))
		{
			status = MI_ERR;
		}
	}

	return status;
}


void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData) {
	uint8_t i, n;

	ClearBitMask(DivIrqReg, 0x04);
	SetBitMask(FIFOLevelReg, 0x80);

	for (i = 0; i < len; i++) {
		Write_MFRC522(FIFODataReg, *(pIndata + i));
	}
	Write_MFRC522(CommandReg, PCD_CALCCRC);


	i = 0xFF;
	do {
		n = MFRC522_readRegister(DivIrqReg);
		i--;
	} while ((i != 0) && !(n & 0x04));


	pOutData[0] = MFRC522_readRegister(CRCResultRegL);
	pOutData[1] = MFRC522_readRegister(CRCResultRegM);
}


uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey,
		uint8_t *serNum) {
	uint8_t status;
	uint32_t recvBits;
	uint8_t i;
	uint8_t buff[12];

	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {
		buff[i + 2] = *(Sectorkey + i);
	}
	for (i = 0; i < 4; i++) {
		buff[i + 8] = *(serNum + i);
	}
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(MFRC522_readRegister(Status2Reg) & 0x08))) {
		status = MI_ERR;
	}

	return status;
}


uint8_t MFRC522_SelectTag(uint8_t *serNum) {
	uint8_t i;
	uint8_t status;
	uint8_t size;
	uint32_t recvBits;
	uint8_t buffer[9];

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i + 2] = *(serNum + i);
	}
	CalulateCRC(buffer, 7, &buffer[7]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {
		size = buffer[0];
	} else {
		size = 0;
	}

	return size;
}

void MFRC522_Halt(void) {
	uint8_t status;
	uint32_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	CalulateCRC(buff, 2, &buff[2]);

	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);

}
void MFRC522_StopCrypto1(void) {

	ClearBitMask(Status2Reg, 0x08);
} // End PCD_StopCrypto1()

