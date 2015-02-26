/* ===================================================================
Copyright (c) 2013, Palo Alto Research Center, Inc.
=================================================================== */


#ifndef _logcf_h
#define _logcf_h

#include <typeinfo>
#include <iostream>
#include <string>

#include "json/json.h"

#define LOG(level) LogCF::Instance()->log(typeid(*this).name(), level)
#define LOG2(level, realm) LogCF::Instance()->log(realm, level)
#define LOGF(level) LogCF::Instance()->log(__FILE__, level)


/** this is a singleton class for logging */
class LogCF {


public:
  static const int FATAL    = 0;
  static const int ERR	    = 1;
  static const int CRITICAL = 2;
  static const int WARNING  = 3;
  static const int INFO     = 4;
  static const int DEBUG1   = 5;
  static const int DEBUG2   = 6;
  static const int TRACE1   = 7;
  static const int TRACE2   = 8;

  static LogCF *Instance();
  static LogCF *Instance(const char *filename);

  std::ostream &log(const char *name, int level = 0);

private:
  static LogCF *m_pInstance;
  Json::Value config;

  LogCF() {};
  LogCF(const char *filename);
  LogCF(LogCF const &) {};
  LogCF &operator=(LogCF const &);


};

#endif
