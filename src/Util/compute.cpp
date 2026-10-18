#include "compute.h"

using namespace Util;

// DESCRIPTION
// Algorithm similar to ranked choice vote
// But instead of just using ranking, takes a score into consideration
// The lowest allowed score is 0, which means this is the ideal candidate for this voter
// Score is essentially a distance from the ideal, perhaps "error" would be a better term
// but that is a loaded term in programming
// EXAMPLE
// Can be used selecting an optimal subset of a color palette
// that works best for a given image
// TODO
// Could work out a ballot data structure that would allow for O(1) erasure by value while maintaining sorted order
SEResult Util::ScoredElimination(const MDVector<float>& scores, uint32_t winnerCount)
{
	assert(winnerCount > 0);
	assert(winnerCount <= scores.width);
	// Convert scores to list of ballots
	// Each contains indices referring to the score that would be sorted into that position
	std::vector<std::vector<SEIndex>> ballots;
	ballots.reserve(scores.height);
	for (uint32_t i = 0; i < scores.height; ++i) {
		// Argsort (ascending)
		// Source - https://gist.github.com/HViktorTsoi/58eabb4f7c5a303ced400bcfa816f6f5
		std::vector<SEIndex> ballot(scores.width);
		std::iota(ballot.begin(), ballot.end(), 0);
		std::sort(ballot.begin(), ballot.end(), [&scores, i](SEIndex left, SEIndex right) -> bool {
			// Sort indices according to corresponding ballot value
			return scores.span[left, i] < scores.span[right, i];
		});
		ballots.push_back(ballot);
	}
	// Elimination loop
	// In theory this could be "while(true)" and rely solely on the break
	// but best to be cautious
	int maxEliminations = scores.width - winnerCount;
	for (int e = 0; e < maxEliminations; e++) {
		// Add elimination error from each ballot
		std::unordered_map<SEIndex, float> eliminationError;
		for (size_t i = 0; i < ballots.size(); i++) {
			auto& b = ballots[i];
			// Add difference in error between first and second choice for this pixel
			// This is the increase in error this voter would suffer if it could no longer
			// have its first choice
			eliminationError[b[0]] += scores.span[b[1], i] - scores.span[b[0], i];
		}
		// Check if enough candidates have been eliminated
		if (eliminationError.size() <= winnerCount) {
			// We could use eliminationError to generate result and "return" here
			// but that would mean two exit points which involves some code duplication
			// and is more complex.
			break;
		}
		// Remove candidate with lowest accumulated elimination error
		// Source - https://stackoverflow.com/a/56479262
		auto it = std::min_element(eliminationError.begin(), eliminationError.end(), [](const auto& l, const auto& r) { return l.second < r.second; });
		for (auto& b : ballots) {
			std::erase(b, it->first);
		}
	}
	// Get unique remaining candidates (winners)
	std::unordered_set<SEIndex> winnersUnique;
	for (const auto& b : ballots) {
		winnersUnique.insert(b[0]);
	}
	std::vector<SEIndex> winners(winnersUnique.begin(), winnersUnique.end());
	// Get the top choice for each voter (out of the remaining candidates)
	std::vector<uint32_t> choice;
	choice.reserve(ballots.size());
	for (auto& b : ballots) {
		choice.push_back(b[0]);
	}
	assert(winners.size() <= winnerCount);
	assert(choice.size() == scores.height);
	return { winners, choice };
}