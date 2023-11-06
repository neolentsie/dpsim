/* Copyright 2017-2021 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#include <dpsim-models/EMT/EMT_Ph3_VoltageGain.h>

using namespace CPS;


EMT::Ph3::VoltageGain::VoltageGain(String uid, String name, Logger::Level logLevel, Bool withTrafo) :
	CompositePowerComp<Real>(uid, name, true, true, logLevel),
	mVs(mAttributes->create<Matrix>("Vs", Matrix::Zero(3, 1))),
	mGainVoltage(mAttributes->create<Real>("Vgain", 0)),
	mGainInputs(mAttributes->createDynamic<Real>("voltagectrl_inputs")),
	mGainOutputs(mAttributes->createDynamic<Real>("voltagectrl_outputs")),
	mGainStates(mAttributes->createDynamic<Real>("voltagectrl_states")){

	mPhaseType = PhaseType::ABC;

	setVirtualNodeNumber(1);
	setTerminalNumber(1);

	SPDLOG_LOGGER_INFO(mSLog, "Create {} {}", type(), name);
	**mIntfVoltage = Matrix::Zero(3, 1);
	**mIntfCurrent = Matrix::Zero(3, 1);

	// Create electrical sub components
	mSubCtrledVoltageSource = EMT::Ph3::VoltageSource::make(**mName + "_src", mLogLevel);

	// Pre-step of the subcontrolled voltage source is handled explicitly in mnaParentPreStep
	addMNASubComponent(mSubCtrledVoltageSource, MNA_SUBCOMP_TASK_ORDER::NO_TASK, MNA_SUBCOMP_TASK_ORDER::TASK_BEFORE_PARENT, true);

	//Log subcomponents
	SPDLOG_LOGGER_INFO(mSLog, "Electrical subcomponents: ");
	for (auto subcomp: mSubComponents)
		SPDLOG_LOGGER_INFO(mSLog, "- {}", subcomp->name());

	// Create control sub components
	mGain = Signal::Gain::make(**mName + "_VCO", mLogLevel);

	// Sub voltage source
	//mVs->setReference(mSubCtrledVoltageSource->mVoltageRef);

	// Gain
	mGain->mInputRef->setReference(mK_p);
	mGainVoltage->setReference(mGain->mOutputRef);


	// input, state and output vector for logging
	mGainInputs->setReference(mGain->mInputCurr);
	mGainStates->setReference(mGain->mStateCurr);
	mGainOutputs->setReference(mGain->mOutputCurr);
}

//setter goal voltage and frequency
void EMT::Ph3::VoltageGain::setParameters(Real K_p) {
	mParametersSet = true;

	SPDLOG_LOGGER_INFO(mSLog, "General Parameters:");
	SPDLOG_LOGGER_INFO(mSLog, "K_p={}", K_p);

	mGain->setParameters(K_p);

	**mK_p = K_p;
}


void EMT::Ph3::VoltageGain::setControllerParameters(Real K_p) {

	mParametersSet = true;

	SPDLOG_LOGGER_INFO(mSLog, "General Parameters:");
	SPDLOG_LOGGER_INFO(mSLog, "K_p={}", K_p);

	mGain->setParameters(K_p);

	**mK_p = K_p;
}


void EMT::Ph3::VoltageGain::initializeFromNodesAndTerminals(Real frequency) {



	MatrixComp filterInterfaceInitialVoltage = MatrixComp::Zero(3, 1);
	MatrixComp filterInterfaceInitialCurrent = MatrixComp::Zero(3, 1);


	// Initialize controlled source
	mSubCtrledVoltageSource->setParameters(mVirtualNodes[0]->initialVoltage(), 0.0);

	// Connect electrical subcomponents
	mSubCtrledVoltageSource->connect({ SimNode::GND, mVirtualNodes[0] });

	// Initialize electrical subcomponents
	for (auto subcomp: mSubComponents) {
		subcomp->initialize(mFrequencies);
		subcomp->initializeFromNodesAndTerminals(frequency);
	}

}

void EMT::Ph3::VoltageGain::mnaParentInitialize(Real omega, Real timeStep, Attribute<Matrix>::Ptr leftVector) {
	mTimeStep = timeStep;

	// TODO: these are actually no MNA tasks
	mMnaTasks.push_back(std::make_shared<ControlPreStep>(*this));
	mMnaTasks.push_back(std::make_shared<ControlStep>(*this));
}

void EMT::Ph3::VoltageGain::addControlPreStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) {
	// add pre-step dependencies of subcomponents
	mGain->signalAddPreStepDependencies(prevStepDependencies, attributeDependencies, modifiedAttributes);
}

void EMT::Ph3::VoltageGain::controlPreStep(Real time, Int timeStepCount) {
	// add pre-step of subcomponents
	mGain->signalPreStep(time, timeStepCount);

}

void EMT::Ph3::VoltageGain::addControlStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) {
	// add step dependencies of subcomponents
	mGain->signalAddStepDependencies(prevStepDependencies, attributeDependencies, modifiedAttributes);

	// add step dependencies of component itself
	attributeDependencies.push_back(mIntfCurrent);
	attributeDependencies.push_back(mIntfVoltage);
	modifiedAttributes.push_back(mVs);
}


void EMT::Ph3::VoltageGain::controlStep(Real time, Int timeStepCount) {

	// add step of subcomponents
	mGain->signalStep(time, timeStepCount);
}

void EMT::Ph3::VoltageGain::mnaParentAddPreStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) {
	prevStepDependencies.push_back(mVs);
	prevStepDependencies.push_back(mIntfCurrent);
	prevStepDependencies.push_back(mIntfVoltage);
	attributeDependencies.push_back(mGain->mOutputPrev);
	modifiedAttributes.push_back(mRightVector);
}

void EMT::Ph3::VoltageGain::mnaParentPreStep(Real time, Int timeStepCount) {

	// set Voltage reference for voltage source
	mSubCtrledVoltageSource->mVoltageRef->set(PEAK1PH_TO_RMS3PH * **mVs);

	std::dynamic_pointer_cast<MNAInterface>(mSubCtrledVoltageSource)->mnaPreStep(time, timeStepCount);
	// pre-step of component itself
	mnaApplyRightSideVectorStamp(**mRightVector);
}

void EMT::Ph3::VoltageGain::mnaParentAddPostStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes, Attribute<Matrix>::Ptr &leftVector) {
	attributeDependencies.push_back(leftVector);
	modifiedAttributes.push_back(mIntfVoltage);
	modifiedAttributes.push_back(mIntfCurrent);
}

void EMT::Ph3::VoltageGain::mnaParentPostStep(Real time, Int timeStepCount, Attribute<Matrix>::Ptr &leftVector) {
	mnaCompUpdateCurrent(**leftVector);
	mnaCompUpdateVoltage(**leftVector);
}

//Current update
void EMT::Ph3::VoltageGain::mnaCompUpdateCurrent(const Matrix& leftvector) {


}

//Voltage update
void EMT::Ph3::VoltageGain::mnaCompUpdateVoltage(const Matrix& leftVector) {
	for (auto virtualNode : mVirtualNodes)
		virtualNode->mnaUpdateVoltage(leftVector);
	(**mIntfVoltage)(0,0) = Math::realFromVectorElement(leftVector, matrixNodeIndex(0,0));
	(**mIntfVoltage)(1,0) = Math::realFromVectorElement(leftVector, matrixNodeIndex(0,1));
	(**mIntfVoltage)(2,0) = Math::realFromVectorElement(leftVector, matrixNodeIndex(0,2));
}
