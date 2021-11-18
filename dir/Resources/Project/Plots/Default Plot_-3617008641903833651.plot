<Plot Name="Default Plot">
    <Manoeuvre Name="Closed" Description="All Actuators at Zero" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Axis 1 Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Axis 2 Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Axis 3 Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Open" Description="All Actuators at 100% Travel" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Axis 1 Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Axis 2 Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Axis 3 Position" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Start" Description="Initial Position" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Frequency" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="0.01"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Min Amplitude" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Max Amplitude" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Phase Offset" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="10"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Osc" Description="Oscillation Sequence" Type="MovementSequence">
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Frequency" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="10" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Min Amplitude" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="10" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Max Amplitude" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="10" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Phase Offset" SequenceType="TimedMove" Interpolation="Linear" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="10" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="End" Description="Oscillation End Position" Type="KeyPosition">
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Frequency" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="0.10000000000000001"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Min Amplitude" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="0"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Max Amplitude" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="1"/>
        </Track>
        <Track Machine="Triple Oscillator" MachineUniqueID="146" Parameter="Phase Offset" SequenceType="Constant" Interpolation="Linear">
            <Target IsEqualToNextOrigin="false" Type="Real" Real="10"/>
        </Track>
    </Manoeuvre>
</Plot>
