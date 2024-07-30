#include "AX5206.h"

void AX5206::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL7222-0010")){
		if(ImGui::BeginTabBar("Drive")){
			if(ImGui::BeginTabItem("Drive")){
				
				if(ImGui::Button("Test")){
					
					struct IDN_List{
						uint16_t actualLength = 0; //number of bytes in the idns array
						uint16_t maxLength = 0;
						uint16_t idns[1000] = {0};
					};
					
					IDN_List test;
					int size;
					bool ret = readSercos_Array(33068, (uint8_t*)&test, size);
					
					for(int i = 0; i < test.actualLength; i++){
						if(test.idns[i] == 0) continue;
						Logger::error("Error {:X}", test.idns[i]);
					}
					
					Logger::warn("{}", test.actualLength);
				}
				
				ImGui::EndTabItem();
				
			}
			ImGui::EndTabBar();
		}
		ImGui::EndTabItem();
	}
}


//P-0010 A
//P-0304 A
//P-0010 B
//P-0304 B
//S-0015 A
//S-0016 A
//S-0015 B
//S-0016 B
//S-0024 A
//S-0024 B
//drive0
//drive1
//P-0556 A
//P-0556 B
//S-0104 A
//S-0104 B


/*
//DRIVE TOOLS
{
<InitCmds>
	<InitCmd DriveNo="0" IDN="30" MotorDataLevel="0" DisableMode="0">
		<RetainData>3e0042004669726d776172653a2076312e303720284275696c64203031313529202f20426f6f746c6f616465723a2076312e303120284275696c6420303030322900</RetainData>
	</InitCmd>
	<InitCmd DriveNo="0" IDN="91" MotorDataLevel="2" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="100" MotorDataLevel="2" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="101" MotorDataLevel="2" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="106" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="107" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="109" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="111" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="113" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="136" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="137" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="201" MotorDataLevel="2" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="204" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="432" MotorDataLevel="0" DisableMode="0">
		<RetainData>1200160053657269616c233a20303030303032393431</RetainData>
	</InitCmd>
	<InitCmd DriveNo="0" IDN="32770" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32771" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32772" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32818" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32819" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32820" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32821" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32822" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32823" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32824" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32825" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32829" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32830" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32834" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32835" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32836" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32838" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32839" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32857" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32860" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32861" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32918" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32920" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="32921" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="33088" MotorDataLevel="0" DisableMode="0">
		<RetainData>240024006b00730065000200000000000000000000000000ec0c3d00ec0c3d000000000000000000</RetainData>
	</InitCmd>
	<InitCmd DriveNo="0" IDN="33093" MotorDataLevel="0" DisableMode="0">
		<RetainData>16001a004170722031372032303132202c2031303a30373a3434</RetainData>
	</InitCmd>
	<InitCmd DriveNo="0" IDN="33219" MotorDataLevel="1" DisableMode="0"/>
	<InitCmd DriveNo="0" IDN="33279" MotorDataLevel="1" DisableMode="0"/>
</InitCmds>

}
*/


/*
//MAILBOX SOE
<InitCmds>
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
</InitCmds>
*/







