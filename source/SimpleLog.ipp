#pragma once

#include <array>
#include <chrono>
#include <concepts>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

/**
* @brief A small header-only logging library.
* @author Narumikazuchi
* @date 06.05.2025
*/
namespace SimpleLog
{
	/**
	* @brief Represents known string-like types.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <typename TString>
	concept _StringLike =
		std::is_base_of_v<std::string, std::remove_cvref_t<TString>>
		|| std::is_base_of_v<std::string_view, std::remove_cvref_t<TString>>
		|| std::is_same_v<char*, std::remove_cvref_t<TString>>
		|| std::is_same_v<char* const, std::remove_cvref_t<TString>>
		|| std::is_same_v<const char*, std::remove_cvref_t<TString>>
		|| std::is_same_v<const char* const, std::remove_cvref_t<TString>>;

	/**
	* @brief Represents string-convertible types.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <typename TString>
	concept _StringConvertible =
		std::is_convertible_v<std::remove_cvref_t<TString>, std::string>
		|| std::is_convertible_v<std::remove_cvref_t<TString>, std::string_view>
		|| std::is_convertible_v<std::remove_cvref_t<TString>, char*>
		|| std::is_convertible_v<std::remove_cvref_t<TString>, char* const>
		|| std::is_convertible_v<std::remove_cvref_t<TString>, const char*>
		|| std::is_convertible_v<std::remove_cvref_t<TString>, const char* const>;

	/**
	* @brief Helper to check if a type is castable to another.
	* @author Narumikazuchi
	* @date 25.07.2025
	*/
	template <typename TFrom, typename TTo>
	concept _Castable = requires (
		TFrom from
	) {
		{ static_cast<TTo>(from) } -> std::same_as<TTo>;
	};

	/**
	* @brief Represents string-castable types.
	* @author Narumikazuchi
	* @date 25.07.2025
	*/
	template <typename TValue>
	concept _StringCastable = 
		_Castable<TValue, std::string>
		|| _Castable<TValue, const std::string>
		|| _Castable<TValue, std::string_view>
		|| _Castable<TValue, const std::string_view>
		|| _Castable<TValue, char*>
		|| _Castable<TValue, const char*>
		|| _Castable<TValue, char* const>
		|| _Castable<TValue, const char* const>;

