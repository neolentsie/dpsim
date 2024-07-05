import dpsimpy
import dpsimpyvillas
from villas.node.node import Node as VILLASnode
import sys
import os.path

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
                        'name': 'v1_0',
                        'type': 'float'
                    },
                    # {
                    #     'name': 'v1_1',
                    #     'type': 'float'
                    # },
                    # {
                    #     'name': 'v1_2',
                    #     'type': 'float'
                    # },
                ]
            },
            'out': {
                'publish': '/mqtt-dpsim'
            }
        },
        'siggen': 
        {
            'type': "signal",
            'signal': ["pulse", "sine"],
            'values': 2,
            'pulse_low': [0.,0.],
            'pulse_high': [5.,0.],
		    'pulse_width': [20., 0.],
            'frequency': [1.0, 50.0],
            'amplitude': [0.0, 20.0],
            'phase': [0.0, 0.0],
            'rate': 1000,
        },
        'file1': {
            'type': 'file',
            'uri': './results1-%y-%m-%d_%H_%M_%S.csv'
        },
        'file2': {
            'type': 'file',
            'uri': f'{base}/results2-%y-%m-%d_%H_%M_%S.csv'
        },
    }

    paths = [
        {
            'in': 'siggen',                # TO CHECK FUNCTIONALITY OF HOOK
            'out': 'file1',
            'hooks': 
            [
                {
                    'type': "pps_ts",
                    'signal': "pulse",
                    'threshold': 2.0,
                    'enabled': True,
                    'expected_smp_rate': 1000
                },
                {
                    'type': "ip-dft-pmu",
                    'enabled': True,
                    'estimation_range': 10.0,
                    'nominal_freq': 50.0,
                    'number_plc': 10.0,
                    'sample_rate': 1000,
                    'dft_rate': 5,
                    'window_type': "hann",
                    'signals': ["sine"],   # CHANGE NAME OF THE SIGNAL TO WHICH HOOK NEEDS TO BE APPLIED
                    'angle_unit': "rad",
                    'timestamp_align': "center",
                    'add_channel_name': True,
                    'phase_offset': 70.0,
                    #'frequency_offset': 0.0015,
                    'amplitude_offset': 0.0
                }
            ]
        },
        # {
        #     'in': 'mqtt',
        #     'out': 'file2',
        #     'hooks': 
        #     [
        #         {
        #             'type': "pps_ts",
        #             'signal': "pulse",
        #             'threshold': 2.0,
        #             'enabled': True,
        #             'expected_smp_rate': 1000
        #         },
        #         {
        #             'type': "ip-dft-pmu",
        #             'enabled': True,
        #             'estimation_range': 10.0,
        #             'nominal_freq': 50.0,
        #             'number_plc': 10.0,
        #             'sample_rate': 1000,
        #             'dft_rate': 5,
        #             'window_type': "hann",
        #             'signals': ["v1_0"],
        #             'angle_unit': "rad",
        #             'timestamp_align': "center",
        #             'add_channel_name': True,
        #             'phase_offset': 0.0,
        #             #'frequency_offset': 0.0015,
        #             'amplitude_offset': 0.0
        #         }, 
        #     ]
        # }
    ]

    config = {
        'nodes': nodes,
        'paths': paths
    }
    return VILLASnode(config=config)

def dpsim():
    sim_name = "ipdft_emt_rlc"
    time_step = 0.00005
    final_time = 10
    dpsimpy.Logger.set_log_dir('logs/' + sim_name)

    # Nodes
    gnd = dpsimpy.emt.SimNode.gnd
    n1 = dpsimpy.emt.SimNode("n1", dpsimpy.PhaseType.ABC)

    # Components
    vs = dpsimpy.emt.ph3.VoltageSource("vs1")
    vs.set_parameters(dpsimpy.Math.single_phase_variable_to_three_phase(complex(20, 0)), 50)

    res = dpsimpy.emt.ph3.Resistor("R1", dpsimpy.LogLevel.debug)
    res.set_parameters(dpsimpy.Math.single_phase_parameter_to_three_phase(100))

    # Topology
    vs.connect([gnd, n1])
    res.connect([n1, gnd])
    sys = dpsimpy.SystemTopology(50, [n1], [vs, res])

    # Logging
    logger = dpsimpy.Logger(sim_name)
    logger.log_attribute("v1", "v", n1)
    logger.log_attribute("iR", "i_intf", res)

    # Simulation
    sim = dpsimpy.RealTimeSimulation(sim_name)
    sim.set_system(sys)
    sim.set_time_step(time_step)
    sim.set_final_time(final_time)
    sim.set_domain(dpsimpy.Domain.EMT)
    sim.add_logger(logger)
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
    intf.export_attribute(n1.attr('v').derive_coeff(0,0), 0)
    # intf.export_attribute(n1.attr('v').derive_coeff(1,0), 1)
    # intf.export_attribute(n1.attr('v').derive_coeff(2,0), 2)
    sim.add_interface(intf)
    return sim, intf

def test_ipdft_emt_rlc():
    sim, intf = dpsim() #intf needs to be extracted from the dpsim-function since the interface object gets deleted otherwise leading to SegFault when starting the simulation
    node = villas()
    node.start()
    sim.run(1)
    node.stop()

if __name__ == '__main__':
    test_ipdft_emt_rlc()