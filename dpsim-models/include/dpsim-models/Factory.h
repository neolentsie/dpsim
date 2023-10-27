#include <dpsim-models/Logger.h>
#include <dpsim-models/Definitions.h>
#include <map>

#include <dpsim-models/Base/Base_ReducedOrderSynchronGenerator.h>
#include <dpsim-models/DP/DP_Ph1_SynchronGenerator4OrderPCM.h>
#include <dpsim-models/DP/DP_Ph1_SynchronGenerator4OrderTPM.h>
#include <dpsim-models/DP/DP_Ph1_SynchronGenerator6OrderPCM.h>

#pragma once


template <class BaseClass>
class Creator {

    public:
    virtual ~Creator(){}

    virtual std::shared_ptr<BaseClass> Create(const std::string & name, CPS::Logger::Level logLevel = CPS::Logger::Level::debug) = 0;
};

template <class DerivedClass, class BaseClass>
class DerivedCreator : public Creator<BaseClass> {

    public:
    std::shared_ptr<BaseClass> Create(const std::string & name, CPS::Logger::Level logLevel = CPS::Logger::Level::debug) {
        return std::shared_ptr<BaseClass>(new DerivedClass(name, logLevel));
    }
};

template <class BaseClass>
class Factory {
    public:
        static Factory & get() {
            static Factory instance;
            return instance;
        }

        std::vector<std::string> getItems() {

            std::vector<std::string> items;
            for (auto g : functionMap) {
                items.push_back(g.first);
            }

            return items;
        }

        std::shared_ptr<BaseClass> create(
            std::string type, const std::string & name, 
            CPS::Logger::Level logLevel = CPS::Logger::Level::debug) {

            auto it = functionMap.find(type);
            if (it != functionMap.end())
                return it->second->Create(name, logLevel);
            else
            	throw CPS::SystemError("Unsupported type '" + type + "'!");	
        }

        void registerExciter(
            const std::string& type, 
            Creator<BaseClass>* Fn) {

            functionMap[type] = Fn;
        }

    private:
        Factory() { }
        Factory(const Factory&);
        ~Factory() {
            auto i = functionMap.begin();
            while (i != functionMap.end()) {
                delete (*i).second;
                ++i;
            }
        }

        std::map<std::string, Creator<BaseClass>*> functionMap;
};

template <class BaseClass>
class FactoryRegistration {
    public:
        FactoryRegistration(std::string type, Creator<BaseClass>* Fn) {
            Factory<BaseClass>::get().registerExciter(type, Fn);
		}
};

namespace SynchronGeneratorFactory {
namespace SP {
namespace Ph1 {
    void registerSynchronGenerators() {
        FactoryRegistration<CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR> _3OrderSP("3", new DerivedCreator<CPS::SP::Ph1::SynchronGenerator3OrderVBR, CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR>);
        FactoryRegistration<CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR> _4OrderSP("4", new DerivedCreator<CPS::SP::Ph1::SynchronGenerator4OrderVBR, CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR>);
        FactoryRegistration<CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR> _5OrderSP("5", new DerivedCreator<CPS::SP::Ph1::SynchronGenerator5OrderVBR, CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR>);
        FactoryRegistration<CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR> _6aOrderSP("6a", new DerivedCreator<CPS::SP::Ph1::SynchronGenerator6aOrderVBR, CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR>);
        FactoryRegistration<CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR> _6bOrderSP("6b", new DerivedCreator<CPS::SP::Ph1::SynchronGenerator6bOrderVBR, CPS::SP::Ph1::ReducedOrderSynchronGeneratorVBR>);
    }
}
}

namespace DP {
namespace Ph1 {
    void registerSynchronGenerators() {
        FactoryRegistration<CPS::Base::ReducedOrderSynchronGenerator<CPS::Complex>> _4OrderDPIter("4PCM", new DerivedCreator<CPS::DP::Ph1::SynchronGenerator4OrderPCM, CPS::Base::ReducedOrderSynchronGenerator<CPS::Complex>>);
        FactoryRegistration<CPS::Base::ReducedOrderSynchronGenerator<CPS::Complex>> _4OrderDPTPM("4TPM", new DerivedCreator<CPS::DP::Ph1::SynchronGenerator4OrderTPM, CPS::Base::ReducedOrderSynchronGenerator<CPS::Complex>>);
        FactoryRegistration<CPS::Base::ReducedOrderSynchronGenerator<CPS::Complex>> _6OrderDPIter("6PCM", new DerivedCreator<CPS::DP::Ph1::SynchronGenerator6OrderPCM, CPS::Base::ReducedOrderSynchronGenerator<CPS::Complex>>);
    }
}
}
}