/*
 * GCodeMachineState.cpp
 *
 *  Created on: 15 Nov 2016
 *      Author: David
 */

#include "GCodeMachineState.h"

GCodeMachineState *GCodeMachineState::freeList = nullptr;
unsigned int GCodeMachineState::numAllocated = 0;

// Create a default initialised GCodeMachineState
GCodeMachineState::GCodeMachineState()
	: previous(nullptr), feedRate(DefaultFeedRate * SecondsToMinutes), fileState(), lockedResources(0), errorMessage(nullptr), lineNumber(0),
	  drivesRelative(false), axesRelative(false), doingFileMacro(false), runningM501(false), runningM502(false),
	  volumetricExtrusion(false), g53Active(false), runningSystemMacro(false), usingInches(false),
	  waitingForAcknowledgement(false), messageAcknowledged(false),
	  indentLevel(0), state(GCodeState::normal)
{
}

// Allocate a new GCodeMachineState
/*static*/ GCodeMachineState *GCodeMachineState::Allocate()
{
	GCodeMachineState *ms = freeList;
	if (ms != nullptr)
	{
		freeList = ms->previous;
		ms->lockedResources = 0;
		ms->errorMessage = nullptr;
		ms->state = GCodeState::normal;
	}
	else
	{
		ms = new GCodeMachineState();
		++numAllocated;
	}
	return ms;
}

/*static*/ void GCodeMachineState::Release(GCodeMachineState *ms)
{
	ms->fileState.Close();
	ms->previous = freeList;
	freeList = ms;
}

/*static*/ unsigned int GCodeMachineState::GetNumInUse()
{
	unsigned int inUse = numAllocated;
	for (GCodeMachineState *ms = freeList; ms != nullptr; ms = ms->previous)
	{
		--inUse;
	}
	return inUse;
}

GCodeMachineState::BlockState& GCodeMachineState::CurrentBlockState()
{
	return blockStates[min<size_t>(indentLevel, ARRAY_SIZE(blockStates) - 1)];
}

void GCodeMachineState::CreateBlock()
{
	++indentLevel;
	CurrentBlockState().SetPlainBlock();
}

void GCodeMachineState::EndBlock()
{
	if (indentLevel != 0)
	{
		--indentLevel;
	}
}

// End
