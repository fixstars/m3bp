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
#ifndef M3BP_OUTPUT_PORT_HPP
#define M3BP_OUTPUT_PORT_HPP

#include <string>
#include <memory>
#include <functional>

namespace m3bp {

/**
 *  A configuration set for an output port.
 */
class OutputPort {

public:
	/**
	 *  Constructs an output port with default settings.
	 */
	OutputPort();

	/**
	 *  Constructos an output port with default settings and set the name.
	 *
	 *  @param[in] name  The name of this port.
	 */
	explicit OutputPort(const std::string &name);

	/**
	 *  Copy constructor.
	 */
	OutputPort(const OutputPort &port);

	/**
	 *  Move constructor.
	 */
	OutputPort(OutputPort &&);

	/**
	 *  Destructor.
	 */
	~OutputPort();


	/**
	 *  Copy substitution operator.
	 */
	OutputPort &operator=(const OutputPort &port);

	/**
	 *  Move substitution operator.
	 */
	OutputPort &operator=(OutputPort &&);


	/**
	 *  Gets the name of this port.
	 *
	 *  @return The name of this port.
	 */
	const std::string &name() const;


	/**
	 *  Gets whether outputs of this port have keys or not.
	 *
	 *  @return whether outputs of this port have keys or not.
	 */
	bool has_key() const;

	/**
	 *  Sets whether outputs of this port have keys or not.
	 *
	 *  @param[in] flag  @c true if outputs of this port have keys.
	 *  @return    A reference to this port.
	 */
	OutputPort &has_key(bool flag);

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;

};

}

#endif

