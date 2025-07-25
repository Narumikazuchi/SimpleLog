# SimpleLog
A small header only library to implement configurable, quick and easily usable logging.

## Configuration
### Global
```cpp
inline SimpleLog::LoggerConfiguration& SimpleLog::CurrentConfiguration();
```  
You can configure the logger by configuring the values of the returned reference of this function.

### Setter
```cpp
inline void SimpleLog::ConfigureLogger(
    const SimpleLog::LoggerConfiguration& configuration
);
```  
Alternatively you can create a configuration and apply it by calling the above function with your configuration object.

### Options
```cpp
struct LoggerConfiguration final
{
public:
    std::filesystem::path LogDirectory	= std::filesystem::path();
    SimpleLog::LogLevel Severity		= SimpleLog::LogLevels::Warning;
    std::string FileNamePrefix			= std::string();
    std::string FileNamePostfix			= std::string();
    bool WriteThreadId					= false;
    bool WriteToConsole					= false;
    bool WriteToFile					= true;
};
```  
#### LogDirectory
Defines where the logfiles will be written when ```WriteToFile = true```.

#### Severity
Defines the severity that will be logged. Severity is comparable, which means all messages with a severity whose value is smaller than the allowed severity will also be logged.
```cpp
namespace SimpleLog::LogLevels
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
```  

#### FileNamePrefix
A prefix that will be prepended to every log file.  

#### FileNamePostfix
A prefix that will be appended to every log file.  

#### WriteThreadId
If true the log message will include the id of the thread that logged the message.  

#### WriteToConsole
If true the log message will be written to the standard output.  

#### WriteToFile
If true the log message will be written to daily rolling file.  

## Usage
The use is very simple you can just use the macro respective of the severity you want to log.
```cpp
#define LogTrace(template, ...)
#define LogDebug(template, ...)
#define LogInformation(template, ...)
#define LogWarning(template, ...)
#define LogError(template, ...)
#define LogCritical(template, ...)
```  
The ```template``` argument is required to be a string literal. You can also add a placeholder '{}' to the template. Doing so will require you to pass an argument for each placeholder to the macro. The values you pass must fulfill at least of of the below conditions:  
- value is of type ```char*``` or ```const char*```
- value is of type ```char* const``` or ```const char* const```
- value is of type ```std::string``` or ```const std::string```
- value is of type ```std::string_view``` or ```const std::string_view```
- value is convertible to type ```char*``` or ```const char*```
- value is convertible to type ```char* const``` or ```const char* const```
- value is convertible to type ```std::string``` or ```const std::string```
- value is convertible to type ```std::string_view``` or ```const std::string_view```
- value is castable to type ```char*``` or ```const char*```
- value is castable to type ```char* const``` or ```const char* const```
- value is castable to type ```std::string``` or ```const std::string```
- value is castable to type ```std::string_view``` or ```const std::string_view```
- value can be appended to a ```std::string``` (```std::string + value```)
- ```std::to_string(value)``` is implemented
- value implements a custom ```ToString()``` method that returns any of ```char*``` or ```const char*``` or ```char* const``` or ```const char* const``` or ```std::string``` or ```const std::string``` or ```std::string_view``` or ```const std::string_view```