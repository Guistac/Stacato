
set(STACATO_SOURCE_FILES

    ${STACATO_SOURCE_DIRECTORY}/Core/config.h.in
    ${STACATO_SOURCE_DIRECTORY}/Core/entry.cpp
    ${STACATO_SOURCE_DIRECTORY}/Core/pch.cpp
    ${STACATO_SOURCE_DIRECTORY}/Core/pch.h

    ${STACATO_SOURCE_DIRECTORY}/Visualizer/ofRenderer.h
    ${STACATO_SOURCE_DIRECTORY}/Visualizer/ofRenderer.cpp
    ${STACATO_SOURCE_DIRECTORY}/Visualizer/Visualizer.h
    ${STACATO_SOURCE_DIRECTORY}/Visualizer/Visualizer.cpp

    ${STACATO_SOURCE_DIRECTORY}/Scripting/Script.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/Script.cpp
    ${STACATO_SOURCE_DIRECTORY}/Scripting/LuaLibraryHelper.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/EnvironnementLibrary.cpp
    ${STACATO_SOURCE_DIRECTORY}/Scripting/EnvironnementLibrary.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/LoggingLibrary.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/LoggingLibrary.cpp
    ${STACATO_SOURCE_DIRECTORY}/Scripting/CanvasLibrary.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/CanvasLibrary.cpp
    ${STACATO_SOURCE_DIRECTORY}/Scripting/PsnLibrary.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/PsnLibrary.cpp
    ${STACATO_SOURCE_DIRECTORY}/Scripting/ArtNetLibrary.h
    ${STACATO_SOURCE_DIRECTORY}/Scripting/ArtNetLibrary.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/ApplicationWindow.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/ApplicationWindow.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Gui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/MenuBar.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Toolbar.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Popups.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Layout.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Layout.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Window.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Window.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/ImGuiCustomConfiguration.h

    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Colors.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Colors.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Fonts.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Fonts.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Images.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Images.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/EnvironnementGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/EnvironnementGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeEditorGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeManagerGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/HomingGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/Dashboard/Dashboard.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/Dashboard/Dashboard.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/Dashboard/Widget.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/Dashboard/Widget.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/Dashboard/Managers.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/Dashboard/Managers.cpp
    
    ${STACATO_SOURCE_DIRECTORY}/Gui/Machine/MachineGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Animation/ManoeuvreGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Animation/AnimationGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Animation/CurveEditorGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeAdder.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeGraphGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeGraphGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/PinGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/PinIcon.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/PlotGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/PlotGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/PlaybackGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/PlaybackGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/SequencerGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/SequencerGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/SequencerLibrary.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/SequencerLibrary.h

    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/DS402Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatDataGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatDeviceListGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatMetricsGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatParametersGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatSlaveGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Fieldbus/EtherCatStartModal.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Project/ProjectGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Project/FileIO.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/CustomWidgets.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/CustomWidgets.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/ReorderableList.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/FileDialog.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/FileDialog.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/HelpMarker.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/HelpMarker.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/Log.h

    ${STACATO_SOURCE_DIRECTORY}/Motion/MotionTypes.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/SubDevice.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/Profile.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/Curve.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/Curve.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/KinematicInterpolation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/LinearInterpolation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/StepInterpolation.cpp

    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatable.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatable.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/AnimationValue.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/AnimationValue.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animation.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/AnimationKey.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/AnimationComposite.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/TargetAnimation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/SequenceAnimation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Manoeuvre.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Manoeuvre.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/AnimationConstraint.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/Playback/Playback.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/Playback/Playback.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Playback/TimeStringConversion.h

    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatablePosition.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatablePosition.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatablePositionGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatableState.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatableState.cpp
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatableReal.h
    ${STACATO_SOURCE_DIRECTORY}/Animation/Animatables/AnimatableReal.cpp

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Template/MachineTemplate.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Template/MachineTemplate.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Template/MachineTemplateGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Plot/Plot.cpp
    ${STACATO_SOURCE_DIRECTORY}/Plot/Plot.h
    ${STACATO_SOURCE_DIRECTORY}/Plot/ManoeuvreList.cpp
    ${STACATO_SOURCE_DIRECTORY}/Plot/ManoeuvreList.h
    ${STACATO_SOURCE_DIRECTORY}/Plot/PlotSaveFile.cpp
    ${STACATO_SOURCE_DIRECTORY}/Plot/Sequencer.cpp
    ${STACATO_SOURCE_DIRECTORY}/Plot/Sequencer.h

    ${STACATO_SOURCE_DIRECTORY}/Networking/Network.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/Network.h
    ${STACATO_SOURCE_DIRECTORY}/Networking/NetworkDevice.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/NetworkDevice.h

    ${STACATO_SOURCE_DIRECTORY}/Environnement/Environnement.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/Environnement.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/EnvironnementScript.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/EnvironnementScript.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/EnvironnementSaveFile.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeGraph.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeGraph.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/Node.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/Node.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/DeviceNode.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/DeviceNode.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeLink.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodePin.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodePin.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeGraphProcessing.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeGraphSaveFile.cpp

    ${STACATO_SOURCE_DIRECTORY}/Nodes/NodeFactory.h
    ${STACATO_SOURCE_DIRECTORY}/Nodes/NodeFactory.cpp
    ${STACATO_SOURCE_DIRECTORY}/Nodes/EtherCatNodes.cpp
    ${STACATO_SOURCE_DIRECTORY}/Nodes/MotionNodes.cpp
    ${STACATO_SOURCE_DIRECTORY}/Nodes/NetworkNodes.cpp
    ${STACATO_SOURCE_DIRECTORY}/Nodes/ProcessorNodes.cpp

    ${STACATO_SOURCE_DIRECTORY}/Project/Project.cpp
    ${STACATO_SOURCE_DIRECTORY}/Project/Project.h
    ${STACATO_SOURCE_DIRECTORY}/Project/ProjectSaveFile.cpp

    ${STACATO_SOURCE_DIRECTORY}/Project/Editor/CommandHistory.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Editor/CommandHistory.cpp
    ${STACATO_SOURCE_DIRECTORY}/Project/Editor/Parameter.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Editor/List.h

    ${STACATO_SOURCE_DIRECTORY}/Project/Base/Component.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/Serializable.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/Document.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/SerializableList.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/Parameter.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/Parameter.cpp
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/ListParameter.h
    ${STACATO_SOURCE_DIRECTORY}/Project/Base/PrototypeBase.h

    ${STACATO_SOURCE_DIRECTORY}/Utilities/CircularBuffer.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/Enumerator.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/Logger.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/ProgressIndicator.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/Random.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/ScrollingBuffer.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/Timing.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/Units.h
    ${STACATO_SOURCE_DIRECTORY}/Utilities/Units.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/EtherCatFieldbus.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/EtherCatFieldbus.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/EtherCatDevice.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/EtherCatDevice.cpp

    ${STACATO_SOURCE_DIRECTORY}/Console/Console.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/Console.h
    ${STACATO_SOURCE_DIRECTORY}/Console/Serial.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/Serial.h
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleHandler.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleHandlerGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleHandler.h
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleMapping.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleMapping.h
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleIODevice.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/ConsoleIODevice.h

    ${STACATO_SOURCE_DIRECTORY}/Console/Consoles/ConsoleStarmania.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/Consoles/ConsoleStarmaniaGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Console/Consoles/ConsoleStarmania.h

    ${STACATO_SOURCE_DIRECTORY}/Tests/C_Curves.cpp
    ${STACATO_SOURCE_DIRECTORY}/Tests/C_Curves.h
    ${STACATO_SOURCE_DIRECTORY}/Tests/CommandZ.cpp
    ${STACATO_SOURCE_DIRECTORY}/Tests/CommandZ.h
    ${STACATO_SOURCE_DIRECTORY}/Tests/EsiReader.cpp
    ${STACATO_SOURCE_DIRECTORY}/Tests/EsiReader.h

    #===================== ETHERCAT DEVICE DRIVERS ======================

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatData.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatData.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatEepromTool.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatError.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatError.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatMetrics.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/EtherCatPDO.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/FieldbusXML.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/DS402.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/DS402.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/DS402Axis.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/DS402Axis.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/ModularDeviceProfile.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/ModularDeviceProfile.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Utilities/ModularDeviceProfileGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32i.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32i.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32iGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/ATV320.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/ATV320.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/ATV320Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/ATV340.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/ATV340.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/ATV340Gui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VIPA-053-1EC01Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VipaModule.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VipaModule.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VipaModuleGui.cpp

    ##${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Nanotec/PD4_E.h
    ##${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Nanotec/PD4_E.cpp
    ##${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Nanotec/PD4_EGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/ABB/MicroFlexE190.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/ABB/MicroFlexE190.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/ABB/MicroFlexE190Gui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/PhoenixContact/BusCouper_IL-EC-BK.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/PhoenixContact/BusCouper_IL-EC-BK.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/PhoenixContact/BusCouper_IL-EC-BK_Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/PhoenixContact/ioModules.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/PhoenixContact/ioModules.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/PhoenixContact/ioModulesGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Beckhoff/EtherCatJunctions.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Beckhoff/EtherCatJunctions.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Beckhoff/EtherCatJunctionsGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/ICPDAS/EtherCatFiberConverter.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/ICPDAS/EtherCatFiberConverter.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/ICPDAS/EtherCatFiberConverterGui.cpp

    #========================== MACHINE TYPES ===========================

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Basic/PositionControlledMachine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Basic/PositionControlledMachine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Basic/PositionControlledMachineGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/FlipStateMachine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/FlipStateMachine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/FlipStateMachineGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/AxisStateMachine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/AxisStateMachine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/AxisStateMachineGui.cpp

    #========================= AXIS TYPES =========================

    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/Axis.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/Axis.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/AxisGui.cpp

    #========================== SAFETY NODES ======================

    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/DeadMansSwitch.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/DeadMansSwitch.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/DeadMansSwitchGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/SafetySignal.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/SafetySignal.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/SafetySignalGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/Brake.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/Brake.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Safety/BrakeGui.cpp


    #========================== PROCESSOR NODES ========================

    ${STACATO_SOURCE_DIRECTORY}/Nodes/Processors/ClockNode.h
    ${STACATO_SOURCE_DIRECTORY}/Nodes/Processors/DisplayNode.h
    ${STACATO_SOURCE_DIRECTORY}/Nodes/Processors/GroupNode.h
    ${STACATO_SOURCE_DIRECTORY}/Nodes/Processors/MathNodes.h
    ${STACATO_SOURCE_DIRECTORY}/Nodes/Processors/PlotterNode.h

    #========================== NETWORKING NODES =========================

    ${STACATO_SOURCE_DIRECTORY}/Networking/Osc/OscDevice.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/Osc/OscDevice.h
    ${STACATO_SOURCE_DIRECTORY}/Networking/Osc/OscDeviceGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/Osc/OscSocket.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/Osc/OscSocket.h
    ${STACATO_SOURCE_DIRECTORY}/Networking/Psn/PsnServer.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/Psn/PsnServer.h
    ${STACATO_SOURCE_DIRECTORY}/Networking/Psn/PsnServerGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/ArtNet/ArtnetNode.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/ArtNet/ArtnetNode.h
    ${STACATO_SOURCE_DIRECTORY}/Networking/ArtNet/ArtnetNodeGui.cpp
)

