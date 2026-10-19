#include "Super3HenProcessor.hpp"

#include "EventSummary.hpp"
#include "HistogramManager.hpp"

#include <TAxis.h>
#include <TH2.h>
#include <TH2F.h>
#include <TH2Poly.h>
#include <TMath.h>
#include <TTree.h>

#include <array>
#include <cmath>
#include <stdexcept>

Super3HenProcessor::Super3HenProcessor(const std::string& log)
: Processor(log, "Super3HenProcessor", {"s3hen"}) {
    this->h1dsettings = {
        {1,{10000,0,10000}},
        {2,{200,0,200}},
        {3,{10000,0,10000}},
        {4,{10000,0,10000}}
    };

    this->h2dsettings = {
        {10,{10000, 0, 10000, 250, 0, 250}},
        {11,{10000, 0, 10000, 20, 0, 20}},
        {12,{10000, 0, 10000, 5, 0, 5}},
        {13,{200, 0, 200, 250, 0, 250}},
        {14,{200, 0, 200, 250, 0, 250}},
        {15,{200, 0, 200, 250, 0, 250}},

    };
    
}

[[maybe_unused]] bool Super3HenProcessor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
    Processor::PreProcess();
    
    auto summary = eventhistory->GetCurrentEventSummary();
    summary->GetDetectorSummary(this->AllDefaultRegex["s3hen"], this->SummaryData);
    
    auto S3H_01 = hismanager->GetPlot<TH1*>("S3H_01");
    auto S3H_02 = hismanager->GetPlot<TH1*>("S3H_02");
    auto S3H_03 = hismanager->GetPlot<TH1*>("S3H_03");
    auto S3H_04 = hismanager->GetPlot<TH1*>("S3H_04");
    auto S3H_10 = hismanager->GetPlot<TH2*>("S3H_10");
    auto S3H_11 = hismanager->GetPlot<TH2*>("S3H_11");
    auto S3H_13 = hismanager->GetPlot<TH2*>("S3H_13");
    auto S3H_14 = hismanager->GetPlot<TH2*>("S3H_14");
    auto S3H_15 = hismanager->GetPlot<TH2*>("S3H_15");
    auto S3H_12 = hismanager->GetPlot<TH2*>("S3H_12");
    
    ProcessorStruct::S3H rootstruct = ProcessorStruct::DEFAULT_S3H_STRUCT;

    for (const auto& evt : this->SummaryData) {

        const std::string holeID = evt->GetSubType();
        const auto [tubeType, tubeClass] = ClassifyTube(*evt);
        const int typeIndex = static_cast<int>(tubeType) - 1;
        const std::string preamp = evt->GetGroup();
        const std::string direction = evt->HasTag("U") ? "U" : "D";
        const std::string holeLabel = (holeID.empty() ? "(unassigned)" : holeID) + (direction.empty() ? "" : " " + direction);
        int holeTypeIndex = -1;
        if (tubeType == TubeType::TwoIn) {
            holeTypeIndex = 0;
        } else if (tubeType == TubeType::VPMC) {
            holeTypeIndex = 1;
        } else if (tubeType == TubeType::TenatmShort || tubeType == TubeType::FRIB4) {
            holeTypeIndex = 2;
        }
        
   
        rootstruct.holeLabel = holeLabel;
        rootstruct.tubeClass = tubeClass;
        rootstruct.preamp = preamp;
        rootstruct.energy = evt->GetEnergy();
        rootstruct.timeStamp = evt->GetTimeStamp();
        rootstruct.pileup = evt->GetPileup();
        rootstruct.saturation = evt->GetSaturation();
        this->s3h_vec.emplace_back(rootstruct);
        rootstruct = ProcessorStruct::DEFAULT_S3H_STRUCT;

        const double energy = evt->GetEnergy();
        S3H_01->Fill(energy);

        if (energy >= this->CoarseThreshold) {
            S3H_03->Fill(energy);
        }
        if (energy >= this->PeakThresholdLow && energy <= this->PeakThresholdHigh) {
            S3H_04->Fill(energy);
        }
        
        if (preamp.size() == 1 && preamp[0] >= 'A' && preamp[0] <= 'T') {
            const int preampIndex = preamp[0] - 'A';
            S3H_11->Fill(evt->GetEnergy(), preampIndex);
        }
        
        
        const HoleKey holeKey{tubeType, holeLabel};
        auto holeBin = this->HoleBins.find(holeKey);

        if (holeBin == this->HoleBins.end()) {
            if (this->HoleBinCounts[typeIndex] >= this->TubeTypeBinCapacities[typeIndex]) [[unlikely]] {
                this->console->warn("S3H {} bin range is full; retaining hole '{}' in the output tree", tubeClass, holeLabel);
            } else {
                const int binIndex = this->TubeTypeBinOffsets[typeIndex] + this->HoleBinCounts[typeIndex]++;
                holeBin = this->HoleBins.emplace(holeKey, binIndex).first;
            }
        }
        if (holeBin != this->HoleBins.end()) {
            S3H_10->Fill(evt->GetEnergy(), holeBin->second);
        }

        if (!evt->HasTag("U") && holeTypeIndex >= 0) {
            auto& holes = this->HoleMultiplicitiesByType[holeTypeIndex];
            auto hole = holes.find(holeID);
            if (hole == holes.end()) {
                if (holes.size() >= static_cast<size_t>(this->HoleTypeBinRanges[holeTypeIndex].capacity)) {
                    this->console->warn("S3H hole type {} capacity full; skipping subtype '{}'", holeTypeIndex + 1, holeID);
                } else {
                    const int localBin = static_cast<int>(holes.size());
                    hole = holes.emplace(holeID, HoleMultiplicity{localBin}).first;
                }
            }
            if (hole != holes.end()) {
                HoleMultiplicity& multiplicity = hole->second;
                ++multiplicity.ungated;
                if (energy >= this->CoarseThreshold) {
                    ++multiplicity.coarse;
                }
                if (energy >= this->PeakThresholdLow && energy <= this->PeakThresholdHigh) {
                    ++multiplicity.peak;
                }
            }
        }

        const auto position = this->MechanicalHolePositions.find(holeID);
        if (!evt->HasTag("U") && position != this->MechanicalHolePositions.end()) {
            this->S3H_100->Fill(position->second.x, position->second.y, 1.0);
            if (energy >= this->CoarseThreshold) {
                this->S3H_101->Fill(position->second.x, position->second.y, 1.0);
            }
            if (energy >= this->PeakThresholdLow && energy <= this->PeakThresholdHigh) {
                this->S3H_102->Fill(position->second.x, position->second.y, 1.0);
            }
        }
        
        S3H_12->Fill(energy, typeIndex);

    }
    
    S3H_02->Fill(this->SummaryData.size());
    for (size_t holeTypeIndex = 0; holeTypeIndex < this->HoleMultiplicitiesByType.size(); ++holeTypeIndex) {
        const int offset = this->HoleTypeBinRanges[holeTypeIndex].offset;
        for (auto& [holeID, multiplicity] : this->HoleMultiplicitiesByType[holeTypeIndex]) {
             const int flatBin = offset + multiplicity.bin;
            if (multiplicity.ungated != 0) {
                S3H_13->Fill(flatBin, multiplicity.ungated);
            }
            if (multiplicity.coarse != 0) {
                S3H_14->Fill(flatBin, multiplicity.coarse);
            }
            if (multiplicity.peak != 0) {
                S3H_15->Fill(flatBin, multiplicity.peak);
            }
            multiplicity.ungated = 0;
            multiplicity.coarse = 0;
            multiplicity.peak = 0;
        }
    }
    this->EndProcess();
    return true;
}

