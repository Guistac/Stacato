#include <pch.h>

#include "Fieldbus/EtherCatDevice.h"

//======================= READING AND WRITING EEPROM DATA =========================

#define MINBUF 128
#define MAXBUF 524288
#define CRCBUF 14
uint8 ebuf[MAXBUF];

#define IHEXLENGTH 0x20

#define MAXSLENGTH        256
char sline[MAXSLENGTH];

int eeprom_read(int slave, int start, int length)
{
    int i, ainc = 4;
    uint16 estat, aiadr;
    uint32 b4;
    uint64 b8;
    uint8 eepctl;

    if ((ec_slavecount >= slave) && (slave > 0) && ((start + length) <= MAXBUF))
    {
        aiadr = 1 - slave;
        eepctl = 2;
        ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl, EC_TIMEOUTRET); /* force Eeprom from PDI */
        eepctl = 0;
        ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl, EC_TIMEOUTRET); /* set Eeprom to master */

        estat = 0x0000;
        aiadr = 1 - slave;
        ec_APRD(aiadr, ECT_REG_EEPSTAT, sizeof(estat), &estat, EC_TIMEOUTRET); /* read eeprom status */
        estat = etohs(estat);
        if (estat & EC_ESTAT_R64)
        {
            ainc = 8;
            for (i = start; i < (start + length); i += ainc)
            {
                b8 = ec_readeepromAP(aiadr, i >> 1, EC_TIMEOUTEEP);
                ebuf[i] = b8 & 0xFF;
                ebuf[i + 1] = (b8 >> 8) & 0xFF;
                ebuf[i + 2] = (b8 >> 16) & 0xFF;
                ebuf[i + 3] = (b8 >> 24) & 0xFF;
                ebuf[i + 4] = (b8 >> 32) & 0xFF;
                ebuf[i + 5] = (b8 >> 40) & 0xFF;
                ebuf[i + 6] = (b8 >> 48) & 0xFF;
                ebuf[i + 7] = (b8 >> 56) & 0xFF;
            }
        }
        else
        {
            for (i = start; i < (start + length); i += ainc)
            {
                b4 = ec_readeepromAP(aiadr, i >> 1, EC_TIMEOUTEEP) & 0xFFFFFFFF;
                ebuf[i] = b4 & 0xFF;
                ebuf[i + 1] = (b4 >> 8) & 0xFF;
                ebuf[i + 2] = (b4 >> 16) & 0xFF;
                ebuf[i + 3] = (b4 >> 24) & 0xFF;
            }
        }

        return 1;
    }

    return 0;
}

int output_bin(char* fname, int length)
{
    FILE* fp;

    int cc;

    fp = fopen(fname, "wb");
    if (fp == NULL)
        return 0;
    for (cc = 0; cc < length; cc++)
        fputc(ebuf[cc], fp);
    fclose(fp);

    return 1;
}


int input_bin(char* fname, int* length)
{
    FILE* fp;

    int cc = 0, c;

    fp = fopen(fname, "rb");
    if (fp == NULL)
        return 0;
    while (((c = fgetc(fp)) != EOF) && (cc < MAXBUF))
        ebuf[cc++] = (uint8)c;
    *length = cc;
    fclose(fp);

    return 1;
}


int eeprom_write(int slave, int start, int length)
{
    int i, dc = 0;
    uint16 aiadr, * wbuf;
    uint8 eepctl;

    if ((ec_slavecount >= slave) && (slave > 0) && ((start + length) <= MAXBUF))
    {
        aiadr = 1 - slave;
        eepctl = 2;
        ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl, EC_TIMEOUTRET); /* force Eeprom from PDI */
        eepctl = 0;
        ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl, EC_TIMEOUTRET); /* set Eeprom to master */

        aiadr = 1 - slave;
        wbuf = (uint16*)&ebuf[0];
        for (i = start; i < (start + length); i += 2)
        {
            ec_writeeepromAP(aiadr, i >> 1, *(wbuf + (i >> 1)), EC_TIMEOUTEEP);
            if (++dc >= 100)
            {
                dc = 0;
                printf(".");
                fflush(stdout);
            }
        }

        return 1;
    }

    return 0;
}


