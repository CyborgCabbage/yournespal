#include "core.h"

using namespace ynp;

std::ofstream* ynp::logFile = nullptr;

std::string ynp::ToLowercase(std::string_view str)
{
	std::string result{ str };
	std::transform(str.begin(), str.end(), result.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return result;
}

SEResult ynp::ScoredElimination(const Eigen::ArrayXXf& scores, uint32_t winnerCount)
{
	// Convert scores to list of ballots
	// Each contains indices
	std::vector<std::vector<SEIndex>> ballots;
	ballots.reserve(scores.cols());
	for (auto col : scores.colwise()) {
		// Argsort
		// Source - https://gist.github.com/HViktorTsoi/58eabb4f7c5a303ced400bcfa816f6f5
		std::vector<SEIndex> ballot(col.size());
		std::iota(ballot.begin(), ballot.end(), 0);
		std::sort(ballot.begin(), ballot.end(), [&col](int left, int right) -> bool {
			// Sort indices according to corresponding ballot value
			return col[left] < col[right];
		});
		ballots.push_back(ballot);
	}
	// Elimination loop (TODO: early exit)
	int maxEliminations = scores.rows() - winnerCount;
	for (int e = 0; e < maxEliminations; e++) {
		// Add elimination error from each ballot
		std::unordered_map<SEIndex, float> eliminationError;
		for (size_t i = 0; i < ballots.size(); i++) {
			auto& b = ballots[i];
			SEIndex first = b[0];
			SEIndex second = b[1];
			eliminationError[b[0]] += scores(second, i) + scores(first, i);
		}
		if (eliminationError.size() <= winnerCount) {
			size_t winnersSize = eliminationError.size();
			// Get vector of winners
			std::vector<SEIndex> winners;
			winners.reserve(eliminationError.size());
			for (const auto& [key, value] : eliminationError) {
				winners.push_back(key);
			}
			// Ballot to eigen
			std::vector<uint32_t> choice;
			choice.reserve(ballots.size());
			for (auto& b : ballots) {
				choice.push_back(b[0]);
			}
			return SEResult{ winners, choice };
		}
		// Remove candidate with lowest elimination error from all ballots
		// Source - https://stackoverflow.com/a/56479262
		auto it = std::min_element(eliminationError.begin(), eliminationError.end(), [](const auto& l, const auto& r) { return l.second < r.second; });
		for (auto& b : ballots) {
			std::erase(b, it->first);
		}
	}
	// Get vector of winners
	std::unordered_set<SEIndex> winners;
	for (const auto& b : ballots) {
		winners.insert(b[0]);
	}
	// Ballot to eigen
	std::vector<uint32_t> choice;
	choice.reserve(ballots.size());
	for (auto& b : ballots) {
		choice.push_back(b[0]);
	}
	return SEResult{ std::vector<SEIndex>(winners.begin(), winners.end()), choice };
}
