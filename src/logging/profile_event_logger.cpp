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
#include <sstream>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstring>
#include <cstdint>
#include "common/make_unique.hpp"
#include "logging/profile_event_logger.hpp"

namespace m3bp {

namespace {

enum class EventMagic {
	CREATE_PHYSICAL_TASK,
	PHYSICAL_DEPENDENCY,
	BEGIN_PREPARATION,
	END_PREPARATION,
	BEGIN_EXECUTION,
	END_EXECUTION,
	ALLOCATE_MEMORY,
	RELEASE_MEMORY,
	LOCK_MEMORY,
	UNLOCK_MEMORY,
	MAGIC_KINDS
};

#define STRING_DEFINITION(str) constexpr char str_ ## str [] = #str
STRING_DEFINITION(create_physical_task);
STRING_DEFINITION(physical_dependency);
STRING_DEFINITION(begin_preparation);
STRING_DEFINITION(end_preparation);
STRING_DEFINITION(begin_execution);
STRING_DEFINITION(end_execution);
STRING_DEFINITION(allocate_memory);
STRING_DEFINITION(release_memory);
STRING_DEFINITION(lock_memory);
STRING_DEFINITION(unlock_memory);

STRING_DEFINITION(timestamp);
STRING_DEFINITION(physical_id);
STRING_DEFINITION(logical_id);
STRING_DEFINITION(task_id);
STRING_DEFINITION(producer);
STRING_DEFINITION(consumer);
STRING_DEFINITION(object_id);
STRING_DEFINITION(size);
STRING_DEFINITION(numa_node);
#undef STRING_DEFINITION

inline uint64_t current_timestamp(){
	const auto tp = std::chrono::steady_clock::now();
	const auto duration =
		std::chrono::duration_cast<std::chrono::microseconds>(
			tp.time_since_epoch());
	return duration.count();
};

template <typename T, const char *NAME>
class BinaryLogField {
public:
	using type = T;
	static const char *name(){ return NAME; }
};

template <typename... Args>
class BinaryLoggerImpl;

template <typename Field, typename... Rest>
class BinaryLoggerImpl<Field, Rest...> {
public:
	static const size_type size =
		sizeof(typename Field::type) + BinaryLoggerImpl<Rest...>::size;

	template <typename... Ts>
	static void write(void *dst, const typename Field::type &x, Ts... rest){
		using T = typename Field::type;
		memcpy(dst, &x, sizeof(T));
		BinaryLoggerImpl<Rest...>::write(
			reinterpret_cast<uint8_t *>(dst) + sizeof(T), rest...);
	}

	static void to_json(std::ostream &os, const void *src){
		using T = typename Field::type;
		T x;
		memcpy(&x, src, sizeof(T));
		os << ",\"" << Field::name() << "\":" << x;
		BinaryLoggerImpl<Rest...>::to_json(
			os, reinterpret_cast<const uint8_t *>(src) + sizeof(T));
	}
};

template <>
class BinaryLoggerImpl<> {
public:
	static const size_type size = 0;
	static void write(void *){ }
	static void to_json(std::ostream &, const void *){ }
};

template <EventMagic MAGIC, const char *TYPE_NAME, typename... Fields>
class BinaryLogger {
public:
	static const size_type size =
		sizeof(EventMagic) + BinaryLoggerImpl<Fields...>::size;

	template <typename... Ts>
	static void write(void *dst, Ts... args){
		const auto magic = MAGIC;
		memcpy(dst, &magic, sizeof(EventMagic));
		BinaryLoggerImpl<Fields...>::write(
			reinterpret_cast<uint8_t *>(dst) + sizeof(EventMagic), args...);
	}

