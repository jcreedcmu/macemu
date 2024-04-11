#pragma once

#include <Windows.h>

typedef ProcPtr TEClickLoopUPP;

// Got these values from
// https://android.googlesource.com/platform/prebuilts/python/windows-x86/+/studio-master-release/x64/Lib/plat-mac/Carbon/Controls.py
#define kControlIndicatorPart 129
#define kControlUpButtonPart 20
#define kControlDownButtonPart 21
#define kControlPageUpPart 22
#define kControlPageDownPart 23

// pascal void DisposeRoutineDescriptor (ControlActionUPP theProcPtr);

enum { OSTrap, ToolTrap };

extern pascal Ptr GetApplLimit();
