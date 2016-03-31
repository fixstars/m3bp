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
#include <thread>
#include "m3bp/configuration.hpp"

namespace m3bp {

class Configuration::Impl {

private:
	unsigned int m_max_concurrency;
	size_type m_partition_count;
	size_type m_default_output_buffer_size;
	size_type m_default_records_per_buffer;
	AffinityMode m_affinity;
	std::string m_profile_log;

public:
	Impl()
		: m_max_concurrency(std::thread::hardware_concurrency())
		, m_partition_count(m_max_concurrency * 8)
		, m_default_output_buffer_size(4 << 20) // 4 [MB]
		, m_default_records_per_buffer(m_default_output_buffer_size / 8)
		, m_affinity(AffinityMode::NONE)
		, m_profile_log()
	{ }

	unsigned int max_concurrency() const noexcept {
		return m_max_concurrency;
	}
	Impl &max_concurrency(unsigned int count) noexcept {
		m_max_concurrency = count;
		return *this;
	}

	size_type partition_count() const noexcept {
		return m_partition_count;
	}
	Impl &partition_count(size_type count) noexcept {
		m_partition_count = count;
		return *this;
	}

	size_type default_output_buffer_size() const noexcept {
		return m_default_output_buffer_size;
	}
	Impl &default_output_buffer_size(size_type size) noexcept {
		m_default_output_buffer_size = size;
		return *this;
	}

	size_type default_records_per_buffer() const noexcept {
		return m_default_records_per_buffer;
	}
	Impl &default_records_per_buffer(size_type max_count) noexcept {
		m_default_records_per_buffer = max_count;
		return *this;
	}


	AffinityMode affinity() const noexcept {
		return m_affinity;
	}
	Impl &affinity(AffinityMode mode) noexcept {
		m_affinity = mode;
		return *this;
	}


	std::string profile_log() const {
		return m_profile_log;
	}
	Impl &profile_log(const std::string &filename){
		m_profile_log = filename;
		return *this;
	}

};


Configuration::Configuration()
	: m_impl(new Impl())
{ }

Configuration::Configuration(const Configuration &config)
	: m_impl(new Impl(*config.m_impl))
{ }

Configuration::Configuration(Configuration &&) noexcept = default;

Configuration::~Configuration() = default;


Configuration &Configuration::operator=(const Configuration &config){
	m_impl.reset(new Impl(*config.m_impl));
	return *this;
}

Configuration &Configuration::operator=(Configuration &&) noexcept = default;


unsigned int Configuration::max_concurrency() const noexcept {
	return m_impl->max_concurrency();
}

Configuration &Configuration::max_concurrency(unsigned int count) noexcept {
	m_impl->max_concurrency(count);
	return *this;
}


size_type Configuration::partition_count() const noexcept {
	return m_impl->partition_count();
}

Configuration &Configuration::partition_count(size_type count) noexcept {
	m_impl->partition_count(count);
	return *this;
}


size_type Configuration::default_output_buffer_size() const noexcept {
	return m_impl->default_output_buffer_size();
}

Configuration &Configuration::default_output_buffer_size(size_type size) noexcept {
	m_impl->default_output_buffer_size(size);
	return *this;
}


size_type Configuration::default_records_per_buffer() const noexcept {
	return m_impl->default_records_per_buffer();
}

Configuration &Configuration::default_records_per_buffer(size_type max_count) noexcept {
	m_impl->default_records_per_buffer(max_count);
	return *this;
}


AffinityMode Configuration::affinity() const noexcept {
	return m_impl->affinity();
}

Configuration &Configuration::affinity(AffinityMode mode) noexcept {
	m_impl->affinity(mode);
	return *this;
}


std::string Configuration::profile_log() const {
	return m_impl->profile_log();
}

Configuration &Configuration::profile_log(const std::string &filename){
	m_impl->profile_log(filename);
	return *this;
}

}

