<Plot Name="Default Plot">
    <Manoeuvre Name="M-0" Description="First Manoeuvre" Type="MovementSequence">
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="2.4730723994278483" TimeOffset="4" RampIn="1" RampsAreEqual="true">
            <Origin Type="Kinematic" Real="0"/>
            <Target Type="Kinematic" Real="1.1446245481159019"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-1" Description="(copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="1.5308537151838189" TimeOffset="0.49256198242436033" RampIn="2" RampsAreEqual="true">
            <Origin Type="Kinematic" Real="1.4471895385282147"/>
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
    <Manoeuvre Name="M-3" Description="(copy) (copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Costière 1" MachineUniqueID="77" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="5" TimeOffset="0" RampIn="1" RampsAreEqual="true">
            <Target Type="Kinematic" Real="0"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-4" Description="(copy) First Manoeuvre" Type="MovementSequence">
        <Track Machine="Tournette B" MachineUniqueID="81" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="3.861246606395504" TimeOffset="1.3135066953633956" RampIn="5" RampsAreEqual="true">
            <Target Type="Kinematic" Real="0.70114369251301145"/>
        </Track>
    </Manoeuvre>
</Plot>