/*
//SLAVE GENERAL
{
<InitCmds>
	<InitCmd>
		<Transition>PI</Transition>
		<Transition>BI</Transition>
		<Transition>SI</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[set device state to INIT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>288</Ado>
		<Data>1100</Data>
		<Retries>3</Retries>
		<Timeout>5000</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>SI</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[clear DC activation]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>2432</Ado>
		<Data>0000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PI</Transition>
		<Transition>SI</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[check device state for INIT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>304</Ado>
		<Data>0000</Data>
		<Retries>3</Retries>
		<Validate>
			<Data>0100</Data>
			<DataMask>0f00</DataMask>
			<Timeout>5000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>BI</Transition>
		<Comment><![CDATA[check device state for INIT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>304</Ado>
		<Data>0000</Data>
		<Retries>3</Retries>
		<Validate>
			<Data>0100</Data>
			<DataMask>0f00</DataMask>
			<Timeout>10000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>IB</Transition>
		<Comment><![CDATA[set device state to INIT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>288</Ado>
		<Data>1100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Timeout>3000</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>IB</Transition>
		<Comment><![CDATA[check device state for INIT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>304</Ado>
		<Data>0000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>0100</Data>
			<DataMask>0f00</DataMask>
			<Timeout>3000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[assign EEPROM to ECAT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1280</Ado>
		<Data>00</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check vendor id]]></Comment>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1282</Ado>
		<Data>000108000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check vendor id]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>1288</Ado>
		<Data>00000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>02000000</Data>
			<Timeout>100</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check product code]]></Comment>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1282</Ado>
		<Data>00010a000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check product code]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>1288</Ado>
		<Data>00000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>12605614</Data>
			<Timeout>100</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check revision number (lo word)]]></Comment>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1282</Ado>
		<Data>00010c000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check revision number (lo word)]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>1288</Ado>
		<Data>0000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>0000</Data>
			<Timeout>100</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check revision number (hi word >=)]]></Comment>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1282</Ado>
		<Data>00010d000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check revision number (hi word >=)]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>1</Cmd>
		<Adp>0</Adp>
		<Ado>1288</Ado>
		<Data>0000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>0c00</Data>
			<Timeout>100</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>IB</Transition>
		<Comment><![CDATA[set physical address]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>16</Ado>
		<Data>e903</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>IB</Transition>
		<Transition>PI</Transition>
		<Transition>SI</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[clear sm 0/1 (mailbox out/in)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2048</Ado>
		<Data>00000000000000000000000000000000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>BI</Transition>
		<Comment><![CDATA[clear sm 0/1 (mailbox out/in)]]></Comment>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>2048</Ado>
		<Data>00000000000000000000000000000000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[set sm 0 (mailbox out)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2048</Ado>
		<Data>0018800026000100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[set sm 1 (mailbox in)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2056</Ado>
		<Data>001a800022000100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IB</Transition>
		<Comment><![CDATA[set sm 0 (bootstrap out)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2048</Ado>
		<Data>0010140226000100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IB</Transition>
		<Comment><![CDATA[set sm 1 (bootstrap in)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2056</Ado>
		<Data>0018140222000100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set DC cycle time]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2464</Ado>
		<Data>90d00300f0b31a00</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set DC start time]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2448</Ado>
		<Data>e093040000000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set DC activation]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2432</Ado>
		<Data>3007</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>PP</Transition>
		<Comment><![CDATA[clear DC activation]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2432</Ado>
		<Data>0000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>SP</Transition>
		<Transition>OP</Transition>
		<Comment><![CDATA[set device state to PREOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>288</Ado>
		<Data>1200</Data>
		<Retries>300</Retries>
		<Timeout>200</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>SP</Transition>
		<Transition>SI</Transition>
		<Transition>OP</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[clear sms]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2064</Ado>
		<Data>00000000000000000000000000000000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set sm 2 (outputs)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2064</Ado>
		<Data>00100c0024000100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set sm 3 (inputs)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2072</Ado>
		<Data>00110c0022000100</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set fmmu 0 (outputs)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1536</Ado>
		<Data>000000010c0000070010000201000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set fmmu 1 (inputs)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1552</Ado>
		<Data>000000010c0000070011000101000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>IB</Transition>
		<Comment><![CDATA[set fmmu 2 (mailbox state)]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1568</Ado>
		<Data>00000009010000000d08000101000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>OS</Transition>
		<Comment><![CDATA[set device state to SAFEOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>288</Ado>
		<Data>0400</Data>
		<Retries>3</Retries>
		<Timeout>200</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>SP</Transition>
		<Transition>SI</Transition>
		<Transition>OP</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[clear fmmu 0]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1536</Ado>
		<Data>00000000000000000000000000000000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>SP</Transition>
		<Transition>SI</Transition>
		<Transition>OP</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[clear fmmu 1]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1552</Ado>
		<Data>00000000000000000000000000000000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>PI</Transition>
		<Transition>BI</Transition>
		<Transition>SI</Transition>
		<Transition>OI</Transition>
		<Comment><![CDATA[clear fmmu 2]]></Comment>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1568</Ado>
		<Data>00000000000000000000000000000000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>SP</Transition>
		<Transition>OP</Transition>
		<Comment><![CDATA[clear DC activation]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>2432</Ado>
		<Data>0000</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>SP</Transition>
		<Transition>OP</Transition>
		<Comment><![CDATA[check device state for PREOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>4</Cmd>
		<Adp>1001</Adp>
		<Ado>304</Ado>
		<Data>000000000000</Data>
		<Retries>3</Retries>
		<Validate>
			<Data>020000000000</Data>
			<DataMask>0f0000000000</DataMask>
			<Timeout>5000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>IB</Transition>
		<Comment><![CDATA[assign EEPROM to PDI]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>1280</Ado>
		<Data>01</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>II</Transition>
		<Comment><![CDATA[assign EEPROM back to ECAT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1280</Ado>
		<Data>00</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[set device state to PREOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>288</Ado>
		<Data>1200</Data>
		<Cnt>1</Cnt>
		<Retries>300</Retries>
		<Timeout>3000</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Comment><![CDATA[check device state for PREOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>4</Cmd>
		<Adp>1001</Adp>
		<Ado>304</Ado>
		<Data>000000000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>020000000000</Data>
			<DataMask>1f0000000000</DataMask>
			<Timeout>3000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>IP</Transition>
		<Transition>BI</Transition>
		<Comment><![CDATA[assign EEPROM back to ECAT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>2</Cmd>
		<Adp>0</Adp>
		<Ado>1280</Ado>
		<Data>00</Data>
		<Retries>3</Retries>
	</InitCmd>
	<InitCmd>
		<Transition>IB</Transition>
		<Comment><![CDATA[set device state to BOOT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>288</Ado>
		<Data>1300</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Timeout>3000</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>IB</Transition>
		<Comment><![CDATA[check device state for BOOT]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>4</Cmd>
		<Adp>1001</Adp>
		<Ado>304</Ado>
		<Data>000000000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>030000000000</Data>
			<DataMask>1f0000000000</DataMask>
			<Timeout>3000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[set device state to SAFEOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>288</Ado>
		<Data>0400</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Timeout>10000</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>PS</Transition>
		<Comment><![CDATA[check device state for SAFEOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>4</Cmd>
		<Adp>1001</Adp>
		<Ado>304</Ado>
		<Data>000000000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>040000000000</Data>
			<DataMask>1f0000000000</DataMask>
			<Timeout>10000</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>OS</Transition>
		<Comment><![CDATA[check device state for SAFEOP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>4</Cmd>
		<Adp>1001</Adp>
		<Ado>304</Ado>
		<Data>000000000000</Data>
		<Retries>3</Retries>
		<Validate>
			<Data>040000000000</Data>
			<DataMask>0f0000000000</DataMask>
			<Timeout>200</Timeout>
		</Validate>
	</InitCmd>
	<InitCmd>
		<Transition>SO</Transition>
		<Comment><![CDATA[set device state to OP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>5</Cmd>
		<Adp>1001</Adp>
		<Ado>288</Ado>
		<Data>0800</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Timeout>10000</Timeout>
	</InitCmd>
	<InitCmd>
		<Transition>SO</Transition>
		<Comment><![CDATA[check device state for OP]]></Comment>
		<Requires>cycle</Requires>
		<Cmd>4</Cmd>
		<Adp>1001</Adp>
		<Ado>304</Ado>
		<Data>000000000000</Data>
		<Cnt>1</Cnt>
		<Retries>3</Retries>
		<Validate>
			<Data>080000000000</Data>
			<DataMask>1f0000000000</DataMask>
			<Timeout>10000</Timeout>
		</Validate>
	</InitCmd>
</InitCmds>

*/
