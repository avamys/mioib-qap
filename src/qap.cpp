#include "qap.hpp"
#include <numeric>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <tuple>
#include <cmath>
#include <random>
#include <map>
#include <climits>

typedef std::pair<int, int> Move;

Permutation QAP::generatePermutation(unsigned const int & n)
{
	Permutation arr(n);
	Permutation res;
	for (int i = 0; i < n; i++)
	{
		arr[i] = i;
	}

	while (arr.size())
	{
		int new_n = arr.size();
		int index = rand() % new_n;
		int num = arr[index];
		std::swap(arr[index], arr[new_n - 1]);
		arr.pop_back();
		res.push_back(num);
	}
	return res;
}

void QAP::readData(const std::string filename)
{
	std::ifstream data;
	data.open(filename);

	int dimension;
	data >> dimension;

	for (int i = 0; i < dimension; i++) {
		facilities.push_back(Permutation());
		for (int i = 0; i < dimension; i++)
		{
			int number;
			data >> number;
			facilities.back().push_back(number);
		}
	}

	for (int i = 0; i < dimension; i++) {
		locations.push_back(Permutation());
		for (int i = 0; i < dimension; i++)
		{
			int number;
			data >> number;
			locations.back().push_back(number);
		}
	}
	data.close();
}

int QAP::getCost(const Permutation &perm)
{
	int cost = 0;
	for (int i = 0; i < perm.size(); i++)
	{
		for (int j = 0; j < perm.size(); j++)
		{
			cost += facilities[i][j] * locations[perm[i]][perm[j]];
		}
	}
	return cost;
}


int QAP::updateCost(const Permutation& old_perm, const Permutation& new_perm, unsigned const int& i, unsigned const int& j)
{
	int new_cost = cost;
	
	int dim = old_perm.size();
	
	for (int idx = 0; idx < dim; idx++)
	{

		if (idx == i)
		{
			new_cost -= facilities[i][idx] * locations[old_perm[i]][old_perm[idx]];
			new_cost += facilities[i][idx] * locations[new_perm[i]][new_perm[idx]];

			new_cost -= facilities[j][idx] * locations[old_perm[j]][old_perm[idx]];
			new_cost += facilities[j][idx] * locations[new_perm[j]][new_perm[idx]];

			new_cost -= facilities[idx][j] * locations[old_perm[idx]][old_perm[j]];
			new_cost += facilities[idx][j] * locations[new_perm[idx]][new_perm[j]];

			continue;
		}

		if (idx == j)
		{
			new_cost -= facilities[idx][j] * locations[old_perm[idx]][old_perm[j]];
			new_cost += facilities[idx][j] * locations[new_perm[idx]][new_perm[j]];

			continue;
		}

		new_cost -= facilities[i][idx] * locations[old_perm[i]][old_perm[idx]];
		new_cost += facilities[i][idx] * locations[new_perm[i]][new_perm[idx]];

		new_cost -= facilities[j][idx] * locations[old_perm[j]][old_perm[idx]];
		new_cost += facilities[j][idx] * locations[new_perm[j]][new_perm[idx]];

		new_cost -= facilities[idx][i] * locations[old_perm[idx]][old_perm[i]];
		new_cost += facilities[idx][i] * locations[new_perm[idx]][new_perm[i]];

		new_cost -= facilities[idx][j] * locations[old_perm[idx]][old_perm[j]];
		new_cost += facilities[idx][j] * locations[new_perm[idx]][new_perm[j]];
	}
	return new_cost;

}

double QAP::findTemp()
{
	int dim = facilities.size();
	unsigned int diff_sum = 0;
	for (int i = 0; i < 10000; i++)
	{
		Permutation perm = generatePermutation(dim);
		cost = getCost(perm);

		int random_i = std::rand() % (dim - 1);
		int random_j = (std::rand() % (dim - random_i - 1)) + random_i + 1;
		Permutation neighbour(perm);
		std::swap(neighbour[random_i], neighbour[random_j]);
		int cost2 = updateCost(perm, neighbour, random_i, random_j);

		diff_sum += abs(cost - cost2);
	}
	double diff_mean = diff_sum / 10000;
	double c = -diff_mean / log(0.9);

	return c;
}

std::tuple<Permutation, int> QAP::randomSearch(unsigned const int &n, const double &time_seconds)
{
	int steps = 0;
	Permutation act = generatePermutation(n);

	clock_t begin = clock();

	do
	{
		act = generatePermutation(n);
		steps++;

	} while (double(clock() - begin) / CLOCKS_PER_SEC < time_seconds);

	return std::make_tuple(act, steps);
}

std::tuple<Permutation, int> QAP::randomWalk(unsigned const int &n, const double &time_seconds)
{
	int steps = 0;
	Permutation act = generatePermutation(n);

	clock_t begin = clock();

	do
	{
		int random_i = std::rand() % (n - 1);
		int random_j = (std::rand() % (n - random_i - 1)) + random_i + 1;

		std::swap(act[random_i], act[random_j]);
		steps++;
	} while (double(clock() - begin) / CLOCKS_PER_SEC < time_seconds);

	return std::make_tuple(act, steps);
}

std::tuple<Permutation, int, int> QAP::localGreedy(unsigned const int &n)
{
	int steps = 0;
	Permutation act = generatePermutation(n);
	cost = getCost(act);

	// EXPERIMENT
	int de_cost = cost;
	
	bool found = false;
	int init_cost;

	clock_t begin = clock();

	do
	{
		init_cost = cost;
		for (int i = 0; i < n - 1; i++)
		{
			for (int j = i + 1; j < n; j++)
			{
				// Terminate
				if (double(clock() - begin) / CLOCKS_PER_SEC > 60)
				{
					return std::make_tuple(act, de_cost, steps);
				}

				Permutation y(act);
				std::swap(y[i], y[j]);

				int new_cost = updateCost(act, y, i, j);
				if (new_cost < cost)
				{
					act = y;
					cost = new_cost;
					found = true;
					steps++;
					break;
				}
			}
			if (found == true)
			{
				break;
			}
		}
		found = false;
	} while (init_cost != cost);

	return std::make_tuple(act, de_cost, steps);
}

