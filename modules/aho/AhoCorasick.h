#ifndef AHO_CORASICK_H_
#define AHO_CORASICK_H_
#include <cstring>
#include <stdlib.h>
#include "AhoExport.h"
#include "IkTypes.h"
#include "IkLabel.h"
#include "AhoTypes.h"
#include "lexrep/LexrepFunctions.h"
#include "ali/ALIFunctions.h"
#ifdef _WIN32
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned int uint32_t;
#else //!_WIN32
#include <stdint.h>
#endif //!_WIN32

namespace iknow {
  namespace model {
    class AHO_API Model {
    public:
      virtual State GotoChar(State s, Char c) const = 0;
      virtual State Goto(State s, const Char* begin, const Char* end) const = 0;
      virtual State GotoWord(State s, const Char* begin, const Char* end) const = 0;
      virtual State Failure(State s) const = 0;
      virtual const Match* OutputBegin(State s) const = 0;
      virtual size_t OutputCount(State s) const = 0;
      virtual size_t MaxWordCount() const = 0;
      virtual bool IsIdeographic() const = 0;
      virtual ~Model() {}
    };

    class AHO_API ALIModel {
    public:
      virtual double ScoreText(const Char*, const Char*, size_t) const { return 0; }
      virtual ~ALIModel() {}
    };

    class AHO_API DataModel : public Model {
    public:
      DataModel(const LexrepData& data) : f_(data) {}
      State GotoChar(State s, Char c) const {
	    return f_.Goto(s,c);
      }
      State Goto(State s, const Char* begin, const Char* end) const {
	    return f_.Goto(s, begin, end);
      }
      State GotoWord(State s, const Char* begin, const Char* end) const {
	    return f_.Goto(s, begin, end);
      }
      State Failure(State s) const {
	    return f_.Failure(s);
      }
      const Match* OutputBegin(State s) const {
	    return f_.OutputBegin(s);
      }
      size_t OutputCount(State s) const {
	    return f_.OutputEnd(s) - f_.OutputBegin(s);
      }
      size_t MaxWordCount() const {
	    return f_.MaxWordCount();
      }
      bool IsIdeographic() const {
	    return f_.IsIdeographic();
      }
    private:
      LexrepFunctions f_;
    };

    class AHO_API ALIDataModel : public ALIModel {
    public:
      ALIDataModel(const ALIData& data) : f_(data) {}
      double ScoreText(const Char* begin, const Char* end, size_t language_total_score) const {
	return f_.ScoreText(begin, end, language_total_score);
      }
    private:
      ALIFunctions f_;
    };
    
    AHO_API const Model* GetModel(const std::string& name, size_t index = 0);
    AHO_API const Model* SetModel(const std::string& name, size_t index, const Model* model);
    AHO_API void ClearModel(const std::string& name);

    AHO_API const ALIModel* GetALIModel(const std::string& name, size_t index = 0);
    AHO_API const ALIModel* SetALIModel(const std::string& name, size_t index, const ALIModel* model);
    AHO_API void ClearALIModel(const std::string& name);

    typedef const unsigned char* RawDataPointer;
    AHO_API RawDataPointer GetRawDataPointer(const std::string& name, size_t index = 0);
    AHO_API void SetRawDataPointer(const std::string& name, RawDataPointer* raw_data_adress);
    AHO_API void CleaRAWrModel(const std::string& name);
  }
}
#endif
