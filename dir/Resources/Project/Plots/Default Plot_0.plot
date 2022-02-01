<Plot Name="Default Plot">
    <Manoeuvre Name="Arrière" Description="Position Ouverte - Jeu arrière" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1.75"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="65"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Avancement" Description="Avance Ouverte (30s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="30" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="30" TimeOffset="0" RampIn="0.5" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="Avant" Description="Position Ouverte - Jeu Avant" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="83"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="7"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Fermeture" Description="Fermeture et Recul (60s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="60" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="60" TimeOffset="0" RampIn="0.5" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="Fermé" Description="Position Fermée Finale" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Retour" Description="Retour vers la position arrière ouverte" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="40" TimeOffset="0" RampIn="0.5" RampsAreEqual="true">
            <Target Type="Kinematic" Real="65"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="false" MovementTime="25" TimeOffset="15" RampIn="0.10000000000000001" RampsAreEqual="true">
            <Target Type="Kinematic" Real="1.75"/>
        </Track>
    </Manoeuvre>
</Plot>
