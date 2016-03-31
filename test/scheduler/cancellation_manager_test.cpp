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
#include <gtest/gtest.h>
#include "scheduler/cancellation_manager.hpp"

TEST(CancellationManager, NoThrow){
	m3bp::CancellationManager cancellation_manager;
	EXPECT_FALSE(cancellation_manager.is_cancelled());
	EXPECT_NO_THROW(cancellation_manager.rethrow_exception());
}

TEST(CancellationManager, MultipleException){
	m3bp::CancellationManager cancellation_manager;
	try{
		throw std::logic_error("logic_error");
	}catch(...){
		cancellation_manager.notify_exception(std::current_exception());
	}
	EXPECT_TRUE(cancellation_manager.is_cancelled());
	try{
		throw std::runtime_error("runtime_error");
	}catch(...){
		cancellation_manager.notify_exception(std::current_exception());
	}
	EXPECT_TRUE(cancellation_manager.is_cancelled());
	EXPECT_THROW(cancellation_manager.rethrow_exception(), std::logic_error);
}

