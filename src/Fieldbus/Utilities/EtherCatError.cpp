#include <pch.h>

#include <ethercat.h>
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

namespace EtherCatError {

	bool hasError() {
		return ec_iserror();
	}

	void logError() {
		ec_errort error;
		ec_poperror(&error);
		std::shared_ptr<EtherCatSlave> errorSlave = nullptr;
		if (error.Slave <= EtherCatFieldbus::slaves.size() && error.Slave > 0) errorSlave = EtherCatFieldbus::slaves[error.Slave - 1];
		const char* slaveName;
		if (errorSlave != nullptr) slaveName = errorSlave->getName();
		else slaveName = "";
		const char* errorString;

		switch (error.Etype) {
			case EC_ERR_TYPE_SDO_ERROR:
				errorString = ec_sdoerror2string(error.AbortCode);
				Logger::debug("SDO Error: Slave '{}' 0x{:X}:{:X} {}", slaveName, error.Index, error.SubIdx, errorString); break;
			case EC_ERR_TYPE_PACKET_ERROR:
				switch (error.AbortCode) {
					case 1: errorString = "Unexpected Frame Returned"; break;
					case 3: errorString = "Data Container too small for type"; break;
					case 10: errorString = "SM larger than EC_MAXSM"; break;
					default: "Unknown Abort Code"; break;
				}
				Logger::warn("PACKET Error: Slave '{}' 0x{:X}:{:X} AbortCode: {:X} {}", slaveName, error.Index, error.SubIdx, error.AbortCode, errorString); break;
			case EC_ERR_TYPE_SDOINFO_ERROR:
				errorString = ec_sdoerror2string(error.AbortCode);
				Logger::debug("SDO-INFO Error: Slave '{}' 0x{:X}:{:X} {}", slaveName, error.Index, error.SubIdx, errorString); break;
            case EC_ERR_TYPE_EMERGENCY:
                Logger::error("EMERGENCY Error: Slave '{}' ErrorCode:{:X}", slaveName, error.ErrorCode); break;
            case EC_ERR_TYPE_SOE_ERROR:
				switch (error.AbortCode) {
					case 1: errorString = "Unexpected Frame Returned"; break;
					case 4: errorString = "No Response"; break;
					default: "Unknown Abort Code"; break;
				}
				errorString = ec_soeerror2string(error.ErrorCode);
                Logger::warn("SOE Error: Slave '{}' AbortCode: {:X} Error: {}", slaveName, error.AbortCode, errorString); break;
            case EC_ERR_TYPE_MBX_ERROR:
				errorString = ec_mbxerror2string(error.ErrorCode);
                Logger::warn("MAILBOX Error: Slave '{}' Error: {}", slaveName, errorString); break;
            default:
                Logger::critical("UNKNOWN ERROR: AbortCode: {:X}", error.AbortCode);
		}
	}
}
