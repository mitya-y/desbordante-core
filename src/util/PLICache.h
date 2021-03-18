#pragma once

class ProfilingContext;

#include "CacheEvictionMethod.h"
#include "CachingMethod.h"
#include "ProfilingContext.h"
#include "ColumnLayoutRelationData.h"

class PLICache {
private:
    class PositionListIndexRank {
    public:
        Vertical const* vertical_;
        PositionListIndex* pli_;
        int addedArity_;

        PositionListIndexRank(Vertical const* vertical, PositionListIndex* pli, int initialArity):
            vertical_(vertical), pli_(pli), addedArity_(initialArity) {}
    };
    using CacheMap = VerticalMap<PositionListIndex>;
    ColumnLayoutRelationData* relationData_;
    std::unique_ptr<CacheMap> index_;
    //usageCounter - for parallelism

    int savedIntersections_ = 0;

    CachingMethod cachingMethod_;
    CacheEvictionMethod evictionMethod_;
    double cachingMethodValue_;
    //long long maximumAvailableMemory_ = 0;
    double maximumEntropy_;
    double meanEntropy_;
    double minEntropy_;
    double medianEntropy_;
    double medianGini_;
    double medianInvertedEntropy_;

    void cachingProcess(Vertical const& vertical,
                        std::unique_ptr<PositionListIndex> pli,
                        ProfilingContext* profilingContext);
public:
    PLICache(ColumnLayoutRelationData* relationData, CachingMethod cachingMethod, CacheEvictionMethod evictionMethod,
             double cachingMethodValue, double minEntropy, double meanEntropy, double medianEntropy,
             double maximumEntropy, double medianGini, double medianInvertedEntropy);

    PositionListIndex* get(Vertical const& vertical);
    PositionListIndex* getOrCreateFor(Vertical const& vertical, ProfilingContext* profilingContext);

    void setMaximumEntropy(double e) { maximumEntropy_ = e; }

    size_t size() const;

    // returns ownership of single column PLIs back to ColumnLayoutRelationData
    virtual ~PLICache();
};