[[maybe_unused]] bool Super3HenProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
    return true;
}

[[maybe_unused]] bool Super3HenProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
    return true;
}

void Super3HenProcessor::Init(const pugi::xml_node& config) {
    this->CoarseThreshold = config.attribute("coarse_n_threshold").as_double(0.0);
    this->PeakThresholdLow = config.attribute("peak_threshold_low").as_double(0.0);
    this->PeakThresholdHigh = config.attribute("peak_threshold_high").as_double(65536.0);
    if (this->PeakThresholdLow < this->CoarseThreshold || this->PeakThresholdHigh < this->PeakThresholdLow) {
        throw std::invalid_argument("Super3HenProcessor peak thresholds must form an ordered window within the coarse selection");
    }

    this->s3h_vec.reserve(20);

    this->console->info("Super3Hen thresholds: coarse >= {}, peak in [{}, {}]", this->CoarseThreshold, this->PeakThresholdLow, this->PeakThresholdHigh);
    this->LoadHistogramSettings(config);
    this->LoadCustomCuts(config);
}

void Super3HenProcessor::Finalize() {
    this->console->info("{} has been finalized", this->ProcessorName);
}

void Super3HenProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
    hismanager->RegisterPlot<TH1F>("S3H_01", "3Hen energy spectrum; Energy; Counts", this->h1dsettings.at(1));
    hismanager->RegisterPlot<TH1F>("S3H_02", "3Hen hit multiplicity spectrum; Hits per event; Counts", this->h1dsettings.at(2));
    hismanager->RegisterPlot<TH1F>("S3H_03", "3Hen energy above coarse neutron threshold; Calibrated energy; Counts", this->h1dsettings.at(3));
    hismanager->RegisterPlot<TH1F>("S3H_04", "3Hen energy within peak neutron window; Calibrated energy; Counts", this->h1dsettings.at(4));
    hismanager->RegisterPlot<TH2F>("S3H_10", "3He Energy ; Energy; Hole ID ()", this->h2dsettings.at(10));
    hismanager->RegisterPlot<TH2F>("S3H_11", "3Hen Energy by preamp; Energy; Preamp", this->h2dsettings.at(11));
    hismanager->RegisterPlot<TH2F>("S3H_12", "3Hen energy by type;  Energy; Type", this->h2dsettings.at(12));

    hismanager->RegisterPlot<TH2F>("S3H_13", "3Hen ungated hit multiplicity by moderator hole; Hole ID; Hits per event", this->h2dsettings.at(13));
    hismanager->RegisterPlot<TH2F>("S3H_14", "3He coarse-threshold hit multiplicity by moderator hole; Hole ID / Direction; Hits per event", this->h2dsettings.at(14));
    hismanager->RegisterPlot<TH2F>("S3H_15", "3He peak-window hit multiplicity by moderator hole; Hole ID / Direction; Hits per event", this->h2dsettings.at(15));
    
	
    this->S3H_100 = hismanager->RegisterPlot("S3H_100", "3Hen hit map; X (drawing units); Y (drawing units)");
	this->S3H_101 = hismanager->RegisterPlot("S3H_101", "3Hen coarse-window hit multiplicity by detector position; X (drawing units); Y (drawing units)");
	this->S3H_102 = hismanager->RegisterPlot("S3H_102", "3Hen peak-window hit multiplicity by detector position; X (drawing units); Y (drawing units)");
    constexpr int polygonVertices = 24;
    const double pi = TMath::Pi();
    for (const auto& row : this->MechanicalHoleRows) {
        for (size_t index = 0; index < row.xPositions.size(); ++index) {
            const std::string holeID = std::string(1, row.prefix) + std::to_string(row.firstHole + static_cast<int>(index));
            const double centerX = row.xPositions[index];
            const double radius = row.diameter / 2.0;
            std::array<double, polygonVertices> polygonX{};
            std::array<double, polygonVertices> polygonY{};
            for (int vertex = 0; vertex < polygonVertices; ++vertex) {
                const double angle = 2.0 * pi * vertex / polygonVertices;
                polygonX[vertex] = centerX + radius * std::cos(angle);
                polygonY[vertex] = row.y + radius * std::sin(angle);
            }
            this->S3H_100->AddBin(polygonVertices, polygonX.data(), polygonY.data());
			this->S3H_101->AddBin(polygonVertices, polygonX.data(), polygonY.data()); 
			this->S3H_102->AddBin(polygonVertices, polygonX.data(), polygonY.data());   
            this->MechanicalHolePositions.emplace(holeID, HolePosition{centerX, row.y});
        }
    }
    
    auto preampPlot = hismanager->GetPlot<TH2*>("S3H_11");
    auto* preampAxis = preampPlot->GetYaxis();
    for (int bin = 1; bin <= 20; ++bin) {
        const char label[] = {static_cast<char>('A' + bin - 1), '\0'};
        preampAxis->SetBinLabel(bin, label);
    }
    
    const std::array<const char*, 5> typeLabels = {"2in", "VPMC", "10atmShort", "FRIB", ""};
    auto* typeAxis = (hismanager->GetPlot<TH2*>("S3H_12"))->GetYaxis();
    for (int i = 0; i < 5; ++i) {
        typeAxis->SetBinLabel(i + 1, typeLabels[i]);
    }
    
}

void Super3HenProcessor::RegisterTree(std::unordered_map<std::string, TTree*>& outputtrees) {
    this->OutputTree = new TTree("Super3Hen", "Super3Hen Processor Output");
    this->OutputTree->Branch("s3hen", &this->s3h_vec);
    outputtrees[this->ProcessorName] = this->OutputTree;
}

void Super3HenProcessor::CleanupTree() {
    this->s3h_vec.clear();
}


std::pair<Super3HenProcessor::TubeType, std::string> Super3HenProcessor::ClassifyTube(const PhysicsData& evt) const {
    if (evt.HasTag("2in")) {
        return {TubeType::TwoIn, "2in"};
    }
    if (evt.HasTag("VPMC")) {
        return {TubeType::VPMC, "VPMC"};
    }
    if (evt.HasTag("10atmShort")) {
        return {TubeType::TenatmShort, "10atmShort"};
    }
    if (evt.HasTag("FRIB10")) {
        return {TubeType::FRIB10, "FRIB10"};
    }
    if (evt.HasTag("FRIB4")) {
        return {TubeType::FRIB4, "FRIB4"};
    }
    return {TubeType::Unknown, ""};
}