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
#ifndef M3BP_TASK_HPP
#define M3BP_TASK_HPP

#include <memory>
#include "m3bp/types.hpp"
#include "m3bp/input_reader.hpp"
#include "m3bp/output_writer.hpp"

namespace m3bp {

namespace internal {

class TaskImpl;

}


/**
 *  Task object that will be passed for user defined processors.
 */
class Task {

public:
	/**
	 *  Constructs an invalid task.
	 */
	Task();

	Task(const Task &) = delete;

	/**
	 *  Move constructor.
	 */
	Task(Task &&task);

	/**
	 *  Destructor.
	 */
	~Task();


	Task &operator=(const Task &) = delete;

	/**
	 *  Move substitution operator.
	 */
	Task &operator=(Task &&task);


	/**
	 *  Gets the identifier of this logical task.
	 *
	 *  @return The identifier of this logical task.
	 */
	identifier_type logical_task_id() const;

	/**
	 *  Gets the identifier of this physical task.
	 *
	 *  @return The identifier of this physical task.
	 */
	identifier_type physical_task_id() const;


	/**
	 *  Gets a reader for an input port.
	 *
	 *  @param[in] port_id  An index of the input port.
	 *  @return    A reader for @c port_id -th input port.
	 */
	InputReader  input(identifier_type port_id);

	/**
	 *  Gets a writer for an output port.
	 *
	 *  @param[in] port_id  An index of the output port.
	 *  @return    A writer for @c port_id -th output port.
	 */
	OutputWriter output(identifier_type port_id);


	/**
	 *  Gets whether the current execution was cancelled or not.
	 *
	 *  @return @c true if the current execution was cancelled.
	 */
	bool is_cancelled() const;

private:
	friend class internal::TaskImpl;
	explicit Task(internal::TaskImpl &&impl);
	std::unique_ptr<internal::TaskImpl> m_impl;

};

}

#endif

