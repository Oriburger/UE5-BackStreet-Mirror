// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BackStreetTarget : TargetRules
{
	public BackStreetTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V3;
        ExtraModuleNames.AddRange( new string[] { "BackStreet" } );
    }
}
