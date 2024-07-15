#include "algorithms/md/hymd/similarity_data.h"

#include <algorithm>

#include "algorithms/md/hymd/indexes/column_similarity_info.h"
#include "algorithms/md/hymd/lowest_cc_value_id.h"

namespace algos::hymd {

SimilarityData SimilarityData::CreateFrom(indexes::RecordsInfo* const records_info,
                                          ColMatchesInfo const column_matches_info_initial,
                                          util::WorkerThreadPool& pool) {
    bool const one_table_given = records_info->OneTableGiven();
    std::size_t const col_match_number = column_matches_info_initial.size();
    std::vector<ColumnMatchInfo> column_matches_info;
    column_matches_info.reserve(col_match_number);
    std::vector<std::vector<model::Index>> column_matches_lhs_ids;
    column_matches_lhs_ids.reserve(col_match_number);
    auto const& left_records = records_info->GetLeftCompressor();
    auto const& right_records = records_info->GetRightCompressor();
    for (auto const& [measure, left_col_index, right_col_index] : column_matches_info_initial) {
        auto const& left_pli = left_records.GetPli(left_col_index);
        // TODO: cache DataInfo.
        std::shared_ptr<preprocessing::DataInfo const> data_info_left =
                preprocessing::DataInfo::MakeFrom(left_pli, measure->GetArgType());
        std::shared_ptr<preprocessing::DataInfo const> data_info_right;
        auto const& right_pli = right_records.GetPli(right_col_index);
        if (one_table_given && left_col_index == right_col_index) {
            data_info_right = data_info_left;
        } else {
            data_info_right = preprocessing::DataInfo::MakeFrom(right_pli, measure->GetArgType());
        }
        // TODO: sort column matches on the number of LHS CCV IDs.
        auto [lhs_ccv_ids, indexes] =
                measure->MakeIndexes(std::move(data_info_left), std::move(data_info_right),
                                     right_pli.GetClusters(), pool);
        column_matches_info.emplace_back(std::move(indexes), left_col_index, right_col_index);
        column_matches_lhs_ids.push_back(std::move(lhs_ccv_ids));
    }
    return {records_info, std::move(column_matches_info), std::move(column_matches_lhs_ids)};
}

std::unordered_set<PairComparisonResult> SimilarityData::CompareAllWith(
        RecordIdentifier const left_record_id) const {
    // TODO: use the "slim" sim index to fill those instead of lookups in similarity matrices
    CompressedRecord const& left_record = GetLeftCompressor().GetRecords()[left_record_id];
    std::unordered_set<PairComparisonResult> comparisons;
    // Optimization not performed in Metanome.
    RecordIdentifier const start_from = single_table_ ? left_record_id + 1 : 0;
    PairComparisonResult pair_sims;
    pair_sims.reserve(GetColumnMatchNumber());
    indexes::CompressedRecords const& right_records = GetRightCompressor().GetRecords();
    // TODO: parallelize this
    std::for_each(right_records.begin() + start_from, right_records.end(), [&](auto const& record) {
        for (auto const& [sim_info, left_col_index, right_col_index] : column_matches_sim_info_) {
            indexes::SimilarityMatrixRow const& sim_matrix_row =
                    sim_info.similarity_matrix[left_record[left_col_index]];
            auto it = sim_matrix_row.find(record[right_col_index]);
            pair_sims.push_back(it == sim_matrix_row.end() ? kLowestCCValueId : it->second);
        }
        comparisons.insert(pair_sims);
        pair_sims.clear();
    });
    return comparisons;
}

PairComparisonResult SimilarityData::CompareRecords(CompressedRecord const& left_record,
                                                    CompressedRecord const& right_record) const {
    PairComparisonResult comparison_result;
    comparison_result.reserve(GetColumnMatchNumber());
    for (auto const& [sim_info, left_col_index, right_col_index] : column_matches_sim_info_) {
        indexes::SimilarityMatrixRow const& row =
                sim_info.similarity_matrix[left_record[left_col_index]];
        auto sim_it = row.find(right_record[right_col_index]);
        comparison_result.push_back(sim_it == row.end() ? kLowestCCValueId : sim_it->second);
    }
    return comparison_result;
}

model::md::DecisionBoundary SimilarityData::GetLhsDecisionBoundary(
        model::Index column_match_index,
        ColumnClassifierValueId classifier_value_id) const noexcept {
    return column_matches_sim_info_[column_match_index]
            .similarity_info
            .classifier_values[column_matches_lhs_ids_[column_match_index][classifier_value_id]];
}

model::md::DecisionBoundary SimilarityData::GetDecisionBoundary(
        model::Index column_match_index,
        ColumnClassifierValueId classifier_value_id) const noexcept {
    return column_matches_sim_info_[column_match_index]
            .similarity_info.classifier_values[classifier_value_id];
}

}  // namespace algos::hymd