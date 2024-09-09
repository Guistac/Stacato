#include "AX5206.h"
#include "Fieldbus/EtherCatFieldbus.h"

void AX5206::onDisconnection(){
	axis0->state = DeviceState::OFFLINE;
	axis1->state = DeviceState::OFFLINE;
	gpio->state = DeviceState::OFFLINE;
}

void AX5206::onConnection(){
	gpio->state = DeviceState::ENABLED;
}

void AX5206::initialize(){
	auto thisAX5206 = std::static_pointer_cast<AX5206>(shared_from_this());
	axis0 = std::make_shared<Axis>(thisAX5206, "Actuator 1", 0);
	axis1 = std::make_shared<Axis>(thisAX5206, "Actuator 2", 1);
	gpio = std::make_shared<Gpio>(thisAX5206, "Gpio");
	
	auto initializeAxis = [](std::shared_ptr<Axis> servo){
		auto& fbcfg = servo->feedbackConfig;
		fbcfg.b_supportsForceFeedback = false;
		fbcfg.b_supportsPositionFeedback = true;
		fbcfg.b_supportsVelocityFeedback = true;
		fbcfg.positionFeedbackType = PositionFeedbackType::ABSOLUTE;
		fbcfg.positionLowerWorkingRangeBound = 0;
		fbcfg.positionUpperWorkingRangeBound = 4096.0;
		auto& acfg = servo->actuatorConfig;
		acfg.b_supportsEffortFeedback = true;
		acfg.b_supportsForceControl = false;
		acfg.b_supportsPositionControl = false;
		acfg.b_supportsVelocityControl = true;
		acfg.b_supportsHoldingBrakeControl = false;
		acfg.b_canQuickstop = false;
		acfg.velocityLimit = 90.0;
		acfg.accelerationLimit = 500.0;
		acfg.decelerationLimit = 500.0;
		acfg.followingErrorLimit = 0.0;
		acfg.forceLimitNegative = 0.0;
		acfg.forceLimitPositive = 0.0;
	};
	
	initializeAxis(axis0);
	initializeAxis(axis1);
	
	actuator0Pin->assignData(std::static_pointer_cast<ActuatorInterface>(axis0));
	actuator1Pin->assignData(std::static_pointer_cast<ActuatorInterface>(axis1));
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	addNodePin(actuator0Pin);
	addNodePin(actuator1Pin);
	addNodePin(gpioPin);
	addNodePin(STO_pin);
	addNodePin(digitalInput0_pin);
	addNodePin(digitalInput1_pin);
	addNodePin(digitalInput2_pin);
	addNodePin(digitalInput3_pin);
	addNodePin(digitalInput4_pin);
	addNodePin(digitalInput5_pin);
	addNodePin(digitalInput6_pin);
	addNodePin(digitalOutput7_pin);
}

bool AX5206::startupConfiguration() {
	
	struct IDN_List{
		uint16_t size = 0; //total bytes in idn list (idn count * 2)
		uint16_t maxLength = 0; //max length of array in slave, not useful when uploading
		uint16_t IDNs[10] = {0};
		uint8_t* getPointer(){ return (uint8_t*)this; }
	};
	
	//Master Data Telegram (MDT) [Master->Slave]
	IDN_List axis0_mdtList;
	axis0_mdtList.size = 6;
	//axis0_mdtList.IDNs[0] = 134; 	//(2) Master Control Word		[FIXED]
	axis0_mdtList.IDNs[0] = 47; 	//(4) Position Command Value
	axis0_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value
	axis0_mdtList.IDNs[2] = 33570;	//(2) Digital Outputs
	
	IDN_List axis1_mdtList;
	axis1_mdtList.size = 4;
	//axis1_mdtList.IDNs[0] = 134; 	//(2) Master Control Word		[FIXED]
	axis1_mdtList.IDNs[0] = 47; 	//(4) Position Command Value
	axis1_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value

	//Acknowledge Telegram (AT) [Slave->Master]
	IDN_List axis0_atList;
	axis0_atList.size = 12;
	//axis0_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis0_atList.IDNs[0] = 51;		//(4) Position Feedback Value 1 (motor feedback)
	axis0_atList.IDNs[1] = 40;		//(4) Velocity Feedback Value 1
	axis0_atList.IDNs[2] = 84;		//(2) Torque Feedback Value
	axis0_atList.IDNs[3] = 11;		//(2) Class 1 Diagnostics
	axis0_atList.IDNs[4] = 33569; 	//(2) Digital Inputs, state
	axis0_atList.IDNs[5] = 34770; 	//(2) Safety option state
	
	IDN_List axis1_atList;
	axis1_atList.size = 8;
	//axis1_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis1_atList.IDNs[0] = 51;		//(4) Position Feedback Value 1 (motor feedback)
	axis1_atList.IDNs[1] = 40;		//(4) Velocity Feedback Value 1
	axis1_atList.IDNs[2] = 84;		//(2) Torque Feedback Value
	axis1_atList.IDNs[3] = 11;		//(2) Class 1 Diagnostics
	
	//========== STARTUP LIST ==========
	
	//upload process data configuration
	writeSercos_Array('S', 16, axis0_atList.getPointer(), sizeof(IDN_List), 0); //AT List [axis0]
	writeSercos_Array('S', 24, axis0_mdtList.getPointer(), sizeof(IDN_List), 0); //MDT List [axis0]
	writeSercos_Array('S', 16, axis1_atList.getPointer(), sizeof(IDN_List), 1); //AT List [axis1]
	writeSercos_Array('S', 24, axis1_mdtList.getPointer(), sizeof(IDN_List), 1); //MDT List [axis1]
	
	writeSercos_U64('P', 10, 0x1FBD, 0);
	writeSercos_U64('P', 10, 0x1FBD, 1);
	
	uploadMotorConfiguration(0, axis0->getMotorType());
	uploadMotorConfiguration(1, axis1->getMotorType());
	
	writeSercos_U16('S', 32, 2, 0); //Operation mode 0 VEL
	writeSercos_U16('S', 32, 2, 1); //Operation mode 0 VEL
	
	writeSercos_U16('P', 2000, 3); //Configured Safety Option (3 == AX5801-0200) [MANDATORY]
	
	writeSercos_U16('P', 800, 0x80, 0); //Set digital pin 7 to User Output

	writeSercos_U16('P', 350, 0, 0); //set error reaction to torque off
	writeSercos_U16('P', 350, 0, 1);

	bool b_invert0 = axis0->invertDirection_param->value;
	bool b_invert1 = axis1->invertDirection_param->value;

	writeSercos_U16('S', 43, b_invert0 ? 0xD : 0x0, 0);
	writeSercos_U16('S', 55, b_invert0 ? 0xD : 0x0, 0);
	writeSercos_U16('S', 43, b_invert1 ? 0xD : 0x0, 1);
	writeSercos_U16('S', 55, b_invert1 ? 0xD : 0x0, 1);

	writeSercos_U32('P', 92, axis0->currentLimit_amps->value * 1000, 0);
	writeSercos_U32('P', 92, axis1->currentLimit_amps->value * 1000, 1);

	writeSercos_U32('S', 159, axis0->positionFollowingErrorLimit_rev->value * axis0->unitsPerRev, 0);
	writeSercos_U32('S', 159, axis1->positionFollowingErrorLimit_rev->value * axis1->unitsPerRev, 1);
	
	//setup cycle times
	uint16_t cycleTime_micros = EtherCatFieldbus::processInterval_milliseconds * 1000;
	uint32_t cycleTime_nanos = cycleTime_micros * 1000;
	uint32_t driveInterruptTime_nanos = 250'000;
	writeSercos_U16('S', 1, cycleTime_micros); //Control unit cycle time (TNcyc) [MANDATORY]
	writeSercos_U16('S', 2, cycleTime_micros); //Communication cycle time (tScyc) [MANDATORY]
	ec_dcsync01(getSlaveIndex(), true, driveInterruptTime_nanos, cycleTime_nanos - driveInterruptTime_nanos, 0); //[MANDATORY]
	
	//TODO: figure out shitftime in dc function
	
	return true;
}

