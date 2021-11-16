<Plot Name="Default Plot">
    <Manoeuvre Name="K-0" Description="Initial Key Position" Type="KeyPosition">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1"/>
        </Track>
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="2"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-0" Description="First Manoeuvre" Type="MovementSequence">
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="2.4730723994278483" TimeOffset="4" RampIn="1" RampsAreEqual="true">
            <Target Type="Kinematic" Real="1.1446245481159019"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-1" Description="(copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="1.5308537151838189" TimeOffset="0.49256198242436033" RampIn="2" RampsAreEqual="true">
            <Target Type="Kinematic" Real="0.3508826252160675"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-2" Description="(copy) (copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="10" TimeOffset="0" RampIn="0.29999999999999999" RampsAreEqual="true">
            <Target Type="Kinematic" Real="2"/>
        </Track>
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="10" TimeOffset="0" RampIn="0.5" RampsAreEqual="false" RampOut="0.080000000000000002">
            <Target Type="Kinematic" Real="3.3724354028271848"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-4" Description="(copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="5" TimeOffset="2" RampIn="0.5" RampsAreEqual="false" RampOut="5"/>
    </Manoeuvre>
    <Manoeuvre Name="M-3" Description="(copy) (copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="7" TimeOffset="3" RampIn="1" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="K-1" Description="Final Key Position" Type="KeyPosition">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="8"/>
        </Track>
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="9"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-7" Description="" Type="TimedMovement">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="12.634577289195239" TimeOffset="0" RampIn="1" RampsAreEqual="false" RampOut="0.10000000000000001">
            <Target Type="Kinematic" Real="2.346741084432109"/>
        </Track>
    </Manoeuvre>
</Plot>