	static void to_json(std::ostream &os, const void *src){
		EventMagic magic;
		memcpy(&magic, src, sizeof(EventMagic));
		assert(magic == MAGIC);
		os << "{\"type\":\"" << TYPE_NAME << "\"";
		BinaryLoggerImpl<Fields...>::to_json(
			os, reinterpret_cast<const uint8_t *>(src) + sizeof(EventMagic));
		os << "}";
	}
};

using CreatePhysicalTaskLogger = BinaryLogger<
	EventMagic::CREATE_PHYSICAL_TASK, str_create_physical_task,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_physical_id>,
	BinaryLogField<identifier_type, str_logical_id>>;

using PhysicalDependencyLogger = BinaryLogger<
	EventMagic::PHYSICAL_DEPENDENCY, str_physical_dependency,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_producer>,
	BinaryLogField<identifier_type, str_consumer>>;

using BeginPreparationLogger = BinaryLogger<
	EventMagic::BEGIN_PREPARATION, str_begin_preparation,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_task_id>>;

using EndPreparationLogger = BinaryLogger<
	EventMagic::END_PREPARATION, str_end_preparation,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_task_id>>;


using BeginExecutionLogger = BinaryLogger<
	EventMagic::BEGIN_EXECUTION, str_begin_execution,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_task_id>>;

using EndExecutionLogger = BinaryLogger<
	EventMagic::END_EXECUTION, str_end_execution,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_task_id>>;


using AllocateMemoryLogger = BinaryLogger<
	EventMagic::ALLOCATE_MEMORY, str_allocate_memory,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_object_id>,
	BinaryLogField<size_type,       str_size>,
	BinaryLogField<identifier_type, str_numa_node>>;

using ReleaseMemoryLogger = BinaryLogger<
	EventMagic::RELEASE_MEMORY, str_release_memory,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_object_id>>;

using LockMemoryLogger = BinaryLogger<
	EventMagic::LOCK_MEMORY, str_lock_memory,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_object_id>>;

using UnlockMemoryLogger = BinaryLogger<
	EventMagic::UNLOCK_MEMORY, str_unlock_memory,
	BinaryLogField<uint64_t,        str_timestamp>,
	BinaryLogField<identifier_type, str_object_id>>;

}


class ProfileEventLogger::LogBlock {

public:
	static const size_type LOG_BLOCK_SIZE = (1 << 20);

private:
	std::unique_ptr<LogBlock> m_prev_block;
	size_type m_written_bytes;
	uint8_t m_log_buffer[LOG_BLOCK_SIZE];

public:
	LogBlock()
		: m_prev_block(nullptr)
		, m_written_bytes(0)
	{ }

	explicit LogBlock(std::unique_ptr<LogBlock> &&prev_block)
		: m_prev_block(std::move(prev_block))
		, m_written_bytes(0)
	{ }

	template <typename Logger, typename... Args>
	bool write(Args... args){
		const auto remains = LOG_BLOCK_SIZE - m_written_bytes;
		if(remains < Logger::size){ return false; }
		Logger::write(m_log_buffer + m_written_bytes, args...);
		m_written_bytes += Logger::size;
		return true;
	}

	size_type size() const {
		return m_written_bytes;
	}
	const void *data() const {
		return m_log_buffer;
	}