std::tuple<Permutation, int, int> QAP::localSteepest(unsigned const int &n)
{
	int steps = 0;
	Permutation act = generatePermutation(n);
	cost = getCost(act);

	// EXPERIMENT
	int de_cost = cost;
	
	Permutation best_perm = act;
	int best_cost = cost;

	clock_t begin = clock();

	do
	{
		cost = best_cost;
		for (int i = 0; i < n - 1; i++)
		{
			for (int j = i + 1; j < n; j++)
			{
				// Terminate
				if (double(clock() - begin) / CLOCKS_PER_SEC > 60)
				{
					return std::make_tuple(act, de_cost, steps);
				}

				Permutation y = act;
				std::swap(y[i], y[j]);

				int new_cost = updateCost(act, y, i, j);
				if (new_cost < best_cost)
				{
					best_perm = y;
					best_cost = new_cost;
				}
			}
		}
		act = best_perm;
		steps++;
	} while (best_cost != cost);

	return std::make_tuple(act, de_cost, steps);
}

Permutation QAP::heuristics(unsigned const int &n)
{
	std::vector<Permutation> P = facilities;
	std::vector<Permutation> L = locations;

	Permutation solution(n);

	auto cols = col_sum(L);
	auto rows = row_sum(P);

	for (int i = 0; i < n; i++)
	{
		auto max_row = std::distance(rows.begin(), std::max_element(rows.begin(), rows.end()));
		auto min_col = std::distance(cols.begin(), std::min_element(cols.begin(), cols.end()));

		solution[max_row] = min_col;

		cols[min_col] = INT_MAX;
		rows[max_row] = INT_MIN;
	}

	return solution;
}

std::tuple<Permutation, int, int> QAP::simmulatedAnnealing(unsigned const int & n, const double & temp)
{
	int steps = 0;
	double temp0 = temp;
	int k = 0;
	int L = (n*(n - 1) / 2); // DO USTALENIA
	Permutation act = generatePermutation(n);
	cost = getCost(act);

	int de_cost = cost;

	bool found = false;
	int init_cost;
	int finish_thr = (int)(L / 2) / 2;
	int counter = 0;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);

	clock_t begin = clock();

	do
	{
		//std::cout << "Iter " << k << ": " << temp0 << std::endl;
		for (int length = 0; length < L; length++)
		{
			init_cost = cost;
			for (int i = 0; i < n - 1; i++)
			{
				for (int j = i + 1; j < n; j++)
				{
					counter++;
					// Terminate
					if (double(clock() - begin) / CLOCKS_PER_SEC > 60 || counter >= finish_thr)
					{
						return std::make_tuple(act, de_cost, steps);
					}

					Permutation y(act);
					std::swap(y[i], y[j]);

					int new_cost = updateCost(act, y, i, j);
					if (new_cost < cost)
					{
						act = y;
						cost = new_cost;
						found = true;
						steps++;
						break;
					}
					else
					{
						double cond = exp(-(new_cost - cost) / temp0);
						double thr = dis(gen);

						if (cond > thr)
						{
							//std::cout << "Difference: " << new_cost - cost<< " || Probability: " << cond << " || " << thr << std::endl;
							act = y;
							cost = new_cost;
							found = true;
							steps++;
							break;
						}
					}
				}
				if (found == true)
				{
					counter = 0;
					break;
				}
			}
			found = false;
		}
		k++;
		temp0 *= 0.8;

	} while (init_cost != cost);

	return std::make_tuple(act, de_cost, steps);
}

std::tuple<Permutation, int, int> QAP::tabu(unsigned const int &n) {

	std::map<Move, int> tabuList;
	std::map<Move, int> costList;

	for (int i = 0; i < n - 1; i++) {
		for (int j = i + 1; j < n; j++) {
			tabuList[Move(i, j)] = 0;
		}
	}
	const int validityTime = 5;

	int steps = 0;
	Permutation act = generatePermutation(n);
	cost = getCost(act);

	Permutation best_perm = act;
	int best_cost = cost;

	clock_t begin = clock();

	do
	{
		Move best_move(0, 0);
		for (int i = 0; i < n - 1; i++)
		{
			for (int j = i + 1; j < n; j++)
			{
				// Terminate
				if (double(clock() - begin) / CLOCKS_PER_SEC > 10)
				{
					return std::make_tuple(act, best_cost, steps);
				}

				Permutation y = act;
				std::swap(y[i], y[j]);

				int cost = updateCost(act, y, i, j);
				if (cost < best_cost)
				{
					best_move = Move(i, j);
				}
				costList[Move(i, j)] = cost;
			}
		}

		if (best_move == Move(0, 0))
		{
			best_move = Move(1, 2);
			int not_the_worse_cost = costList[Move(1, 2)];
			for (auto &&elem : tabuList)
			{
				if (elem.second > 0)
				{
					continue;
				}
				if (not_the_worse_cost < costList[elem.first])
				{
					not_the_worse_cost = costList[elem.first];
					best_move = elem.first;
				}
			}
		}

		for (auto&& elem : tabuList)
		{
			if (elem.second > 0)
			{
				elem.second--;
			}
		}

		tabuList[best_move] = validityTime;
		std::swap(act[best_move.first], act[best_move.second]);

		steps++;
	} while (1);

	return std::make_tuple(act, best_cost, steps);
}
