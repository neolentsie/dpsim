# This example reads a sine signal from VILLASnode that modifies the active power set point
# of the PQ load. The n2 voltage is exported to a file via MQTT and VILLASnode.

import sys
import os.path

import dpsimpy
import dpsimpyvillas
from villas.node.node import Node as VILLASnode
from math import pi

base = os.path.splitext(os.path.basename(sys.argv[0]))[0]

def villas():

    nodes = {
        'mqtt': {
            'type': 'mqtt',
            'format': 'json',
            'host': 'mqtt',
            'in': {
                'subscribe': '/dpsim-mqtt',
                'signals':  [
                    {
                        'name': 'v1',
                        'type': 'complex'
                    },
                    {
                        'name': 'v2',
                        'type': 'complex'
                    },
                    {
                        'name': 'v3',
                        'type': 'complex'
                    },
                ]
            },
            'out': {
                'publish': '/mqtt-dpsim'
            }
        },
        'file1': {
            'type': 'file',
            'uri': f'{base}-results-%y-%m-%d_%H_%M_%S.csv'
        },
    }

    paths = [
        {
            'in': 'mqtt',
            'out': 'file1'
        }
    ]

    config = {
        'nodes': nodes,
        'paths': paths
    }
    return VILLASnode(config=config)

def dpsim():
    name = 'ipdft_3bus'
    # ----------------- Configurations ----------------- #
    # Network
    Vnom = 6.9e3;
    nomFreq = 50;
    nomOmega= nomFreq* 2* pi;
    # Generator 1 (Bus 1)
    nomPower_G1 = 100e6
    nomPhPhVoltRMS_G1 = 6.9e3
    nomFreq_G1 = 50
    initActivePower_G1 = 0.3386e6
    initReactivePower_G1= 0.3222e6
    setPointVoltage_G1=nomPhPhVoltRMS_G1
    powerflow_bus_type_G1 = dpsimpy.PowerflowBusType.VD
    # Generator 2 (Bus 2)
    nomPower_G2 = 100e6
    nomPhPhVoltRMS_G2 = 6.9e3
    nomFreq_G2 = 50
    initActivePower_G2 = 0.2222e6
    initReactivePower_G2=(-1)*0.0285e6
    setPointVoltage_G2=nomPhPhVoltRMS_G2
    powerflow_bus_type_G2 = dpsimpy.PowerflowBusType.PV
    # Transformer
    t1_ratio = 1;
    t2_ratio = 1;
    # Load (Bus 3)
    activePower_L= 0.5556e6;
    reactivePower_L= 0.2778e6;
    # Transmission Lines
    lineResistance = 0.025;
    lineInductance = 0.075/ (2* pi* 50);
    lineCapacitance = 0;
    # ----------------- End of Configurations ----------------- #
    # Nodes
    gnd = dpsimpy.sp.SimNode.gnd
    n1_PF = dpsimpy.sp.SimNode("n1", dpsimpy.PhaseType.Single)
    n2_PF = dpsimpy.sp.SimNode("n2", dpsimpy.PhaseType.Single)
    n3_PF = dpsimpy.sp.SimNode("n3", dpsimpy.PhaseType.Single)
    # Generators
    gen1_PF = dpsimpy.sp.ph1.SynchronGenerator("Generator_1")
    gen1_PF.set_parameters(nomPower_G1, nomPhPhVoltRMS_G1, initActivePower_G1, setPointVoltage_G1 * t1_ratio, powerflow_bus_type_G1, initReactivePower_G1)
    gen1_PF.set_base_voltage(Vnom)
    gen2_PF = dpsimpy.sp.ph1.SynchronGenerator("Generator_2")
    gen2_PF.set_parameters(nomPower_G1, nomPhPhVoltRMS_G2, initActivePower_G2, setPointVoltage_G2 * t2_ratio, powerflow_bus_type_G2, initReactivePower_G2)
    gen2_PF.set_base_voltage(Vnom)
    # Load
    load_PF = dpsimpy.sp.ph1.Shunt("Load")
    load_PF.set_parameters(activePower_L / (Vnom ** 2), -reactivePower_L / (Vnom ** 2))
    load_PF.set_base_voltage(Vnom)
    # Lines
    line_PF_1 = dpsimpy.sp.ph1.PiLine("PiLine_12")
    line_PF_1.set_parameters(lineResistance, lineInductance, lineCapacitance)
    line_PF_1.set_base_voltage(Vnom)
    line_PF_2 = dpsimpy.sp.ph1.PiLine("PiLine_13")
    line_PF_2.set_parameters(lineResistance, lineInductance, lineCapacitance)
    line_PF_2.set_base_voltage(Vnom)
    line_PF_3 = dpsimpy.sp.ph1.PiLine("PiLine_23")
    line_PF_3.set_parameters(lineResistance, 2 * lineInductance, 2 * lineCapacitance)
    line_PF_3.set_base_voltage(Vnom)
    # Topology
    gen1_PF.connect([n1_PF])
    gen2_PF.connect([n2_PF])
    load_PF.connect([n3_PF])
    line_PF_1.connect([n1_PF, n2_PF])
    line_PF_2.connect([n1_PF, n3_PF])
    line_PF_3.connect([n2_PF, n3_PF])
    systemPF = dpsimpy.SystemTopology(50, [n1_PF, n2_PF, n3_PF], [gen1_PF, gen2_PF, line_PF_1, line_PF_2, line_PF_3, load_PF])

    sim = dpsimpy.RealTimeSimulation(name, dpsimpy.LogLevel.debug)
    sim.set_system(systemPF)
    sim.set_domain(dpsimpy.Domain.SP)
    sim.set_solver(dpsimpy.Solver.NRP)
    sim.set_time_step(0.1)
    sim.set_final_time(10)
    sim.do_init_from_nodes_and_terminals(False)

    logger = dpsimpy.Logger(name)
    sim.add_logger(logger)
    sim.log_attribute('n1.v', n1_PF.attr('v'))
    sim.log_attribute('n2.v', n2_PF.attr('v'))
    sim.log_attribute('n3.v', n3_PF.attr('v'))

    intf_config = {
        "type": "mqtt",
        "format": "json",
        "host": "mqtt",
        "in": {
            "subscribe": "/mqtt-dpsim",
        },
        "out": {
            "publish": "/dpsim-mqtt"
        }
    }

    intf = dpsimpyvillas.InterfaceVillas(name="dpsim-mqtt", config=intf_config)
    intf.export_attribute(n1_PF.attr('v').derive_coeff(0,0), 0)
    intf.export_attribute(n2_PF.attr('v').derive_coeff(0,0), 1)
    intf.export_attribute(n3_PF.attr('v').derive_coeff(0,0), 2)
    sim.add_interface(intf)

    return sim, intf

def test_ipdft_3bus():
    sim, intf = dpsim() #intf needs to be extracted from the dpsim-function since the interface object gets deleted otherwise leading to SegFault when starting the simulation
    node = villas()

    node.start()
    sim.run(1)
    node.stop()

if __name__ == '__main__':
    test_ipdft_3bus()
