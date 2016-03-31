/*
 * Copyright 2016 Fixstars Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef M3BP_CONFIGURATION_HPP
#define M3BP_CONFIGURATION_HPP

#include <memory>
#include "m3bp/types.hpp"

namespace m3bp {

/**
 *  Constant values for setting affinity mode.
 */
enum class AffinityMode {
	/**
	 *  Do not use explicit affinity configuration.
	 */
	NONE,
	/**
	 */
	COMPACT,
	/**
	 */
	SCATTER
};

/**
 *  A property set of global configuration.
 */
class Configuration {

public:
	/**
	 *  Constructs a property set with default values.
	 */
	Configuration();

	/**
	 *  Copy constructor.
	 */
	Configuration(const Configuration &config);

	/**
	 *  Move constructor.
	 */
	Configuration(Configuration &&config) noexcept;

	/**
	 *  Destructor.
	 */
	~Configuration();


	/**
	 *  Copy substitution operator.
	 */
	Configuration &operator=(const Configuration &config);

	/**
	 *  Move substitution operator.
	 */
	Configuration &operator=(Configuration &&config) noexcept;


	/**
	 *  Returns the maximum concurrency.
	 *
	 *  @return The maximum concurrency.
	 */
	unsigned int max_concurrency() const noexcept;

	/**
	 *  Sets the maximum concurrency.
	 *
	 *  @param[in] count  A new value of maximum concurrency.
	 *  @return    The reference to this property set.
	 */
	Configuration &max_concurrency(unsigned int count) noexcept;


	/**
	 *  Returns the number of partitions which is used for scatter-gather
	 *  operations.
	 *
	 *  @return The number of partitions.
	 */
	size_type partition_count() const noexcept;

	/**
	 *  Sets the number of partitions which is used for scatter-gather
	 *  operations.
	 *
	 *  @param[in] count  A new value of the number of partitions.
	 *  @return    The reference to this property set.
	 */
	Configuration &partition_count(size_type count) noexcept;


	/**
	 *  Returns the default size for each raw block of output buffer.
	 *
	 *  @return The default size for each raw block of output buffer in bytes.
	 */
	size_type default_output_buffer_size() const noexcept;

	/**
	 *  Sets the default size for each raw block of output buffer.
	 *
	 *  @param[in] size  The default size for each raw block of output buffer
	 *                   in bytes.
	 *  @return    The reference to this property set.
	 */
	Configuration &default_output_buffer_size(size_type size) noexcept;


	/**
	 *  Returns the default maximum number of records for each raw block of
	 *  output buffer.
	 *
	 *  @return The default maximum number of records for each raw block of
	 *          output buffer.
	 */
	size_type default_records_per_buffer() const noexcept;

	/**
	 *  Sets the default maximum number of records for each raw block of
	 *  output buffer.
	 *
	 *  @param[in] count  The new default maximum number of records for each
	 *                    raw block of output buffer.
	 *  @return    The reference to this property set.
	 */
	Configuration &default_records_per_buffer(size_type max_count) noexcept;


	/**
	 *  Returns the current configuration of thread affinity.
	 *
	 *  @return The current configuration of thread affinity.
	 */
	AffinityMode affinity() const noexcept;

	/**
	 *  Sets the configuration of thread affinity.
	 *
	 *  @param[in] mode  The new configuration of thread affinity.
	 *  @return    The reference to this property set.
	 */
	Configuration &affinity(AffinityMode mode) noexcept;


	/**
	 *  Returns the destination of the profile log.
	 *
	 *  @return An empty string if profile logging is disabled,
	 *          a path to the profile log otherwise.
	 */
	std::string profile_log() const;

	/**
	 *  Sets the destination of the profile log.
	 *
	 *  @param[in] filename  An empty string if profile logging will be
	 *                       disabled, a path to the destination of profile
	 *                       logs otherwise.
	 *  @return    The reference to this property set.
	 */
	Configuration &profile_log(const std::string &filename);

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;

};

}

#endif

