#ifndef __SUPER3HEN_PROCESSOR_HPP__
#define __SUPER3HEN_PROCESSOR_HPP__

#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Processor.hpp"
#include "Super3HenStruct.hpp"

class TH2Poly;

class Super3HenProcessor : public Processor {
    public:
    Super3HenProcessor(const std::string&);
    virtual ~Super3HenProcessor() = default;
    [[maybe_unused]] virtual bool PreProcess(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*) final;
    [[maybe_unused]] virtual bool Process(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*) final;
    [[maybe_unused]] virtual bool PostProcess(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*) final;
    
    virtual void Finalize() final;
    virtual void Init(const pugi::xml_node&);
    virtual void DeclarePlots(PLOTS::PlotRegistry*);
    virtual void RegisterTree(std::unordered_map<std::string, TTree*>&) final;
    virtual void CleanupTree() final;
    
    private:
    enum class TubeType : int {
        TwoIn = 1,
        VPMC = 2,
        TenatmShort = 3,
        FRIB10 = TenatmShort,
        FRIB4 = 4,
        Unknown = 5
    };
    using HoleKey = std::pair<TubeType, std::string>;
    
    struct HoleRow {
        char prefix;
        int firstHole;
        double y;
        double diameter;
        std::vector<double> xPositions;
    };
    struct HolePosition {
        double x;
        double y;
    };
    struct HoleMultiplicity {
        int bin = 0;
        int ungated = 0;
        int coarse = 0;
        int peak = 0;
    };
    struct HoleBinRange {
        int offset;
        int capacity;
    };
    const std::vector<HoleRow> MechanicalHoleRows = {
        {'A', 1, 14.25, 2.05, {-1.25, 1.25}},
        {'A', 3, 13.49, 2.05, {-6.25, -3.75, 3.75, 6.25}},
        {'A', 7, 12.40, 2.05, {-11.05, -8.55, 8.55, 11.05}},
        {'A', 11, 11.75, 2.05, {-1.25, 1.25}},
        {'A', 13, 10.99, 2.05, {-6.25, -3.75, 3.75, 6.25}},
        {'A', 17, 9.90, 2.05, {-13.50, -11.00, -8.50, 8.50, 11.00, 13.50}},
        {'A', 23, 7.40, 2.05, {-13.50, -11.00, -8.50, 8.50, 11.00, 13.50}},
        {'A', 29, 4.90, 2.05, {-13.50, -11.00, -8.50, 8.50, 11.00, 13.50}},
        {'A', 35, -4.90, 2.05, {-13.50, -11.00, -8.50, 8.50, 11.00, 13.50}},
        {'A', 41, -7.40, 2.05, {-13.50, -11.00, -8.50, 8.50, 11.00, 13.50}},
        {'A', 47, -9.90, 2.05, {-13.50, -11.00, -8.50, 8.50, 11.00, 13.50}},
        {'A', 53, -10.99, 2.05, {-6.25, -3.75, 3.75, 6.25}},
        {'A', 57, -11.75, 2.05, {-1.25, 1.25}},
        {'A', 59, -12.40, 2.05, {-11.05, -8.55, 8.55, 11.05}},
        {'A', 63, -13.49, 2.05, {-6.25, -3.75, 3.75, 6.25}},
        {'A', 67, -14.25, 2.05, {-1.25, 1.25}},
        {'B', 1, 9.75, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 5, 8.99, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 11, 8.25, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 15, 7.49, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 21, 6.75, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 25, 5.99, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 31, 5.25, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 35, 4.49, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 41, 3.75, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 45, 3.00, 1.05, {-11.25, -9.75, -8.25, -6.75, -5.25, -3.75, 3.75, 5.25, 6.75, 8.25, 9.75, 11.25}},
        {'B', 57, 1.50, 1.05, {-11.25, -9.75, -8.25, -6.75, -5.25, -3.75, 3.75, 5.25, 6.75, 8.25, 9.75, 11.25}},
        {'B', 69, 0.00, 1.05, {-11.25, -9.75, -8.25, -6.75, -5.25, -3.75, 3.75, 5.25, 6.75, 8.25, 9.75, 11.25}},
        {'B', 81, -1.50, 1.05, {-11.25, -9.75, -8.25, -6.75, -5.25, -3.75, 3.75, 5.25, 6.75, 8.25, 9.75, 11.25}},
        {'B', 93, -3.00, 1.05, {-11.25, -9.75, -8.25, -6.75, -5.25, -3.75, 3.75, 5.25, 6.75, 8.25, 9.75, 11.25}},
        {'B', 105, -3.75, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 109, -4.49, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 115, -5.25, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 119, -5.99, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 125, -6.75, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 129, -7.49, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 135, -8.25, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'B', 139, -8.99, 1.05, {-6.56, -5.06, -3.56, 3.56, 5.06, 6.56}},
        {'B', 145, -9.75, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'C', 1, 2.25, 1.05, {-2.25, -0.75, 0.75, 2.25}},
        {'C', 5, 0.75, 1.05, {-2.25, 2.25}},
        {'C', 7, -0.75, 1.05, {-2.25, 2.25}},
        {'C', 9, -2.25, 1.05, {-2.25, -0.75, 0.75, 2.25}}
    };
    enum {
        TotalTubeBins = 300,
        TwoInBinCapacity = 68,
        VPMCBinCapacity = 116,
        TenAtmShortBinCapacity = 52, // this includes the three  10Atm 10" tubes used in 2026 ORNL only has 49 12" tubes
        FRIBBinCapacity = 12,
        UnknownBinCapacity = TotalTubeBins - TwoInBinCapacity - VPMCBinCapacity - TenAtmShortBinCapacity - FRIBBinCapacity,
        TubeTypeCount = 5
    };
    const std::array<int, 5> TubeTypeBinCapacities = {
        TwoInBinCapacity, VPMCBinCapacity, TenAtmShortBinCapacity, FRIBBinCapacity, UnknownBinCapacity
    };
    const std::array<int, 5> TubeTypeBinOffsets = {
        0,
        TwoInBinCapacity,
        TwoInBinCapacity + VPMCBinCapacity,
        TwoInBinCapacity + VPMCBinCapacity + TenAtmShortBinCapacity,
        TwoInBinCapacity + VPMCBinCapacity + TenAtmShortBinCapacity + FRIBBinCapacity
    };
    std::pair<TubeType, std::string> ClassifyTube(const PhysicsData&) const;
    double CoarseThreshold = 0.0;
    double PeakThresholdLow = 0.0;
    double PeakThresholdHigh = 65536.0;
    std::map<HoleKey, int> HoleBins;
    const std::array<HoleBinRange, 4> HoleTypeBinRanges = {{
        {0, TwoInBinCapacity},
        {TwoInBinCapacity, VPMCBinCapacity},
        {TwoInBinCapacity + VPMCBinCapacity, TenAtmShortBinCapacity},
        {TwoInBinCapacity + VPMCBinCapacity + TenAtmShortBinCapacity, FRIBBinCapacity}
    }};
    std::array<std::map<std::string, HoleMultiplicity>, 4> HoleMultiplicitiesByType;
    std::map<std::string, HolePosition> MechanicalHolePositions;
    std::array<int, TubeTypeCount> HoleBinCounts{};
    TH2Poly* S3H_100 = nullptr;
    TH2Poly* S3H_101 = nullptr;
    TH2Poly* S3H_102 = nullptr;
    std::vector<ProcessorStruct::S3H> s3h_vec;
};

#endif