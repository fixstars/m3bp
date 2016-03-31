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
#include "m3bp/input_port.hpp"

namespace m3bp {

class InputPort::Impl {

private:
	std::string m_name;
	Movement m_movement;
	ValueComparatorType m_value_comparator;

public:
	Impl()
		: m_name()
		, m_movement(Movement::UNDEFINED)
		, m_value_comparator()
	{ }

	explicit Impl(std::string name)
		: m_name(std::move(name))
		, m_movement(Movement::UNDEFINED)
		, m_value_comparator()
	{ }

	const std::string &name() const {
		return m_name;
	}

	Movement movement() const {
		return m_movement;
	}
	Impl &movement(Movement m){
		m_movement = m;
		return *this;
	}

	ValueComparatorType value_comparator() const {
		return m_value_comparator;
	}
	Impl &value_comparator(ValueComparatorType comparator){
		m_value_comparator = std::move(comparator);
		return *this;
	}

};


InputPort::InputPort()
	: m_impl(new Impl())
{ }

InputPort::InputPort(const std::string &name)
	: m_impl(new Impl(name))
{ }

InputPort::InputPort(const InputPort &port)
	: m_impl(new Impl(*port.m_impl))
{ }

InputPort::InputPort(InputPort &&) = default;

InputPort::~InputPort() = default;


InputPort &InputPort::operator=(const InputPort &port){
	m_impl.reset(new Impl(*port.m_impl));
	return *this;
}

InputPort &InputPort::operator=(InputPort &&) = default;


const std::string &InputPort::name() const {
	return m_impl->name();
}


Movement InputPort::movement() const {
	return m_impl->movement();
}

InputPort &InputPort::movement(Movement m){
	m_impl->movement(m);
	return *this;
}


InputPort::ValueComparatorType InputPort::value_comparator() const {
	return std::move(m_impl->value_comparator());
}

InputPort &InputPort::value_comparator(ValueComparatorType comparator){
	m_impl->value_comparator(comparator);
	return *this;
}

}

