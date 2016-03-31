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
#include "m3bp/output_port.hpp"

namespace m3bp {

class OutputPort::Impl {

private:
	std::string m_name;
	bool m_has_key;

public:
	Impl()
		: m_name()
		, m_has_key(false)
	{ }

	explicit Impl(std::string name)
		: m_name(std::move(name))
		, m_has_key(false)
	{ }

	const std::string &name() const {
		return m_name;
	}

	bool has_key() const {
		return m_has_key;
	}
	Impl &has_key(bool flag){
		m_has_key = flag;
		return *this;
	}

};


OutputPort::OutputPort()
	: m_impl(new Impl())
{ }

OutputPort::OutputPort(const std::string &name)
	: m_impl(new Impl(name))
{ }

OutputPort::OutputPort(const OutputPort &port)
	: m_impl(new Impl(*port.m_impl))
{ }

OutputPort::OutputPort(OutputPort &&) = default;

OutputPort::~OutputPort() = default;


OutputPort &OutputPort::operator=(const OutputPort &port){
	m_impl.reset(new Impl(*port.m_impl));
	return *this;
}

OutputPort &OutputPort::operator=(OutputPort &&) = default;


const std::string &OutputPort::name() const {
	return m_impl->name();
}


bool OutputPort::has_key() const {
	return m_impl->has_key();
}

OutputPort &OutputPort::has_key(bool flag){
	m_impl->has_key(flag);
	return *this;
}

}

