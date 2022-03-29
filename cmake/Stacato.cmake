
set(STACATO_SOURCE_FILES

    ${STACATO_SOURCE_DIRECTORY}/Core/config.h.in
    ${STACATO_SOURCE_DIRECTORY}/Core/entry.cpp
    ${STACATO_SOURCE_DIRECTORY}/Core/pch.cpp
    ${STACATO_SOURCE_DIRECTORY}/Core/pch.h

    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/ApplicationWindow.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/ApplicationWindow.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Gui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/MenuBar.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Toolbar.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/Popups.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/ApplicationWindow/ImGuiCustomConfiguration.h

    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Colors.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Colors.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Fonts.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Assets/Fonts.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/EnvironnementGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/EnvironnementGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/EnvironnementEditorGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/MachineManagerGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/DeviceManagerGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/HomingGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/MachineListGui.cpp
    
    ${STACATO_SOURCE_DIRECTORY}/Gui/Machine/MachineGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Machine/AnimatableParameterGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Motion/ManoeuvreGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Motion/ParameterTrackGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeAdder.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeGraphGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeGraphGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/NodeGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/PinGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Environnement/NodeGraph/PinIcon.cpp

    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/PlotGui.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Plot/PlotGui.cpp

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

    ${STACATO_SOURCE_DIRECTORY}/Gui/StageView/GlApplet.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/StageView/GlApplet.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/StageView/StageView.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/StageView/StageView.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/StageView/StageViewApplet.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/StageView/StageViewApplet.h

    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/CustomWidgets.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/CustomWidgets.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/DraggableList.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/FileDialog.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/FileDialog.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/HelpMarker.cpp
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/HelpMarker.h
    ${STACATO_SOURCE_DIRECTORY}/Gui/Utilities/Log.h

    ${STACATO_SOURCE_DIRECTORY}/Motion/MotionTypes.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Playback.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Playback.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/SubDevice.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Profile.h

    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/Curve.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/Curve.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/KinematicInterpolation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/LinearInterpolation.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Curve/StepInterpolation.cpp

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/AnimatableParameter.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/AnimatableParameter.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/AnimatableParameterValue.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/AnimatableParameterValue.h

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Template/MachineTemplate.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Template/MachineTemplate.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Template/MachineTemplateGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Motion/Manoeuvre/Manoeuvre.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Manoeuvre/Manoeuvre.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Manoeuvre/ManoeuvreXML.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Manoeuvre/ParameterTrack.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Manoeuvre/ParameterTrack.h

    ${STACATO_SOURCE_DIRECTORY}/Plot/Plot.cpp
    ${STACATO_SOURCE_DIRECTORY}/Plot/Plot.h
    ${STACATO_SOURCE_DIRECTORY}/Plot/PlotSaveFile.cpp

    ${STACATO_SOURCE_DIRECTORY}/Networking/Network.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/Network.h
    ${STACATO_SOURCE_DIRECTORY}/Networking/NetworkDevice.cpp
    ${STACATO_SOURCE_DIRECTORY}/Networking/NetworkDevice.h

    ${STACATO_SOURCE_DIRECTORY}/Environnement/Environnement.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/Environnement.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/EnvironnementSaveFile.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/Node.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/Node.cpp
    ${STACATO_SOURCE_DIRECTORY}/Environnement/DeviceNode.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/DeviceNode.cpp

    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeGraph.h
    ${STACATO_SOURCE_DIRECTORY}/Environnement/NodeGraph/NodeGraph.cpp
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

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/SchneiderElectric/Lexium32Gui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VIPA-053-1EC01Gui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VipaModule.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VipaModule.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Yaskawa/VipaModuleGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Nanotec/PD4_E.h
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Nanotec/PD4_E.cpp
    ${STACATO_SOURCE_DIRECTORY}/Fieldbus/Devices/Nanotec/PD4_EGui.cpp

    #========================== MACHINE TYPES ===========================

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Safety/DeadMansSwitch.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Safety/DeadMansSwitch.h

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Basic/PositionControlledSingleAxisMachine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Basic/PositionControlledSingleAxisMachine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Basic/PositionControlledSingleAxisMachineGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Animated/BinaryOscillator6x.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Animated/BinaryOscillator6x.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Animated/BinaryOscillator6xGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Animated/Oscillator3x.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Animated/Oscillator3x.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Animated/Oscillator3xGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Special/LinearMecanumClaw.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Special/LinearMecanumClaw.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Special/LinearMecanumClawGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/HoodedLiftStateMachine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/HoodedLiftStateMachine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/StateMachines/HoodedLiftStateMachineGui.cpp

    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Utility/PositionFeedbackMachine.cpp
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Utility/PositionFeedbackMachine.h
    ${STACATO_SOURCE_DIRECTORY}/Machine/Machines/Utility/PositionFeedbackMachineGui.cpp

    #========================= AXIS TYPES =========================

    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/PositionControlledAxis.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/PositionControlledAxis.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/PositionControlledAxisGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/VelocityControlledAxis.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/VelocityControlledAxis.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Axis/VelocityControlledAxisGui.cpp

    #========================== MOTION ADAPTERS =======================

    ${STACATO_SOURCE_DIRECTORY}/Motion/Adapters/ActuatorToServoActuator.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Adapters/ActuatorToServoActuator.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Adapters/ActuatorToServoActuatorGui.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Adapters/GpioActuator.cpp
    ${STACATO_SOURCE_DIRECTORY}/Motion/Adapters/GpioActuator.h
    ${STACATO_SOURCE_DIRECTORY}/Motion/Adapters/GpioActuatorGui.cpp

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
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dir/Resources"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE MACOS)
    set_property(TARGET ${PROJECT_NAME} PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/dir/Resources")
    set_property(GLOBAL PROPERTY XCODE_SCHEME_EXECUTABLE ${PROJECT_NAME})
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/src/Core/Info.plist")
endif()

