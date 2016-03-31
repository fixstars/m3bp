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
#include "m3bp/logger.hpp"
#include "api/internal/logger_impl.hpp"
#include "logging/general_logger.hpp"

namespace m3bp {

LogDestination::LogDestination()
	: m_impl(new internal::LogDestinationImpl())
{ }

LogDestination::LogDestination(const LogDestination &dest)
	: m_impl(new internal::LogDestinationImpl(*dest.m_impl))
{ }

LogDestination::LogDestination(internal::LogDestinationImpl &&impl)
	: m_impl(new internal::LogDestinationImpl(std::move(impl)))
{ }

LogDestination::LogDestination(LogDestination &&) noexcept = default;

LogDestination::~LogDestination() = default;


LogDestination &LogDestination::operator=(const LogDestination &dest){
	m_impl.reset(new internal::LogDestinationImpl(*dest.m_impl));
	return *this;
}

LogDestination &
LogDestination::operator=(LogDestination &&) noexcept = default;


Logger::LogDestinationPtr
Logger::add_destination_stream(std::ostream &os, LogLevel level){
	auto backend = GeneralLogger::create_stream_backend(os);
	auto frontend = GeneralLogger::create_frontend(backend, level);
	GeneralLogger::add_destination(frontend);
	auto handle = std::make_shared<LogDestination>();
	internal::LogDestinationImpl::get_impl(*handle).sink(frontend);
	return handle;
}

Logger::LogDestinationPtr Logger::add_destination_text_file(
	const std::string &filename_pattern, LogLevel level)
{
	auto backend = GeneralLogger::create_text_file_backend(filename_pattern);
	auto frontend = GeneralLogger::create_frontend(backend, level);
	GeneralLogger::add_destination(frontend);
	auto handle = std::make_shared<LogDestination>();
	internal::LogDestinationImpl::get_impl(*handle).sink(frontend);
	return handle;
}


void Logger::remove_destination(LogDestinationPtr destination){
	if(destination){
		auto sink =
			internal::LogDestinationImpl::get_impl(*destination).sink();
		GeneralLogger::remove_destination(sink);
	}
}

}

