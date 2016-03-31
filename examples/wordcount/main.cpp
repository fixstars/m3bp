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
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <thread>
#include <cstring>
#include "m3bp/m3bp.hpp"

class FileInputProcessor : public m3bp::ProcessorBase {
private:
	std::string m_filename;
	std::shared_ptr<std::ifstream> m_stream; // std::ifstream is noncopyable
public:
	FileInputProcessor(const std::string &filename)
		: m3bp::ProcessorBase(
			{ }, // no input ports
			{ // output ports
				m3bp::OutputPort("line")
					.has_key(false)
			})
		, m_filename(filename)
		, m_stream()
	{
		max_concurrency(1);
	}
	virtual void global_initialize(m3bp::Task &task) override {
		m_stream.reset(new std::ifstream());
		m_stream->open(m_filename.c_str());
		task_count(1);
	}
	virtual void global_finalize(m3bp::Task &task) override {
		m_stream->close();
		m_stream.reset();
	}
	virtual void run(m3bp::Task &task) override {
		auto writer = task.output(0);
		auto buffer = writer.allocate_buffer();
		buffer.offset_table()[0] = 0;
		std::size_t written_bytes = 0, written_count = 0;
		std::string line;
		while(std::getline(*m_stream, line)){
			const std::size_t record_size = line.size() + 1;
			if(written_bytes + record_size > buffer.data_buffer_size() ||
			   written_count == buffer.max_record_count())
			{
				writer.flush_buffer(std::move(buffer), written_count);
				buffer = writer.allocate_buffer(record_size, 1);
				buffer.offset_table()[0] = 0;
				written_bytes = written_count = 0;
			}
			buffer.offset_table()[++written_count] = written_bytes + record_size;
			char *ptr = static_cast<char *>(buffer.data_buffer()) + written_bytes;
			std::copy(line.begin(), line.end(), ptr);
			ptr[line.size()] = '\0';
			written_bytes += record_size;
		}
		if(written_count > 0){
			writer.flush_buffer(std::move(buffer), written_count);
		}
	}
};

class WordCountMapper : public m3bp::ProcessorBase {
public:
	WordCountMapper()
		: m3bp::ProcessorBase(
			{ // input ports
				m3bp::InputPort("line")
					.movement(m3bp::Movement::ONE_TO_ONE)
			},
			{ // output ports
				m3bp::OutputPort("word_pair")
					.has_key(true)
			})
	{ }
	virtual void run(m3bp::Task &task) override {
		auto reader = task.input(0);
		const auto input_buffer  = reader.raw_buffer();
		const auto in_record_count = input_buffer.record_count();
		const uint8_t *in_data_buffer = static_cast<const uint8_t *>(input_buffer.key_buffer());
		const auto in_offset_table = input_buffer.key_offset_table();

		auto writer = task.output(0);
		auto output_buffer = writer.allocate_buffer();
		output_buffer.offset_table()[0] = 0;
		std::size_t written_bytes = 0, written_count = 0;
		for(std::size_t i = 0; i < in_record_count; ++i){
			const char *line = reinterpret_cast<const char *>(in_data_buffer + in_offset_table[i]);
			std::size_t head = 0;
			while(line[head] != '\0'){
				std::size_t tail = head;
				while(isalpha(line[tail])){ ++tail; }
				if(head == tail){
					++head;
					continue;
				}
				const std::size_t len = tail - head;
				const std::size_t record_size = len + 1 + sizeof(int);
				if(written_bytes + record_size > output_buffer.data_buffer_size() ||
				   written_count == output_buffer.max_record_count())
				{
					writer.flush_buffer(std::move(output_buffer), written_count);
					output_buffer = writer.allocate_buffer(record_size, 1);
					output_buffer.offset_table()[0] = 0;
					written_bytes = written_count = 0;
				}
				char *dst = static_cast<char *>(output_buffer.data_buffer()) + written_bytes;
				for(std::size_t j = 0; j < len; ++j){
					dst[j] = tolower(line[head + j]);
				}
				dst[len] = '\0';
				const int one = 1;
				memcpy(dst + len + 1, &one, sizeof(int));

				written_bytes += record_size;
				++written_count;
				output_buffer.key_length_table()[written_count - 1] = len + 1;
				output_buffer.offset_table()[written_count] = written_bytes;
				head = tail;
			}
		}
		if(written_count > 0){
			writer.flush_buffer(std::move(output_buffer), written_count);
		}
	}
};

