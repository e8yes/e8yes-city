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

#pragma once

#include "procedural/probing/topology_definition.hpp"
#include <random>
#include <vector>

namespace e8 {
namespace procedural {
namespace probing {

// A source sampler generates and stores source vertex samples of a topology.
class SourceSamplerInterface {
public:
  // Samplers are expected to sample with replacement. Therefore, repeated
  // samples can be represented by a single instance for performance purposes.
  struct Sample {
    Sample(unsigned source_index, unsigned frequency, float probability)
        : source_index(source_index), frequency(frequency),
          probability(probability) {}

    // Index of the vertex to be treated as source.
    unsigned source_index;

    // The number of times this source index appears in the sample set.
    unsigned frequency;

    // The probability that this source index is sampled.
    float probability;
  };

  // This constructor doesn't initialize the internal sample array. The array
  // remains to be zero sized after construction.
  SourceSamplerInterface(unsigned sample_count);
  virtual ~SourceSamplerInterface() = default;

  // Returns the current sample set.
  std::vector<Sample> const &SourceSamples() const;

  // The number of samples in the current sample set. Note, it is NOT equivalent
  // to the size of the sample array.
  unsigned SampleCount() const;

  // Generates a new set of samples.
  virtual void UpdateSamples() = 0;

protected:
  unsigned const sample_count_;
  std::vector<Sample> samples_;
};

// This sampler generates vertex samples uniformly over the topology.
class SourceUniformSampler final : public SourceSamplerInterface {
public:
  SourceUniformSampler(Topology const &topology, unsigned sample_count,
                       std::default_random_engine *random_engine);
  ~SourceUniformSampler() override = default;

  // O(s), where s is the sample size
  void UpdateSamples() override;

private:
  unsigned const vertex_count_;
  std::default_random_engine *const random_engine_;
  std::uniform_int_distribution<unsigned> unif_;
};

// This sampler generates vertex samples based on vertex importance. The
// probability a vertex being sampled is it's importance.
class SourceImportanceSampler final : public SourceSamplerInterface {
public:
  SourceImportanceSampler(Topology const &topology, unsigned sample_count,
                          std::default_random_engine *random_engine);
  ~SourceImportanceSampler() override = default;

  // O(s*log(n)), where s is the sample count, and n is the vertex count.
  void UpdateSamples() override;

private:
  std::vector<float> const vertex_pmf_;
  std::vector<float> const vertex_cdf_;
  std::default_random_engine *const random_engine_;
  std::uniform_real_distribution<float> unif_;
};

// This sampler doesn't sample at all. It returns the entire vertex population.
class SourcePopulationSampler final : public SourceSamplerInterface {
public:
  SourcePopulationSampler(Topology const &topology);
  ~SourcePopulationSampler() override = default;

  // It does nothing.
  void UpdateSamples() override;
};

} // namespace probing
} // namespace procedural
} // namespace e8
