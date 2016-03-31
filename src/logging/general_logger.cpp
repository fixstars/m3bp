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

#include <boost/utility/empty_deleter.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>
#include "logging/general_logger.hpp"

namespace m3bp {

template <typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT> &operator<<(
	std::basic_ostream<CharT, TraitsT> &os, m3bp::LogLevel level)
{
	switch(level){
		case m3bp::LogLevel::TRACE:    os << "trace";     break;
		case m3bp::LogLevel::DEBUG:    os << "debug";     break;
		case m3bp::LogLevel::INFO:     os << "info";      break;
		case m3bp::LogLevel::WARNING:  os << "warning";   break;
		case m3bp::LogLevel::ERROR:    os << "error";     break;
		case m3bp::LogLevel::CRITICAL: os << "ciritical"; break;
	}
	return os;
};

template <typename CharT, typename TraitsT>
inline std::basic_istream<CharT, TraitsT> &operator>>(
	std::basic_istream<CharT, TraitsT> &is, m3bp::LogLevel &level)
{
	int n = 0;
	is >> n;
	level = static_cast<m3bp::LogLevel>(n);
	return is;
}

namespace {

enum Tag {
	LOGGER_TAG_NULL,
	LOGGER_TAG_GENERAL
};

namespace iostreams = boost::iostreams;
iostreams::stream<iostreams::null_sink> g_null_stream((iostreams::null_sink()));

}

namespace log_attr  = boost::log::attributes;
namespace log_expr  = boost::log::expressions;
namespace log_sinks = boost::log::sinks;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity,  "Severity",  m3bp::LogLevel);
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime);
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID",  log_attr::current_thread_id::value_type);
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr,  "Tag",       Tag);

BOOST_LOG_GLOBAL_LOGGER_INIT(
	general_logger, boost::log::sources::severity_logger_mt<LogLevel>)
{
	auto logger = boost::log::sources::severity_logger_mt<LogLevel>();

	logger.add_attribute("TimeStamp", log_attr::local_clock());
	logger.add_attribute("ThreadID",  log_attr::current_thread_id());
	logger.add_attribute("Tag",       log_attr::constant<Tag>(LOGGER_TAG_GENERAL));

	// Add empty sink for suppressing default sink
	auto null_backend = GeneralLogger::create_stream_backend(g_null_stream);
	auto null_frontend = GeneralLogger::create_frontend(null_backend, LogLevel::TRACE);
	null_frontend->set_filter(tag_attr == LOGGER_TAG_NULL);
	GeneralLogger::add_destination(null_frontend);

	return logger;
}


GeneralLogger::StreamBackendPtr
GeneralLogger::create_stream_backend(std::ostream &os){
	auto backend = boost::make_shared<log_sinks::text_ostream_backend>();
	boost::shared_ptr<std::ostream> stream_ptr(&os, boost::empty_deleter());
	backend->add_stream(stream_ptr);
	backend->auto_flush(true);
	return backend;
}

GeneralLogger::TextFileBackendPtr
GeneralLogger::create_text_file_backend(const std::string &filename_pattern){
	return boost::make_shared<log_sinks::text_file_backend>(
		boost::log::keywords::file_name = filename_pattern);
}


GeneralLogger::FrontendPtr
GeneralLogger::create_frontend(StreamBackendPtr backend, LogLevel level){
	using boost::posix_time::ptime;
	auto frontend = boost::make_shared<
		log_sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);
	frontend->set_filter(tag_attr == LOGGER_TAG_GENERAL && severity >= level);
	frontend->set_formatter(log_expr::stream
		<< "[" << log_expr::format_date_time<ptime>("TimeStamp", "%Y-%m-%d %T.%f") << "] "
		<< "[" << thread_id << "] "
		<< "[" << severity << "] "
		<< log_expr::message);
	return frontend;
}

GeneralLogger::FrontendPtr
GeneralLogger::create_frontend(TextFileBackendPtr backend, LogLevel level){
	using boost::posix_time::ptime;
	auto frontend = boost::make_shared<
		log_sinks::synchronous_sink<boost::log::sinks::text_file_backend>>(backend);
	frontend->set_filter(tag_attr == LOGGER_TAG_GENERAL && severity >= level);
	frontend->set_formatter(log_expr::stream
		<< "[" << log_expr::format_date_time<ptime>("TimeStamp", "%Y-%m-%d %T.%f") << "] "
		<< "[" << thread_id << "] "
		<< "[" << severity << "] "
		<< log_expr::message);
	return frontend;
}


void GeneralLogger::add_destination(FrontendPtr sink){
	boost::log::core::get()->add_sink(sink);
}

void GeneralLogger::remove_destination(FrontendPtr sink){
	boost::log::core::get()->remove_sink(sink);
}

}

