#include "cps/CIM/Reader.h"
#include <DPsim.h>
#include <cps/CSVReader.h>
#include "../Examples.h"

using namespace std;
using namespace DPsim;
using namespace CPS;
using namespace CPS::CIM;

int main(int argc, char** argv){

	// Simulation parameters
	String simName = "EMT_CIGRE_MV_withDG";
	Examples::CIGREMV::ScenarioConfig scenario;
	std::list<fs::path> filenames;
	Real timeStep;
	Real finalTime;
		
	// Set remaining simulation parameters using default values or command line infos
	std::cout<<std::experimental::filesystem::current_path()<<std::endl;
	CommandLineArgs args(argc, argv);
	if (argc <= 1) {
		filenames = DPsim::Utils::findFiles({
			"Rootnet_FULL_NE_28J17h_DI.xml",
			"Rootnet_FULL_NE_28J17h_EQ.xml",
			"Rootnet_FULL_NE_28J17h_SV.xml",
			"Rootnet_FULL_NE_28J17h_TP.xml"
		}, "dpsim/Examples/CIM/grid-data/CIGRE_MV/NEPLAN/CIGRE_MV_no_tapchanger_noLoad1_LeftFeeder_With_LoadFlow_Results", "CIMPATH");
		timeStep = 0.1e-3;
		finalTime = 1;
	}
	else {
		filenames = args.positionalPaths();
		timeStep = args.timeStep;
		finalTime = args.duration;
	}
	
	// ----- POWERFLOW FOR INITIALIZATION -----
	// read original network topology
	String simNamePF = simName + "_Powerflow";
	Logger::setLogDir("logs/" + simNamePF);
    CIM::Reader reader(simNamePF, Logger::Level::debug, Logger::Level::debug);
    SystemTopology systemPF = reader.loadCIM(scenario.systemFrequency, filenames, Domain::SP);
	Examples::CIGREMV::addInvertersToCIGREMV(systemPF, scenario, Domain::SP);

	// define logging
    auto loggerPF = DPsim::DataLogger::make(simNamePF);
    for (auto node : systemPF.mNodes)
    {
        loggerPF->addAttribute(node->name() + ".V", node->attribute("v"));
    }

	// run powerflow
    Simulation simPF(simNamePF, Logger::Level::debug);
	simPF.setSystem(systemPF);
	simPF.setTimeStep(1);
	simPF.setFinalTime(2);
	simPF.setDomain(Domain::SP);
	simPF.setSolverType(Solver::Type::NRP);
	simPF.doInitFromNodesAndTerminals(true);
    simPF.addLogger(loggerPF);
    simPF.run();

	
	// ----- DYNAMIC SIMULATION -----
	Logger::setLogDir("logs/" + simName);
	CIM::Reader reader2(simName, Logger::Level::debug, Logger::Level::debug);
    SystemTopology systemEMT = reader2.loadCIM(scenario.systemFrequency, filenames, CPS::Domain::EMT, PhaseType::ABC);
	Examples::CIGREMV::addInvertersToCIGREMV(systemEMT, scenario, Domain::EMT);
	reader2.initDynamicSystemTopologyWithPowerflow(systemPF, systemEMT);

	auto logger = DPsim::DataLogger::make(simName);

	// log node voltages
	for (auto node : systemEMT.mNodes)
		logger->addAttribute(node->name() + ".V", node->attribute("v"));

	// log line currents
	for (auto comp : systemEMT.mComponents) {
		if (dynamic_pointer_cast<CPS::EMT::Ph3::PiLine>(comp))
			logger->addAttribute(comp->name() + ".I", comp->attribute("i_intf"));
	}
	
	// log load currents
	for (auto comp : systemEMT.mComponents) {
		if (dynamic_pointer_cast<CPS::EMT::Ph3::RXLoad>(comp))
			logger->addAttribute(comp->name() + ".I", comp->attribute("i_intf"));
	}
	
	// log output of PV connected at N11
	auto pv = systemEMT.component<CPS::SimPowerComp<Real>>("pv_N11");
	Examples::CIGREMV::logPVAttributes(logger, pv);

	Simulation sim(simName, Logger::Level::debug);
	sim.setSystem(systemEMT);
	sim.setTimeStep(timeStep);
	sim.setFinalTime(finalTime);
	sim.setDomain(Domain::EMT);
	sim.setSolverType(Solver::Type::MNA);
	sim.doInitFromNodesAndTerminals(true);
	sim.doSteadyStateInit(false);
	sim.addLogger(logger);
	
	sim.run();
}