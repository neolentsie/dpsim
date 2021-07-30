/* Copyright 2017-2020 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#include <cps/Definitions.h>
#include <cps/SystemTopology.h>
#include <cps/Components.h>

#pragma once

namespace CPS {
namespace CIM {
namespace Examples {

namespace Components {

// P. Kundur, "Power System Stability and Control", Example 13.2, pp. 864-869.
namespace KundurExample1 {
    struct Network {
        Real nomVoltage = 400e3;
    };

    struct Gen {
        Real nomPower = 2220e6;
        Real nomVoltage = 400e3;
        Real H = 3.5;
        Real XpdPU = 0.3;
        Real RsPU = 0;
        Real D = 1.0;
    };
    struct Line1 {
        // Vnom = 400kV
        Real lineResistance = 0.0721;
        Real lineReactance = 36.0360; 
        Real lineSusceptance = 0;
        Real lineConductance =0;
    };

    struct Transf1 {
	    Real nomVoltageHV = 400e3; 
        Real nomVoltageMV = 400e3;
        Real transformerResistance = 0; // referred to HV side
        Real transformerReactance = 10.8108; // referred to HV side
    };
}

namespace SynchronousGeneratorKundur {
    struct MachineParameters {
        // Define machine parameters in per unit
        Real nomPower = 555e6;
        Real nomVoltage = 24e3; // Phase-to-Phase RMS
        Real nomFreq = 60; 
        Real nomFieldCurr = 1300;
        Int poleNum = 2;
        Real H = 3.7;

        // Fundamental parameters
        Real Rs = 0.003;
        Real Ll = 0.15;
        Real Lmd = 1.6599;
        Real Lmq = 1.61;
        Real Rfd = 0.0006;
        Real Llfd = 0.1648;
        Real Rkd = 0.0284;
        Real Llkd = 0.1713;
        Real Rkq1 = 0.0062;
        Real Llkq1 = 0.7252;
        Real Rkq2 = 0.0237;
        Real Llkq2 = 0.125;

        // Operational parameters
        Real Td0_t = 8.0669;
        Real Td0_s = 0.0300;
        Real Td_t = 1.3368;
        Real Td_s = 0.0230;
        Real Ld_t = 0.2999;
        Real Ld_s = 0.2299;
        Real Tq0_t = 0.9991;
        Real Tq0_s = 0.0700;
        Real Lq_t = 0.6500;
        Real Lq_s = 0.2500;
        Real Ld = 1.8099;
        Real Lq = 1.7600;

        Real fieldVoltage = 7.0821; //TODO: specify operating conditions
    };
}

namespace CIGREHVEuropean {
    struct LineParameters {
        // 220 kV
        Real Vnom = 220e3;
        Real nomFreq = 50;
        Real nomOmega= nomFreq* 2*PI;
        //PiLine parameters (given in [p.u/km] with Z_base= 529ohm)
        Real lineResistancePerKm = 1.35e-4  * 529;
        Real lineReactancePerKm = 8.22e-4  * 529;
        Real lineSusceptancePerKm = 1.38e-3  / 529;
        Real lineConductancePerKm = 0;
    };
}
namespace CIGREHVAmerican {
    struct LineParameters {
        // 230 kV
        Real Vnom = 230e3;
        Real nomFreq = 60;
        Real nomOmega= nomFreq* 2*PI;
        //PiLine parameters (given in [p.u/km] with Z_base= 529ohm)
        Real lineResistancePerKm = 1.27e-4 * 529;
        Real lineReactancePerKm = 9.05e-4 * 529;
        Real lineSusceptancePerKm = 1.81e-3 / 529;
        Real lineConductancePerKm = 0;
    };
}
}
namespace SMIB {
    struct ScenarioConfig {
        //-----------Network-----------//
        Real Vnom = 230e3;
        Real nomFreq = 60;
        Real nomOmega= nomFreq* 2*PI;
        //-----------Generator-----------//
        Real nomPower = 500e6;
        Real nomPhPhVoltRMS = 22e3;
        Real H = 5;
        Real Xpd=0.31;
        Real Rs = 0.003*0;
        Real D = 1.5;
        // Initialization parameters
        Real initMechPower= 300e6;
        Real initActivePower = 300e6;
        Real setPointVoltage=nomPhPhVoltRMS + 0.05*nomPhPhVoltRMS;
        //-----------Transformer-----------//
        Real t_ratio=Vnom/nomPhPhVoltRMS;
        //-----------Transmission Line-----------//
        // CIGREHVAmerican (230 kV)
        Components::CIGREHVAmerican::LineParameters lineCIGREHV;
        Real lineLeng=100;
        Real lineResistance = lineCIGREHV.lineResistancePerKm*lineLeng;
        Real lineInductance = lineCIGREHV.lineReactancePerKm/nomOmega*lineLeng;
        Real lineCapacitance = lineCIGREHV.lineSusceptancePerKm/nomOmega*lineLeng;
        Real lineConductance =lineCIGREHV.lineConductancePerKm*lineLeng;;
    };
}

namespace ThreeBus {
    struct ScenarioConfig {
        
        //-----------Network-----------//
        Real Vnom = 230e3;
        Real nomFreq = 60;
        Real nomOmega= nomFreq* 2*PI;

        //-----------Generator 1 (bus1)-----------//
        Real nomPower_G1 = 300e6;
        Real nomPhPhVoltRMS_G1 = 25e3;
        Real nomFreq_G1 = 60;
        Real H_G1 = 6; 
        Real Xpd_G1=0.3; //in p.u
        Real Rs_G1 = 0.003*0; //in p.u
        Real D_G1 = 1.5; //in p.u 
        // Initialization parameters 
        Real initActivePower_G1 = 270e6;
        Real initMechPower_G1 = 270e6;
        Real setPointVoltage_G1=nomPhPhVoltRMS_G1+0.05*nomPhPhVoltRMS_G1;
        
        //-----------Generator 2 (bus2)-----------//
        Real nomPower_G2 = 50e6;
        Real nomPhPhVoltRMS_G2 = 13.8e3;
        Real nomFreq_G2 = 60;
        Real H_G2 = 2; 
        Real Xpd_G2=0.1; //in p.u
        Real Rs_G2 = 0.003*0; //in p.u
        Real D_G2 =1.5; //in p.u
        // Initialization parameters 
        Real initActivePower_G2 = 45e6;
        Real initMechPower_G2 = 45e6;
        Real setPointVoltage_G2=nomPhPhVoltRMS_G2-0.05*nomPhPhVoltRMS_G2;

        //-----------Transformers-----------//
        Real t1_ratio=Vnom/nomPhPhVoltRMS_G1;
        Real t2_ratio=Vnom/nomPhPhVoltRMS_G2;

        //-----------Load (bus3)-----------
        Real activePower_L= 310e6;
        Real reactivePower_L= 150e6;

        // -----------Transmission Lines-----------//
        // CIGREHVAmerican (230 kV)
        Components::CIGREHVAmerican::LineParameters lineCIGREHV;
        //line 1-2 (180km)
        Real lineResistance12 = lineCIGREHV.lineResistancePerKm*180;
        Real lineInductance12 = lineCIGREHV.lineReactancePerKm/nomOmega*180;
        Real lineCapacitance12 = lineCIGREHV.lineSusceptancePerKm/nomOmega*180;
        Real lineConductance12 = lineCIGREHV.lineConductancePerKm*180;
        //line 1-3 (150km)
        Real lineResistance13 = lineCIGREHV.lineResistancePerKm*150;
        Real lineInductance13 = lineCIGREHV.lineReactancePerKm/nomOmega*150;
        Real lineCapacitance13 = lineCIGREHV.lineSusceptancePerKm/nomOmega*150;
        Real lineConductance13 = lineCIGREHV.lineConductancePerKm*150;
        //line 2-3 (80km)
        Real lineResistance23 = lineCIGREHV.lineResistancePerKm*80;
        Real lineInductance23 = lineCIGREHV.lineReactancePerKm/nomOmega*80;
        Real lineCapacitance23 = lineCIGREHV.lineSusceptancePerKm/nomOmega*80;
        Real lineConductance23 = lineCIGREHV.lineConductancePerKm*80;
    };
}

namespace SGIB {

    struct ScenarioConfig {
        Real systemFrequency = 50;
        Real systemNominalVoltage = 20e3;

        // Line parameters (R/X = 1)
        Real length = 5;
        Real lineResistance = 0.5 * length;
	    Real lineInductance = 0.5/314 * length;
        Real lineCapacitance = 50e-6/314 * length;

        // PV controller parameters
        Real scaling_P = 1;
        Real scaling_I = 0.1;

        Real KpPLL = 0.25*scaling_P;
        Real KiPLL = 2*scaling_I;
        Real KpPowerCtrl = 0.001*scaling_P;
        Real KiPowerCtrl = 0.08*scaling_I;
        Real KpCurrCtrl = 0.3*scaling_P;
        Real KiCurrCtrl = 10*scaling_I;
        Real OmegaCutoff = 2 * PI * systemFrequency;

        // Initial state values
        Real thetaPLLInit = 0; // only for debug
        Real phiPLLInit = 0; // only for debug
        Real phi_dInit = 0;
        Real phi_qInit = 0;
        Real gamma_dInit = 0;
        Real gamma_qInit = 0;

        // Nominal generated power values of PV
        Real pvNominalVoltage = 1500.;
        Real pvNominalActivePower = 100e3;
        Real pvNominalReactivePower = 50e3;

        // PV filter parameters
        Real Lf = 0.002;
        Real Cf = 789.3e-6;
        Real Rf = 0.1;
        Real Rc = 0.1;

        // PV connection transformer parameters
        Real transformerNominalPower = 5e6;
        Real transformerInductance = 0.928e-3;

        // Further parameters
        Real systemOmega = 2 * PI * systemFrequency;
    };
}

namespace CIGREMV {

    struct ScenarioConfig {
        Real systemFrequency = 50;
        Real systemNominalVoltage = 20e3;
        Real penetrationLevel = 1;
        Real totalLoad = 4319.1e3; // calculated total load in CIGRE MV left feeder (see CIM data)

        // parameters of one PV unit
        Real pvUnitNominalVoltage = 1500.;
        Real pvUnitNominalPower = 50e3;
        Real pvUnitPowerFactor = 1;

        // calculate PV units per plant to reach penetration level
        Int numberPVUnits = Int(totalLoad * penetrationLevel / pvUnitNominalPower);
        Int numberPVPlants = 9;
        Int numberPVUnitsPerPlant = numberPVUnits / numberPVPlants;

        // PV controller parameters
        Real scaling_P = 10.0;
        Real scaling_I = 1000.0;

        Real KpPLL = 0.25/scaling_P;
        Real KiPLL = 2/scaling_I;
        Real KpPowerCtrl = 0.001/scaling_P;
        Real KiPowerCtrl = 0.08/scaling_I;
        Real KpCurrCtrl = 0.3/scaling_P;
        Real KiCurrCtrl = 10/scaling_I;
        Real OmegaCutoff = 2 * PI * systemFrequency;

        // PV filter parameters
        Real Lf = 0.002;
        Real Cf = 789.3e-6;
        Real Rf = 0.1;
        Real Rc = 0.1;

        // PV connection transformer parameters
        Real transformerNominalPower = 5e6;
        Real transformerInductance = 0.928e-3;

        // Further parameters
        Real systemOmega = 2 * PI * systemFrequency;

        // Initial state values (use known values with scaled control params)
        Real thetaPLLInit = 314.168313-systemOmega;
        Real phiPLLInit = 8e-06;
        Real pInit = 450000.716605;
        Real qInit = -0.577218;
        Real phi_dInit = 3854.197405*scaling_I;
        Real phi_qInit = -0.003737*scaling_I;
        Real gamma_dInit = 128.892668*scaling_I;
        Real gamma_qInit = 23.068682*scaling_I;
    };

    void addInvertersToCIGREMV(SystemTopology& system, CIGREMV::ScenarioConfig scenario, Domain domain) {
        Real pvActivePower = scenario.pvUnitNominalPower*scenario.numberPVUnitsPerPlant;
        Real pvReactivePower = sqrt(std::pow(pvActivePower / scenario.pvUnitPowerFactor, 2) - std::pow(pvActivePower, 2));

        // add PVs to network topology
        for (Int n = 3; n <= 11; ++n) {
            // TODO: cast to BaseAverageVoltageSourceInverter and move set functions out of case distinction
            if (domain == Domain::SP) {
                SimNode<Complex>::Ptr connectionNode = system.node<CPS::SimNode<Complex>>("N" + std::to_string(n));
                auto pv = SP::Ph1::AvVoltageSourceInverterDQ::make("pv_" + connectionNode->name(), "pv_" + connectionNode->name(), Logger::Level::debug, true);
                pv->setParameters(scenario.systemOmega, scenario.pvUnitNominalVoltage, pvActivePower, pvReactivePower);
                pv->setControllerParameters(scenario.KpPLL, scenario.KiPLL, scenario.KpPowerCtrl, scenario.KiPowerCtrl, scenario.KpCurrCtrl, scenario.KiCurrCtrl, scenario.OmegaCutoff);
                pv->setFilterParameters(scenario.Lf, scenario.Cf, scenario.Rf, scenario.Rc);
                pv->setTransformerParameters(scenario.systemNominalVoltage, scenario.pvUnitNominalVoltage, scenario.systemNominalVoltage/scenario.pvUnitNominalVoltage, 0, 0, scenario.transformerInductance);
                pv->setInitialStateValues(scenario.pInit, scenario.qInit, scenario.phi_dInit, scenario.phi_qInit, scenario.gamma_dInit, scenario.gamma_qInit);
                system.addComponent(pv);
                system.connectComponentToNodes<Complex>(pv, { connectionNode });
            } else if (domain == Domain::DP) {
                SimNode<Complex>::Ptr connectionNode = system.node<CPS::SimNode<Complex>>("N" + std::to_string(n));
                auto pv = DP::Ph1::AvVoltageSourceInverterDQ::make("pv_" + connectionNode->name(), "pv_" + connectionNode->name(), Logger::Level::debug, true);
                pv->setParameters(scenario.systemOmega, scenario.pvUnitNominalVoltage, pvActivePower, pvReactivePower);
                pv->setControllerParameters(scenario.KpPLL, scenario.KiPLL, scenario.KpPowerCtrl, scenario.KiPowerCtrl, scenario.KpCurrCtrl, scenario.KiCurrCtrl, scenario.OmegaCutoff);
                pv->setFilterParameters(scenario.Lf, scenario.Cf, scenario.Rf, scenario.Rc);
                pv->setTransformerParameters(scenario.systemNominalVoltage, scenario.pvUnitNominalVoltage, scenario.systemNominalVoltage/scenario.pvUnitNominalVoltage, 0, 0, scenario.transformerInductance);
                pv->setInitialStateValues(scenario.pInit, scenario.qInit, scenario.phi_dInit, scenario.phi_qInit, scenario.gamma_dInit, scenario.gamma_qInit);
                system.addComponent(pv);
                system.connectComponentToNodes<Complex>(pv, { connectionNode });
            } else if (domain == Domain::EMT) {
                SimNode<Real>::Ptr connectionNode = system.node<CPS::SimNode<Real>>("N" + std::to_string(n));
                auto pv = EMT::Ph3::AvVoltageSourceInverterDQ::make("pv_" + connectionNode->name(), "pv_" + connectionNode->name(), Logger::Level::debug, true);
                pv->setParameters(scenario.systemOmega, scenario.pvUnitNominalVoltage, pvActivePower, pvReactivePower);
                pv->setControllerParameters(scenario.KpPLL, scenario.KiPLL, scenario.KpPowerCtrl, scenario.KiPowerCtrl, scenario.KpCurrCtrl, scenario.KiCurrCtrl, scenario.OmegaCutoff);
                pv->setFilterParameters(scenario.Lf, scenario.Cf, scenario.Rf, scenario.Rc);
                pv->setTransformerParameters(scenario.systemNominalVoltage, scenario.pvUnitNominalVoltage, scenario.transformerNominalPower, scenario.systemNominalVoltage/scenario.pvUnitNominalVoltage, 0, 0, scenario.transformerInductance, scenario.systemOmega);
                pv->setInitialStateValues(scenario.pInit, scenario.qInit, scenario.phi_dInit, scenario.phi_qInit, scenario.gamma_dInit, scenario.gamma_qInit);
                system.addComponent(pv);
                system.connectComponentToNodes<Real>(pv, { connectionNode });
            }
        }
    }

    void logPVAttributes(DPsim::DataLogger::Ptr logger, CPS::TopologicalPowerComp::Ptr pv) {

        // power controller
        std::vector<String> inputNames = {  pv->name() + "_powerctrl_input_pref", pv->name() + "_powerctrl_input_qref",
                                            pv->name() + "_powerctrl_input_vcd", pv->name() + "_powerctrl_input_vcq",
                                            pv->name() + "_powerctrl_input_ircd", pv->name() + "_powerctrl_input_ircq"};
        logger->addAttribute(inputNames, pv->attribute("powerctrl_inputs"));
        std::vector<String> stateNames = {  pv->name() + "_powerctrl_state_p", pv->name() + "_powerctrl_state_q",
                                            pv->name() + "_powerctrl_state_phid", pv->name() + "_powerctrl_state_phiq",
                                            pv->name() + "_powerctrl_state_gammad", pv->name() + "_powerctrl_state_gammaq"};
        logger->addAttribute(stateNames, pv->attribute("powerctrl_states"));
        std::vector<String> outputNames = {  pv->name() + "_powerctrl_output_vsd", pv->name() + "_powerctrl_output_vsq"};
        logger->addAttribute(outputNames, pv->attribute("powerctrl_outputs"));

        // interface variables
        logger->addAttribute(pv->name() + "_v_intf", pv->attribute("v_intf"));
        logger->addAttribute(pv->name() + "_i_intf", pv->attribute("i_intf"));

        // additional variables
        logger->addAttribute(pv->name() + "_pll_output", pv->attribute("pll_output"));
        logger->addAttribute(pv->name() + "_vsref", pv->attribute("Vsref"));
        logger->addAttribute(pv->name() + "_vs", pv->attribute("Vs"));
    }

}
        std::shared_ptr<DPsim::SwitchEvent> createEventAddPowerConsumption(String nodeName, Real eventTime, Real additionalActivePower, SystemTopology& system, Domain domain, DPsim::DataLogger::Ptr logger) {

        // TODO: use base classes ph1
        if (domain == CPS::Domain::DP) {
            auto loadSwitch = DP::Ph1::Switch::make("Load_Add_Switch_" + nodeName, Logger::Level::debug);
            auto connectionNode = system.node<CPS::SimNode<Complex>>(nodeName);
            Real resistance = std::abs(connectionNode->initialSingleVoltage())*std::abs(connectionNode->initialSingleVoltage())/additionalActivePower;
            loadSwitch->setParameters(1e9, resistance);
            loadSwitch->open();
            system.addComponent(loadSwitch);
            system.connectComponentToNodes<Complex>(loadSwitch, { CPS::SimNode<Complex>::GND, connectionNode});
            logger->addAttribute("switchedload_i", loadSwitch->attribute("i_intf"));
            return DPsim::SwitchEvent::make(eventTime, loadSwitch, true);
        } else if (domain == CPS::Domain::SP) {
            auto loadSwitch = SP::Ph1::Switch::make("Load_Add_Switch_" + nodeName, Logger::Level::debug);
            auto connectionNode = system.node<CPS::SimNode<Complex>>(nodeName);
            Real resistance = std::abs(connectionNode->initialSingleVoltage())*std::abs(connectionNode->initialSingleVoltage())/additionalActivePower;
            loadSwitch->setParameters(1e9, resistance);
            loadSwitch->open();
            system.addComponent(loadSwitch);
            system.connectComponentToNodes<Complex>(loadSwitch, { CPS::SimNode<Complex>::GND, connectionNode});
            logger->addAttribute("switchedload_i", loadSwitch->attribute("i_intf"));
            return DPsim::SwitchEvent::make(eventTime, loadSwitch, true);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<DPsim::SwitchEvent3Ph> createEventAddPowerConsumption3Ph(String nodeName, Real eventTime, Real additionalActivePower, SystemTopology& system, Domain domain, DPsim::DataLogger::Ptr logger) {

        // TODO: use base classes ph3
         if (domain == CPS::Domain::EMT) {
            auto loadSwitch = EMT::Ph3::Switch::make("Load_Add_Switch_" + nodeName, Logger::Level::debug);
            auto connectionNode = system.node<CPS::SimNode<Real>>(nodeName);
            Real resistance = std::abs(connectionNode->initialSingleVoltage())*std::abs(connectionNode->initialSingleVoltage())/additionalActivePower;
            loadSwitch->setParameters(Matrix::Identity(3,3)*1e9, Matrix::Identity(3,3)*resistance);
            loadSwitch->openSwitch();
            system.addComponent(loadSwitch);
            system.connectComponentToNodes<Real>(loadSwitch, { CPS::SimNode<Real>::GND, system.node<CPS::SimNode<Real>>(nodeName) });
            logger->addAttribute("switchedload_i", loadSwitch->attribute("i_intf"));
            return DPsim::SwitchEvent3Ph::make(eventTime, loadSwitch, true);
        } else {
            return nullptr;
        }
    }

}
}
}
