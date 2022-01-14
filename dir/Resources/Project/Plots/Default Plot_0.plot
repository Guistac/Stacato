<Plot Name="Default Plot">
    <Manoeuvre Name="M-0" Description="" Type="MovementSequence">
        <Track Machine="Linear Machine" MachineUniqueID="51" Parameter="Position" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="5" TimeOffset="0" RampIn="1" RampsAreEqual="true">
            <Origin Type="Kinematic" Real="0"/>
            <Target Type="Kinematic" Real="1"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-0*" Description="(copy) " Type="MovementSequence">
        <Track Machine="Linear Machine" MachineUniqueID="51" Parameter="Position" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="5" TimeOffset="0" RampIn="1" RampsAreEqual="true">
            <Target Type="Kinematic" Real="2"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-2" Description="" Type="KeyPosition">
        <Track Machine="Linear Machine" MachineUniqueID="51" Parameter="Position" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="5"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-2*" Description="(copy) " Type="KeyPosition">
        <Track Machine="Linear Machine" MachineUniqueID="51" Parameter="Position" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="5"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-2*" Description="(copy) " Type="KeyPosition">
        <Track Machine="Linear Machine" MachineUniqueID="51" Parameter="Position" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="5"/>
        </Track>
    </Manoeuvre>
</Plot>
