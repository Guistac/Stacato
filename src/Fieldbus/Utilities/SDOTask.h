#pragma once

class SDOTask{
public:
	
	template<typename T>
	static std::shared_ptr<SDOTask> prepareUpload(uint16_t index, uint8_t subindex, T data, std::string taskName){
		auto newTask = std::make_shared<SDOTask>();
		newTask->setData(data);
		newTask->direction = Direction::UPLOAD;
		newTask->index_ = index;
		newTask->subindex_ = subindex;
		newTask->taskName_ = taskName;
		return newTask;
	}
	
	static std::shared_ptr<SDOTask> prepareDownload(uint16_t index, uint8_t subindex, std::string taskName,
													std::function<void(SDOTask& thisTask)> downloadSuccessCallback = [](SDOTask&){}){
		auto newTask = std::make_shared<SDOTask>();
		newTask->direction = Direction::DOWNLOAD;
		newTask->index_ = index;
		newTask->subindex_ = subindex;
		newTask->taskName_ = taskName;
		newTask->downloadSuccessCallback_ = downloadSuccessCallback;
		return newTask;
	}
	
	bool execute(std::shared_ptr<EtherCatDevice> device){
		bool success = true;
		switch(direction){
			case Direction::UPLOAD:
				success = upload(device);
				if(success) Logger::trace("[{}] Uploaded PDO {}", device->getName(), taskName_);
				else Logger::warn("[{}] Failed to upload PDO {}", device->getName(), taskName_);
				break;
			case Direction::DOWNLOAD:
				success = download(device);
				if(success) Logger::trace("[{}] Downloaded PDO {}", device->getName(), taskName_);
				else Logger::warn("[{}] Failed to download PDO {}", device->getName(), taskName_);
				break;
		}
		return success;
	}
	
	bool upload(std::shared_ptr<EtherCatDevice> device){
		switch(dataType){
			case DataType::UNSIGNED_8:	return device->writeSDO_U8(index_, subindex_, u8);
			case DataType::SIGNED_8:	return device->writeSDO_S8(index_, subindex_, s8);
			case DataType::UNSIGNED_16:	return device->writeSDO_U16(index_, subindex_, u16);
			case DataType::SIGNED_16:	return device->writeSDO_S16(index_, subindex_, s16);
			case DataType::UNSIGNED_32:	return device->writeSDO_U32(index_, subindex_, u32);
			case DataType::SIGNED_32:	return device->writeSDO_S32(index_, subindex_, s32);
			case DataType::UNSIGNED_64:	return device->writeSDO_U64(index_, subindex_, u64);
			case DataType::SIGNED_64:	return device->writeSDO_S64(index_, subindex_, s64);
		}
	}
	bool download(std::shared_ptr<EtherCatDevice> device){
		switch(dataType){
			case DataType::UNSIGNED_8:	return device->readSDO_U8(index_, subindex_, u8);
			case DataType::SIGNED_8:	return device->readSDO_S8(index_, subindex_, s8);
			case DataType::UNSIGNED_16:	return device->readSDO_U16(index_, subindex_, u16);
			case DataType::SIGNED_16:	return device->readSDO_S16(index_, subindex_, s16);
			case DataType::UNSIGNED_32:	return device->readSDO_U32(index_, subindex_, u32);
			case DataType::SIGNED_32:	return device->readSDO_S32(index_, subindex_, s32);
			case DataType::UNSIGNED_64:	return device->readSDO_U64(index_, subindex_, u64);
			case DataType::SIGNED_64:	return device->readSDO_S64(index_, subindex_, s64);
		}
	}
	
	void onSuccess(){
		if(direction == Direction::DOWNLOAD){
			downloadSuccessCallback_(*this);
		}
	}
	
	uint8_t getU8(){ return u8; }
	int8_t getS8(){ return s8; }
	uint16_t getU16(){ return u16; }
	int16_t getS16(){ return s16; }
	uint32_t getU32(){ return u32; }
	int32_t getS32(){ return s32; }
	uint64_t getU64(){ return u64; }
	int64_t getS64(){ return s64; }
	
	
private:
	
	enum class Direction{
		UPLOAD,
		DOWNLOAD
	};
	
	enum class DataType{
		UNSIGNED_8,
		SIGNED_8,
		UNSIGNED_16,
		SIGNED_16,
		UNSIGNED_32,
		SIGNED_32,
		UNSIGNED_64,
		SIGNED_64,
	};
	
	template<typename T>
	void setData(T data);
	
	std::string taskName_ = "";
	uint16_t index_;
	uint8_t subindex_;
	DataType dataType;
	Direction direction;
	std::function<void(SDOTask&)> downloadSuccessCallback_ = [](SDOTask& task){};
	union{
		uint8_t u8;
		int8_t s8;
		uint16_t u16;
		int16_t s16;
		uint32_t u32;
		int32_t s32;
		uint64_t u64;
		int64_t s64;
	};
};

template<>
inline void SDOTask::setData(uint8_t data){
	dataType = DataType::UNSIGNED_8;
	u8 = data;
}

template<>
inline void SDOTask::setData(int8_t data){
	dataType = DataType::SIGNED_8;
	s8 = data;
}

template<>
inline void SDOTask::setData(uint16_t data){
	dataType = DataType::UNSIGNED_16;
	u16 = data;
}

template<>
inline void SDOTask::setData(int16_t data){
	dataType = DataType::SIGNED_16;
	s16 = data;
}

template<>
inline void SDOTask::setData(uint32_t data){
	dataType = DataType::UNSIGNED_32;
	u32 = data;
}

template<>
inline void SDOTask::setData(int32_t data){
	dataType = DataType::SIGNED_32;
	s32 = data;
}

template<>
inline void SDOTask::setData(uint64_t data){
	dataType = DataType::UNSIGNED_64;
	u64 = data;
}

template<>
inline void SDOTask::setData(int64_t data){
	dataType = DataType::SIGNED_64;
	s64 = data;
}
