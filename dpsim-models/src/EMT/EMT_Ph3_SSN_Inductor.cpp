/* Copyright 2017-2021 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#include <dpsim-models/EMT/EMT_Ph3_SSN_Inductor.h>

using namespace CPS;

EMT::Ph3::SSN::Inductor::Inductor(String uid, String name, Logger::Level logLevel)
	: SimPowerComp<Real>(uid, name, logLevel) {
	mPhaseType = PhaseType::ABC;
	setTerminalNumber(2);
	**mIntfVoltage = Matrix::Zero(3, 1);
	**mIntfCurrent = Matrix::Zero(3, 1);
    historicCurrent = Matrix::Zero(3, 1);

	///FIXME: Initialization should happen in the base class declaring the attribute. However, this base class is currently not an AttributeList...
	mInductance = CPS::Attribute<Matrix>::create("L", mAttributes);
}

EMT::Ph3::SSN::Inductor::Inductor(String name, Logger::Level logLevel)
					    : Inductor(name, name, logLevel)
{
}

SimPowerComp<Real>::Ptr EMT::Ph3::SSN::Inductor::clone(String name) {
	auto copy = Inductor::make(name, mLogLevel);
	copy->setParameters(**mInductance);
	return copy;
}

void EMT::Ph3::SSN::Inductor::initializeFromNodesAndTerminals(Real frequency) {

	Real omega = 2 * PI * frequency;
	MatrixComp impedance = MatrixComp::Zero(3, 3);
	impedance <<
		Complex(0, omega * (**mInductance)(0, 0)), Complex(0, omega * (**mInductance)(0, 1)), Complex(0, omega * (**mInductance)(0, 2)),
		Complex(0, omega * (**mInductance)(1, 0)), Complex(0, omega * (**mInductance)(1, 1)), Complex(0, omega * (**mInductance)(1, 2)),
		Complex(0, omega * (**mInductance)(2, 0)), Complex(0, omega * (**mInductance)(2, 1)), Complex(0, omega * (**mInductance)(2, 2));

	MatrixComp vInitABC = Matrix::Zero(3, 1);
	vInitABC(0, 0) = RMS3PH_TO_PEAK1PH * initialSingleVoltage(1) - RMS3PH_TO_PEAK1PH * initialSingleVoltage(0);
	vInitABC(1, 0) = vInitABC(0, 0) * SHIFT_TO_PHASE_B;
	vInitABC(2, 0) = vInitABC(0, 0) * SHIFT_TO_PHASE_C;
	**mIntfVoltage = vInitABC.real();
	MatrixComp admittance = impedance.inverse();
	**mIntfCurrent = (admittance * vInitABC).real();

	mSLog->info("\nInductance [H]: {:s}"
				"\nImpedance [Ohm]: {:s}",
				Logger::matrixToString(**mInductance),
				Logger::matrixCompToString(impedance));
	mSLog->info(
		"\n--- Initialization from powerflow ---"
		"\nVoltage across: {:s}"
		"\nCurrent: {:s}"
		"\nTerminal 0 voltage: {:s}"
		"\nTerminal 1 voltage: {:s}"
		"\n--- Initialization from powerflow finished ---",
		Logger::matrixToString(**mIntfVoltage),
		Logger::matrixToString(**mIntfCurrent),
		Logger::phasorToString(RMS3PH_TO_PEAK1PH * initialSingleVoltage(0)),
		Logger::phasorToString(RMS3PH_TO_PEAK1PH * initialSingleVoltage(1)));
}

void EMT::Ph3::SSN::Inductor::mnaInitialize(Real omega, Real timeStep, Attribute<Matrix>::Ptr leftVector) {
	MNAInterface::mnaInitialize(omega, timeStep);

	updateMatrixNodeIndices();
    //update history term
	Dufour_B_k_hat = (timeStep / 2. * (**mInductance).inverse());
	Dufour_W_k_n = Dufour_B_k_hat;
    historicCurrent = Dufour_B_k_hat * **mIntfVoltage + **mIntfCurrent;

	mMnaTasks.push_back(std::make_shared<MnaPreStep>(*this));
	mMnaTasks.push_back(std::make_shared<MnaPostStep>(*this, leftVector));
	**mRightVector = Matrix::Zero(leftVector->get().rows(), 1);

	mSLog->info(
		"\n--- MNA initialization ---"
		"\nInitial voltage {:s}"
		"\nInitial current {:s}"
		"\nhistoric current {:s}"
		"\n--- MNA initialization finished ---",
		Logger::matrixToString(**mIntfVoltage),
		Logger::matrixToString(**mIntfCurrent),
		Logger::matrixToString(historicCurrent));
	mSLog->flush();
}

void EMT::Ph3::SSN::Inductor::mnaApplySystemMatrixStamp(Matrix& systemMatrix) {
	if (terminalNotGrounded(0)) {
		// set upper left block, 3x3 entries
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 0), matrixNodeIndex(0, 0), Dufour_W_k_n(0, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 0), matrixNodeIndex(0, 1), Dufour_W_k_n(0, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 0), matrixNodeIndex(0, 2), Dufour_W_k_n(0, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 1), matrixNodeIndex(0, 0), Dufour_W_k_n(1, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 1), matrixNodeIndex(0, 1), Dufour_W_k_n(1, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 1), matrixNodeIndex(0, 2), Dufour_W_k_n(1, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 2), matrixNodeIndex(0, 0), Dufour_W_k_n(2, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 2), matrixNodeIndex(0, 1), Dufour_W_k_n(2, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 2), matrixNodeIndex(0, 2), Dufour_W_k_n(2, 2));
	}
	if (terminalNotGrounded(1)) {
		// set buttom right block, 3x3 entries
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 0), matrixNodeIndex(1, 0), Dufour_W_k_n(0, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 0), matrixNodeIndex(1, 1), Dufour_W_k_n(0, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 0), matrixNodeIndex(1, 2), Dufour_W_k_n(0, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 1), matrixNodeIndex(1, 0), Dufour_W_k_n(1, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 1), matrixNodeIndex(1, 1), Dufour_W_k_n(1, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 1), matrixNodeIndex(1, 2), Dufour_W_k_n(1, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 2), matrixNodeIndex(1, 0), Dufour_W_k_n(2, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 2), matrixNodeIndex(1, 1), Dufour_W_k_n(2, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 2), matrixNodeIndex(1, 2), Dufour_W_k_n(2, 2));
	}
	// Set off diagonal blocks, 2x3x3 entries
	if (terminalNotGrounded(0) && terminalNotGrounded(1)) {
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 0), matrixNodeIndex(1, 0), -Dufour_W_k_n(0, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 0), matrixNodeIndex(1, 1), -Dufour_W_k_n(0, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 0), matrixNodeIndex(1, 2), -Dufour_W_k_n(0, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 1), matrixNodeIndex(1, 0), -Dufour_W_k_n(1, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 1), matrixNodeIndex(1, 1), -Dufour_W_k_n(1, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 1), matrixNodeIndex(1, 2), -Dufour_W_k_n(1, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 2), matrixNodeIndex(1, 0), -Dufour_W_k_n(2, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 2), matrixNodeIndex(1, 1), -Dufour_W_k_n(2, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(0, 2), matrixNodeIndex(1, 2), -Dufour_W_k_n(2, 2));


		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 0), matrixNodeIndex(0, 0), -Dufour_W_k_n(0, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 0), matrixNodeIndex(0, 1), -Dufour_W_k_n(0, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 0), matrixNodeIndex(0, 2), -Dufour_W_k_n(0, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 1), matrixNodeIndex(0, 0), -Dufour_W_k_n(1, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 1), matrixNodeIndex(0, 1), -Dufour_W_k_n(1, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 1), matrixNodeIndex(0, 2), -Dufour_W_k_n(1, 2));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 2), matrixNodeIndex(0, 0), -Dufour_W_k_n(2, 0));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 2), matrixNodeIndex(0, 1), -Dufour_W_k_n(2, 1));
		Math::addToMatrixElement(systemMatrix, matrixNodeIndex(1, 2), matrixNodeIndex(0, 2), -Dufour_W_k_n(2, 2));
	}
}

void EMT::Ph3::SSN::Inductor::mnaApplyRightSideVectorStamp(Matrix& rightVector) {
	// Update internal state
	historicCurrent = Dufour_B_k_hat * **mIntfVoltage + **mIntfCurrent;
	if (terminalNotGrounded(0)) {
		Math::setVectorElement(rightVector, matrixNodeIndex(0, 0), historicCurrent(0, 0));
		Math::setVectorElement(rightVector, matrixNodeIndex(0, 1), historicCurrent(1, 0));
		Math::setVectorElement(rightVector, matrixNodeIndex(0, 2), historicCurrent(2, 0));
	}
	if (terminalNotGrounded(1)) {
		Math::setVectorElement(rightVector, matrixNodeIndex(1, 0), -historicCurrent(0, 0));
		Math::setVectorElement(rightVector, matrixNodeIndex(1, 1), -historicCurrent(1, 0));
		Math::setVectorElement(rightVector, matrixNodeIndex(1, 2), -historicCurrent(2, 0));
	}
	mSLog->debug(
		"\nHistory current term (mnaApplyRightSideVectorStamp): {:s}",
		Logger::matrixToString(historicCurrent));
	mSLog->flush();
}

void EMT::Ph3::SSN::Inductor::mnaAddPreStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) {
	// actually depends on L, but then we'd have to modify the system matrix anyway
	prevStepDependencies.push_back(attribute("v_intf"));
	prevStepDependencies.push_back(attribute("i_intf"));
	modifiedAttributes.push_back(attribute("right_vector"));
}

void EMT::Ph3::SSN::Inductor::mnaPreStep(Real time, Int timeStepCount) {
	mnaApplyRightSideVectorStamp(**mRightVector);
}

void EMT::Ph3::SSN::Inductor::mnaAddPostStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes, Attribute<Matrix>::Ptr &leftVector) {
	attributeDependencies.push_back(leftVector);
	modifiedAttributes.push_back(attribute("v_intf"));
	modifiedAttributes.push_back(attribute("i_intf"));
}

void EMT::Ph3::SSN::Inductor::mnaPostStep(Real time, Int timeStepCount, Attribute<Matrix>::Ptr &leftVector) {
	mnaUpdateVoltage(**leftVector);
	mnaUpdateCurrent(**leftVector);
}

void EMT::Ph3::SSN::Inductor::mnaUpdateVoltage(const Matrix& leftVector) {
	// v1 - v0
	**mIntfVoltage = Matrix::Zero(3, 1);
	if (terminalNotGrounded(1)) {
		(**mIntfVoltage)(0, 0) = Math::realFromVectorElement(leftVector, matrixNodeIndex(1, 0));
		(**mIntfVoltage)(1, 0) = Math::realFromVectorElement(leftVector, matrixNodeIndex(1, 1));
		(**mIntfVoltage)(2, 0) = Math::realFromVectorElement(leftVector, matrixNodeIndex(1, 2));
	}
	if (terminalNotGrounded(0)) {
		(**mIntfVoltage)(0, 0) = (**mIntfVoltage)(0, 0) - Math::realFromVectorElement(leftVector, matrixNodeIndex(0, 0));
		(**mIntfVoltage)(1, 0) = (**mIntfVoltage)(1, 0) - Math::realFromVectorElement(leftVector, matrixNodeIndex(0, 1));
		(**mIntfVoltage)(2, 0) = (**mIntfVoltage)(2, 0) - Math::realFromVectorElement(leftVector, matrixNodeIndex(0, 2));
	}
	mSLog->debug(
		"\nUpdate Voltage: {:s}",
		Logger::matrixToString(**mIntfVoltage)
	);
}

void EMT::Ph3::SSN::Inductor::mnaUpdateCurrent(const Matrix& leftVector) {
	**mIntfCurrent = historicCurrent + Dufour_W_k_n * **mIntfVoltage;
	mSLog->debug(
		"\nUpdate Current: {:s}",
		Logger::matrixToString(**mIntfCurrent)
	);
	mSLog->flush();
}
