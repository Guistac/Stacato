# Stacato (Stage Control Automation Toolbox)

### Priority Features
- Feature complete plot editor and playback engine (animated tracks / on-the-fly playback start or catch-up, position capture, timed manoeuvres)
- Physical Console compatibility (Serial Link, Communication protocol Software Button Assignement)
- Homing Procedure User Interface
- Simulation Compatible Machines

### Future Milestones
- Stage Visualizer based on 3D models (.dwg importer, stage editor and visualizer)
- Realtime Collision Prediction using stage geometry
- Timecode Device Interface
- Manoeuvre Sequencer (Timeline)
- Gui Layout Modes for different screen counts, formats and usage scenarios
- ArtNet Node for sending 3D feedback data
- Copy/Paste in environnement editor and plot editor
- Undo/Redo in environnement editor and plot editor
- EtherCAT transmission error counter / Network cable diagnostics
- Dead Mans Switch
- Add Custom Layout System to Machine View

### Housekeeping
- Refactor Node Pins (to hold void* of data instead of having a field for each type)
- Add onPinUpdate method to nodes for settings propagation
