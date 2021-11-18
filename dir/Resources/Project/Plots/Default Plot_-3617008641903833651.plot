<Plot Name="Default Plot">
    <Manoeuvre Name="M-0" Description="" Type="KeyPosition">
        <Track Machine="Costière" MachineUniqueID="1" Parameter="Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="100"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-1" Description="" Type="MovementSequence">
        <Track Machine="Costière" MachineUniqueID="1" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="10" TimeOffset="0" RampIn="5" RampsAreEqual="true">
            <Target Type="Kinematic" Real="0"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-1*" Description="(copy) " Type="MovementSequence">
        <Track Machine="Costière" MachineUniqueID="1" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="25.785910778454699" TimeOffset="0" RampIn="3" RampsAreEqual="true">
            <Target Type="Kinematic" Real="470.54112371886725"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-1**" Description="(copy) (copy) " Type="MovementSequence">
        <Track Machine="Costière" MachineUniqueID="1" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="25.57644256437608" TimeOffset="0" RampIn="10" RampsAreEqual="true">
            <Target Type="Kinematic" Real="40.23765813380669"/>
        </Track>
    </Manoeuvre>
</Plot>
