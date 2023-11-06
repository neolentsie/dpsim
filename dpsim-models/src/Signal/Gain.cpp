/* Copyright 2017-2021 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#include <dpsim-models/Signal/Gain.h>

using namespace CPS;
using namespace CPS::Signal;

Gain::Gain(String name, Logger::Level logLevel) :
	SimSignalComp(name, name, logLevel),
    mInputRef(mAttributes->createDynamic<Real>("input_ref")),
    /// CHECK: Which of these really need to be attributes?
    mInputPrev(mAttributes->create<Real>("input_prev")),
    mStatePrev(mAttributes->create<Real>("state_prev")),
    mOutputPrev(mAttributes->create<Real>("output_prev")),
    mInputCurr(mAttributes->create<Real>("input_curr")),
    mStateCurr(mAttributes->create<Real>("state_curr")),
    mOutputCurr(mAttributes->create<Real>("output_curr")),
	mOutputRef(mAttributes->createDynamic<Real>("output_ref")) { }

void Gain::setParameters(Real K_p) {
	// Input is Gain parameter K_p
    mK_p = K_p;
    SPDLOG_LOGGER_INFO(mSLog, "K_p = {}", mK_p);

	**mInputCurr = mK_p;
    **mInputRef = mK_p;
}


void Gain::setInitialValues(Real input_init, Real state_init, Real output_init) {
	**mInputCurr = input_init;
    **mStateCurr = state_init;
    **mOutputCurr = output_init;

    SPDLOG_LOGGER_INFO(mSLog, "Initial values:");
    SPDLOG_LOGGER_INFO(mSLog, "inputCurrInit = {}, stateCurrInit = {}, outputCurrInit = {}", **mInputCurr, **mStateCurr, **mOutputCurr);
}

void Gain::signalAddPreStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) {
    prevStepDependencies.push_back(mInputCurr);
	prevStepDependencies.push_back(mOutputCurr);
	modifiedAttributes.push_back(mInputPrev);
    modifiedAttributes.push_back(mOutputPrev);
};

void Gain::signalPreStep(Real time, Int timeStepCount) {
    **mInputPrev = **mInputCurr;
    **mStatePrev = **mStateCurr;
    **mOutputPrev = **mOutputCurr;
}

void Gain::signalAddStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) {
	attributeDependencies.push_back(mInputRef);
	modifiedAttributes.push_back(mInputCurr);
    modifiedAttributes.push_back(mOutputCurr);
};

void Gain::signalStep(Real time, Int timeStepCount) {
    **mInputCurr = **mInputRef;

    SPDLOG_LOGGER_INFO(mSLog, "Time {}:", time);
    SPDLOG_LOGGER_INFO(mSLog, "Input values: inputCurr = {}, inputPrev = {}, statePrev = {}", **mInputCurr, **mInputPrev, **mStatePrev);


    **mOutputCurr = mK_p * **mInputCurr;


    SPDLOG_LOGGER_INFO(mSLog, "State values: stateCurr = {}", **mStateCurr);
    SPDLOG_LOGGER_INFO(mSLog, "Output values: outputCurr = {}:", **mOutputCurr);
}

Task::List Gain::getTasks() {
	return Task::List({std::make_shared<PreStep>(*this), std::make_shared<Step>(*this)});
}
