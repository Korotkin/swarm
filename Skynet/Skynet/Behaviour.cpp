#include "Behaviour.h"

#include "GoalAction.h"
#include "BasicUnitAction.h"
#include "TrainScarabAction.h"
#include "PsiStormAction.h"
#include "MineDragAction.h"
#include "ArconZealotKillUnDetected.h"
#include "ArbiterAction.h"
#include "DetectorAction.h"
#include "QueenAction.h"
#include "BurrowAction.h"

Behaviour::Behaviour(Unit unit)
	: mUnit(unit)
{
	createDefaultActions();
}

Behaviour::Behaviour(Unit unit, const std::list<MicroAction> &microActions)
	: mUnit(unit)
	, mMicroActions(microActions)
{
}

void Behaviour::addMicroAction(MicroAction action)
{
	mMicroActions.push_front(action);
}

void Behaviour::update(const Goal &squadGoal, const UnitGroup &squadUnitGroup)
{
	for (MicroAction action : mMicroActions)
	{
		if(action->update(squadGoal, squadUnitGroup))
			break;
	}

	for(std::list<MicroAction>::iterator it = mMicroActions.begin(); it != mMicroActions.end();)
	{
		if((*it)->hasEnded())
			mMicroActions.erase(it++);
		else
			++it;
	}
}

void Behaviour::onDeleted()
{
	for (MicroAction action : mMicroActions)
	{
		action->removeUnit(mUnit);
	}

	mMicroActions.clear();
	mUnit.reset();
}

void Behaviour::set(Unit unit)
{
	mUnit = unit;
	createDefaultActions();
}

void Behaviour::set(Unit unit, const std::list<MicroAction> &microActions)
{
	mUnit = unit;
	mMicroActions = microActions;
}

void Behaviour::createDefaultActions()
{
	const BWAPI::UnitType &unitType = mUnit->getType();

	std::list<std::set<BWAPI::UnitType>> targetPriorities;

	std::set<BWAPI::UnitType> firstTargets;

	if(unitType != BWAPI::UnitTypes::Protoss_Zealot)
		firstTargets.insert(BWAPI::UnitTypes::Terran_Vulture_Spider_Mine);

	if(unitType == BWAPI::UnitTypes::Protoss_Corsair)
		firstTargets.insert(BWAPI::UnitTypes::Zerg_Scourge);

	if(unitType == BWAPI::UnitTypes::Protoss_Zealot || unitType == BWAPI::UnitTypes::Protoss_Archon)
	{
		firstTargets.insert(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
		mMicroActions.push_back(MicroAction(new ArconZealotKillUnDetected(mUnit)));
	}

	if(unitType.isDetector())
		mMicroActions.push_back(MicroAction(new DetectorAction(mUnit)));

	if(unitType == BWAPI::UnitTypes::Protoss_Arbiter)
		mMicroActions.push_back(MicroAction(new ArbiterAction(mUnit)));

	if(unitType == BWAPI::UnitTypes::Protoss_Reaver)
		mMicroActions.push_back(MicroAction(new TrainScarabAction(mUnit)));

	if(unitType == BWAPI::UnitTypes::Protoss_High_Templar)
		mMicroActions.push_back(MicroAction(new PsiStormAction(mUnit)));

	if(unitType == BWAPI::UnitTypes::Protoss_Zealot || unitType == BWAPI::UnitTypes::Zerg_Zergling)
		mMicroActions.push_back(MicroAction(new MineDragAction(mUnit)));

	// Zerg part
	if (unitType == BWAPI::UnitTypes::Zerg_Zergling || 
	    unitType == BWAPI::UnitTypes::Zerg_Hydralisk || 
	    unitType == BWAPI::UnitTypes::Zerg_Lurker)
	{
		firstTargets.insert(BWAPI::UnitTypes::Protoss_High_Templar);
		firstTargets.insert(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
		firstTargets.insert(BWAPI::UnitTypes::Protoss_Dark_Archon);
	}
 
	if (unitType == BWAPI::UnitTypes::Zerg_Scourge) 
		firstTargets.insert(BWAPI::UnitTypes::Protoss_Corsair);
	
	if (unitType == BWAPI::UnitTypes::Zerg_Queen) 
		mMicroActions.push_back(MicroAction(new QueenAction(mUnit)));
	
        if (unitType.isBurrowable())
                mMicroActions.push_back(MicroAction(new BurrowAction(mUnit)));
        
	targetPriorities.push_back(firstTargets);
	mMicroActions.push_back(MicroAction(new BasicUnitAction(mUnit, targetPriorities)));
	mMicroActions.push_back(MicroAction(new GoalAction(mUnit)));
	
}