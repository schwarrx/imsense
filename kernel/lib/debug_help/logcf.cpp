/* ===================================================================
Copyright (c) 2013, Palo Alto Research Center, Inc.
=================================================================== */

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "logcf.h"

#ifdef NAMED_LOG_LEVELS
static const char *level_names[] = {(char *)"FATAL"
                                    , (char *)"ERROR"
                                    , (char *)"CRITICAL"
                                    , (char *)"WARNING"
                                    , (char *)"INFO"
                                    , (char *)"DEBUG1"
                                    , (char *)"DEBUG2"
                                    , (char *)"TRACE1"
                                    , (char *)"TRACE2"
                                   };
#endif

LogCF *LogCF::m_pInstance = NULL;

LogCF::LogCF(const char *filename) {

  std::ifstream specfile;

  try {
    specfile.open(filename);
    specfile >> config;
    specfile.close();
  } catch (std::exception &e) {
    std::cout << "Did not load configuration for logger." << std::endl;
    std::cout << "  error: " << e.what() << std::endl;
  }

  // std::cout << config << std::endl;
}

LogCF *LogCF::Instance() {
  if (!m_pInstance) {
    m_pInstance = new LogCF;
  }

  return m_pInstance;
}

LogCF *LogCF::Instance(const char *filename) {
  if (!m_pInstance) {
    m_pInstance = new LogCF(filename);
  }

  return m_pInstance;
}

std::ostream &LogCF::log(const char *name, int level) {
  if (config.empty() ||
      config.get("levels", Json::nullValue) == Json::nullValue ||
      config.get("levels", Json::nullValue).empty() ||
      config.get("levels", Json::nullValue).get(name,
          Json::nullValue) == Json::nullValue ||
      config.get("levels", Json::nullValue).get(name,
          Json::nullValue).asInt() >= level) {
#ifdef NAMED_LOG_LEVELS

    if (level < 0 || level > 8) { // Invalid level, print numeric instead of failing
      std::cout << "[" << name << ", " << level << "] ";
    } else { // Valid level, use appropriate level name
      std::cout << "[" << name << ", " << level_names[level] << "] ";
    }

#else
    std::cout << "[" << name << ", " << level << "] ";
#endif
    return std::cout;
  } else {
    static std::ostream out(0);
    return out;
  }
}
