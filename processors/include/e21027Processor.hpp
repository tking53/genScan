#ifndef __E21027_PROCESSOR_HPP__
#define __E21027_PROCESSOR_HPP__

#include "Processor.hpp"
#include "MtasProcessor.hpp"
#include "MtasImplantProcessor.hpp"
#include "PidProcessor.hpp"
#include "VetoProcessor.hpp"

#include "Gates.hpp"

#include <map>
#include <thread>
#include <vector>

class e21027Processor : public Processor{
	public:
		e21027Processor(const std::string&);
		virtual ~e21027Processor();
		[[maybe_unused]] virtual bool PreProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree(std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
		virtual void RegisterCuts(CUTS::CutRegistry*);

		void AddIonToCorrelation(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void AddBetaToCorrelation(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void DoIsomerCorrelation(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);

		//internal thread helper, should not be called directly if someone composes from here
		//void IonCorrelationHelper(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		//void BetaCorrelationHelper(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		//void IsomerCorrelationHelper(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);

	private:
		void Reset();
		bool HasMTAS;
		bool HasSIPM;
		bool HasPID;
		bool HasVeto;

		unsigned int NThreads;
		std::vector<std::thread> Workers;

		bool FoundFirst;
		double FirstTime;
		double LastTime;

		std::string implant;
		std::string beta;
		std::string gamma;
		std::string unknown;

		std::vector<std::string> isotopetags;
		std::map<std::string,int> implant_isotopes;
		std::map<std::string,int> rit_vetoed_isotopes;

		std::vector<Gate<double>> MTAS_Total_Gates;

		std::shared_ptr<MtasProcessor> MtasProc;
		std::shared_ptr<MtasImplantProcessor> ImplantProc;
		std::shared_ptr<PidProcessor> PidProc;
		std::shared_ptr<VetoProcessor> VetoProc;
};

#endif
