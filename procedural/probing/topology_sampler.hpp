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

//
class SourceSamplerInterface {
public:
  //
  struct Sample {
    Sample(unsigned source_index, unsigned frequency, float probability)
        : source_index(source_index), frequency(frequency),
          probability(probability) {}

    //
    unsigned source_index;

    //
    unsigned frequency;

    //
    float probability;
  };

  //
  SourceSamplerInterface(unsigned sample_count);
  virtual ~SourceSamplerInterface() = default;

  //
  std::vector<Sample> const &SourceSamples() const;

  //
  unsigned SampleCount() const;

  //
  virtual void UpdateSamples() = 0;

protected:
  unsigned const sample_count_;
  std::vector<Sample> samples_;
};

//
class SourceUniformSampler final : public SourceSamplerInterface {
public:
  SourceUniformSampler(Topology const &topology, unsigned sample_count,
                       std::default_random_engine *random_engine);
  ~SourceUniformSampler() override = default;

  //
  void UpdateSamples() override;

private:
  unsigned const vertex_count_;
  std::default_random_engine *const random_engine_;
  std::uniform_int_distribution<unsigned> unif_;
};

//
class SourceImportanceSampler final : public SourceSamplerInterface {
public:
  SourceImportanceSampler(Topology const &topology, unsigned sample_count,
                          std::default_random_engine *random_engine);
  ~SourceImportanceSampler() override = default;

  //
  void UpdateSamples() override;

private:
  std::vector<float> const vertex_pmf_;
  std::vector<float> const vertex_cdf_;
  std::default_random_engine *const random_engine_;
  std::uniform_real_distribution<float> unif_;
};

//
class SourcePopulationSampler final : public SourceSamplerInterface {
public:
  SourcePopulationSampler(Topology const &topology);
  ~SourcePopulationSampler() override = default;

  //
  void UpdateSamples() override;

private:
  std::vector<Sample> population_;
};

} // namespace probing
} // namespace procedural
} // namespace e8
