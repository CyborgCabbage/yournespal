#pragma once
#include <common.h>
#include <Util/mdvector.h>

namespace Util {

// Pair<Winners List, Ballots>
using SEIndex = uint16_t;
using SEResult = std::pair<std::vector<SEIndex>, std::vector<uint32_t>>;

SEResult ScoredElimination(const MDVector<float>& scores, uint32_t winnerCount);

}