bool AX5206::AM8051_1G20_0000_startupList(uint8_t axis){
	bool ret0 = writeSercos_U16('P', 304, 1, axis); //Report diagnostics information [report only errors]
	bool ret1 = writeSercos_U16('P', 216, 8900, axis); //Max DC link Voltage [890.0V]
	bool ret2 = writeSercos_U16('P', 201, 4000, axis); //Nominal mains voltage [400.0V]
	bool ret3 = writeSercos_U16('P', 202, 100, axis); //mains voltage positive tolerance range [10.0V]
	bool ret4 = writeSercos_U16('P', 203, 100, axis); //mains voltage negative tolerance range [10.0V]
	bool ret5 = writeSercos_U16('P', 204, 9, axis); //Power management control word [internal brake resistor, others] [0x9] corrected for [REV 203]
	uint8_t P0053[16] = {
		//[MANDATORY]
		0x0c, 0x00, 0x22, 0x00, 0x41, 0x4d, 0x38, 0x30,
		0x35, 0x31, 0x2d, 0x78, 0x47, 0x78, 0x30, 0x00};
	bool ret6 = writeSercos_Array('P', 53, P0053, 16, axis); //configured motor type [Motor Name String]
	bool ret7 = writeSercos_U16('P', 50, 0, axis); //motor contruction type [synchronous, rotary]
	uint8_t P0062[12] = {
		//[OPTIONAL]
		0x08, 0x00, 0x08, 0x00, 0x44, 0x07, 0x50, 0x00,
		0x64, 0x00, 0x01, 0x00};
	bool ret8 = writeSercos_Array('P', 62, P0062, 12, axis); //Thermal Motor Model [...] Corrected for [REV 203]
	bool ret9 = writeSercos_U16('P', 51, 4, axis); //Number of pole pairs [4]
	bool ret10 = writeSercos_U32('S', 111, 4750, axis); //Motor continuous stall current [4.750A]
	bool ret11 = writeSercos_U32('S', 196, 4200, axis); //Motor rated current [4.200A]
	bool ret12 = writeSercos_U32('S', 109, 20900, axis); //Motor peak current [20.900A]
	bool ret13 = writeSercos_U16('P', 77, 4000, axis); //Motor rated voltage [400.0V]
	bool ret14 = writeSercos_U16('P', 67, 8900, axis); //Motor winding : Dielectric strength [890.0V]
	uint8_t P0089[40] = {
		//[OPTIONAL]
		0x24, 0x00, 0x24, 0x00, 0x26, 0x83, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	bool ret15 = writeSercos_Array('P', 89, P0089, 40, axis); //Motor data constraints [...]
	bool ret16 = writeSercos_U32('P', 73, 1780, axis); //Motor peak torque [17.8Nm]
	uint8_t P0055[84] = {
		//[OPTIONAL]
		0x50, 0x00, 0x50, 0x00, 0x2a, 0x08, 0x00, 0x00,
		0x54, 0x10, 0x00, 0x00, 0x7e, 0x18, 0x00, 0x00,
		0xa8, 0x20, 0x00, 0x00, 0x04, 0x29, 0x00, 0x00,
		0xd4, 0x30, 0x00, 0x00, 0x08, 0x39, 0x00, 0x00,
		0x3c, 0x41, 0x00, 0x00, 0x70, 0x49, 0x00, 0x00,
		0xa4, 0x51, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
		0xf4, 0x01, 0x00, 0x00, 0xd3, 0x02, 0x00, 0x00,
		0x9f, 0x03, 0x00, 0x00, 0x56, 0x04, 0x00, 0x00,
		0x00, 0x05, 0x00, 0x00, 0x96, 0x05, 0x00, 0x00,
		0x22, 0x06, 0x00, 0x00, 0x90, 0x06, 0x00, 0x00,
		0xf4, 0x06, 0x00, 0x00};
	bool ret17 = writeSercos_Array('P', 74, P0055, 84, axis); //Motor torque/force characteristic
	bool ret18 = writeSercos_U16('P', 55, 730, axis); //Motor EMF [73.0 mV/rpm]
	uint8_t P0066[12] = {
		0x08, 0x00, 0x08, 0x00, 0x68, 0x01, 0x00, 0x00,
		0xa0, 0x05, 0x00, 0x00};
	bool ret19 = writeSercos_Array('P', 66, P0066, 12, axis); //Electric motor model [...]
	uint8_t P0075[84] = {
		//[OPTIONAL]
		0x50, 0x00, 0x50, 0x00, 0x2a, 0x08, 0x00, 0x00,
		0x54, 0x10, 0x00, 0x00, 0x7e, 0x18, 0x00, 0x00,
		0xa8, 0x20, 0x00, 0x00, 0x04, 0x29, 0x00, 0x00,
		0xd4, 0x30, 0x00, 0x00, 0x08, 0x39, 0x00, 0x00,
		0x3c, 0x41, 0x00, 0x00, 0x70, 0x49, 0x00, 0x00,
		0xa4, 0x51, 0x00, 0x00, 0x82, 0x05, 0x00, 0x00,
		0x50, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00,
		0xba, 0x04, 0x00, 0x00, 0x60, 0x04, 0x00, 0x00,
		0xd1, 0x03, 0x00, 0x00, 0x22, 0x03, 0x00, 0x00,
		0x95, 0x02, 0x00, 0x00, 0x07, 0x02, 0x00, 0x00,
		0x7a, 0x01, 0x00, 0x00};
	bool ret20 = writeSercos_Array('P', 75, P0075, 84, axis); //Motor inductance characteristic [...]
	bool ret21 = writeSercos_U32('S', 113, 9000, axis); //Maximum motor speed [9000rpm]
	bool ret22 = writeSercos_U32('S', 91, 153008209, axis); //Bipolar velocity limit value [wtf is this unit]
	uint8_t P0071[12] = {
		//[OPTIONAL]
		0x08, 0x00, 0x08, 0x00, 0xe0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00};
	bool ret23 = writeSercos_Array('P', 71, P0071, 12, axis); //Mechanical motor data [...]
	bool ret24 = writeSercos_U16('S', 201, 1200, axis); //Motor warning temperature [120.0°C]
	bool ret25 = writeSercos_U16('S', 204, 1400, axis); //Motor shut down temperature [140.0°C]
	uint8_t P0076[44] = {
		//[MANDATORY]
		0x28, 0x00, 0x28, 0x00, 0x70, 0xfe, 0x65, 0xff,
		0x5a, 0x00, 0x4f, 0x01, 0x44, 0x02, 0x39, 0x03,
		0x2e, 0x04, 0x28, 0x05, 0x18, 0x06, 0x12, 0x07,
		0x67, 0x01, 0xb8, 0x01, 0x16, 0x02, 0x82, 0x02,
		0xfa, 0x02, 0x80, 0x03, 0x13, 0x04, 0xb4, 0x04,
		0x61, 0x05, 0x18, 0x06};
	bool ret26 = writeSercos_Array('P', 76, P0076, 44, axis); //Motor temperature sensor characteristic
	bool ret27 = writeSercos_U16('P', 451, 1, axis); //Current controller settings 2 [various...]
	bool ret28 = writeSercos_U16('P', 57, 27000, axis); //Electrical commutation offset [270.00°]
	bool ret29 = writeSercos_U16('P', 61, 7, axis); //Motor temperature sensor type [via feedback process data, characteristic table]
	bool ret30 = writeSercos_U16('P', 2, 62, axis); //Current ctrl cycle time [62.5µs]
	uint8_t P0054[20] = {
		//[OPTIONAL]
		0x10, 0x00, 0x22, 0x00, 0x41, 0x58, 0x35, 0x32,
		0x30, 0x36, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x23, 0x23, 0x23, 0x23};
	bool ret31 = writeSercos_Array('P', 54, P0054, 20, axis); //configured drive type [...]
	bool ret32 = writeSercos_U32('P', 93, 4750, axis); //configured channel current [4.750A]
	bool ret33 = writeSercos_U32('P', 92, 9500, axis); //configured channel peak current [9.500A]
	bool ret34 = writeSercos_U16('P', 70, 489, axis); //motor continuous stall torque [4.89Nm] [u16/u32]
	bool ret35 = writeSercos_U16('S', 106, 432, axis); //Current loop proportional gain 1 [43.2V/A]
	bool ret36 = writeSercos_U16('S', 107, 8, axis); //Current control loop integral action time 1 [0.8ms]
	uint8_t P0150[228] = {
		//[MANDATORY]
		0xe0, 0x00, 0xe0, 0x00, 0x03, 0x00, 0x00, 0x00,
		0x53, 0x69, 0x63, 0x6b, 0x23, 0x45, 0x4b, 0x4d,
		0x33, 0x36, 0x2d, 0x30, 0x4b, 0x46, 0x30, 0x41,
		0x30, 0x53, 0x31, 0x36, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0e, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0e, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x0c, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x96, 0x00, 0x96, 0x00, 0x69, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00};
	bool ret37 = writeSercos_Array('P', 150, P0150, 228, axis); //Feedback 1 type
	uint8_t P0514[16] = {
		//[OPTIONAL]
		0x0c, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00,
		0xf4, 0x01, 0xe8, 0x03, 0xbc, 0x02, 0x00, 0x00};
	bool ret38 = writeSercos_Array('P', 514, P0514, 16, axis); //Velocity Observer [...]
	bool ret39 = writeSercos_U16('P', 511, 150, axis); //Velocity filter 1: Low pass time constant [0.150ms]
	bool ret40 = writeSercos_U32('S', 100, 144, axis); //Velocity loop proportional gain [0.144 N/(m/s)]
	bool ret41 = writeSercos_U16('S', 101, 80, axis); //Velocity loop intergral action time [8.0ms]
		
	return true;
	
}

bool AX5206::AM8052_1J20_0000_startupList(uint8_t axis){
	
	bool ret0 = writeSercos_U16('P', 304, 1, axis); //Report diagnostics information [report only errors]
	bool ret1 = writeSercos_U16('P', 216, 8900, axis); //Max DC link Voltage [890.0V]
	bool ret2 = writeSercos_U16('P', 201, 4000, axis); //Nominal mains voltage [400.0V]
	bool ret3 = writeSercos_U16('P', 202, 100, axis); //mains voltage positive tolerance range [10.0V]
	bool ret4 = writeSercos_U16('P', 203, 100, axis); //mains voltage negative tolerance range [10.0V]
	bool ret5 = writeSercos_U16('P', 204, 9, axis); //Power management control word [internal brake resistor, others] [0x9] corrected for [REV 203]
	uint8_t P0053[16] = {
		0x0c, 0x00, 0x22, 0x00, 0x41, 0x4d, 0x38, 0x30,
		0x35, 0x32, 0x2d, 0x78, 0x4a, 0x78, 0x30, 0x00};
	bool ret6 = writeSercos_Array('P', 53, P0053, 16, axis); //configured motor type [Motor Name String]
	bool ret7 = writeSercos_U16('P', 50, 0, axis); //motor contruction type [synchronous, rotary]
	uint8_t P0062[12] = {
		0x08, 0x00, 0x08, 0x00, 0xe8, 0x08, 0x50, 0x00,
		0x64, 0x00, 0x01, 0x00};
	bool ret8 = writeSercos_Array('P', 62, P0062, 12, axis); //Thermal Motor Model [...] Corrected for [REV 203]
	bool ret9 = writeSercos_U16('P', 51, 4, axis); //Number of pole pairs [4]
	bool ret10 = writeSercos_U32('S', 111, 6300, axis); //Motor continuous stall current [6.3000A]
	bool ret11 = writeSercos_U32('S', 196, 5200, axis); //Motor rated current [5.200A]
	bool ret12 = writeSercos_U32('S', 109, 33600, axis); //Motor peak current [33.600A]
	bool ret13 = writeSercos_U16('P', 77, 4000, axis); //Motor rated voltage [400.0V]
	bool ret14 = writeSercos_U16('P', 67, 8900, axis); //Motor winding : Dielectric strength [890.0V]
	uint8_t P0089[40] = {
		0x24, 0x00, 0x24, 0x00, 0x26, 0x83, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	bool ret15 = writeSercos_Array('P', 89, P0089, 40, axis); //Motor data constraints [...]
	bool ret16 = writeSercos_U32('P', 73, 3530, axis); //Motor peak torque [35.3Nm]
	uint8_t P0055[84] = {
		0x50, 0x00, 0x50, 0x00, 0x20, 0x0d, 0x00, 0x00,
		0x40, 0x1a, 0x00, 0x00, 0x74, 0x27, 0x00, 0x00,
		0x58, 0x34, 0x00, 0x00, 0xa0, 0x41, 0x00, 0x00,
		0xe8, 0x4e, 0x00, 0x00, 0xcc, 0x5b, 0x00, 0x00,
		0x14, 0x69, 0x00, 0x00, 0xf8, 0x75, 0x00, 0x00,
		0x40, 0x83, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00,
		0xe3, 0x03, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00,
		0x3a, 0x07, 0x00, 0x00, 0xac, 0x08, 0x00, 0x00,
		0xf6, 0x09, 0x00, 0x00, 0x22, 0x0b, 0x00, 0x00,
		0x30, 0x0c, 0x00, 0x00, 0x0c, 0x0d, 0x00, 0x00,
		0xca, 0x0d, 0x00, 0x00};
	bool ret17 = writeSercos_Array('P', 74, P0055, 84, axis); //Motor torque/force characteristic
	bool ret18 = writeSercos_U16('P', 55, 890, axis); //Motor EMF [89.0 mV/rpm]
	uint8_t P0066[12] = {
		0x08, 0x00, 0x08, 0x00, 0xe6, 0x00, 0x00, 0x00,
		0x1a, 0x04, 0x00, 0x00};
	bool ret19 = writeSercos_Array('P', 66, P0066, 12, axis); //Electric motor model [...]
	uint8_t P0075[84] = {
		0x50, 0x00, 0x50, 0x00, 0x20, 0x0d, 0x00, 0x00,
		0x40, 0x1a, 0x00, 0x00, 0x74, 0x27, 0x00, 0x00,
		0x58, 0x34, 0x00, 0x00, 0xa0, 0x41, 0x00, 0x00,
		0xe8, 0x4e, 0x00, 0x00, 0xcc, 0x5b, 0x00, 0x00,
		0x14, 0x69, 0x00, 0x00, 0xf8, 0x75, 0x00, 0x00,
		0x40, 0x83, 0x00, 0x00, 0x06, 0x04, 0x00, 0x00,
		0xdf, 0x03, 0x00, 0x00, 0xa6, 0x03, 0x00, 0x00,
		0x6f, 0x03, 0x00, 0x00, 0x2f, 0x03, 0x00, 0x00,
		0xc8, 0x02, 0x00, 0x00, 0x49, 0x02, 0x00, 0x00,
		0xe2, 0x01, 0x00, 0x00, 0x7a, 0x01, 0x00, 0x00,
		0x13, 0x01, 0x00, 0x00};
	bool ret20 = writeSercos_Array('P', 75, P0075, 84, axis); //Motor inductance characteristic [...]
	bool ret21 = writeSercos_U32('S', 113, 9000, axis); //Maximum motor speed [9000rpm]
	bool ret22 = writeSercos_U32('S', 91, 153008209, axis); //Bipolar velocity limit value [wtf is this unit]
	uint8_t P0071[12] = {
		0x08, 0x00, 0x08, 0x00, 0x98, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00};
	bool ret23 = writeSercos_Array('P', 71, P0071, 12, axis); //Mechanical motor data [...]
	bool ret24 = writeSercos_U16('S', 201, 1200, axis); //Motor warning temperature [120.0°C]
	bool ret25 = writeSercos_U16('S', 204, 1400, axis); //Motor shut down temperature [140.0°C]
	uint8_t P0076[44] = {
		0x28, 0x00, 0x28, 0x00, 0x70, 0xfe, 0x65, 0xff,
		0x5a, 0x00, 0x4f, 0x01, 0x44, 0x02, 0x39, 0x03,
		0x2e, 0x04, 0x28, 0x05, 0x18, 0x06, 0x12, 0x07,
		0x67, 0x01, 0xb8, 0x01, 0x16, 0x02, 0x82, 0x02,
		0xfa, 0x02, 0x80, 0x03, 0x13, 0x04, 0xb4, 0x04,
		0x61, 0x05, 0x18, 0x06};
	bool ret26 = writeSercos_Array('P', 76, P0076, 44, axis); //Motor temperature sensor characteristic [...]
	bool ret27 = writeSercos_U16('P', 451, 1, axis); //Current controller settings [various...]
	bool ret28 = writeSercos_U16('P', 57, 27000, axis); //Electrical commutation offset [270.00°]
	bool ret29 = writeSercos_U16('P', 61, 7, axis); //Motor temperature sensor type [via feedback process data, characteristic table]
	bool ret30 = writeSercos_U16('P', 2, 62, axis); //Current ctrl cycle time [62.5µs]
	uint8_t P0054[20] = {
		0x10, 0x00, 0x22, 0x00, 0x41, 0x58, 0x35, 0x32,
		0x30, 0x36, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x23, 0x23, 0x23, 0x23
	};
	bool ret31 = writeSercos_Array('P', 54, P0054, 20, axis); //configured drive type [...]
	bool ret32 = writeSercos_U32('P', 93, 6000, axis); //configured channel current [6.000A]
	bool ret33 = writeSercos_U32('P', 92, 12600, axis); //configured channel peak current [12.000A]
	bool ret34 = writeSercos_U16('P', 70, 819, axis); //motor continuous stall torque [8.19Nm] [u16/u32]
	bool ret35 = writeSercos_U16('S', 106, 315, axis); //Current loop proportional gain 1 [31.5V/A]
	bool ret36 = writeSercos_U16('S', 107, 8, axis); //Current control loop integral action time 1 [0.8ms]
	uint8_t P0150[228] = {
		0xe0, 0x00, 0xe0, 0x00, 0x03, 0x00, 0x00, 0x00,
		0x53, 0x69, 0x63, 0x6b, 0x23, 0x45, 0x4b, 0x4d,
		0x33, 0x36, 0x2d, 0x30, 0x4b, 0x46, 0x30, 0x41,
		0x30, 0x53, 0x31, 0x36, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x0c, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x96, 0x00, 0x96, 0x00, 0x69, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00};
	bool ret37 = writeSercos_Array('P', 150, P0150, 228, axis); //Feedback 1 type [...]
	uint8_t P0514[16] = {
		0x0c, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x00,
		0xf4, 0x01, 0xe8, 0x03, 0xbc, 0x02, 0x00, 0x00};
	bool ret38 = writeSercos_Array('P', 514, P0514, 16, axis); //Velocity Observer [...]
	bool ret39 = writeSercos_U16('P', 511, 150, axis); //Velocity filter 1: Low pass time constant [0.150ms]
	bool ret40 = writeSercos_U32('S', 100, 208, axis); //Velocity loop proportional gain [0.191 N/(m/s)]
	bool ret41 = writeSercos_U16('S', 101, 80, axis); //Velocity loop intergral action time [8.0ms]
		
	return true;
	
}

void AX5206::readInputs(){
	memcpy(&acknowledgeTelegram, identity->inputs, identity->Ibytes);
		
	//Drive Data
	processData.b_stoActive = acknowledgeTelegram.safetyOptionState & 0x1;
	processData.digitalInput0 = acknowledgeTelegram.digitalInputsState & 0x1;
	processData.digitalInput1 = acknowledgeTelegram.digitalInputsState & 0x2;
	processData.digitalInput2 = acknowledgeTelegram.digitalInputsState & 0x4;
	processData.digitalInput3 = acknowledgeTelegram.digitalInputsState & 0x8;
	processData.digitalInput4 = acknowledgeTelegram.digitalInputsState & 0x10;
	processData.digitalInput5 = acknowledgeTelegram.digitalInputsState & 0x20;
	processData.digitalInput6 = acknowledgeTelegram.digitalInputsState & 0x40;
	
	*STOValue = invertSTO_param->value ? !processData.b_stoActive : processData.b_stoActive;
	*digitalin0Value = invertDigitalIn0_param->value ? !processData.digitalInput0 : processData.digitalInput0;
	*digitalin1Value = invertDigitalIn1_param->value ? !processData.digitalInput1 : processData.digitalInput1;
	*digitalin2Value = invertDigitalIn2_param->value ? !processData.digitalInput2 : processData.digitalInput2;
	*digitalin3Value = invertDigitalIn3_param->value ? !processData.digitalInput3 : processData.digitalInput3;
	*digitalin4Value = invertDigitalIn4_param->value ? !processData.digitalInput4 : processData.digitalInput4;
	*digitalin5Value = invertDigitalIn5_param->value ? !processData.digitalInput5 : processData.digitalInput5;
	*digitalin6Value = invertDigitalIn6_param->value ? !processData.digitalInput6 : processData.digitalInput6;

	//Axis Data
	axis0->updateInputs(acknowledgeTelegram.ax0_driveStatusWord,
						acknowledgeTelegram.ax0_positionFeedbackValue1,
						acknowledgeTelegram.ax0_velocityFeedbackValue,
						acknowledgeTelegram.ax0_torqueFeedbackValue,
						acknowledgeTelegram.ax0_class1Diagnostics,
						processData.b_stoActive);
	axis1->updateInputs(acknowledgeTelegram.ax1_driveStatusWord,
						acknowledgeTelegram.ax1_positionFeedbackValue1,
						acknowledgeTelegram.ax1_velocityFeedbackValue,
						acknowledgeTelegram.ax1_torqueFeedbackValue,
						acknowledgeTelegram.ax1_class1Diagnostics,
						processData.b_stoActive);
		
}

void AX5206::writeOutputs(){
	
	//Drive Data
	
	if(digitalOutput7_pin->isConnected()) digitalOutput7_pin->copyConnectedPinValue();
	processData.digitalOutput7 = *digitalout7Value;
	if(invertDigitalOut7_param->value) processData.digitalOutput7 = ! processData.digitalOutput7;
	masterDataTelegram.digitalOutput = processData.digitalOutput7 ? 0x80 : 0x0;

	uint16_t previousControlWord0 = masterDataTelegram.ax0_masterControlWord;
	uint16_t previousControlWord1 = masterDataTelegram.ax1_masterControlWord;

	//Axis Data
	axis0->updateOutputs(masterDataTelegram.ax0_masterControlWord,
						 masterDataTelegram.ax0_velocityCommandValue,
						 masterDataTelegram.ax0_positionCommandValue);
	axis1->updateOutputs(masterDataTelegram.ax1_masterControlWord,
						 masterDataTelegram.ax1_velocityCommandValue,
						 masterDataTelegram.ax1_positionCommandValue);
	
	if(previousControlWord0 != masterDataTelegram.ax0_masterControlWord)
		Logger::info("Axis 0 ctrlwrd {:x}", masterDataTelegram.ax0_masterControlWord);
	if(previousControlWord1 != masterDataTelegram.ax1_masterControlWord)
		Logger::info("Axis 1 ctrlwrd {:x}", masterDataTelegram.ax1_masterControlWord);

	memcpy(identity->outputs, &masterDataTelegram, identity->Obytes);
}

void AX5206::Axis::updateInputs(uint16_t statusw, int32_t pos, int32_t vel, int16_t tor, uint16_t err, bool sto){

	statusWord.decode(statusw);

	bool previousEstop = actuatorProcessData.b_isEmergencyStopActive;
	if(previousEstop != sto){
		if(sto) Logger::warn("[{}] Axis {} : STO Active", drive->getName(), channel);
		else Logger::info("[{}] Axis {} : STO Cleared", drive->getName(), channel);
	}

	bool previousFault = b_hasFault;
	if(statusWord.shutdownError != previousFault){
		if(!statusWord.shutdownError) Logger::info("[{}] Axis {} : Fault Cleared", drive->getName(), channel);
		else Logger::error("[{}] Axis {} : Fault !", drive->getName(), channel);
	}

	if(class1Errors != err){
		if(err != 0x0){
			Logger::error("[{}] Axis {} Errors:", drive->getName(), channel);
			Logger::error("{}", drive->getClass1Errors(err));
		}
		else Logger::error("[{}] Axis {} : Error cleared !", drive->getName(), channel);
	}
	class1Errors = err;

	if(actuatorProcessData.b_enabling){
		if(statusWord.isEnabled()){
			actuatorProcessData.b_enabling = false;
			Logger::info("[{}] Axis {} Enabled", drive->getName(), channel);
		}
		else if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - actuatorProcessData.enableRequest_nanos > enableTimeout_nanos){
			actuatorProcessData.b_enabling = false;
			controlWord.disable();
			Logger::warn("[{}] Axis {} Enable timeout", drive->getName(), channel);
		}
	}
	else if(isEnabled() && !statusWord.isEnabled()){
		Logger::error("[{} Axis {}] isEnabled: {}", drive->getName(), channel, isEnabled());
		actuatorProcessData.b_enabling = false;
		controlWord.disable();
		Logger::error("[{} Axis {}] Disabled (st:{} fl:{})", drive->getName(), channel, statusWord.status, statusWord.followsCommand);
	}


	//update interface state
	if(statusWord.isEnabled()) {
		if(controlWord.isRequestingEnable()) state = DeviceState::ENABLED;
		else state = DeviceState::DISABLING;
	}
	else{
		if(actuatorProcessData.b_enabling) state = DeviceState::ENABLING;
		else if(sto || !statusWord.isReady()) state = DeviceState::NOT_READY;
		else if(!drive->isStateOperational()) state = DeviceState::NOT_READY;
		else state = DeviceState::READY;
	}
	feedbackProcessData.positionActual = double(pos) / unitsPerRev;
	feedbackProcessData.velocityActual = double(vel) / (unitsPerRPM * 60.0);
	actuatorProcessData.effortActual = std::abs(double(tor) / 1000.0);
	actuatorProcessData.b_isEmergencyStopActive = sto;
	b_hasFault = statusWord.shutdownError;
}



void AX5206::Axis::updateOutputs(uint16_t& controlw, int32_t& vel, uint32_t& pos){
	//controlWord.toggleSyncBit();
	if(actuatorProcessData.b_enable){
		actuatorProcessData.b_enable = false;
		actuatorProcessData.b_enabling = true;
		actuatorProcessData.enableRequest_nanos = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
		if(statusWord.shutdownError) drive->requestFaultReset(channel);
	}
	if(actuatorProcessData.b_enabling){
		if(!statusWord.shutdownError) controlWord.enable();
	}
	if(actuatorProcessData.b_disable){
		actuatorProcessData.b_disable = false;
		actuatorProcessData.b_enabling = false;
		controlWord.disable();
		Logger::error("[{} Axis {}] Disable request", drive->getName(), channel);
	}
	
	vel = actuatorProcessData.velocityTarget * unitsPerRPM * 60.0;
	pos = actuatorProcessData.positionTarget * unitsPerRev;

	controlWord.encode(controlw);
}

void AX5206::requestFaultReset(uint8_t axis){
	std::thread faultResetThread([this,axis](){
		//set & enable command
		if(writeSercos_U16('S', 99, 3, axis)) Logger::info("Requested fault reset");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		//cancel command
		if(writeSercos_U16('S', 99, 0, axis)) Logger::info("Fault reset request finished");
	});
	faultResetThread.detach();
}

bool AX5206::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* pinInversionXML = xml->InsertNewChildElement("PinInversion");
	invertSTO_param->save(pinInversionXML);
	invertDigitalIn0_param->save(pinInversionXML);
	invertDigitalIn1_param->save(pinInversionXML);
	invertDigitalIn2_param->save(pinInversionXML);
	invertDigitalIn3_param->save(pinInversionXML);
	invertDigitalIn4_param->save(pinInversionXML);
	invertDigitalIn5_param->save(pinInversionXML);
	invertDigitalIn6_param->save(pinInversionXML);
	invertDigitalOut7_param->save(pinInversionXML);
	
	XMLElement* axis0XML = xml->InsertNewChildElement("Axis0");
	axis0->save(axis0XML);
	
	XMLElement* axis1XML = xml->InsertNewChildElement("Axis1");
	axis1->save(axis1XML);
	
	return true;
}

