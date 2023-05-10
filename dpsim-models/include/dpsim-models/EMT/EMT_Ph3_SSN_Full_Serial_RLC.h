/* Copyright 2017-2021 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#pragma once

#include <dpsim-models/MNASimPowerComp.h>
#include <dpsim-models/Solver/MNAInterface.h>
#include <dpsim-models/Base/Base_Ph3_Inductor.h>
#include <dpsim-models/Base/Base_Ph3_Capacitor.h>
#include <dpsim-models/Base/Base_Ph3_Resistor.h>

namespace CPS{
    namespace EMT{
        namespace Ph3{
            namespace SSN{
				/// \brief Full_Serial_RLC
				///
				/// This element represents an one port circuit consisting of a resistor,
				/// an inductor and a capacitor connected in series. The terminals are at
				/// the beginning and the end of the component chain.
				///	The states are the capacitor voltage and the inductor current, the output
				/// is the latter of those states (inductor current). The input is the voltage
				/// across the whole circuit. States and past inputs are updated after each
				/// time step and are used to calculate the current (input) voltage, 
				/// represented as MNA node voltages.
                class Full_Serial_RLC:
				    public MNASimPowerComp<Real>,
				    public SharedFactory<Full_Serial_RLC>,
					public Base::Ph3::Resistor,
					public Base::Ph3::Inductor,
					public Base::Ph3::Capacitor
                    {
                protected:
                    Matrix State = Matrix::Zero(6, 1);
                    Matrix yHistory =  Matrix::Zero(3, 1);

					Matrix Dufour_u_n_t = Matrix::Zero(3, 1);

                    Matrix Dufour_A_k_hat = Matrix::Zero(6, 6);
					Matrix Dufour_B_k_hat = Matrix::Zero(6, 3);
                    Matrix Dufour_B_k_n_hat = Matrix::Zero(6, 3);
					Matrix Dufour_W_k_n = Matrix::Zero(3, 3);
                    Matrix Dufour_C_k_n = Matrix(3, 6);
				public:
                    /// Defines UID, name, component parameters and logging level
				    Full_Serial_RLC(String uid, String name, Logger::Level logLevel = Logger::Level::off);
				    /// Defines name and logging level
				    Full_Serial_RLC(String name, Logger::Level logLevel = Logger::Level::off)
											    : Full_Serial_RLC(name, name, logLevel) {}

				    SimPowerComp<Real>::Ptr clone(String name);
                    void setParameters(Matrix resistance, Matrix inductance, Matrix capacitance);

				    // #### General ####
				    /// Initializes component from power flow data
				    void initializeFromNodesAndTerminals(Real frequency);

			    	// #### MNA section ####
				    /// Initializes internal variables of the component
				    void mnaCompInitialize(Real omega, Real timeStep, Attribute<Matrix>::Ptr leftVector) override;
				    /// Stamps system matrix
				    void mnaCompApplySystemMatrixStamp(SparseMatrixRow& systemMatrix) override;
				    /// Stamps right side (source) vector
				    void mnaCompApplyRightSideVectorStamp(Matrix& rightVector) override;
				    /// Update interface voltage from MNA system result
				    void mnaCompUpdateVoltage(const Matrix& leftVector) override;
				    /// Update interface current from MNA system result
				    void mnaCompUpdateCurrent(const Matrix& leftVector) override;
				    /// MNA pre step operations
				    void mnaCompPreStep(Real time, Int timeStepCount) override;
				    /// MNA post step operations
				    void mnaCompPostStep(Real time, Int timeStepCount, Attribute<Matrix>::Ptr &leftVector) override;
				    /// Add MNA pre step dependencies
				    void mnaCompAddPreStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes) override;
				    /// Add MNA post step dependencies
				    void mnaCompAddPostStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes, Attribute<Matrix>::Ptr &leftVector) override;

					void ssnUpdateState();

					bool isLinear() const
					{
						return true;
					}
                };    
            }
        }
    }
}