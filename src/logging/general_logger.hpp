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
#ifndef M3BP_LOGGING_GENERAL_LOGGER_HPP
#define M3BP_LOGGING_GENERAL_LOGGER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/basic_sink_frontend.hpp>
#include "m3bp/logger.hpp"

#define M3BP_GENERAL_LOG(sv) \
	BOOST_LOG_SEV(m3bp::general_logger::get(), m3bp::LogLevel::sv)

namespace m3bp {

BOOST_LOG_GLOBAL_LOGGER(
	general_logger, boost::log::sources::severity_logger_mt<m3bp::LogLevel>);

class GeneralLogger {

public:
	using StreamBackendPtr =
		boost::shared_ptr<boost::log::sinks::text_ostream_backend>;
	using TextFileBackendPtr =
		boost::shared_ptr<boost::log::sinks::text_file_backend>;
	using FrontendPtr =
		boost::shared_ptr<boost::log::sinks::basic_sink_frontend>;

private:
	GeneralLogger();
	GeneralLogger(const GeneralLogger &) = delete;
	GeneralLogger &operator=(const GeneralLogger &) = delete;

public:
	static StreamBackendPtr create_stream_backend(std::ostream &os);
	static TextFileBackendPtr create_text_file_backend(
		const std::string &filename_pattern);

	static FrontendPtr create_frontend(
		StreamBackendPtr backend, LogLevel level);
	static FrontendPtr create_frontend(
		TextFileBackendPtr backend, LogLevel level);

	static void add_destination(FrontendPtr sink);
	static void remove_destination(FrontendPtr sink);

};

}

#endif

