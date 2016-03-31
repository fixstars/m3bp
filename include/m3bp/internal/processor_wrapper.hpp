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
#ifndef M3BP_INTERNAL_PROCESSOR_WRAPPER_HPP
#define M3BP_INTERNAL_PROCESSOR_WRAPPER_HPP

#include <memory>

namespace m3bp {

class ProcessorBase;

namespace internal {

class ProcessorWrapper {

private:
	class TypedProcessorWrapperBase {

	public:
		virtual ~TypedProcessorWrapperBase(){ }

		virtual TypedProcessorWrapperBase *clone() const = 0;

		virtual const ProcessorBase &processor() const = 0;
		virtual ProcessorBase &processor() = 0;

	};

	template <typename ProcessorType>
	class TypedProcessorWrapper : public TypedProcessorWrapperBase {

	private:
		std::unique_ptr<ProcessorType> m_processor;

	public:
		explicit TypedProcessorWrapper(const ProcessorType &proc)
			: TypedProcessorWrapperBase()
			, m_processor(new ProcessorType(proc))
		{ }

		virtual TypedProcessorWrapperBase *clone() const override {
			return new TypedProcessorWrapper<ProcessorType>(*m_processor);
		}

		virtual const ProcessorBase &processor() const override {
			return *m_processor;
		}
		virtual ProcessorBase &processor() override {
			return *m_processor;
		}

	};

	std::unique_ptr<TypedProcessorWrapperBase> m_typed_wrapper;

public:
	ProcessorWrapper();

	template <typename ProcessorType>
	explicit ProcessorWrapper(const ProcessorType &proc)
		: m_typed_wrapper(new TypedProcessorWrapper<ProcessorType>(proc))
	{ }

	ProcessorWrapper(const ProcessorWrapper &pw);
	ProcessorWrapper(ProcessorWrapper &&pw);

	ProcessorWrapper &operator=(const ProcessorWrapper &pw);
	ProcessorWrapper &operator=(ProcessorWrapper &&pw);

	const ProcessorBase *operator->() const;
	ProcessorBase *operator->();

	const ProcessorBase &operator*() const;
	ProcessorBase &operator*();

};

}
}

#endif

