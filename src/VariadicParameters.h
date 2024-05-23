#ifndef _VARIADIC_PARAMETERS_
#define _VARIADIC_PARAMETERS_

#include <stdint.h>
#include "EmbeddedStorage.h"

/// <summary>
/// Recursive variadic template parameters helper.
/// </summary>
class SizeParameter
{
public:
	template<size_t... Parameters>
	static constexpr size_t Count()
	{
		return sizeof...(Parameters);
	}

	template<typename... Parameters>
	static constexpr size_t Sum(const size_t first, Parameters... parameters) {
		return first + Sum(parameters...);
	}

	template<typename... Parameters>
	static constexpr size_t SumUpTo(const size_t target, const size_t first, Parameters... parameters) {
		return (first * (target > 0)) + SumUpTo<1>(target, parameters...);
	}

	template<typename... Parameters>
	static constexpr size_t Size(const size_t target, Parameters... parameters) {
		return Size<0>(target, parameters...);
	}

private:
	static constexpr size_t Sum(const size_t first) {
		return first;
	}

private:
	template<const size_t depth>
	static constexpr size_t SumUpTo(const size_t target, const size_t first) {
		return first * (target > depth);
	}

	template<const size_t depth,
		typename... Parameters>
	static constexpr size_t SumUpTo(const size_t target, const size_t first, Parameters... parameters) {
		return (first * (target > depth)) + SumUpTo<depth + 1 >(target, parameters...);
	}

private:
	template<const size_t depth>
	static constexpr size_t Size(const size_t target, const size_t first) {
		return first * (target == depth);
	}
	template<const size_t depth,
		typename... Parameters>
	static constexpr size_t Size(const size_t target, const size_t first, Parameters... parameters) {
		return (first * (target == depth)) + Size<depth + 1>(target, parameters...);
	}
};

class StorageParameter
{
public:
	template<typename... Parameters>
	static constexpr size_t Count()
	{
		return sizeof...(Parameters);
	}

	template<typename... Parameters>
	static constexpr size_t Sum() {
		return Sum<0, Parameters...>();
	}

	template<typename... Parameters>
	static constexpr size_t Size(const size_t target) {
		return Size<0, Parameters...>(target);
	}

	template<typename... Parameters>
	static constexpr size_t SizeByKey(const uint32_t key) {
		return SizeByKey<0, Parameters...>(key);
	}

	template<typename... Parameters>
	static constexpr size_t SumUpTo(const size_t target) {
		return SumUpTo<0, Parameters...>(target);
	}

	template<typename... Parameters>
	static constexpr size_t SumUpToKey(const uint32_t key) {
		return SumUpToKey<0, Parameters...>(key, false);
	}

private:
	template<const size_t depth>
	static constexpr size_t Sum() {
		return 0;
	}

	template<const size_t depth, typename First, typename... Parameters>
	static constexpr size_t Sum() {
		return EmbeddedStorage::GetStorageSize(First::Size, First::WearLevelOption) + Sum<depth + 1, Parameters...>();
	}

private:
	template<const size_t depth>
	static constexpr size_t SumUpTo(const size_t target) {
		return 0;
	}

	template<const size_t depth,
		typename First,
		typename... Parameters>
	static constexpr size_t SumUpTo(const size_t target) {
		return (EmbeddedStorage::GetStorageSize(First::Size, First::WearLevelOption) * (target > depth)) + SumUpTo<depth + 1, Parameters...>(target);
	}

	template<const size_t depth>
	static constexpr size_t SumUpToKey(const uint32_t target, const bool found) {
		return 0;
	}

	template<const size_t depth,
		typename First,
		typename... Parameters>
	static constexpr size_t SumUpToKey(const uint32_t key, const bool found) {
		return (EmbeddedStorage::GetStorageSize(First::Size, First::WearLevelOption) * (!found && (key != First::Key))) + SumUpToKey<depth + 1, Parameters...>(key, found || (key == First::Key));
	}

private:
	template<const size_t depth>
	static constexpr size_t Size(const size_t target) {
		return 0;
	}

	template<const size_t depth,
		typename First,
		typename... Parameters>
	static constexpr size_t Size(const size_t target) {
		return (EmbeddedStorage::GetStorageSize(First::Size, First::WearLevelOption) * (target == depth)) + Size<depth + 1, Parameters...>(target);
	}

	template<const size_t depth>
	static constexpr size_t SizeByKey(const uint32_t key) {
		return 0;
	}

	template<const size_t depth,
		typename First,
		typename... Parameters>
	static constexpr size_t SizeByKey(const uint32_t key) {
		return (EmbeddedStorage::GetStorageSize(First::Size, First::WearLevelOption) * (key == First::Key)) + SizeByKey<depth + 1, Parameters...>(key);
	}
};
#endif