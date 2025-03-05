# Viz2 Attention Tracking Tool for Architects

## Overview

Viz2 is a Unreal Engine editor project, which allows for visual attention tracking and metrics calculation in both Virtual Reality and with standard first person (FPS) controls. VR mode assumes the use of the HP G2 Reverb Omnicept headset, which enables eye-tracking via the HPGlia plugin, an updated version of which is included here.
This program was made for and with [taktonika](https://www.taktonika.com), and with their [Rhinoceros](https://www.rhino3d.com/)-based asset pipeline in mind.

It is specifically an editor project, which means it is not made for building a standalone app. The code is organized into two plugins, EyeTrackingUtilityEditor and EyeTrackingUtilityRuntime.
An editor widget allows (in addition to a few other conveniences) for the conversion of one or more static meshes into trackable actors (*heatmap-ready actors*) with one click. Additionally, metrics calculation can be activated for each of those actors via a check box in their details panel, where they can also given a name under which they'd appear on the metrics panel.
Play-In-Editor can be started both as VR and non-VR (FPS), controls are adapted automatically. In both modes, attention recordings can be made; in both FPS mode and from the editor widget, the drawing of the resulting heatmaps can then be re-watched in realtime, or loaded at once.
As of yet, metrics are only accessible via FPS mode.

Heatmaps are painted using render targets, i.e. alpha maps drawn in real time. This unfortunately means that they won't be correct if there are overlapping sections in the UV maps!

## Build & Run

Please note that this software has only ever been built, run, and tested on Windows 10 (64-bit).
As is, it requires Unreal Engine 5.3 specifically, as well as a current version of Microsoft Visual Studio. To use newer Unreal Engine versions, both this project and the HPGlia plugin would have to be updated manually.

To build and run the project, create a VS solution from the `.uproject` file: Right click -> `Generate Visual Studio project files.`
Build and run the solution. From the editor, load the widget: Right click `Content/EyeTracking/UI/WBP_EyeTrackingInterface_RT` -> `Run Editor Utility Widget`.
The controls configuration can be found in the IMC assets in `Content/EyeTracking/Input`.

For VR in Unreal Engine 5.x, [SteamVR](https://store.steampowered.com/app/250820/SteamVR/) in tandem with [OpenXR Explorer](https://github.com/maluoi/openxr-explorer) is recommended.
To use eye-tracking with the HP headset, a (free) [developer license](https://omnicept-console.hpbp.io) is required. The credentials have to be copied into the `Make HPGliaConnectionSettings` node in the Blueprint `Content/EyeTracking/Blueprint/BP_EyeTrackingCharacter_BASE`.

## Demo

[Demo](https://youtu.be/GPDkiOAOd_M). The environment shown in the video is not included, but a simple test map with a few recorded heatmaps is.

## Note

An earlier version of this project used vertex color painting instead of render targets. That didn't work, since it required somewhat ordered and rather dense asset topologies which the Rhinoceros assets this project was made to work with wouldn't necessarily possess.
However, a side product of this detour was another small Unreal Engine plugin to automatically increase vertex density of cuboids along the longest two axes, to enable vertex color painting on walls, floors, and ceilings.
This plugin can be found [here](https://github.com/sc0000/ue-cuboid-subdivider).