# match file structure for source files in ide
source_group(TREE ${STACATO_SOURCE_DIRECTORY} FILES ${STACATO_SOURCE_FILES})
# append generated configuration header
list(APPEND STACATO_SOURCE_FILES ${PROJECT_BINARY_DIR}/src/Core/config.h)

#[[
    set(MACOSX_BUNDLE_ICON_FILE Stacato_AppIcon.icns)
    set(Stacato_ICON ${CMAKE_CURRENT_SOURCE_DIR}/dir/Resources/Stacato_AppIcon.icns)
    set_source_files_properties(${Stacato_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(Stacato_FILE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/dir/Resources/Stacato_FileIcon.icns)
    set_source_files_properties(${Stacato_FILE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
]]

# Stacato Executable
if(WIN32)
    option(STACATO_WIN32_APPLICATION ON)
    if(STACATO_WIN32_APPLICATION)
        set(STACATO_EXECUTABLE_TYPE WIN32)
    endif()
elseif(APPLE)
    set(STACATO_EXECUTABLE_TYPE MACOSX_BUNDLE)
endif()
add_executable(${PROJECT_NAME} ${STACATO_EXECUTABLE_TYPE} ${Stacato_ICON} ${Stacato_FILE_ICON} ${STACATO_SOURCE_FILES})






# Precompiled Headers
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/Core/pch.h)

# Add the following include directories to the project
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src		
    ${PROJECT_SOURCE_DIR}/src/Core  # to include the precompiled header file
    ${PROJECT_BINARY_DIR}/src/Core  # to include the custom config.h file
    ${PROJECT_BINARY_DIR}/deps/glm
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
)