bool AX5206::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	if(XMLElement* pinInversionXML = xml->FirstChildElement("PinInversion")){
		invertSTO_param->load(pinInversionXML);
		invertDigitalIn0_param->load(pinInversionXML);
		invertDigitalIn1_param->load(pinInversionXML);
		invertDigitalIn2_param->load(pinInversionXML);
		invertDigitalIn3_param->load(pinInversionXML);
		invertDigitalIn4_param->load(pinInversionXML);
		invertDigitalIn5_param->load(pinInversionXML);
		invertDigitalIn6_param->load(pinInversionXML);
		invertDigitalOut7_param->load(pinInversionXML);
	}
	
	if(XMLElement* axis0XML = xml->FirstChildElement("Axis0")){
		axis0->load(axis0XML);
	}
	
	if(XMLElement* axis1XML = xml->FirstChildElement("Axis1")){
		axis1->load(axis1XML);
	}
	
	return true;
}

bool AX5206::Axis::save(tinyxml2::XMLElement* xml){
	motorType->save(xml);
	velocityLimit_revps->save(xml);
	accelerationLimit_revps2->save(xml);
	positionFollowingErrorLimit_rev->save(xml);
	currentLimit_amps->save(xml);
	invertDirection_param->save(xml);
	return true;
}

bool AX5206::Axis::load(tinyxml2::XMLElement* xml){
	motorType->load(xml);
	velocityLimit_revps->load(xml);
	accelerationLimit_revps2->load(xml);
	positionFollowingErrorLimit_rev->load(xml);
	currentLimit_amps->load(xml);
	invertDirection_param->load(xml);
	return true;
}

