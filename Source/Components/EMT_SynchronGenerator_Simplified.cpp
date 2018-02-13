﻿/** Simplified Synchron generator (EMT)
*
* @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
* @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
*
* DPsim
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************/

#include "EMT_SynchronGenerator_Simplified.h"
#include "../IntegrationMethod.h"

using namespace DPsim;

Components::EMT::SynchronGeneratorSimplified::SynchronGeneratorSimplified(String name, Int node1, Int node2, Int node3,
	Real nomPower, Real nomVolt, Real nomFreq, Int poleNumber, Real nomFieldCur,
	Real Rs, Real Ll, Real Lmd, Real Lmd0, Real Lmq, Real Lmq0,
	Real Rfd, Real Llfd, Real Rkd, Real Llkd,
	Real Rkq1, Real Llkq1, Real Rkq2, Real Llkq2,
	Real inertia, Logger::Level logLevel)
	: SynchronGeneratorBase(name, node1, node2, node3, nomPower, nomVolt, nomFreq, poleNumber, nomFieldCur,
		Rs, Ll, Lmd, Lmd0, Lmq, Lmq0, Rfd, Llfd, Rkd, Llkd, Rkq1, Llkq1, Rkq2, Llkq2,
		inertia, logLevel) {

}

Components::EMT::SynchronGeneratorSimplified::~SynchronGeneratorSimplified()
{
	if (mLogLevel != Logger::Level::NONE) {
		delete mLog;
	}
}

//void Components::EMT::SynchronGeneratorSimplified::applySystemMatrixStamp(SystemModel& system)
//{
//		mR_eq2 <<
//				-(mRs), (mLl + mLmq),
//				-(mLl + mLmd) + mLmd*mLmd / (mLlfd + mLmd), -(mRs);
//
//		mKrs_teta <<
//				2. / 3. * cos(mThetaMech), 2. / 3. * cos(mThetaMech - 2. * M_PI / 3.), 2. / 3. * cos(mThetaMech + 2. * M_PI / 3.),
//				2. / 3. * sin(mThetaMech), 2. / 3. * sin(mThetaMech - 2. * M_PI / 3.), 2. / 3. * sin(mThetaMech + 2. * M_PI / 3.);
//
//		mKrs_teta_inv <<
//				cos(mThetaMech), sin(mThetaMech),
//				cos(mThetaMech - 2. * M_PI / 3.), sin(mThetaMech - 2. * M_PI / 3.),
//				cos(mThetaMech + 2. * M_PI / 3.), sin(mThetaMech + 2. * M_PI / 3.);
//
//		mG_eq_abc2 = mKrs_teta_inv*mR_eq2.inverse()*mKrs_teta;
//		mG_eq_abc2 = mG_eq_abc2*mBase_Z;
//
//		//Update Equivalent Resistance
//		system.addRealToSystemMatrix(mNode1, mNode1, mG_eq_abc2(0, 0));
//		system.addRealToSystemMatrix(mNode1, mNode2, mG_eq_abc2(0, 1));
//		system.addRealToSystemMatrix(mNode1, mNode3, mG_eq_abc2(0, 2));
//		system.addRealToSystemMatrix(mNode2, mNode1, mG_eq_abc2(1, 0));
//		system.addRealToSystemMatrix(mNode2, mNode2, mG_eq_abc2(1, 1));
//		system.addRealToSystemMatrix(mNode2, mNode3, mG_eq_abc2(1, 2));
//		system.addRealToSystemMatrix(mNode3, mNode1, mG_eq_abc2(2, 0));
//		system.addRealToSystemMatrix(mNode3, mNode2, mG_eq_abc2(2, 1));
//		system.addRealToSystemMatrix(mNode3, mNode3, mG_eq_abc2(2, 2));
//}

