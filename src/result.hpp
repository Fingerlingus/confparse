#pragma once

#include <type_traits>
#include <utility>

#ifndef NO_DISCARD
#	define NO_DISCARD [[nodiscard]]
#endif

template<typename E = int> class error {
public:
	using error_type = E;
	static_assert(!std::is_same<error_type, void>::value,
		"error_type cannot be void");

	error() = delete;
	constexpr error(const error_type& e) : m_val(e) {}
	constexpr error(error_type&& e) : m_val(std::move(e)) {}

	NO_DISCARD constexpr const error_type& value() const& { return m_val; }
	NO_DISCARD constexpr error_type&& value()&& { return std::move(m_val); }
	NO_DISCARD constexpr const error_type&& value() const&& {
		return std::move(m_val);
	}

private:
	error_type m_val;
};

template<typename T, typename E = int> class result {
public:
	using self_type = result<T, E>;
	using value_type = T;
	using error_type = E;

	static_assert(!std::is_reference<value_type>::value,
		"value_type cannot be a reference");
	static_assert(!std::is_reference<error_type>::value,
		"error_type cannot be a reference");
	static_assert(!std::is_same<error<error_type>,
		typename std::remove_cv<value_type>::type>::value,
		"value_type cannot be error<error_type>");

	result() = delete;
	result(const self_type&) = default;
	result(self_type&&) = default;
	~result() { }

	self_type& operator=(const self_type&) = default;
	self_type& operator=(self_type&&) = default;

	constexpr result(const value_type& t)
		: m_result(t),
		m_is_error(false) {

	}

	constexpr result(value_type&& t)
		: m_result(std::move(t)),
		m_is_error(false) {
		t = value_type();
	}

	constexpr result(const error<error_type>& e)
		: m_error(e),
		m_is_error(true) {

	}

	constexpr result(error<error_type>&& e)
		: m_error(std::move(e.value())),
		m_is_error(true) {

	}

	NO_DISCARD constexpr self_type& operator=(const value_type& t) {
		m_result = t;
		m_is_error = false;
		return *this;
	}

	NO_DISCARD constexpr self_type& operator=(value_type&& t) {
		m_result = std::move(t);
		m_is_error = false;
		return *this;
	}

	NO_DISCARD constexpr self_type&
		operator=(const error<error_type>& e) {
		m_error = e;
		m_is_error = true;
		return *this;
	}

	NO_DISCARD constexpr self_type& operator=(error<error_type>&& e) {
		m_error = std::move(e);
		m_is_error = true;
		return *this;
	}

	NO_DISCARD operator const value_type&() const {
		return m_result;
	}

	NO_DISCARD operator const error<error_type>&() const {
		return error<error_type>(m_error);
	}

	NO_DISCARD constexpr value_type&& value() { return std::move(m_result); }
	NO_DISCARD constexpr const value_type& value() const { return m_result; }
	NO_DISCARD constexpr error_type&& err() { return std::move(m_error); }
	NO_DISCARD constexpr const error_type& err() const { return m_error; }

	NO_DISCARD constexpr bool is_err() const {
		return m_is_error;
	}

	NO_DISCARD constexpr operator bool() const { return is_err(); }
	NO_DISCARD constexpr bool operator!() const { return is_err(); }

	NO_DISCARD constexpr value_type& operator->()& { return m_result; }
	NO_DISCARD constexpr const value_type& operator->() const& { return m_result; }

private:
	union {
		value_type m_result;
		error_type m_error;
	};
	bool m_is_error;
};