	/**
	* @brief Represents string-appendable types.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <typename TValue>
	concept _StringAppendable = requires(
		std::string string,
		TValue value
	) {
		{ string += value };
	};

	/**
	* @brief Represents stringifyable types.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <typename TValue>
	concept _StdStringify = requires(
		TValue value
	) {
		{ std::to_string(value) } -> std::same_as<std::string>;
	};

	/**
	* @brief Represents stringifyable types.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <typename TValue>
	concept _Stringify =
		requires(TValue value) { { value.ToString() } -> std::same_as<std::string>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<const std::string&>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<std::string_view>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<const std::string_view&>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<char*>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<char* const>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<const char*>; }
		|| requires(TValue value) { { value.ToString() } -> std::same_as<const char* const>; };

	/**
	* @brief Represents loggable types.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <typename TValue>
	concept _Loggable =
		_StringLike<TValue>
		|| _StringConvertible<TValue>
		|| _StringAppendable<TValue>
		|| _StdStringify<TValue>
		|| _Stringify<TValue>;

	/**
	* @brief Represents a string literal for template parameters.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <size_t NSize>
	struct StringLiteral final
	{
	public:
		constexpr StringLiteral(
			const char (&value)[NSize]
		) : Value()
		{
			size_t index = 0ULL;
			while (index < NSize)
			{
				this->Value[index] = value[index];
				index += 1ULL;
			}
		};

		~StringLiteral() = default;

		char Value[NSize];
	};

	/**
	* @brief Counts and checks whether the number of placeholders in the SValue literal matches the number of TArguments.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <StringLiteral SValue, typename... TArguments>
	constexpr bool PlaceholderCountMatchesArgumentCount()
	{
		constexpr size_t NSize = sizeof(SValue.Value);
		size_t index = 0ULL;
		size_t count = 0ULL;

		char lastCharacter = 0;
		while (index < NSize)
		{
			if (SValue.Value[index] == '}'
				&& lastCharacter == '{')
			{
				count += 1ULL;
			}

			lastCharacter = SValue.Value[index];
			index += 1ULL;
		}

		return count == sizeof...(TArguments);
	}

	/**
	* @brief Enumeration representing different log levels.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	enum class LogSeverity : uint8_t
	{
		Disabled	= 0,	///< Logging is disabled.
		Critical	= 1,	///< Critical errors that will crash the application.
		Error		= 2,	///< Errors that keep a feature from functioning, but do not crash the entire application.
		Warning		= 3,	///< Warnings that indicate potential problems.
		Information	= 4,	///< Informational messages to provide context or status updates.
		Debug		= 5,	///< Detailed debugging information (only used during development).
		Trace		= 6,	///< Information detailing the order of events in the entire application.
	};

	/**
	* @brief A wrapper class for working with log levels as values and providing utility functions.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	struct LogLevel final
	{
	public:
		/**
		* @brief Parse a string representation of a log level into a `LogLevel` object.
		* @param value The string to parse (case-insensitive).
		* @return A `LogLevel` object representing the parsed log level, or `LogLevels::Disabled` if parsing fails.
		* @author Narumikazuchi
		* @date 20.06.2025
		*/
		static inline LogLevel Parse(
			const std::string& value
		) {
			std::string_view view = value;
			return LogLevel::Parse(
				view
			);
		};
		/**
		* @brief Parse a string representation of a log level into a `LogLevel` object.
		* @param value The string to parse (case-insensitive).
		* @return A `LogLevel` object representing the parsed log level, or `LogLevels::Disabled` if parsing fails.
		* @author Narumikazuchi
		* @date 20.06.2025
		*/
		static inline LogLevel Parse(
			std::string_view value
		) {
			std::string lowered = std::string();
			lowered.reserve(
				value.size()
			);
			for (char character : value)
			{
				lowered.push_back(
					std::tolower(
						character
					)
				);
			}

			static const std::string trace = std::string("trace");
			static const std::string debug = std::string("debug");
			static const std::string info = std::string("info");
			static const std::string information = std::string("information");
			static const std::string warn = std::string("warn");
			static const std::string warning = std::string("warning");
			static const std::string error = std::string("error");
			static const std::string critical = std::string("critical");
			if (lowered == trace)
			{
				return LogLevel(
					LogSeverity::Trace
				);
			}
			else if (lowered == debug)
			{
				return LogLevel(
					LogSeverity::Debug
				);
			}
			else if (lowered == information
					|| lowered == info)
			{
				return LogLevel(
					LogSeverity::Information
				);
			}
			else if (lowered == warning
					|| lowered == warn)
			{
				return LogLevel(
					LogSeverity::Warning
				);
			}
			else if (lowered == error)
			{
				return LogLevel(
					LogSeverity::Error
				);
			}
			else if (lowered == critical)
			{
				return LogLevel(
					LogSeverity::Critical
				);
			}
			else
			{
				return LogLevel(
					LogSeverity::Disabled
				);
			}
		};

		constexpr LogLevel() :
			m_Value(LogSeverity::Disabled)
		{ };

		constexpr LogLevel(
			LogSeverity value
		) : m_Value(value)
		{ };

		~LogLevel() = default;

		constexpr operator LogSeverity() const
		{
			return m_Value;
		}

		constexpr bool operator<(
			LogLevel other
		) const {
			return static_cast<uint8_t>(m_Value) < static_cast<uint8_t>(other.m_Value);
		};

		constexpr bool operator>(
			LogLevel other
		) const {
			return static_cast<uint8_t>(m_Value) > static_cast<uint8_t>(other.m_Value);
		};

