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
#ifndef M3BP_EXCEPTION_HPP
#define M3BP_EXCEPTION_HPP

#include <string>
#include <exception>

namespace m3bp {

/**
 *  Thrown when an invalid specifications are supplied for a processor.
 */
class ProcessorDefinitionError : public std::logic_error {

public:
	/**
	 *  Constructs an error with a message.
	 *
	 *  @param[in] what_arg  Details of a thrown error.
	 */
	explicit ProcessorDefinitionError(const char *what_arg)
		: std::logic_error(what_arg)
	{ }

};

/**
 *  Thrown if an invalid edge is contained in a flow graph.
 */
class RoutingError : public std::logic_error {

public:
	/**
	 *  Constructs an error with a message.
	 *
	 *  @param[in] what_arg  Details of a thrown error.
	 */
	explicit RoutingError(const char *what_arg)
		: std::logic_error(what_arg)
	{ }

};

/**
 *  Thrown if an invalid {@link OutputBuffer} is passed for
 *  {@link OutputWriter::flush_buffer()}.
 */
class InvalidBuffer : public std::logic_error {

public:
	/**
	 *  Constructs an error with a message.
	 *
	 *  @param[in] what_arg  Details of a thrown error.
	 */
	explicit InvalidBuffer(const char *what_arg)
		: std::logic_error(what_arg)
	{ }

};

}

#endif