std::string AX5206::Axis::getStatusString(){
	if(!isOnline()) return "Actuator Offline";
	else if(isEnabled()) return "Actuator Enabled";
	else if(isEnabling()) return "Actuator Enabling...";
	else if(isReady()) return "Actuator Ready";
	else{
		if(isEmergencyStopActive()) return "STO is active";
		if(!drive->isStateOperational()) return "Drive is not in operational state";
		if(hasFault()) {
			std::string output = "Axis has errors:\n";
			return output + drive->getClass1Errors(class1Errors);
		}
		else return "Actuator not ready";
	}
}

void AX5206::getInvalidIDNsForSafeOp(){
	struct IDN_List{
		uint16_t actualLength = 0; //number of bytes in the idns array
		uint16_t maxLength = 0;
		uint16_t idns[10000] = {0};
	}invalidIDNs;
	int size = 100;
	
	if(!readSercos_Array('S', 21, (uint8_t*)&invalidIDNs, size)) Logger::warn("Failed to download IDN list");
	
	int invalidIDNcount = invalidIDNs.actualLength / 2;
	Logger::info("Invalid IDNs for PreOp->SafeOp Transition ({})", invalidIDNcount);
	for(int i = 0; i < invalidIDNcount; i++){
		uint16_t idn = invalidIDNs.idns[i];
		if(idn > 32768) Logger::info("    P-0-{}", idn - 32768);
		else Logger::info("    S-0-{}", idn);
	}
}