	const LogBlock *prev_block() const {
		return m_prev_block.get();
	}
};


ProfileEventLogger::ProfileEventLogger()
	: m_current_block(nullptr)
{ }

ProfileEventLogger::~ProfileEventLogger() = default;


void ProfileEventLogger::enable(){
	m_current_block = make_unique<LogBlock>();
}

void ProfileEventLogger::disable(){
	m_current_block.release();
}


void ProfileEventLogger::log_create_physical_task(
	PhysicalTaskIdentifier physical_id, LogicalTaskIdentifier logical_id)
{
	write_binary<CreatePhysicalTaskLogger>(
		current_timestamp(),
		physical_id.identifier(),
		logical_id.identifier());
}

void ProfileEventLogger::log_physical_dependency(
	PhysicalTaskIdentifier producer, PhysicalTaskIdentifier consumer)
{
	write_binary<PhysicalDependencyLogger>(
		current_timestamp(),
		producer.identifier(),
		consumer.identifier());
}


void ProfileEventLogger::log_begin_preparation(PhysicalTaskIdentifier task_id){
	write_binary<BeginPreparationLogger>(
		current_timestamp(),
		task_id.identifier());
}

void ProfileEventLogger::log_end_preparation(PhysicalTaskIdentifier task_id){
	write_binary<EndPreparationLogger>(
		current_timestamp(),
		task_id.identifier());
}


void ProfileEventLogger::log_begin_execution(PhysicalTaskIdentifier task_id){
	write_binary<BeginExecutionLogger>(
		current_timestamp(),
		task_id.identifier());
}

void ProfileEventLogger::log_end_execution(PhysicalTaskIdentifier task_id){
	write_binary<EndExecutionLogger>(
		current_timestamp(),
		task_id.identifier());
}


void ProfileEventLogger::log_allocate_memory(
	identifier_type mobj_id, size_type size)
{
	write_binary<AllocateMemoryLogger>(
		current_timestamp(), mobj_id, size, -1);
}

void ProfileEventLogger::log_allocate_memory(
	identifier_type mobj_id, size_type size, identifier_type numa_node)
{
	write_binary<AllocateMemoryLogger>(
		current_timestamp(), mobj_id, size, numa_node);
}

void ProfileEventLogger::log_release_memory(identifier_type mobj_id){
	write_binary<ReleaseMemoryLogger>(current_timestamp(), mobj_id);
}


void ProfileEventLogger::log_lock_memory(const MemoryReference &mobj){
	write_binary<LockMemoryLogger>(current_timestamp(), mobj.identifier());
}

void ProfileEventLogger::log_unlock_memory(const MemoryReference &mobj){
	write_binary<UnlockMemoryLogger>(current_timestamp(), mobj.identifier());
} 


std::string ProfileEventLogger::to_json() const {
	const LogBlock *cur_block = m_current_block.get();
	std::vector<std::string> json_blocks;
	while(cur_block){
		const size_type block_size = cur_block->size();
		const auto data = reinterpret_cast<const uint8_t *>(cur_block->data());
		std::ostringstream oss;
		for(size_type p = 0; p < block_size; ){
			if(p != 0){ oss << ","; }
			EventMagic magic;
			memcpy(&magic, data + p, sizeof(EventMagic));
			switch(magic){
				case EventMagic::CREATE_PHYSICAL_TASK:
					p += write_json<CreatePhysicalTaskLogger>(oss, data + p);
					break;
				case EventMagic::PHYSICAL_DEPENDENCY:
					p += write_json<PhysicalDependencyLogger>(oss, data + p);
					break;
				case EventMagic::BEGIN_PREPARATION:
					p += write_json<BeginPreparationLogger>(oss, data + p);
					break;
				case EventMagic::END_PREPARATION:
					p += write_json<EndPreparationLogger>(oss, data + p);
					break;
				case EventMagic::BEGIN_EXECUTION:
					p += write_json<BeginExecutionLogger>(oss, data + p);
					break;
				case EventMagic::END_EXECUTION:
					p += write_json<EndExecutionLogger>(oss, data + p);
					break;
				case EventMagic::ALLOCATE_MEMORY:
					p += write_json<AllocateMemoryLogger>(oss, data + p);
					break;
				case EventMagic::RELEASE_MEMORY:
					p += write_json<ReleaseMemoryLogger>(oss, data + p);
					break;
				case EventMagic::LOCK_MEMORY:
					p += write_json<LockMemoryLogger>(oss, data + p);
					break;
				case EventMagic::UNLOCK_MEMORY:
					p += write_json<UnlockMemoryLogger>(oss, data + p);
					break;
				default:
					assert(!"unsupported event");
			}
		}
		if(oss.str().size() > 0){
			json_blocks.emplace_back(oss.str());
		}
		cur_block = cur_block->prev_block();
	}
	std::ostringstream oss;
	oss << "[";
	while(!json_blocks.empty()){
		oss << std::move(json_blocks.back());
		json_blocks.pop_back();
		if(!json_blocks.empty()){ oss << ","; }
	}
	oss << "]";
	return oss.str();
}


template <typename Logger, typename... Args>
void ProfileEventLogger::write_binary(Args... args){
	if(!m_current_block){ return; }
	while(!m_current_block->write<Logger>(args...)){
		auto next_block = make_unique<LogBlock>(std::move(m_current_block));
		m_current_block = std::move(next_block);
	}
}

template <typename Logger>
size_type ProfileEventLogger::write_json(
	std::ostream &os, const void *ptr) const
{
	Logger::to_json(os, ptr);
	return Logger::size;
}

}

