<Plot Name="Default Plot">
    <Manoeuvre Name="M-0*" Description="(copy) " Type="KeyPosition">
        <Track Machine="Single Axis Linear Machine" MachineUniqueID="46" Parameter="Position" SequenceType="NoMove" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-0**" Description="(copy) (copy) " Type="TimedMovement">
        <Track Machine="Single Axis Linear Machine" MachineUniqueID="46" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="1" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
            <Target Type="Kinematic" Real="1"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-0**" Description="(copy) (copy) " Type="MovementSequence">
        <Track Machine="Single Axis Linear Machine" MachineUniqueID="46" Parameter="Position" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="1" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
            <Origin Type="Kinematic" Real="0"/>
            <Target Type="Kinematic" Real="1"/>
        </Track>
    </Manoeuvre>
</Plot>
