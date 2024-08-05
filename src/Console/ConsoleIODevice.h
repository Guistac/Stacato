#pragma once

class PushButton;
class Joystick2X;
class Joystick3X;
class LED;
class LED_PWM;
class LED_RGB;
class LED_Button;
class LED_PWM_Button;
class LED_RGB_Button;

class IODevice : public std::enable_shared_from_this<IODevice>{
public:
	
	enum class Type{
		PUSHBUTTON, 	//OK
		SWITCH,
		POTENTIOMETER,
		ENCODER,
		JOYSTICK_1AXIS,
		JOYSTICK_2AXIS,	//OK
		JOYSTICK_3AXIS,
		LED,			//OK
		LED_PWM,		//OK
		LED_RGB,		//OK
		LED_BUTTON,		//OK
		LED_PWM_BUTTON,	//OK
		LED_RGB_BUTTON,	//OK
		UNKNOWN
	};
	
	static uint8_t getCodeFromType(Type deviceType);
	static Type getTypeFromCode(uint8_t deviceTypeCode);
	static const char* getTypeString(Type deviceType);
	
	static std::shared_ptr<IODevice> make(Type deviceType);
	
	std::shared_ptr<PushButton>		toPushButton();
	std::shared_ptr<Joystick2X>		toJoystick2X();
	std::shared_ptr<Joystick3X>		toJoystick3X();
	std::shared_ptr<LED>			toLED();
	std::shared_ptr<LED_PWM>		toLED_PWM();
	std::shared_ptr<LED_RGB>		toLED_RGB();
	std::shared_ptr<LED_Button>		toLED_Button();
	std::shared_ptr<LED_PWM_Button>	toLED_PWM_Button();
	std::shared_ptr<LED_RGB_Button>	toLED_RGB_Button();
	
	typedef std::function<void()> IOUpdateCallback;
	void setInputUpdateCallback(IOUpdateCallback callback){ inputUpdateCallback = callback; }
	void setOutputUpdateCallback(IOUpdateCallback callback){ outputUpdateCallback = callback; }
	
	void updateInput(uint8_t* data, int size){
		if(readInput(data, size)){
			inputUpdateCallback();
		}
	}
	
	bool updateOutput(uint8_t* data, int* size){
		outputUpdateCallback();
		if(outputChanged()) {
			writeOutput(data, size);
			return true;
		}
		return false;
	}
	
	virtual Type getType() = 0;
	
	virtual bool readInput(uint8_t* data, int size) { return false; }
	virtual bool outputChanged() { return false; }
	virtual void writeOutput(uint8_t* data, int* size) {}
	
protected:
	IOUpdateCallback inputUpdateCallback = [](){};
	IOUpdateCallback outputUpdateCallback = [](){};
};





class PushButton : public virtual IODevice{
public:
	
	virtual Type getType() override { return Type::PUSHBUTTON; }
	
	virtual bool readInput(uint8_t* data, int size) override {
		if(size != 1) return false;
		bool b_newState;
		switch(data[0]){
			case 0: b_newState = false; break;
			case 1: b_newState = true; break;
			default: return false;
		}
		if(b_newState != b_isPressed){
			b_isPressed = b_newState;
			return true;
		}
		return false;
	}
	
	bool isPressed(){ return b_isPressed; }
	
private:
	bool b_isPressed = false;
};

class Joystick2X : public virtual IODevice{
public:
	
	virtual Type getType() override { return Type::JOYSTICK_2AXIS; }
	
	virtual bool readInput(uint8_t* data, int size) override {
		if(size != 2) return false;
		glm::vec2 newPosition((int8_t)data[0] / 127.f, (int8_t)data[1] / 127.f);
		if(position != newPosition){
			position = newPosition;
			return true;
		}
		return false;
	}
	
	glm::vec2 getPosition(){ return position; }
	
private:
	glm::vec2 position = {.0f, .0f};
};

class Joystick3X : public virtual IODevice{
public:
	
	virtual Type getType() override { return Type::JOYSTICK_3AXIS; }
	
	virtual bool readInput(uint8_t* data, int size) override {
		if(size != 3) return false;
		glm::vec3 newPosition((int8_t)data[0] / 127.f, (int8_t)data[1] / 127.f, (int8_t)data[2] / 127.f);
		if(position != newPosition){
			position = newPosition;
			return true;
		}
		return false;
	}
	
	glm::vec3 getPosition(){ return position; }
	
private:
	glm::vec3 position = {.0f, .0f, .0f};
	
};





class LED : public virtual IODevice{
public:
	
	virtual Type getType() override { return Type::LED; }
	
	virtual bool outputChanged() override {
		return b_state != b_previousState;
	};
	
	virtual void writeOutput(uint8_t* data, int* size) override {
		b_previousState = b_state;
		data[0] = b_state;
		*size = 1;
	}
	
	virtual void setState(bool state){ b_state = state; }
	
private:
	bool b_state = false;
	bool b_previousState = false;
	uint8_t outputData[1];
};

class LED_PWM : public virtual IODevice{
public:
	
	virtual Type getType() override { return Type::LED_PWM; }
	
	virtual bool outputChanged() override {
		return f_brightness != f_previousBrightness;
	};
	
	virtual void writeOutput(uint8_t* data, int* size) override {
		f_previousBrightness = f_brightness;
		data[0] = f_brightness * 255.f;
		*size = 1;
	}
	
	void setBrightness(float brightness){ f_brightness = brightness; }
	
private:
	float f_brightness = 0.f;
	float f_previousBrightness = 0.f;
	uint8_t outputData[1];
};

class LED_RGB : public virtual IODevice{
public:
	
	virtual Type getType() override { return Type::LED_RGB; }
	
	virtual bool outputChanged() override {
		return v_color != v_previousColor;
	};
	
	virtual void writeOutput(uint8_t* data, int* size) override {
		v_previousColor = v_color;
		data[0] = v_color.x * 255.f;
		data[1] = v_color.y * 255.f;
		data[2] = v_color.z * 255.f;
		*size = 3;
	}
	
	void setColor(glm::vec3 color){ v_color = color; }
	
private:
	glm::vec3 v_color;
	glm::vec3 v_previousColor;
	uint8_t outputData[3];
};


class LED_Button : public LED, public PushButton{
public:
	virtual Type getType() override { return Type::LED_BUTTON; }
};


class LED_PWM_Button : public LED_PWM, public PushButton{
public:
	virtual Type getType() override { return Type::LED_PWM_BUTTON; }
};


class LED_RGB_Button : public LED_RGB, public PushButton{
public:
	virtual Type getType() override { return Type::LED_RGB_BUTTON; }
};
