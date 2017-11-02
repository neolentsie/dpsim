/** Ideal voltage source
 *
 * @file
 * @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
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

#pragma once

#include "BaseComponent.h"

namespace DPsim {

	/// Ideal Voltage source model:
	/// This model uses modified nodal analysis to represent an ideal voltage source.
	/// For a voltage source between nodes j and k, a new variable (current across the voltage source) is added to the left side vector
	/// as unkown and it is taken into account for the equation of node j as positve and for the equation of node k as negative. Moreover
	/// a new equation ej - ek = V is added to the problem.
	class IdealVoltageSource : public BaseComponent {

	protected:
		//  ### Ideal Voltage source parameters ###
		/// Complex voltage [V]
		Complex mVoltage;

		Real mVoltageAtSourcer;
		Real mVoltageAtSourcei;
		Real mVoltageAtDestr;
		Real mVoltageAtDesti;
		Real mCurrentr;
		Real mCurrenti;

		/// Number of voltage source (first, second...)
		Int mNumber;

	public:
		IdealVoltageSource() { ; };

		/// define paramenters of the voltage source
		IdealVoltageSource(String name, Int src, Int dest, Complex voltage);

		void init(Real om, Real dt) { }

		/// Inserts the current across the voltage source in the equations of node j and k and add the equantion ej - ek = V to the problem
		void applySystemMatrixStamp(SystemModel& system);

		/// Stamps voltage source to the current vector
		void applyRightSideVectorStamp(SystemModel& system);

		/// Stamps voltage source to the current vector
		void step(SystemModel& system, Real time);

		void postStep(SystemModel& system) { }

		virtual Complex getCurrent(SystemModel& system);
	};
}
