<Plot Name="Default Plot">
    <Manoeuvre Name="OSC-1" Description="Séquence Portières" Type="MovementSequence">
        <Track Machine="Actuateurs Portières" MachineUniqueID="55" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Actuateurs Portières" MachineUniqueID="55" Parameter="Frequency" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="0.029999999999999999"/>
                    <Target Type="Real" Real="0.029999999999999999"/>
                </ParameterTrack>
                <ParameterTrack Machine="Actuateurs Portières" MachineUniqueID="55" Parameter="Min Amplitude" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="0"/>
                    <Target Type="Real" Real="0"/>
                </ParameterTrack>
                <ParameterTrack Machine="Actuateurs Portières" MachineUniqueID="55" Parameter="Max Amplitude" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="0.29999999999999999"/>
                    <Target Type="Real" Real="1"/>
                </ParameterTrack>
                <ParameterTrack Machine="Actuateurs Portières" MachineUniqueID="55" Parameter="Phase Offset" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="200" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="3"/>
                    <Target Type="Real" Real="3"/>
                </ParameterTrack>
            </ChildParameterTracks>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="OSC-2" Description="Séquence Roues" Type="MovementSequence">
        <Track Machine="Moteurs Roues" MachineUniqueID="129" Parameter="Oscillator" HasChildParameterTracks="true">
            <ChildParameterTracks>
                <ParameterTrack Machine="Moteurs Roues" MachineUniqueID="129" Parameter="Min Off Time" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="50" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="1"/>
                    <Target Type="Real" Real="8"/>
                </ParameterTrack>
                <ParameterTrack Machine="Moteurs Roues" MachineUniqueID="129" Parameter="Max Off Time" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="50" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="2"/>
                    <Target Type="Real" Real="10"/>
                </ParameterTrack>
                <ParameterTrack Machine="Moteurs Roues" MachineUniqueID="129" Parameter="Min On Time" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="50" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="8"/>
                    <Target Type="Real" Real="1"/>
                </ParameterTrack>
                <ParameterTrack Machine="Moteurs Roues" MachineUniqueID="129" Parameter="Max On Time" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="false" TargetIsNextOrigin="false" MovementTime="50" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
                    <Origin Type="Real" Real="10"/>
                    <Target Type="Real" Real="3"/>
                </ParameterTrack>
            </ChildParameterTracks>
        </Track>
    </Manoeuvre>
</Plot>