int eeprom_writealias(int slave, int alias, uint16 crc)
{
    uint16 aiadr;
    uint8 eepctl;
    int ret;

    if ((ec_slavecount >= slave) && (slave > 0) && (alias <= 0xffff)){
        aiadr = 1 - slave;
        eepctl = 2;
        ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl, EC_TIMEOUTRET); /* force Eeprom from PDI */
        eepctl = 0;
        ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl, EC_TIMEOUTRET); /* set Eeprom to master */
        ret = ec_writeeepromAP(aiadr, 0x04, alias, EC_TIMEOUTEEP);
        if (ret) ret = ec_writeeepromAP(aiadr, 0x07, crc, EC_TIMEOUTEEP);
        return ret;
    }

    return 0;
}

void calc_crc(uint8* crc, uint8 b)
{
    int j;
    *crc ^= b;
    for (j = 0; j <= 7; j++)
    {
        if (*crc & 0x80)
            *crc = (*crc << 1) ^ 0x07;
        else
            *crc = (*crc << 1);
    }
}

uint16 SIIcrc(uint8* buf)
{
    int i;
    uint8 crc;

    crc = 0xff;
    for (i = 0; i <= 13; i++)
    {
        calc_crc(&crc, *(buf++));
    }
    return (uint16)crc;
}


bool EtherCatDevice::downloadEEPROM(char* fileName) {
    eepromDownloadState = DataTransferState::State::TRANSFERRING;
    if (eeprom_read(getSlaveIndex(), 0x0000, MINBUF) != 1) { // read first 128 bytes
        eepromDownloadState = DataTransferState::State::FAILED;
        return false;
    }
    uint16* wbuf = (uint16*)&ebuf[0];   //cast to buffer of eeprom words (2 bytes)
    int eepromSize = (*(wbuf + 0x3E) + 1) * 128; //get eeprom size
    if (eepromSize > MAXBUF) eepromSize = MAXBUF;
    if (eepromSize > MINBUF) {
        if (eeprom_read(getSlaveIndex(), MINBUF, eepromSize - MINBUF) != 1) {
            eepromDownloadState = DataTransferState::State::FAILED;
            return false; // read reminder
        }
    }
    if (output_bin(fileName, eepromSize) != 1) {
        eepromDownloadState = DataTransferState::State::FAILED;
        return false;
    }
    eepromDownloadState = DataTransferState::State::SUCCEEDED;
    return true;
}

bool EtherCatDevice::flashEEPROM(char* fileName) {
    eepromFlashState = DataTransferState::State::TRANSFERRING;

    int estart = 0;
    int rc = 0;
    int esize;
    uint16* wbuf = (uint16*)&ebuf[0];   //cast to buffer of eeprom words (2 bytes)

    rc = input_bin(fileName, &esize);

    if (rc > 0) {
        wbuf = (uint16*)&ebuf[0];
        fflush(stdout);
        int result = eeprom_write(getSlaveIndex(), estart, esize);
        if (result == 1) {
            eepromFlashState = DataTransferState::State::SUCCEEDED;
            return true;
        }
        eepromFlashState = DataTransferState::State::FAILED;
        return false;
    }
    eepromFlashState = DataTransferState::State::FAILED;
    return false;
}

bool EtherCatDevice::setStationAlias(uint16_t alias) {
    stationAliasAssignState = DataTransferState::State::TRANSFERRING;
    if (eeprom_read(getSlaveIndex(), 0x0000, CRCBUF)) { // read first 14 bytes
        uint16* wbuf = (uint16*)&ebuf[0];
        *(wbuf + 0x04) = alias;
        if (eeprom_writealias(getSlaveIndex(), alias, SIIcrc(&ebuf[0]))) {
            stationAliasAssignState = DataTransferState::State::SUCCEEDED;
            return true;
        }
        stationAliasAssignState = DataTransferState::State::FAILED;
        return false;
    }
    stationAliasAssignState = DataTransferState::State::FAILED;
    return false;
}