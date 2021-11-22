<Plot Name="Default Plot">
    <Manoeuvre Name="Start" Description="Oscillation Start Position" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="0.029999999999999999"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="0"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="0.29999999999999999"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="10"/>
                </ParameterTrack>
            </ChildParameterTracks>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Sequence" Description="Oscillation Sequence" Type="MovementSequence">
        <Track Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
            </ChildParameterTracks>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="End" Description="Oscillation End Position" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="0.040000000000000001"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="0"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="1"/>
                </ParameterTrack>
                <ParameterTrack Machine="Triple Oscillator" MachineUniqueID="55" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Linear">
                    <Target IsEqualToNextOrigin="false" Type="Real" Real="30"/>
                </ParameterTrack>
            </ChildParameterTracks>
        </Track>
    </Manoeuvre>
</Plot>