		constexpr bool operator<=(
			LogLevel other
		) const {
			return static_cast<uint8_t>(m_Value) <= static_cast<uint8_t>(other.m_Value);
		};

		constexpr bool operator>=(
			LogLevel other
		) const {
			return static_cast<uint8_t>(m_Value) >= static_cast<uint8_t>(other.m_Value);
		};

		constexpr bool operator==(
			LogLevel other
		) const {
			return static_cast<uint8_t>(m_Value) == static_cast<uint8_t>(other.m_Value);
		};

		constexpr bool operator!=(
			LogLevel other
		) const {
			return static_cast<uint8_t>(m_Value) != static_cast<uint8_t>(other.m_Value);
		};

		/**
		* @brief Parse this instance to it's string representation.
		* @return The value of this instance represented as a string.
		* @author Narumikazuchi
		* @date 13.01.2025
		*/
		inline const std::string& ToString() const
		{
			static const std::string trace = std::string("Trace");
			static const std::string debug = std::string("Debug");
			static const std::string information = std::string("Information");
			static const std::string warning = std::string("Warning");
			static const std::string error = std::string("Error");
			static const std::string critical = std::string("Critical");
			static const std::string unknown = std::string("Unknown");
			switch (m_Value)
			{
				case LogSeverity::Trace:
				{
					return trace;
				}
				case LogSeverity::Debug:
				{
					return debug;
				}
				case LogSeverity::Information:
				{
					return information;
				}
				case LogSeverity::Warning:
				{
					return warning;
				}
				case LogSeverity::Error:
				{
					return error;
				}
				case LogSeverity::Critical:
				{
					return critical;
				}
				default:
				{
					return unknown;
				}
			}
		};

