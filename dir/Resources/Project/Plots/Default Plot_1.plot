<Plot Name="Default Plot">
    <Manoeuvre Name="Mise" Description="Mise Ouvert Arrière" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1.75"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="65"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Avancement" Description="Avance Ouverte (47s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="47" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
            <Target Type="Kinematic" Real="7"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="47" TimeOffset="0" RampIn="0.5" RampsAreEqual="true">
            <Target Type="Kinematic" Real="83"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Recul" Description="Recul Ouvert (30s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="30" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
            <Target Type="Kinematic" Real="1.75"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="30" TimeOffset="0" RampIn="0.5" RampsAreEqual="true">
            <Target Type="Kinematic" Real="65"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Fermeture" Description="Fermeture (1m40s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="100" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true">
            <Target Type="Kinematic" Real="1"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="100" TimeOffset="0" RampIn="0.5" RampsAreEqual="true">
            <Target Type="Kinematic" Real="0"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Retour Mise Décalé" Description="Retour vers la mise" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="25" TimeOffset="15" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="40" TimeOffset="0" RampIn="0.5" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="Mise Finale" Description="Mise Finale Ouvert Arrière" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1.75"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="65"/>
        </Track>
    </Manoeuvre>
</Plot>