class WordCountReducer : public m3bp::ProcessorBase {
public:
	WordCountReducer()
		: m3bp::ProcessorBase(
			{ // input ports
				m3bp::InputPort("word_pair")
					.movement(m3bp::Movement::SCATTER_GATHER)
			},
			{ // output ports
				m3bp::OutputPort("result")
					.has_key(true)
			})
	{ }
	virtual void run(m3bp::Task &task) override {
		auto reader = task.input(0);
		const auto input_buffer = reader.raw_buffer();
		const auto group_count = input_buffer.record_count();
		const uint8_t *key_buffer = static_cast<const uint8_t *>(input_buffer.key_buffer());
		const auto key_offset_table = input_buffer.key_offset_table();
		const uint8_t *value_buffer = static_cast<const uint8_t *>(input_buffer.value_buffer());
		const auto value_offset_table = input_buffer.value_offset_table();

		auto writer = task.output(0);
		auto output_buffer = writer.allocate_buffer();
		output_buffer.offset_table()[0] = 0;
		std::size_t written_bytes = 0, written_count = 0;
		for(std::size_t i = 0; i < group_count; ++i){
			const auto word = (const char *)(key_buffer + key_offset_table[i]);
			const std::size_t value_count =
				(value_offset_table[i + 1] - value_offset_table[i]) / sizeof(int);
			const int *values =
				reinterpret_cast<const int *>(value_buffer + value_offset_table[i]);
			int value_sum = 0;
			for(std::size_t j = 0; j < value_count; ++j){
				value_sum += values[j];
			}
			const std::size_t word_len = strlen(word);
			const std::size_t record_size = word_len + 1 + sizeof(int);
			if(written_bytes + record_size > output_buffer.data_buffer_size() ||
			   written_count == output_buffer.max_record_count())
			{
				writer.flush_buffer(std::move(output_buffer), written_count);
				output_buffer = writer.allocate_buffer(record_size, 1);
				output_buffer.offset_table()[0] = 0;
				written_bytes = written_count = 0;
			}
			char *dst = static_cast<char *>(output_buffer.data_buffer()) + written_bytes;
			for(std::size_t j = 0; j < word_len; ++j){
				dst[j] = word[j];
			}
			dst[word_len] = '\0';
			memcpy(dst + word_len + 1, &value_sum, sizeof(int));

			written_bytes += record_size;
			++written_count;
			output_buffer.key_length_table()[written_count - 1] = word_len + 1;
			output_buffer.offset_table()[written_count] = written_bytes;
		}
		if(written_count > 0){
			writer.flush_buffer(std::move(output_buffer), written_count);
		}
	}
};

class FileOutputProcessor : public m3bp::ProcessorBase {
private:
	std::string m_filename;
	std::shared_ptr<std::ofstream> m_stream; // std::ofstream is noncopyable
public:
	FileOutputProcessor(const std::string &filename)
		: m3bp::ProcessorBase(
			{ // input ports
				m3bp::InputPort("result")
					.movement(m3bp::Movement::ONE_TO_ONE)
			},
			{ }) // no output ports
		, m_filename(filename)
		, m_stream()
	{
		max_concurrency(1);
	}
	virtual void global_initialize(m3bp::Task &task) override {
		m_stream.reset(new std::ofstream(m_filename.c_str()));
	}
	virtual void global_finalize(m3bp::Task &task) override {
		m_stream->close();
		m_stream.reset();
	}
	virtual void run(m3bp::Task &task) override {
		auto reader = task.input(0);
		const auto input_buffer  = reader.raw_buffer();
		const auto in_record_count = input_buffer.record_count();
		const uint8_t *in_data_buffer = static_cast<const uint8_t *>(input_buffer.key_buffer());
		const auto in_offset_table = input_buffer.key_offset_table();
		for(std::size_t i = 0; i < in_record_count; ++i){
			const char *word =
				reinterpret_cast<const char *>(in_data_buffer + in_offset_table[i]);
			const auto len = strlen(word);
			const int value = *reinterpret_cast<const int *>(word + len + 1);
			(*m_stream) << word << " " << value << std::endl;
		}
	}
};

int main(int argc, const char *argv[]){
	if(argc < 3){
		std::cerr << "Usage: " << argv[0] << " input output" << std::endl;
		return 0;
	}
	m3bp::Logger::add_destination_stream(std::clog, m3bp::LogLevel::DEBUG);
	m3bp::Logger::add_destination_text_file("wordcount-trace.log", m3bp::LogLevel::TRACE);
	m3bp::FlowGraph graph;
	const auto input   = graph.add_vertex("input",   FileInputProcessor(argv[1]));
	const auto mapper  = graph.add_vertex("mapper",  WordCountMapper());
	const auto reducer = graph.add_vertex("reducer", WordCountReducer());
	const auto output  = graph.add_vertex("output",  FileOutputProcessor(argv[2]));
	graph.add_edge(input.output_port(0),   mapper.input_port(0));
	graph.add_edge(mapper.output_port(0),  reducer.input_port(0));
	graph.add_edge(reducer.output_port(0), output.input_port(0));
	m3bp::Configuration config;
	m3bp::Context ctx;
	ctx.set_flow_graph(graph);
	ctx.set_configuration(config);
	ctx.execute();
	ctx.wait();
	return 0;
}

