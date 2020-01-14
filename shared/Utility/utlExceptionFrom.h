#ifndef _SHARED_UTILITY_EXCEPTIONFROM_H
#define _SHARED_UTILITY_EXCEPTIONFROM_H
#include <string>
#include <exception>
#include <vector>

//a superclass for all exception from's
class Exception : public std::exception {
public:
  explicit Exception(const std::string& reason) : reason_(reason) {
  }
  std::string get_reason() const { return reason_; }
  const char * what() const throw() { return reason_.c_str(); }
  virtual ~Exception() throw (){}
private:
  void operator=(const Exception&);
  const std::string reason_;
};

template<typename T>
class ExceptionFrom : public Exception {
public:
  explicit ExceptionFrom(const std::string& reason) : Exception(reason) {}
};

//This exception type is designed to hold a Cache status string to be passed
//directly out of the callout code and interpreted only by Cache.
class StatusException : public Exception {
public:
  explicit StatusException(const std::string& status) : Exception("StatusException"), status_(status) {}
  std::string get_status() const { return status_; }
  virtual ~StatusException() throw (){}
private:
  const std::string status_;
};

template<typename T>
class StatusExceptionFrom : public StatusException {
public:
  explicit StatusExceptionFrom(const std::string& status) : StatusException(status) {}
};

//This exception type is designed to create a Cache message-template exception, using the message
//template database from databases/sys/localize/Errors.xml
class MessageException : public Exception {
public:
  explicit MessageException(const std::string& message_name,
			   const std::string& p1 = "",
			   const std::string& p2 = "",
			   const std::string& p3 = "",
			   const std::string& p4 = "") : Exception(message_name) {
    
    if (p1 != "") { parameters_.push_back(p1); } else return;
    if (p2 != "") { parameters_.push_back(p2); } else return;
    if (p3 != "") { parameters_.push_back(p3); } else return;
    if (p4 != "") { parameters_.push_back(p4); } else return;
  }
  
  std::string get_message_name() const { return get_reason(); }
  std::string get_parameter(size_t i) const { return  i < get_parameter_count() ? parameters_.at(i) : ""; }
  size_t get_parameter_count() const { return parameters_.size(); }
  virtual ~MessageException() throw (){}

private:
  typedef std::vector<std::string> ParameterVector;
  ParameterVector parameters_;
};

template<typename T>
class MessageExceptionFrom : public MessageException {
public:
  explicit MessageExceptionFrom(const std::string& message_name, 
			       const std::string& p1 = "",
			       const std::string& p2 = "",
			       const std::string& p3 = "",
			       const std::string& p4 = "") :
    MessageException(message_name, p1, p2, p3, p4) {}
};


#endif //_SHARED_UTILITY_EXCEPTIONFROM_H
