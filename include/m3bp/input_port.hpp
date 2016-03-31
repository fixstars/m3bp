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
#ifndef M3BP_INPUT_PORT_HPP
#define M3BP_INPUT_PORT_HPP

#include <string>
#include <memory>
#include <functional>

namespace m3bp {

enum class Movement {
	/// Movement is not specified. (Using this movement will occurs an error.)
	UNDEFINED = 0,
	/**
	 *  Records will be passed directly.
	 */
	ONE_TO_ONE,
	/**
	 *  Group input records by keys.
	 */
	SCATTER_GATHER,
	/**
	 *  Gather all records into a buffer and broadcast it to all overrided
	 *  methods in {@link ProcessorBase}.
	 */
	BROADCAST
};


/**
 *  A configuration set for an input port.
 */
class InputPort {

public:
	using ValueComparatorType =
		std::function<bool(const void *, const void *)>;

	/**
	 *  Constructs an input port with default settings.
	 */
	InputPort();

	/**
	 *  Constructs an input port with default settings and set the name.
	 *
	 *  @param[in] name  The name of this port.
	 */
	explicit InputPort(const std::string &name);

	/**
	 *  Copy constructor.
	 */
	InputPort(const InputPort &port);

	/**
	 *  Move constructor.
	 */
	InputPort(InputPort &&);

	/**
	 *  Destructor.
	 */
	~InputPort();


	/**
	 *  Copy substitution operator.
	 */
	InputPort &operator=(const InputPort &port);

	/**
	 *  Move substitution operator.
	 */
	InputPort &operator=(InputPort &&);

	/**
	 *  Gets the name of this port.
	 *
	 *  @return The name of this port.
	 */
	const std::string &name() const;


	/**
	 *  Gets the kind of movement that is used for this port.
	 *
	 *  @return The kind of movement that is used for this port.
	 */
	Movement movement() const;

	/**
	 *  Sets the kind of movement that is used for this port.
	 *
	 *  @param[in] m  The kind of movement that is used for this port.
	 *  @return    A reference to this port.
	 */
	InputPort &movement(Movement m);


	/**
	 *  Gets the comparator that is used to sort values in each groups.
	 *
	 *  @return The comparator that is used to sort values in each groups.
	 */
	ValueComparatorType value_comparator() const;

	/**
	 *  Sets the comparator that is used to sort values in each groups.
	 *
	 *  @param[in] comparator  The comparator that is used to sort values in each groups.
	 *  @return    A reference to this port.
	 */
	InputPort &value_comparator(ValueComparatorType comparator);


private:
	class Impl;
	std::unique_ptr<Impl> m_impl;

};

}

#endif