# Include Dependencies
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies.cmake)

# Link Dependencies
target_link_libraries(${PROJECT_NAME} ${STACATO_DEPENDENCIES})










if(WIN32)
    target_link_directories(${PROJECT_NAME} PRIVATE
        #=== directory of the wpcap .lib binary included in soem repo ===
        #=== soem CMakeLists.txt handles linking wpcap.lib ===
        ${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Lib/x64/
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        #=== include path for wpcap.lib headers ===
        ${PROJECT_SOURCE_DIR}/dependencies/soem/oshw/win32/wpcap/Include
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WINDOWS _WIN32_WINNT=0x0601 WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dir"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    set_target_properties(${PROJECT_NAME}
        #sets debug working direction for visual studio solution
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dir"
    )
    # Make the executable a default target to build & run in Visual Studio
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()


if(APPLE)

    set(resourceDirectory ${PROJECT_SOURCE_DIR}/dir/Resources)
    set(applicationResourceDirectory $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources)

    set(removeWorkingDirectoryLogsCommand ${CMAKE_COMMAND} -E remove_directory ${resourceDirectory}/logs)
    set(addWorkingDirectoryLogDirectoryCommand ${CMAKE_COMMAND} -E make_directory ${resourceDirectory}/logs)
    set(removeReleaseExecutableResources ${CMAKE_COMMAND} -E remove_directory ${applicationResourceDirectory})

    set(copyResourcesToExecutableCommand ${CMAKE_COMMAND} -E copy_directory ${resourceDirectory} ${applicationResourceDirectory})  

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${removeWorkingDirectoryLogsCommand}
        COMMAND ${addWorkingDirectoryLogDirectoryCommand}
        COMMAND "$<$<CONFIG:Release>:${removeReleaseExecutableResources}>"
        COMMAND "$<$<CONFIG:Release>:${copyResourcesToExecutableCommand}>"
        COMMAND_EXPAND_LISTS
    )    

    target_compile_definitions(${PROJECT_NAME} PRIVATE MACOS)
    set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dir/Resources")
    set_property(GLOBAL PROPERTY XCODE_SCHEME_EXECUTABLE ${PROJECT_NAME})
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/src/Core/Info.plist")
endif()

