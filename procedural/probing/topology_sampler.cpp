// e8City
// Copyright (C) 2023 e8yes
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "procedural/probing/topology_sampler.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <cassert>
#include <random>
#include <unordered_map>
#include <vector>

namespace e8 {
namespace procedural {
namespace probing {
namespace {

float const kCdfMaxError = 1e-3f;

std::vector<float> VertexPmf(Topology const &topology) {
  unsigned vertex_count = boost::num_vertices(topology);
  assert(vertex_count > 0);

  std::vector<float> pmf(vertex_count);
  for (unsigned i = 0; i < vertex_count; ++i) {
    assert(topology[i].importance >= 0 && topology[i].importance <= 1);
    pmf[i] = topology[i].importance;
  }
  return pmf;
}

std::vector<float> VertexCdf(Topology const &topology) {
  unsigned vertex_count = boost::num_vertices(topology);
  assert(vertex_count);

  std::vector<float> cdf(vertex_count);
  cdf[0] = topology[0].importance;
  for (unsigned i = 1; i < vertex_count; ++i) {
    cdf[i] = cdf[i - 1] + topology[i].importance;
  }

  assert(std::abs(cdf.back() - 1.0f) < kCdfMaxError);
  return cdf;
}

unsigned NextImportanceSample(std::vector<float> const &vertex_cdf,
                              std::uniform_real_distribution<float> &unif,
                              std::default_random_engine *random_engine) {
  float u = unif(*random_engine);
  auto it = std::lower_bound(vertex_cdf.begin(), vertex_cdf.end(), u);

  if (it == vertex_cdf.end()) {
    return vertex_cdf.size() - 1;
  }
  return it - vertex_cdf.begin();
}

} // namespace

SourceSamplerInterface::SourceSamplerInterface(unsigned sample_count)
    : sample_count_(sample_count) {
  assert(sample_count > 0);
  samples_.reserve(sample_count);
}

std::vector<SourceSamplerInterface::Sample> const &
SourceSamplerInterface::SourceSamples() const {
  return samples_;
}

unsigned SourceSamplerInterface::SampleCount() const { return sample_count_; }

SourceUniformSampler::SourceUniformSampler(
    Topology const &topology, unsigned sample_count,
    std::default_random_engine *random_engine)
    : SourceSamplerInterface(sample_count),
      vertex_count_(boost::num_vertices(topology)),
      random_engine_(random_engine), unif_(0, boost::num_vertices(topology)) {}

void SourceUniformSampler::UpdateSamples() {
  std::unordered_map<unsigned, unsigned> sample_frequency;
  sample_frequency.reserve(sample_count_);
  for (unsigned i = 0; i < sample_count_; ++i) {
    unsigned source_index = unif_(*random_engine_);
    ++sample_frequency[source_index];
  }

  float uniform_pmf = 1.0 / sample_count_;

  samples_.clear();
  std::transform(
      sample_frequency.begin(), sample_frequency.end(),
      std::back_inserter(samples_),
      [uniform_pmf](std::pair<unsigned, unsigned> const &sample_and_frequency) {
        auto const &[source_index, frequency] = sample_and_frequency;
        return Sample(source_index, frequency, uniform_pmf);
      });
}

SourceImportanceSampler::SourceImportanceSampler(
    Topology const &topology, unsigned sample_count,
    std::default_random_engine *random_engine)
    : SourceSamplerInterface(sample_count), vertex_pmf_(VertexPmf(topology)),
      vertex_cdf_(VertexCdf(topology)), random_engine_(random_engine),
      unif_(0, 1) {}

void SourceImportanceSampler::UpdateSamples() {
  std::unordered_map<unsigned, unsigned> sample_frequency;
  sample_frequency.reserve(sample_count_);
  for (unsigned i = 0; i < sample_count_; ++i) {
    unsigned source_index =
        NextImportanceSample(vertex_cdf_, unif_, random_engine_);
    ++sample_frequency[source_index];
  }

  samples_.clear();
  std::transform(
      sample_frequency.begin(), sample_frequency.end(),
      std::back_inserter(samples_),
      [this](std::pair<unsigned, unsigned> const &sample_and_frequency) {
        auto const &[source_index, frequency] = sample_and_frequency;
        return Sample(source_index, frequency, this->vertex_pmf_[source_index]);
      });
}

} // namespace probing
} // namespace procedural
} // namespace e8
