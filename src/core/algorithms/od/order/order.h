#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/container_hash/hash.hpp>

#include "algorithms/algorithm.h"
#include "model/table/column_layout_typed_relation_data.h"
#include "config/tabular_data/input_table_type.h"
#include "sorted_partitions.h"
#include "order_enums.h"

namespace algos::order {

class Order : public Algorithm {
public:
    using Node = std::vector<unsigned int>;
    using Hash = boost::hash<std::vector<unsigned int>>;
    using AttributeList = std::vector<unsigned int>;
    using SortedPartitions = std::unordered_map<Node, SortedPartition, Hash>;
    using LatticeLevel = std::unordered_set<Node, Hash>;
    using CandidateSets =
            std::unordered_map<AttributeList, std::unordered_set<AttributeList, Hash>, Hash>;
    using OrderDependencies =
            std::unordered_map<AttributeList, std::unordered_set<AttributeList, Hash>, Hash>;
    using TypedRelation = model::ColumnLayoutTypedRelationData;

    config::InputTable input_table_;
    std::unique_ptr<TypedRelation> typed_relation_;
    SortedPartitions sorted_partitions_;
    std::vector<AttributeList> single_attributes_;
    CandidateSets previous_candidate_sets_;
    CandidateSets candidate_sets_;
    OrderDependencies valid_;
    OrderDependencies merge_invalidated_;
    unsigned int level_;

    void RegisterOptions();
    void LoadDataInternal() override;
    void ResetState() override;
    void CreateSortedPartitions();
    void CreateSortedPartitionsFromSingletons(AttributeList const& attr_list);
    ValidityType CheckForSwap(SortedPartition const& l, SortedPartition const& r);
    void ComputeDependencies(LatticeLevel const& lattice_level);
    std::vector<AttributeList> Extend(AttributeList const& lhs, AttributeList const& rhs);
    bool IsMinimal(AttributeList const& a);
    void UpdateCandidateSets();
    void MergePrune();
    void Prune(LatticeLevel& lattice_level);
    LatticeLevel GenerateNextLevel(LatticeLevel const& l);
    void PrintValidOD();
    unsigned long long ExecuteInternal() final;

public:
    Order();
};

}  // namespace algos::order