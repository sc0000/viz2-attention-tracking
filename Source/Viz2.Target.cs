// Copyright (c) 2025 Sebastian Cyliax

using UnrealBuildTool;
using System.Collections.Generic;

public class Viz2Target : TargetRules
{
	public Viz2Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Viz2" } );
	}
}