void Components::EMT::SynchronGeneratorSimplified::initialize(Real om, Real dt,
	Real initActivePower, Real initReactivePower, Real initTerminalVolt,
	Real initVoltAngle, Real initFieldVoltage, Real initMechPower)
{
	// Create matrices for state space representation
	mVoltages = Matrix::Zero(3, 1);
	mFluxes = Matrix::Zero(3, 1);
	mCurrents = Matrix::Zero(3, 1);
	mInductanceMat = Matrix::Zero(3, 3);
	mResistanceMat = Matrix::Zero(3, 3);
	mReactanceMat = Matrix::Zero(3, 3);
	mOmegaFluxMat = Matrix::Zero(3, 3);

	mInductanceMat <<
		-(mLl + mLmq), 0, 0,
		0, -(mLl + mLmd), mLmd,
		0, -mLmd, mLlfd + mLmd;

	mResistanceMat <<
		mRs, 0, 0,
		0, mRs, 0,
		0, 0, -mRfd;

	mOmegaFluxMat <<
		0, 1, 0,
		-1, 0, 0,
		0, 0, 0;

	// Determinant of Ld (inductance matrix of d axis)
	//detLd = -(mLmd + mLl)*(mLlfd + mLmd) + mLmd*mLmd;

	mReactanceMat = mInductanceMat.inverse();

	// steady state per unit initial value
	initStatesInPerUnit(initActivePower, initReactivePower, initTerminalVolt, initVoltAngle, initFieldVoltage, initMechPower);

	// Calculation of operational parameters
	mXd = mOmMech*(mLl + mLmd);
	mXq = mOmMech*(mLl + mLmq);
	mTd0_t = (mLlfd + mLmd) / (mRfd*mBase_OmMech);
	mXd_t = mOmMech*(mLl + mLmd - mLmd*mLmd / (mLlfd + mLmd));
	mEq_t = mVq + mXd_t*mId;
	mEf = mOmMech*(mLmd / mRfd) * mVfd;

	mVd = -mPsiq;
	mVq = mPsid;

	mVa = inverseParkTransform(mThetaMech, mVd* mBase_v, mVq* mBase_v, mV0* mBase_v)(0);
	mVb = inverseParkTransform(mThetaMech, mVd* mBase_v, mVq* mBase_v, mV0* mBase_v)(1);
	mVc = inverseParkTransform(mThetaMech, mVd* mBase_v, mVq* mBase_v, mV0* mBase_v)(2);

	mIa = inverseParkTransform(mThetaMech, mId* mBase_i, mIq* mBase_i, mI0* mBase_i)(0);
	mIb = inverseParkTransform(mThetaMech, mId* mBase_i, mIq* mBase_i, mI0* mBase_i)(1);
	mIc = inverseParkTransform(mThetaMech, mId* mBase_i, mIq* mBase_i, mI0* mBase_i)(2);
}

void Components::EMT::SynchronGeneratorSimplified::step(SystemModel& system, Real time)
{

	mG_load(0, 0) = system.getCurrentSystemMatrix()(mNode1, mNode1);
	mG_load(0, 1) = system.getCurrentSystemMatrix()(mNode1, mNode2);
	mG_load(0, 2) = system.getCurrentSystemMatrix()(mNode1, mNode3);
	mG_load(1, 0) = system.getCurrentSystemMatrix()(mNode2, mNode1);
	mG_load(1, 1) = system.getCurrentSystemMatrix()(mNode2, mNode2);
	mG_load(1, 2) = system.getCurrentSystemMatrix()(mNode2, mNode3);
	mG_load(2, 0) = system.getCurrentSystemMatrix()(mNode3, mNode1);
	mG_load(2, 1) = system.getCurrentSystemMatrix()(mNode3, mNode2);
	mG_load(2, 2) = system.getCurrentSystemMatrix()(mNode3, mNode3);
	mR_load = mG_load.inverse() / mBase_Z;


	stepInPerUnit(system.getOmega(), system.getTimeStep(), time, system.getNumMethod());


	Matrix mConductanceMat = Matrix::Zero(3, 3);
	Matrix mIabc_eq = Matrix::Zero(3, 1);
	mConductanceMat = (mR_eq_abc*mBase_Z).inverse();
	mIabc_eq = mConductanceMat*mEq_abc*mBase_v;

	// Update current source accordingly
	if (mNode1 >= 0) {
			system.addRealToRightSideVector(mNode1, mIa);
	}
	if (mNode2 >= 0) {
			system.addRealToRightSideVector(mNode2, mIb);
	}
	if (mNode3 >= 0) {
			system.addRealToRightSideVector(mNode3, mIc);
	}

	if (mLogLevel != Logger::Level::NONE) {
		Matrix logValues(getFluxes().rows() + getVoltages().rows() + getCurrents().rows() + 3, 1);
		logValues << getFluxes(), getVoltages(), getCurrents(), getElectricalTorque(), getRotationalSpeed(), getRotorPosition();
		mLog->LogDataLine(time, logValues);
	}
}