	private:
		LogSeverity m_Value;
	};

	namespace LogLevels
	{
		/**
		* @brief Logging is disabled.
		*/
		inline constexpr LogLevel Disabled = LogLevel(LogSeverity::Disabled);
		/**
		* @brief Critical errors that will crash the application.
		*/
		inline constexpr LogLevel Critical = LogLevel(LogSeverity::Critical);
		/**
		* @brief Errors that keep a feature from functioning, but do not crash the entire application.
		*/
		inline constexpr LogLevel Error = LogLevel(LogSeverity::Error);
		/**
		* @brief Warnings that indicate potential problems.
		*/
		inline constexpr LogLevel Warning = LogLevel(LogSeverity::Warning);
		/**
		* @brief Informational messages to provide context or status updates.
		*/
		inline constexpr LogLevel Information = LogLevel(LogSeverity::Information);
		/**
		* @brief Detailed debugging information (only used during development).
		*/
		inline constexpr LogLevel Debug = LogLevel(LogSeverity::Debug);
		/**
		* @brief Information detailing the order of events in the entire application.
		*/
		inline constexpr LogLevel Trace = LogLevel(LogSeverity::Trace);
	}

	/**
	* @brief Provides all configuration valus that influence the logger.
	* @author Narumikazuchi
	* @date 01.07.2025
	*/
	struct LoggerConfiguration final
	{
	public:
		std::filesystem::path LogDirectory	= std::filesystem::path();
		LogLevel Severity					= LogLevels::Warning;
		std::string FileNamePrefix			= std::string();
		std::string FileNamePostfix			= std::string();
		bool WriteThreadId					= false;
		bool WriteToConsole					= false;
		bool WriteToFile					= true;
	};

	/**
	* @brief Gets the current configuration for the logger.
	* @author Narumikazuchi
	* @date 01.07.2025
	*/
	inline LoggerConfiguration& CurrentConfiguration()
	{
		static LoggerConfiguration configuration = LoggerConfiguration();

		return configuration;
	}

	/**
	* @brief Configures the logger variables (where to store files, which severity level to log).
	* @author Narumikazuchi
	* @date 01.07.2025
	*/
	inline void ConfigureLogger(
		const LoggerConfiguration& configuration
	) {
		CurrentConfiguration() = configuration;

		if (configuration.LogDirectory.empty() == true)
		{
			static std::filesystem::path userDirectory = std::filesystem::path();

			if (userDirectory.empty() == true)
			{
				std::string rawPath = std::string();
				std::filesystem::path path = std::filesystem::path();

		#ifdef _WIN32
				PWSTR winPath = nullptr;
				if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &winPath)))
				{
					std::wstring wString = std::wstring(
						winPath
					);
					CoTaskMemFree(
						winPath
					);
					rawPath = std::string(
						(const char*)wString.data(),
						wString.size()
					);
					path = wString;
				}
		#elif __linux__
				const char* homeDir = std::getenv(
					"HOME"
				);
				if (homeDir == nullptr)
				{
					return;
				}

				path = homeDir;
				path = path / "Documents";
		#endif // _WIN32 or __linux__

				userDirectory = path;
			}

			CurrentConfiguration().LogDirectory = userDirectory;
		}

		if (std::filesystem::exists(CurrentConfiguration().LogDirectory) == false)
		{
			std::filesystem::create_directories(
				CurrentConfiguration().LogDirectory
			);
		}
	};

	/**
	* @brief Helper function for logging. Standalone use not supported.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <size_t NSize, typename TArgument>
	inline void UnrollArgument(
		std::array<std::string, NSize>& strings,
		size_t index,
		TArgument&& argument
	) {
		if constexpr (_StringLike<std::remove_reference_t<decltype(argument)>> == true)
		{
			if constexpr (std::is_same_v<std::string, decltype(argument)> == true
						  || std::is_same_v<std::string&, decltype(argument)> == true
						  || std::is_same_v<const std::string&, decltype(argument)> == true)
			{
				strings[index] = argument;
			}
			else
			{
				strings[index] = std::string(
					argument
				);
			}
		}
		else if constexpr (_StringConvertible<std::remove_reference_t<decltype(argument)>> == true)
		{
			strings[index] = std::string(
				argument
			);
		}
		else if constexpr (_StringCastable<std::remove_reference_t<decltype(argument)>> == true)
		{
			strings[index] = static_cast<std::string>(argument);
		}
		else if constexpr (_StdStringify<std::remove_reference_t<decltype(argument)>> == true)
		{
			std::string temp = std::to_string(
				argument
			);
			strings[index] = temp;
		}
		else if constexpr (_Stringify<std::remove_reference_t<decltype(argument)>> == true)
		{
			auto& value = argument.ToString();
			if constexpr (std::is_same_v<std::string, decltype(value)> == true
						  || std::is_same_v<std::string&, decltype(value)> == true
						  || std::is_same_v<const std::string&, decltype(value)> == true)
			{
				strings[index] = value;
			}
			else
			{
				strings[index] = std::string(
					value
				);
			}
		}
		else if constexpr (_StringAppendable<std::remove_reference_t<decltype(argument)>> == true)
		{
			std::string temp = std::string();
			temp += argument;
			strings[index] = temp;
		}
	};

	/**
	* @brief Helper function for logging. Standalone use not supported.
	* @author Narumikazuchi
	* @date 06.05.2025
	*/
	template <size_t NSize, typename TTuple, size_t... NIndecies>
	inline void UnrollArguments(
		std::array<std::string, NSize>& strings,
		TTuple&& tuple,
		std::index_sequence<NIndecies...>
	) {
		(UnrollArgument(
			strings,
			NIndecies,
			std::get<NIndecies>(tuple)
		),
		...);
	};

	/**
	* @brief Writes a message to the log file with variable arguments.
	*
	* This function writes a log message to the logger's output stream, which is usually a text file. The message includes the severity level, module name, function name, and a formatted string with optional arguments.
	*
	* @param level The LogLevel of the message.
	* @param module The name of the module that generated the message.
	* @param line The line number where in the module the log is happening.
	* @param function The name of the function that generated the message.
	* @param arguments The variable arguments to pass to the formatting function.
	* @author Narumikazuchi
	* @date 01.07.2025
	*/
	template <StringLiteral STemplate, _Loggable... TArguments>
		requires (PlaceholderCountMatchesArgumentCount<STemplate, TArguments...>())
	inline void WriteLog(
		const LogLevel level,
		const std::string& module,
		const std::string& line,
		const std::string& function,
		TArguments&&... arguments
	) {
		// Check if log level is satisfied
		if (level > CurrentConfiguration().Severity)
		{
			return;
		}

		if (CurrentConfiguration().WriteToConsole == false
			&& CurrentConfiguration().WriteToFile == false
		) {
			return;
		}

		// Fetch current time
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		time_t time = std::chrono::system_clock::to_time_t(
			now
		);
		std::tm tm;
	#ifdef _WIN32
		if (localtime_s(&tm, &time) != 0)
	#elif __linux__
		if (localtime_r(&time, &tm) == nullptr)
	#endif
		{
			tm = std::tm();
		}

		// Start to format message
		std::string timestamp = std::string();
		std::string severity = std::string();
		std::string message = std::string();

		// Timestamp
		if (tm.tm_hour < 10)
		{
			timestamp += "0";
		}

		timestamp += std::to_string(
			tm.tm_hour
		);
		timestamp += ":";
		if (tm.tm_min < 10)
		{
			timestamp += "0";
		}

		timestamp += std::to_string(
			tm.tm_min + 1
		);
		timestamp += ":";
		if (tm.tm_sec < 10)
		{
			timestamp += "0";
		}

		timestamp += std::to_string(
			tm.tm_sec + 1
		);

		// LogLevel
		size_t characters = level.ToString().size();
		severity += level.ToString();
		while (characters < 12ULL)
		{
			severity += " ";
			characters += 1ULL;
		}

		// Thread ID
		if (CurrentConfiguration().WriteThreadId == true)
		{
			std::stringstream id = std::stringstream();
			id << std::this_thread::get_id();
			message += "Thread #";
			message += id.str();
			message += "\t\t";
		}

		// Module
		std::string moduleFile = std::string();
		size_t index = module.find_last_of(
			'/'
		);
		if (index == std::string::npos)
		{
			index = module.find_last_of(
				'\\'
			);
		}

		if (index == std::string::npos)
		{
			index = 0ULL;
		}
		else
		{
			index += 1ULL;
		}

		moduleFile = module.substr(
			index
		);
		characters = moduleFile.size() + line.size() + 1ULL;
		message += moduleFile;
		message += ":";
		message += line;
		while (characters < 64ULL)
		{
			message += " ";
			characters += 1ULL;
		}

		message += "\t\t";

		// Function
		characters = function.size();
		message += function;
		while (characters < 32ULL)
		{
			message += " ";
			characters += 1ULL;
		}

		message += "\t\t";

		// Format Message
		std::array<std::string, sizeof...(TArguments)> unrolledArguments = { };
		UnrollArguments(
			unrolledArguments,
			std::make_tuple(
				std::forward<TArguments>(arguments)...
			),
			std::make_index_sequence<sizeof...(TArguments)>()
		);

		char lastCharacter = 0;
		size_t argumentIndex = 0ULL;

		index = 0ULL;
		constexpr size_t count = sizeof(STemplate.Value) - 1ULL; // Exclude \0 character
		while (index < count)
		{
			char character = STemplate.Value[index];
			if (character == '{'
				&& lastCharacter == '{')
			{
				message += '{';
			}
			else if (character == '}'
					 && lastCharacter == '}')
			{
				message += '}';
			}
			else if (character == '}'
					 && lastCharacter == '{')
			{
				if (argumentIndex >= sizeof...(TArguments))
				{
					throw std::runtime_error(
						"Not enough arguments passed to satisfy message template."
					);
				}
			
				message += unrolledArguments[argumentIndex];
				argumentIndex += 1ULL;
			}
			else if (character != '{'
					 && character != '}')
			{
				message += character;
			}

			lastCharacter = character;
			index += 1ULL;
		}

		// Write to console
		if (CurrentConfiguration().WriteToConsole == true)
		{
			std::cout << timestamp << "\t\t[";
			switch (level)
			{
				case LogLevels::Debug:
				{
					std::cout << "\033[36";
					break;
				}
				case LogLevels::Information:
				{
					std::cout << "\033[32";
					break;
				}
				case LogLevels::Warning:
				{
					std::cout << "\033[33";
					break;
				}
				case LogLevels::Error:
				{
					std::cout << "\033[31";
					break;
				}
				case LogLevels::Critical:
				{
					std::cout << "\033[41";
					break;
				}
				default:
				{
					break;
				}
			}

			std::cout << severity << "]\033[m\t\t" << message << "\n" << std::flush;
		}

		// Check if log directory has been set
		if (CurrentConfiguration().WriteToFile == false
			|| CurrentConfiguration().LogDirectory.empty() == true
		) {
			return;
		}

		// Generate filename
		std::filesystem::path filePath = std::filesystem::path();
		if (tm.tm_year == 0)
		{
			std::string filename = CurrentConfiguration().FileNamePrefix;
			filename += "General";
			filename += CurrentConfiguration().FileNamePostfix;
			filename += ".log";
			filePath = CurrentConfiguration().LogDirectory / filename;
		}
		else
		{
			filePath = CurrentConfiguration().LogDirectory;
			std::string filename = std::string();
			filename += CurrentConfiguration().FileNamePrefix;
			filename += std::to_string(
				tm.tm_year + 1900
			);
			filename += "_";
			if (tm.tm_mon < 9)
			{
				filename += "0";
			}
			
			filename += std::to_string(
				tm.tm_mon + 1
			);
			filename += "_";
			if (tm.tm_mday < 10)
			{
				filename += "0";
			}
			
			filename += std::to_string(
				tm.tm_mday
			);
			filename += CurrentConfiguration().FileNamePostfix;
			filename += ".log";
			filePath /= filename;
		}

		// Log to file
		std::ofstream file(
			filePath,
			std::ios_base::out | std::ios_base::app
		);

		if (file.is_open() == true)
		{
			file << timestamp << "\t\t[" << severity << "]\t\t" << message << "\n" << std::flush;
			file.close();
		}
	};

	// Weird macro magic to get the line number
	#define STRINGIFY(x) #x
	#define AS_STRING(x) STRINGIFY(x)
	#define LINE_AS_STRING AS_STRING(__LINE__)

	// Logging functions for easier use (__FILE__, __LINE__ and __func__ are automatically included)
	#define LogTrace(template, ...) WriteLog<template>(SimpleLog::LogLevels::Trace, __FILE__, LINE_AS_STRING, __func__ __VA_OPT__(,) __VA_ARGS__)
	#define LogDebug(template, ...) WriteLog<template>(SimpleLog::LogLevels::Debug, __FILE__, LINE_AS_STRING, __func__ __VA_OPT__(,) __VA_ARGS__)
	#define LogInformation(template, ...) WriteLog<template>(SimpleLog::LogLevels::Information, __FILE__, LINE_AS_STRING, __func__ __VA_OPT__(,) __VA_ARGS__)
	#define LogWarning(template, ...) WriteLog<template>(SimpleLog::LogLevels::Warning, __FILE__, LINE_AS_STRING, __func__ __VA_OPT__(,) __VA_ARGS__)
	#define LogError(template, ...) WriteLog<template>(SimpleLog::LogLevels::Error, __FILE__, LINE_AS_STRING, __func__ __VA_OPT__(,) __VA_ARGS__)
	#define LogCritical(template, ...) WriteLog<template>(SimpleLog::LogLevels::Critical, __FILE__, LINE_AS_STRING, __func__ __VA_OPT__(,) __VA_ARGS__)
}