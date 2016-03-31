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
#ifndef M3BP_API_INTERNAL_LOGGER_IMPL_HPP
#define M3BP_API_INTERNAL_LOGGER_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include "logging/general_logger.hpp"

namespace m3bp {
namespace internal {

class LogDestinationImpl {

private:
	boost::shared_ptr<boost::log::sinks::basic_sink_frontend> m_sink;

public:
	LogDestinationImpl()
		: m_sink(nullptr)
	{ }

	LogDestinationImpl(
		boost::shared_ptr<boost::log::sinks::basic_sink_frontend> sink)
		: m_sink(std::move(sink))
	{ }


	boost::shared_ptr<boost::log::sinks::basic_sink_frontend> sink() const {
		return m_sink;
	}

	LogDestinationImpl &sink(
		boost::shared_ptr<boost::log::sinks::basic_sink_frontend> ptr)
	{
		m_sink = ptr;
		return *this;
	}


	static LogDestinationImpl &get_impl(LogDestination &dest){
		return *dest.m_impl;
	}

	static LogDestination wrap_impl(LogDestinationImpl &&impl){
		return LogDestination(std::move(impl));
	}

};

}
}

#endif