void Components::EMT::SynchronGeneratorSimplified::stepInPerUnit(Real om, Real dt, Real time, NumericalMethod numMethod)
{

		mVabc <<
				mVa / mBase_v,
				mVb / mBase_v,
				mVc / mBase_v;


		// Calculation of rotational speed with euler
		mElecTorque = (mPsid*mIq - mPsiq*mId);
		mOmMech = mOmMech + dt * (1 / (2 * mH) * (mMechTorque - mElecTorque));

		// Calculation of rotor angular position
		mThetaMech = mThetaMech + dt * (mOmMech * mBase_OmMech);

		mPsifd = mPsifd + dt*mBase_OmMech*(mVfd - mRfd*mIfd);

		mR_eq <<
				-mRs, (mLl + mLmq),
				-(mLl + mLmd) + mLmd*mLmd / (mLlfd + mLmd), -mRs;
		mE_eq <<
				0,
				(mLmd)*mPsifd / (mLlfd + mLmd);

		mKrs_teta <<
				2. / 3. * cos(mThetaMech), 2. / 3. * cos(mThetaMech - 2. * M_PI / 3.), 2. / 3. * cos(mThetaMech + 2. * M_PI / 3.),
				2. / 3. * sin(mThetaMech), 2. / 3. * sin(mThetaMech - 2. * M_PI / 3.), 2. / 3. * sin(mThetaMech + 2. * M_PI / 3.);

		mKrs_teta_inv <<
				cos(mThetaMech), sin(mThetaMech),
				cos(mThetaMech - 2. * M_PI / 3.), sin(mThetaMech - 2. * M_PI / 3.),
				cos(mThetaMech + 2. * M_PI / 3.), sin(mThetaMech + 2. * M_PI / 3.);

		mR_eq_abc = mKrs_teta_inv*mR_eq*mKrs_teta;
		mG_eq_abc = (mR_eq_abc + mR_load).inverse();
		mEq_abc = mKrs_teta_inv*mE_eq;

	
		mIabc = -mG_eq_abc*mEq_abc;


		mIdq = mKrs_teta*mIabc;

		mId = mIdq(0);
		mIq = mIdq(1);
		mIfd = (mPsifd + mLmd*mId) / (mLlfd + mLmd);

		mCurrents(0, 0) = mIq;
		mCurrents(1, 0) = mId;
		mCurrents(2, 0) = mIfd;

		mFluxes = mInductanceMat*mCurrents;
		
		mPsiq = mFluxes(0, 0);
		mPsid = mFluxes(1, 0);
		mPsifd = mFluxes(2, 0);

		mIa = mBase_i *  mIabc(0);
		mIb = mBase_i *  mIabc(1);
		mIc = mBase_i *  mIabc(2);


		mVoltages << mVq,
				mVd,
				mVfd;
}


void Components::EMT::SynchronGeneratorSimplified::postStep(SystemModel& system)
{
	if (mNode1 >= 0) {
		mVa = system.getRealFromLeftSideVector(mNode1);
	}
	else {
		mVa = 0;
	}

	if (mNode2 >= 0) {
		mVb = system.getRealFromLeftSideVector(mNode2);
	}
	else {
		mVb = 0;
	}

	if (mNode3 >= 0) {
		mVc = system.getRealFromLeftSideVector(mNode3);
	}
	else {
		mVc = 0;
	}



}

Matrix Components::EMT::SynchronGeneratorSimplified::parkTransform(Real theta, Real a, Real b, Real c)
{
	Matrix dq0vector(3, 1);

	// Park transform according to Kundur
	Real d, q, zero;

	d = 2. / 3. * cos(theta)*a + 2. / 3. * cos(theta - 2. * M_PI / 3.)*b + 2. / 3. * cos(theta + 2. * M_PI / 3.)*c;
	q = -2. / 3. * sin(theta)*a - 2. / 3. * sin(theta - 2. * M_PI / 3.)*b - 2. / 3. * sin(theta + 2. * M_PI / 3.)*c;

	//Real zero;
	zero = 1. / 3. * a, 1. / 3. * b, 1. / 3. * c;

	dq0vector << d,
		q,
		0;

	return dq0vector;
}

Matrix Components::EMT::SynchronGeneratorSimplified::inverseParkTransform(Real theta, Real d, Real q, Real zero)
{
	Matrix abcVector(3, 1);

	// Park transform according to Kundur
	Real a, b, c;

	a = cos(theta)*d - sin(theta)*q + 1.*zero;
	b = cos(theta - 2. * M_PI / 3.)*d - sin(theta - 2. * M_PI / 3.)*q + 1.*zero;
	c = cos(theta + 2. * M_PI / 3.)*d - sin(theta + 2. * M_PI / 3.)*q + 1.*zero;

	abcVector << a,
		b,
		c;

	return abcVector;
}
