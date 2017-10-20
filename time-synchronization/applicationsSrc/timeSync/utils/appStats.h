#ifndef APP_STATS_H


// Frequency counter (bounded value, occurence counter)
// nb hops for query: at receiver node
// nb hops for store request: at receiver node
// cache hit/miss:

// Occurence

// avg time for query

#include <string>
#include <vector>

namespace AppStats  {
  
  class Collector {
  protected:
    static bool msgPrinted;
    
  public:
    std::string name;
    
    Collector(std::string n);
    Collector(const Collector &c);
    virtual ~Collector();

    virtual void print() = 0;
    
    void printTitle();
    void checkAndPrintTitle();
    
  };

  // Generic collector (min, max, mean, sd)
  class GenericCollector: public Collector {
  public:
    long double min;
    long double max;
    long double mean;
    long double sd;
    
    GenericCollector(std::string n);
    GenericCollector(const GenericCollector &bc);
    ~GenericCollector();

    void print();
  };
  
  // Store nothing, just update min/max/mean
  class BasicCollector: public GenericCollector {
  public:
    long double numValues;
    
    BasicCollector(std::string n);
    BasicCollector(const BasicCollector &bc);
    ~BasicCollector();

    void add(long double v);
  };

  // Store occurences
  class FrequencyCollector: public GenericCollector {
  protected:
    std::vector<unsigned int> occurences;

  public:
    FrequencyCollector(std::string n);
    FrequencyCollector(const FrequencyCollector &fc);
    ~FrequencyCollector();
    
    void setValues();
    
    void add(unsigned int index);   
  };

  template <class T>
  class AllValuesCollector: public GenericCollector {
  public:
    std::vector<T> values;
    
    AllValuesCollector(std::string n);
    AllValuesCollector(const AllValuesCollector<T> &avc);
    ~AllValuesCollector();

    void setValues();
    
    void add(T v);
  };

  // Counters  
  class Counter: public Collector {
  public:
    unsigned int max;
    unsigned int min;
    
    Counter(std::string n);
    Counter(const Counter &cc);
    ~Counter();

    void print();
  };

  class BasicCounter: public Counter {
  public:

    unsigned int counter;
    
    BasicCounter(std::string n);
    BasicCounter(const BasicCounter &bc);
    ~BasicCounter();

    void incr();
    void decr();
  };

  /*class TimedCounter {

    };*/

  //

  class BooleanOccurences: public Collector {
  public:
    unsigned int _true;
    unsigned int _false;

    BooleanOccurences(std::string n);
    BooleanOccurences(const BooleanOccurences &bo);
    ~BooleanOccurences();

    void print();

    void add(bool v);
  };
  
}

#endif
