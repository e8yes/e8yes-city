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

#define BOOST_TEST_MAIN
#include "procedural/probing/topology_definition.hpp"
#include "procedural/probing/topology_sampler.hpp"
#include <boost/test/unit_test.hpp>
#include <random>
#include <vector>

namespace e8 {
namespace procedural {
namespace probing {
namespace {

unsigned const kSourceCount = 100;
unsigned const kSampleCount = 10;

float ValueOf(unsigned vertex_index) { return vertex_index; }

Topology CreateSources(unsigned source_count) {
  Topology result(source_count);

  float value_sum = (ValueOf(0) + ValueOf(source_count - 1)) * source_count / 2;
  for (unsigned i = 0; i < source_count; ++i) {
    result[i].importance = ValueOf(i) / value_sum;
  }

  return result;
}

float EstimatedAverageValue(SourceSamplerInterface *sampler,
                            unsigned source_count, unsigned num_experiments) {
  double estimate_sum = 0;

  for (unsigned i = 0; i < num_experiments; ++i) {
    sampler->UpdateSamples();

    float value_sum = 0.0f;
    unsigned actual_sample_count = 0;
    for (auto const &sample : sampler->SourceSamples()) {
      assert(sample.source_index < source_count);
      assert(sample.correction >= 1.0f / sampler->PopulationCount());

      float value =
          sample.frequency * ValueOf(sample.source_index) * sample.correction;

      actual_sample_count += sample.frequency;
      value_sum += value;
    }
    assert(actual_sample_count == sampler->SampleCount());
    estimate_sum += value_sum / sampler->SampleCount();
  }

  return estimate_sum / num_experiments;
}

float TrueAverageValue(unsigned source_count) {
  return (ValueOf(0) + ValueOf(source_count - 1)) / 2.0f;
}

BOOST_AUTO_TEST_CASE(CheckUniformSamplerBiasIsZero) {
  Topology sources = CreateSources(kSourceCount);
  std::default_random_engine random_engine(/*seed=*/13U);
  SourceUniformSampler sampler(sources, kSampleCount, &random_engine);
  BOOST_CHECK_EQUAL(kSourceCount, sampler.PopulationCount());
  BOOST_CHECK_CLOSE(
      TrueAverageValue(kSourceCount),
      EstimatedAverageValue(&sampler, kSourceCount, /*num_experiments=*/1000),
      1);
}

BOOST_AUTO_TEST_CASE(CheckImportanceSamplerBiasIsZero) {
  Topology sources = CreateSources(kSourceCount);
  std::default_random_engine random_engine(/*seed=*/13U);
  SourceImportanceSampler sampler(sources, kSampleCount, &random_engine);
  BOOST_CHECK_EQUAL(kSourceCount, sampler.PopulationCount());
  BOOST_CHECK_CLOSE(
      TrueAverageValue(kSourceCount),
      EstimatedAverageValue(&sampler, kSourceCount, /*num_experiments=*/100),
      1);
}

BOOST_AUTO_TEST_CASE(CheckPopulationSamplerReturnsThePopulation) {
  Topology sources = CreateSources(kSourceCount);
  SourcePopulationSampler sampler(sources);
  sampler.UpdateSamples();

  BOOST_CHECK_EQUAL(kSourceCount, sampler.PopulationCount());
  BOOST_CHECK_EQUAL(kSourceCount, sampler.SampleCount());

  std::vector<SourcePopulationSampler::Sample> const &samples =
      sampler.SourceSamples();
  for (unsigned i = 0; i < kSourceCount; ++i) {
    BOOST_CHECK_EQUAL(1, samples[i].correction);
    BOOST_CHECK_EQUAL(1, samples[i].frequency);
    BOOST_CHECK_EQUAL(i, samples[i].source_index);
  }
}

} // namespace
} // namespace probing
} // namespace procedural
} // namespace e8
