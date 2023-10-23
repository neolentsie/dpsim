/* Copyright 2017-2021 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#include <dpsim-models/DP/DP_Ph1_SynchronGenerator4OrderPCM.h>

using namespace CPS;

DP::Ph1::SynchronGenerator4OrderPCM::SynchronGenerator4OrderPCM
    (const String& uid, const String& name, Logger::Level logLevel)
	: Base::ReducedOrderSynchronGenerator<Complex>(uid, name, logLevel),
	mEdq_t(mAttributes->create<Matrix>("Edq_t"))  {

	mSGOrder = SGOrder::SG4Order;
	mPhaseType = PhaseType::Single;
	setTerminalNumber(1);

	/// initialize attributes
	mNumIter = mAttributes->create<Int>("NIterations", 0);

	// model variables
	**mEdq_t = Matrix::Zero(2,1);
}

DP::Ph1::SynchronGenerator4OrderPCM::SynchronGenerator4OrderPCM
	(const String& name, Logger::Level logLevel)
	: SynchronGenerator4OrderPCM(name, name, logLevel) {
}

SimPowerComp<Complex>::Ptr DP::Ph1::SynchronGenerator4OrderPCM::clone(String name) {
	auto copy = SynchronGenerator4OrderPCM::make(name, mLogLevel);

	return copy;
}

void DP::Ph1::SynchronGenerator4OrderPCM::specificInitialization() {
	// calculate state representation matrix
	calculateStateSpaceMatrices();

	// initial voltage behind the transient reactance in the dq0 reference frame
	(**mEdq_t)(0,0) = (**mVdq)(0,0) - (**mIdq)(1,0) * mLq_t;
	(**mEdq_t)(1,0) = (**mVdq)(1,0) + (**mIdq)(0,0) * mLd_t;

	SPDLOG_LOGGER_DEBUG(mSLog,
		"\n--- Model specific initialization  ---"
		"\nInitial Ed_t (per unit): {:f}"
		"\nInitial Eq_t (per unit): {:f}"
		"\nMax number of iterations: {:d}"
		"\nTolerance: {:f}"
		"\nSG Model: 4 Order PCM"
		"\n--- Model specific initialization finished ---",

		(**mEdq_t)(0,0),
		(**mEdq_t)(1,0),
		mMaxIter,
		mTolerance
	);
	mSLog->flush();

	mDomainInterface.setDPShiftFrequency(mBase_OmMech);
}

void DP::Ph1::SynchronGenerator4OrderPCM::calculateStateSpaceMatrices() {
	// Initialize matrices of state representation
	mAStateSpace <<	-mLq / mTq0_t / mLq_t,	0,
              		0,						-mLd / mTd0_t / mLd_t;
	mBStateSpace <<	(mLq-mLq_t) / mTq0_t / mLq_t,	0.0,
					0.0,							(mLd-mLd_t) / mTd0_t / mLd_t;
	mCStateSpace <<	0,
					1 / mTd0_t;

	// Precalculate trapezoidal based matrices (avoids redundant matrix inversions in correction steps)
	Math::calculateStateSpaceTrapezoidalMatrices(mAStateSpace, mBStateSpace, mCStateSpace, mTimeStep, mAdTrapezoidal, mBdTrapezoidal, mCdTrapezoidal);
}

void DP::Ph1::SynchronGenerator4OrderPCM::stepInPerUnit() {
	// set number of iterations equal to zero
	**mNumIter = 0;

	// store values currently at t=k for later use
	mEdqtPrevStep = **mEdq_t;
	mIdqPrevStep = **mIdq;
	mVdqPrevStep = **mVdq;

	// update DQ-DP transforms according to mThetaMech
	mDomainInterface.updateDQToDPTransform(**mThetaMech, mSimTime);
	mDomainInterface.updateDPToDQTransform(**mThetaMech, mSimTime);

	// predict emf at t=k+1 (euler) using
	(**mEdq_t) = Math::StateSpaceEuler(**mEdq_t, mAStateSpace, mBStateSpace, mCStateSpace * **mEf, mTimeStep, **mVdq);

	// predict stator currents at t=k+1 (assuming Vdq(k+1)=Vdq(k))
	(**mIdq)(0,0) = ((**mEdq_t)(1,0) - (**mVdq)(1,0)) / mLd_t;
	(**mIdq)(1,0) = ((**mVdq)(0,0) - (**mEdq_t)(0,0)) / mLq_t;

	// convert currents to dp domain
	(**mIntfCurrent)(0,0) = mDomainInterface.applyDQToDPTransform(**mIdq) * mBase_I_RMS;
}

void DP::Ph1::SynchronGenerator4OrderPCM::mnaCompApplyRightSideVectorStamp(Matrix& rightVector) {
	Math::setVectorElement(rightVector, matrixNodeIndex(0,0), (**mIntfCurrent)(0, 0));
}

void DP::Ph1::SynchronGenerator4OrderPCM::correctorStep() {

	// increase number of iterations
	**mNumIter = **mNumIter + 1;

	// correct electrical vars
	// calculate emf at j and k+1 (trapezoidal rule)
	(**mEdq_t) = Math::applyStateSpaceTrapezoidalMatrices(mAdTrapezoidal, mBdTrapezoidal, mCdTrapezoidal * **mEf, mEdqtPrevStep, **mVdq, mVdqPrevStep);

	// calculate corrected stator currents at t=k+1 (assuming Vdq(k+1)=VdqPrevIter(k+1))
	(**mIdq)(0,0) = ((**mEdq_t)(1,0) - (**mVdq)(1,0) ) / mLd_t;
	(**mIdq)(1,0) = ((**mVdq)(0,0) - (**mEdq_t)(0,0) ) / mLq_t;

	// convert corrected currents to dp domain
	(**mIntfCurrent)(0,0) = mDomainInterface.applyDQToDPTransform(**mIdq) * mBase_I_RMS;

	// stamp currents
	mnaCompApplyRightSideVectorStamp(**mRightVector);
}

void DP::Ph1::SynchronGenerator4OrderPCM::updateVoltage(const Matrix& leftVector) {

	// store voltage value currently at j-1 for later use
	mVdqPrevIter = **mVdq;

	//
	(**mIntfVoltage)(0, 0) = Math::complexFromVectorElement(leftVector, matrixNodeIndex(0, 0));

	// convert armature voltage into dq reference frame
	**mVdq = mDomainInterface.applyDPToDQTransform((**mIntfVoltage)(0, 0)) / mBase_V_RMS;
}

bool DP::Ph1::SynchronGenerator4OrderPCM::requiresIteration() {
	if (**mNumIter >= mMaxIter) {
		// maximum number of iterations reached
		return false;
	} else if (**mNumIter == 0) {
		// no corrector step has been performed yet,
		// convergence cannot be confirmed
		return true;
	} else {
		// check voltage convergence according to tolerance
		Matrix voltageDifference = **mVdq - mVdqPrevIter;
		if (Math::abs(voltageDifference(0,0)) > mTolerance || Math::abs(voltageDifference(1,0)) > mTolerance)
			return true;
		else
			return false;
	}
}

void DP::Ph1::SynchronGenerator4OrderPCM::mnaCompPostStep(const Matrix& leftVector) {
}
