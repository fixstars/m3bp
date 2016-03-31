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
#ifndef M3BP_PROCESSOR_BASE_HPP
#define M3BP_PROCESSOR_BASE_HPP

#include <limits>
#include <memory>
#include <vector>
#include "m3bp/input_port.hpp"
#include "m3bp/output_port.hpp"
#include "m3bp/types.hpp"

namespace m3bp {

namespace internal {

class ProcessorBaseImpl;

}

class Task;

/**
 *  Base class of user defined processors.
 *
 *  Each processor has to inherit this class and implement run().
 *  User defined processors also have to be copyable and movable.
 */
class ProcessorBase {

public:
	/**
	 *  Constructs a 0-input/0-output processor.
	 */
	ProcessorBase();

	/**
	 *  Constructs a processor with input/output specifications.
	 *
	 *  @param[in] iports  Specifications of input ports.
	 *  @param[in] oports  Specifications of output ports.
	 *  @exception ProcessorDefinitionError
	 *                     If invalid specifications are supplied.
	 */
	ProcessorBase(
		const std::vector<InputPort>  &iports,
		const std::vector<OutputPort> &oports);

	/**
	 *  Copy constructor.
	 *
	 *  @param[in] proc  A reference to the original processor.
	 */
	ProcessorBase(const ProcessorBase &proc);

	/**
	 *  Move constructor.
	 *
	 *  @param[in] proc  A reference to the original processor.
	 */
	ProcessorBase(ProcessorBase &&proc) noexcept;

	/**
	 *  Destructs this processor.
	 */
	virtual ~ProcessorBase();


	/**
	 *  Copy substitution operator.
	 *
	 *  @param[in] proc  A reference to the original processor.
	 */
	ProcessorBase &operator=(const ProcessorBase &proc);

	/**
	 *  Move substitution operator.
	 *
	 *  @param[in] proc  A reference to the original processor.
	 */
	ProcessorBase &operator=(ProcessorBase &&proc) noexcept;


	/**
	 *  Gets specifications of input ports.
	 *
	 *  @return A reference to the vector of input specifications.
	 */
	const std::vector<InputPort> &input_ports() const;

	/**
	 *  Gets specifications of output ports.
	 *
	 *  @return A reference to the vector of output specifications.
	 */
	const std::vector<OutputPort> &output_ports() const;


	/**
	 *  Gets the number of tasks for this processor.
	 *
	 *  @return The number of tasks for this processor.
	 */
	size_type task_count() const noexcept;

	/**
	 *  Gets the maximum concurrency of this processor.
	 *
	 *  @return The maximum concurrency of this processor.
	 */
	size_type max_concurrency() const noexcept;


	/**
	 *  Callback for global initialization.
	 */
	virtual void global_initialize(Task & /* task */){ }

	/**
	 *  Callback for global finalization.
	 */
	virtual void global_finalize(Task & /* task */){ }


	/**
	 *  Callback for thread local initialization.
	 */
	virtual void thread_local_initialize(Task & /* task */){ }

	/**
	 *  Callback for thread local finalization.
	 */
	virtual void thread_local_finalize(Task & /* task */){ }


	/**
	 *  Callback for processing records.
	 */
	virtual void run(Task &task) = 0;


protected:
	/**
	 *  Sets the number of tasks for this processor.
	 *
	 *  This function will be used in constructor or global_inititalize
	 *  functions. {@link run()} will be called new_count times on an
	 *  execution.
	 *
	 *  @return A reference to this processor.
	 *  @exception ProcessorDefinitionError
	 *                 If this processor has a non-broadcast input port
	 */
	ProcessorBase &task_count(size_type new_count);

	/**
	 *  Sets the maximum concurrency of this processor.
	 *
	 *  This function will be used in constructor or global_inititalize
	 *  functions.
	 *
	 *  @return A reference to this processor.
	 */
	ProcessorBase &max_concurrency(size_type new_concurrency);


private:
	friend class internal::ProcessorBaseImpl;
	std::unique_ptr<internal::ProcessorBaseImpl> m_impl;

};

}

#endif

