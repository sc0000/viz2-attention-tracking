// Copyright (c) 2025 Sebastian Cyliax

using UnrealBuildTool;
using System.Collections.Generic;

public class Viz2EditorTarget : TargetRules
{
	public Viz2EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Viz2" } );
	}
}
