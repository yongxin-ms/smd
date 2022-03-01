#pragma once
#include <smd.h>


class TestPointer {
public:
	TestPointer() {
		TestStdNewDelete();
		TestNewDelete();
		TestArrayPointer();
		TestPointerToObject();
	}

private:

	// 标准的分配与回收用法
	void TestStdNewDelete() {
		auto mem_usage = smd::g_alloc->GetUsed();

		// 开辟共享内存
		smd::shm_pointer<int> shm_ptr = smd::g_alloc->New<int>();
		assert(shm_ptr != smd::shm_nullptr);

		// 指针在本次分配的相对位置
		SMD_LOG_INFO("Raw pointer:%lld", shm_ptr.Raw());

		// 访问
		*shm_ptr = 10;
		assert(*shm_ptr == 10);

		// 回收共享内存
		smd::g_alloc->Delete(shm_ptr);
		assert(shm_ptr == smd::shm_nullptr);

		// 没有内存泄露
		assert(mem_usage == smd::g_alloc->GetUsed());
		SMD_LOG_INFO("TestStdNewDelete complete");
	}

	// 另一种分配与回收的用法
	void TestNewDelete() {
		auto mem_usage = smd::g_alloc->GetUsed();

		// 开辟共享内存
		int* ptr = smd::g_alloc->New<int>().Ptr();
		assert(ptr != nullptr);

		// 两种指针的互相转换
		smd::shm_pointer<int> shm_ptr = smd::g_alloc->ToShmPointer<int>(ptr);
		assert(shm_ptr.Ptr() == ptr);

		// 指针在共享内存中的相对位置
		SMD_LOG_INFO("Raw pointer:%lld", shm_ptr.Raw());

		// 访问
		*shm_ptr = 10;
		assert(*shm_ptr == 10);

		// 回收共享内存
		smd::g_alloc->Delete(ptr);
		assert(ptr == nullptr);

		// 没有内存泄露
		assert(mem_usage == smd::g_alloc->GetUsed());
		SMD_LOG_INFO("TestNewDelete complete");
	}

	// 指向一个数组
	void TestArrayPointer() {
		auto mem_usage = smd::g_alloc->GetUsed();
		const int ARRAY_SIZE = 16;

		// 开辟共享内存
		auto shm_ptr = smd::g_alloc->Malloc<size_t>(ARRAY_SIZE);
		assert(shm_ptr != smd::shm_nullptr);

		// 指针在共享内存中的相对位置
		SMD_LOG_INFO("Raw pointer:%lld", shm_ptr.Raw());

		// 随机访问
		for (auto i = 0; i < ARRAY_SIZE; i++) {
			shm_ptr[i] = i * 10;
		}

		// 随机访问
		for (auto i = 0; i < ARRAY_SIZE; i++) {
			SMD_LOG_INFO("Array[%d] = %d", i, shm_ptr[i]);
		}

		//和普通指针一样可以++
		auto p = shm_ptr;
		for (auto i = 0; i < ARRAY_SIZE; i++, p++) {
			SMD_LOG_INFO("Array[%d] = %d", i, *p);
		}

		// 回收共享内存
		smd::g_alloc->Free(shm_ptr, ARRAY_SIZE);
		assert(shm_ptr == smd::shm_nullptr);

		// 没有内存泄露
		assert(mem_usage == smd::g_alloc->GetUsed());
		SMD_LOG_INFO("TestPointer complete");
	}

	// 指向一个对象
	void TestPointerToObject() {
		auto mem_usage = smd::g_alloc->GetUsed();

		struct StA {
			int a1;
			int a2;
		};

		class StB {
		public:
			StB() { m_pointer_a = smd::g_alloc->New<StA>(); }
			~StB() { smd::g_alloc->Delete(m_pointer_a); }

			smd::shm_pointer<StA> m_pointer_a;
			int m_b1;
		};

		// 开辟共享内存
		smd::shm_pointer<StB> shm_ptr = smd::g_alloc->New<StB>();
		assert(shm_ptr != smd::shm_nullptr);

		// 指针在共享内存中的相对位置
		SMD_LOG_INFO("Raw pointer:%lld", shm_ptr.Raw());

		// 访问
		shm_ptr->m_b1 = 10;
		shm_ptr->m_pointer_a->a1 = 5;
		shm_ptr->m_pointer_a->a2 = 127;

		assert(shm_ptr->m_b1 == 10);
		assert(shm_ptr->m_pointer_a->a1 == 5);
		assert(shm_ptr->m_pointer_a->a2 == 127);

		// 回收共享内存
		smd::g_alloc->Delete(shm_ptr);
		assert(shm_ptr == smd::shm_nullptr);

		// 没有内存泄露
		assert(mem_usage == smd::g_alloc->GetUsed());
		SMD_LOG_INFO("TestPointer complete");
	}
};
