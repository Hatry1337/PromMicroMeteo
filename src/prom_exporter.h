#include <Arduino.h>

struct MetricLabel {
    String name;
    String value;
};

struct MetricData {
    float   *floatData;
    int     *intData;
    String  *strData;
};

enum MeticType {
    GAUGE,
    COUNTER
};

class PrometheusMetric {
public:
  PrometheusMetric(String name, MeticType type = GAUGE) { _name = name; _type = type; }

  void setDataPtr(void *ptr) { _ptr = ptr; };
  MeticType getType() { return _type; }
  String getName() { return _name; }
  void setHelp(String help) { _help = help; }

  MetricData getData();
protected:
  String    _name;
  String    _help;
  void      *_ptr;
  MeticType _type;
};

class PrometheusStringMetric : PrometheusMetric {
public:
    void setDataPtr(String *ptr) { _ptr = ptr; };
};

class PrometheusIntMetric : PrometheusMetric {
public:
    void setDataPtr(int *ptr) { _ptr = ptr; };
};

class PrometheusLongMetric : PrometheusMetric {
public:
    void setDataPtr(long *ptr) { _ptr = ptr; };
};

class PrometheusFlaotMetric : PrometheusMetric {
public:
    void setDataPtr(float *ptr) { _ptr = ptr; };
};

class PrometheusExporter {
public:
  PrometheusExporter(PrometheusMetric *metrics[]) { _metrics = metrics; }
  PrometheusMetric** getMetrics() { return _metrics; }
  
  String renderMetrics();
private:
  PrometheusMetric **_metrics = NULL;
};