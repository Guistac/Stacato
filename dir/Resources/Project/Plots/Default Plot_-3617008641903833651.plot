<Plot Name="Default Plot">
    <Manoeuvre Name="M-0" Description="" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="0"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="1"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="2"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="3"/>
                </ParameterTrack>
            </ChildParameterTracks>
        </Track>
        <Track Machine="Costière" MachineUniqueID="208" Parameter="Position" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="-1"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-0*" Description="(copy) " Type="MovementSequence">
        <Track Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="1" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="1" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="1" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="1" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
            </ChildParameterTracks>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="M-0**" Description="(copy) (copy) " Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="10"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="15"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="20"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="201" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="25"/>
                </ParameterTrack>
            </ChildParameterTracks>
        </Track>
        <Track Machine="Costière" MachineUniqueID="208" Parameter="Position" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="5"/>
        </Track>
    </Manoeuvre>
</Plot>
