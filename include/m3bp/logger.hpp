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
#ifndef M3BP_LOGGER_HPP
#define M3BP_LOGGER_HPP

#include <memory>

namespace m3bp {

namespace internal {

class LogDestinationImpl;

}

enum class LogLevel {
	TRACE = 0,
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	CRITICAL
};

class LogDestination {

public:
	LogDestination();
	LogDestination(const LogDestination &);
	LogDestination(LogDestination &&) noexcept;
	~LogDestination();

	LogDestination &operator=(const LogDestination &);
	LogDestination &operator=(LogDestination &&) noexcept;

private:
	friend class internal::LogDestinationImpl;
	std::unique_ptr<internal::LogDestinationImpl> m_impl;
	LogDestination(internal::LogDestinationImpl &&impl);

};


class Logger {

public:
	using LogDestinationPtr = std::shared_ptr<LogDestination>;


	/**
	 *  @brief Adds a stream as a destination of log messages.
	 *  @param     os     A stream that is used for logging.
	 *  @param[in] level  A verbosity for this destination.
	 *  @return    A handle object for this destination.
	 */
	static LogDestinationPtr add_destination_stream(
		std::ostream &os, LogLevel level);

	/**
	 *  @brief Adds a text file as a destination of log messages.
	 *  @param[in] filename_pattern  A name of file that is used for logging.
	 *  @param[in] level             A verbosity for this destination.
	 *  @return    A handle object for this destination.
	 */
	static LogDestinationPtr add_destination_text_file(
		const std::string &filename_pattern, LogLevel level);


	/**
	 *  @brief Removes a destination of log messages.
	 *  @param[in] destination  A handle object for the destination that will
	 *                          be removed.
	 */
	static void remove_destination(LogDestinationPtr destination);

};

}

#endif