void AX5206::getInvalidIDNsForOp(){
	struct IDN_List{
		uint16_t actualLength = 0; //number of bytes in the idns array
		uint16_t maxLength = 0;
		uint16_t idns[10000] = {0};
	}invalidIDNs;
	int size = 100;
	
	if(!readSercos_Array('S', 22, (uint8_t*)&invalidIDNs, size)) Logger::warn("Failed to download IDN list");
	
	int invalidIDNcount = invalidIDNs.actualLength / 2;
	Logger::info("Invalid IDNs for SafeOp->Op Transition ({})", invalidIDNcount);
	for(int i = 0; i < invalidIDNcount; i++){
		uint16_t idn = invalidIDNs.idns[i];
		if(idn > 32768) Logger::info("    P-0-{}", idn - 32768);
		else Logger::info("    S-0-{}", idn);
	}
}

std::string AX5206::getClass1Errors(uint16_t c1diag){
	if(c1diag == 0x0) return "No Errors";
	std::string output;
	if(c1diag & 0x1) output += "Motor overload shut down\n";
	if(c1diag & 0x2) output += "Heatsink overtemperature shutdown\n";
	if(c1diag & 0x4) output += "Motor overtemperature shut down\n";
	if(c1diag & 0x8) output += "Drive overtemperature shut down\n";
	if(c1diag & 0x10) output += "Control voltage error\n";
	if(c1diag & 0x20) output += "Feedback error\n";
	if(c1diag & 0x40) output += "Commutation error\n";
	if(c1diag & 0x80) output += "Overcurrent error\n";
	if(c1diag & 0x100) output += "Overvoltage error\n";
	if(c1diag & 0x200) output += "Undervoltage error\n";
	if(c1diag & 0x400) output += "Power supply phase error\n";
	if(c1diag & 0x800) output += "Excessive position deviation\n";
	if(c1diag & 0x1000) output += "communication error\n";
	if(c1diag & 0x2000) output += "Overtravel limit exceeded\n";
	if(c1diag & 0x4000) output += "Unknown error\n";
	if(c1diag & 0x8000) output += "Manufacturer specific error\n";
	output.pop_back(); //remove last return character
	return output;
}

