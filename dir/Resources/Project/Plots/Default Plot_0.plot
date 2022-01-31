<Plot Name="Default Plot">
    <Manoeuvre Name="Arrière" Description="Position Ouverte - Jeu Arrière" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="1.75"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="65"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Avancement" Description="Avance Ouverte (30s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="30" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="30" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="Avant" Description="Position Ouverte - Jeu Avant" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="7.2999999999999998"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="83"/>
        </Track>
    </Manoeuvre>
    <Manoeuvre Name="Fermeture" Description="Fermeture et Recul (70s)" Type="MovementSequence">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="70" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="TimedMove" Interpolation="Trapezoidal" OriginIsPreviousTarget="true" TargetIsNextOrigin="true" MovementTime="70" TimeOffset="0" RampIn="0.10000000000000001" RampsAreEqual="true"/>
    </Manoeuvre>
    <Manoeuvre Name="Fermé" Description="Position Fermée - Jeu Avant" Type="KeyPosition">
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Position Costière" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="2"/>
        </Track>
        <Track Machine="Coeur" MachineUniqueID="212" Parameter="Angle Coeur" HasChildParameterTracks="false" SequenceType="Constant" Interpolation="Trapezoidal">
            <Target IsEqualToNextOrigin="false" Type="Kinematic" Real="0"/>
        </Track>
    </Manoeuvre>
</Plot>
