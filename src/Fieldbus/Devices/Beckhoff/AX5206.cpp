#include "AX5206.h"
#include "Fieldbus/EtherCatFieldbus.h"

void AX5206::onDisconnection(){}
void AX5206::onConnection(){}
void AX5206::initialize(){}

bool AX5206::startupConfiguration() {
	
	//write startup list
	
	//MDT Master->Slave
	//AT Slave->Master
	
	
	struct IDN_List{
		uint16_t size = 0; //total bytes in idn list (idn count * 2)
		uint16_t maxLength = 0; //max length of array in slave, not useful when uploading
		uint16_t IDNs[10] = {0};
		uint8_t* getPointer(){ return (uint8_t*)this; }
	};
	
	IDN_List axis0_mdtList;
	axis0_mdtList.size = 10;
	axis0_mdtList.IDNs[0] = 134; 	//(2) Master Control Word							[FIXED]
	axis0_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value							[MANDATORY]
	axis0_mdtList.IDNs[2] = 47; 	//(4) Position Command Value
	axis0_mdtList.IDNs[3] = 80; 	//(2) Torque Command Value
	axis0_mdtList.IDNs[4] = 33570;	//(2) Digital Outputs
	IDN_List axis1_mdtList;
	axis1_mdtList.size = 8;
	axis1_mdtList.IDNs[0] = 134; 	//(2) Master Control Word							[FIXED]
	axis1_mdtList.IDNs[1] = 36; 	//(4) Velocity Command Value							[MANDATORY]
	axis1_mdtList.IDNs[2] = 47; 	//(4) Position Command Value
	axis1_mdtList.IDNs[3] = 80; 	//(2) Torque Command Value

	
	IDN_List axis0_atList;
	axis0_atList.size = 12;
	axis0_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis0_atList.IDNs[1] = 51;	//(4) Position Feedback Value 1 (motor feedback)	[MANDATORY?]
	axis0_atList.IDNs[2] = 40;	//(4) Velocity Feedback Value 1
	axis0_atList.IDNs[3] = 84;	//(2) Torque Feedback Value
	axis0_atList.IDNs[4] = 189;	//(4) Following Distance
	axis0_atList.IDNs[5] = 33569; //(2) Digital Inputs, state
	IDN_List axis1_atList;
	axis1_atList.size = 10;
	axis1_atList.IDNs[0] = 135;	//(2) Drive Status Word (u16)						[FIXED]
	axis1_atList.IDNs[1] = 51;	//(4) Position Feedback Value 1 (motor feedback)	[MANDATORY?]
	axis1_atList.IDNs[2] = 40;	//(4) Velocity Feedback Value 1
	axis1_atList.IDNs[3] = 84;	//(2) Torque Feedback Value
	axis1_atList.IDNs[4] = 189;	//(4) Following Distance
	
	writeSercos_U64(32778, 0, 0); //P-0010
	writeSercos_U16(33072, 1, 0); //P-0304
	writeSercos_U64(32778, 0, 1); //P-0010
	writeSercos_U16(33072, 1, 1); //P-0304
	
	writeSercos_U16(15, 7, 0); //Telegram Type, [axis0]
	writeSercos_U16(15, 7, 1); //Telegram Type [axis1]
	writeSercos_Array(24, axis0_mdtList.getPointer(), sizeof(IDN_List), 0); //MDT List [axis0]
	writeSercos_Array(24, axis1_mdtList.getPointer(), sizeof(IDN_List), 1); //MDT List [axis1]
	writeSercos_Array(16, axis0_atList.getPointer(), sizeof(IDN_List), 0); //AT List [axis0]
	writeSercos_Array(16, axis1_atList.getPointer(), sizeof(IDN_List), 1); //AT List [axis1]
	
	writeSercos_U16(1, EtherCatFieldbus::processInterval_milliseconds * 1000); //Control unit cycle time (TNcyc)
	writeSercos_U16(2, EtherCatFieldbus::processInterval_milliseconds * 1000); //Communication cycle time (tScyc)
	
	writeSercos_U16(32, 11, 0);	//Primary Operation Mode [axis0]
	writeSercos_U16(32, 11, 1);	//Primary Operation Mode [axis1]
	writeSercos_U16(33072, 1, 0); //P-0-304 Report diagnostics information [axis0]
	writeSercos_U16(33072, 1, 1); //P-0-304 Report diagnostics information [axis1]
	writeSercos_U16(32969, 4000); //P-0-201 Nominal Mains Voltage
	
	writeSercos_U16(34768, 1);
	
/*
	writeSercos_U16(33324, 0, 0); //position interpolation type
	writeSercos_U16(33324, 0, 1); //position interpolation type
	writeSercos_U16(0104, 100, 0); //position loop Kv
	writeSercos_U16(0104, 100, 1); //position loop Kv
 */
	
	uint32_t cycleTime_nanos = EtherCatFieldbus::processInterval_milliseconds * 1000000;
	//uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
	ec_dcsync01(getSlaveIndex(), true, 250'000, cycleTime_nanos - 250'000, 0);
	//ec_dcsync01(getSlaveIndex(), true, 500'000,	 cycleTime_nanos - 500'000, cycleOffset_nanos);
	
	IDN_List test;
	int size = 200;
	bool ret = readSercos_Array(16, (uint8_t*)&test, size, 1);

	
	return true;
	
	/*
	 <InitCmd Fixed="true">
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Telegram type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>15</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0700</Data>
	 </InitCmd>
	 <InitCmd Fixed="true">
		 <Transition>PS</Transition>
		 <Comment><![CDATA[AT list]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>16</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>020002003300</Data>
	 </InitCmd>
	 <InitCmd Fixed="true">
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Telegram type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>1</DriveNo>
		 <IDN>15</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0700</Data>
	 </InitCmd>
	 <InitCmd Fixed="true">
		 <Transition>PS</Transition>
		 <Comment><![CDATA[AT list]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>1</DriveNo>
		 <IDN>16</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>020002003300</Data>
	 </InitCmd>
	 <InitCmd Fixed="true">
		 <Transition>PS</Transition>
		 <Comment><![CDATA[MDT list]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>24</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>020002002400</Data>
	 </InitCmd>
	 <InitCmd Fixed="true">
		 <Transition>PS</Transition>
		 <Comment><![CDATA[MDT list]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>1</DriveNo>
		 <IDN>24</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>020002002400</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Tncyc - NC cycle time]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>1</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>d007</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Tscyc - Comm cycle time]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>2</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>d007</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Operation mode]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0200</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Operation mode]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>1</DriveNo>
		 <IDN>32</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0200</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Logger message level]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>33072</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0100</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Logger message level]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>1</DriveNo>
		 <IDN>33072</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0100</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Nominal main voltage]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32969</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>fc08</Data>
	 </InitCmd>
	 */
	
	
	/*
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Main voltage positive tolerance range]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32970</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>c800</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Main voltage negative tolerance range]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32971</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>c800</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Power Management control word]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32972</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0100</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Feedback 1 type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32918</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>e000e000070000004861726f77652331354252583730304431304100000000000000000000000000000000000000000000000000040000000000000003000400000000000300020832000000102700007e04ee02f401e80300000000000000000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a6004a019b0000000000000000000000000000000000000000000000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Configured drive type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32822</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>100022004158353230362d303030302d23232323</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Configured motor type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32821</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>10002200414d333033312d304530302d30303030</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor peak current]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>109</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>e02e0000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor continuous stall current]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>111</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>ae0b0000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Maximum motor speed]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>113</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>401f0000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Number of pole pairs]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32819</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0400</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor EMF]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32823</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0501</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor continuous stall torque]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32838</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>7800</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Electrical commutation offset]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32825</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>7869</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Electric motor model]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32834</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>08000800ca0100005c030000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Thermal motor model]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32830</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>080008004803500064000100</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Positive acceleration limit value]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>136</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>5e960900</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Negative acceleration limit value]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>137</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>5e960900</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor construction type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32818</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Thermal overload factor (motor winding)]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32836</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0400040064000000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor warning temperature]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>201</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>2003</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor shut down temperature]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>204</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>7805</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Mechanical motor data]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32839</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>080008002100000000000000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Time limitation for peak current]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32820</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>b80b</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Current controller settings]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>33219</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor data constraints]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32857</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>24002400028000003e0000003e000000c980000084030000fc080000000000000000000000000000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Current loop proportional gain 1]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>106</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>f500</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Current control loop integral action time 1]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>107</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0800</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Max motor speed with max torque]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32824</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>5b100000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Configured channel peak current]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32860</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>5c170000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Configured channel current]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32861</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>ae0b0000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Bipolar velocity limit value]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>91</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>1f7bb706</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Current ctrl cycle time]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32770</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>3e00</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Velocity ctrl cycle time]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32771</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>7d00</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Velocity filter low pass time constant]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>33279</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>9600</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Position ctrl cycle time]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32772</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>fa00</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Feedback 1 gear numerator]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32920</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>01000000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Feedback 1 gear denominator]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32921</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>01000000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor temperature sensor type]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32829</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>0200</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Velocity loop proportional gain]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>100</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>58020000</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Velocity loop integral action time]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>101</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>2800</Data>
	 </InitCmd>
	 <InitCmd>
		 <Transition>PS</Transition>
		 <Comment><![CDATA[Motor winding: Dielectric strength]]></Comment>
		 <Timeout>0</Timeout>
		 <OpCode>3</OpCode>
		 <DriveNo>0</DriveNo>
		 <IDN>32835</IDN>
		 <Elements>64</Elements>
		 <Attribute>0</Attribute>
		 <Data>c422</Data>
	 </InitCmd>
	 */
	
	
}

void AX5206::readInputs(){}
void AX5206::writeOutputs(){}

bool AX5206::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}

bool AX5206::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}