void AX5206::getShutdownErrorList(){
	uint16_t err0, err1;
	bool ret0 = readSercos_U16('S', 11, err0);
	bool ret1 = readSercos_U16('S', 11, err1);
	Logger::warn("Axis 0 Errors: ");
	Logger::error("{}", getClass1Errors(err0));
	//Logger::warn("Axis 1 Errors: "); //this does not seem to show anything
	//Logger::error("{}", getClass1Errors(err1));
}

void AX5206::getErrorHistory(){
	struct ErrorList{
		uint16_t actualSize;
		uint16_t maxSize;
		uint32_t errors[110];
	}errorList;
	int errorListSize = sizeof(ErrorList);
	struct ErrorTimeList{
		uint16_t actualSize;
		uint16_t maxSize;
		uint32_t errorTimes[110];
	}errorTimeList;
	int errorTimeListSize = sizeof(ErrorTimeList);
	
	auto secondsToTimeString = [](int total_seconds) -> std::string {
		int hours = total_seconds / 3600;
		int minutes = (total_seconds % 3600) / 60;
		int seconds = total_seconds % 60;
		char buffer[128];
		snprintf(buffer, 128, "%02d:%02d:%02d", hours, minutes, seconds);
		return std::string(buffer);
	};
	
	auto showErrorHistory = [&](uint8_t axis){
		bool ret1 = readSercos_Array('P', 300, (uint8_t*)&errorList, errorListSize, axis);
		bool ret2 = readSercos_Array('P', 301, (uint8_t*)&errorTimeList, errorTimeListSize, axis);
		Logger::info("{} {}", errorListSize, errorTimeListSize);
		if(ret1 && ret2){
			int errorCount = errorList.actualSize / 4;
			Logger::warn("Error History Axis [{}] : {} Errors", axis, errorCount);
			for(int i = 0; i < errorCount; i++){
				uint32_t errorCode = errorList.errors[i];
				std::string errorString = getErrorString(errorCode);
				std::string timeString = secondsToTimeString(errorTimeList.errorTimes[i]);
				if(errorString == "") Logger::error("    {:X} ({})", errorCode, timeString);
				else Logger::warn("    [Axis{}] {:X} : {} ({})", axis, errorCode, errorString, timeString);
			}
		}
	};
	
	showErrorHistory(0);
	showErrorHistory(1);
}

void AX5206::getDiagnosticsMessage(){
	std::string ax0diag;
	bool ret0 = readSercos_String('S', 95, ax0diag, 0); //same idn for axis 1 never displays anything
	Logger::warn("{}", ax0diag);
}

std::string AX5206::getErrorString(uint32_t errorCode){
	switch(errorCode){
		case 0xF100: return "Axis state machine: Communication error";
		case 0xF101: return "Axis state machine: Initialize error (selected uninitialized operating mode)";
		case 0xF106: return "Axis state machine: No motor configured";
		case 0xF107: return "Axis state machine: Current control not ready to enable";
		case 0xF152: return "Initialization of the feedback: Command failed";
		case 0xF166: return "Process data mapping: MDT - S-0-0024";
		case 0xF2A0: return "COMMUTATION ERROR (very rare)";
		case 0xF2A7: return "Torque off triggered from \"shorted coils brake\" or \"DC brake\"";
		case 0xF414: return "Distributed clocks: hardware sync";
		case 0xF415: return "Distributed clocks: Process data synchronization lost";
		case 0xF4A1: return "SoE Communication: Internal error";
		case 0xF4A3: return "SoE Communication: Internal error";
		case 0xF4A5: return "SoE Communication: Parameter error (invalid data for transition to safeop)";
		case 0xF720: return "Feedback \"general\": Initialization failed";
		case 0xF850: return "One cable feedback: Link missing";
		case 0xF851: return "One cable feedback: Initialization failed";
		case 0xF852: return "One cable feedback: Position error limit exceeded";
		case 0xF857: return "One cable feedback: Read of the cyclic data failed";
		case 0xF859: return "One cable feedback: Write system control failed";
		case 0xF85F: return "One cable feedback: Read of the cyclic data failed";
		case 0xFC03: return "Control voltage error: undervoltage";
		case 0xFD08: return "Motor management: Drive type don't match";
		case 0xFD09: return "Motor management: Motor type don't match";
		case 0xFD0A: return "Configured channel peak current is greater than the motor peak current";
		case 0xFD0C: return "Configured channel current is greater than the motor continuous stall current";
		case 0xFD11: return "Periphery voltage too low";
		case 0xFD1A: return "A/D-Converter: Motor temperature - measuring error";
		case 0xFD43: return "Mains supply: Power down";
		case 0xFD44: return "Mains supply: Phase error";
		case 0xFD47: return "Power management error";
		case 0xFDD3: return "Safety switch off while the axis was enabled";
		default: return "";
	};
}

std::string AX5206::getGpioStatus(){
	return "No status string yet :(";
}
