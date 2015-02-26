/* ===================================================================
Copyright (c) 2013, Palo Alto Research Center, Inc.
=================================================================== */

#ifndef _timestamp_h
#define _timestamp_h

#include <string>
#include <iostream>
#include <ctime>


/**
   Tiny, easy to use time stamping class.

   Example:
   \code
   TimeStamp ts("myfunction");
   DoingSomething();
   cout << ts << endl;
   \endcode

   This will take a timestamp beforing doing something, and then
   prints the number of seconds it took to do something.
 */
class TimeStamp {

private:
  clock_t stamp;
  std::string name;

  /** time already accumulated before last stamping, see
    pause/resume */
  double accumulated;

  bool paused;

public:

  TimeStamp(std::string _name, bool _paused = false) {
    stamp = clock();
    name = _name;
    accumulated = 0;
    paused = _paused;
  }

  /** get the name of this stamp */
  std::string getName() const {
    return name;
  }

  /**
    get the value of the stamp
    @return value of the stamp
   */
  clock_t get() const {
    return stamp;
  }

  /**
    get the difference from the stamp to the current time in seconds
    @return time in seconds since stamp was taken/created
   */
  double getDiff() const {
    if (paused) {
      return accumulated;
    } else {
      return accumulated +
             (((double)(clock() - stamp)) / ((double)CLOCKS_PER_SEC));
    }
  }

  /** pause recording time, and take note of time elapsed so far */
  void pause() {
    if (!paused) {
      accumulated = getDiff();
      paused = true;
    }
  }

  /** resume time tracking after pausing */
  void resume() {
    if (paused) {
      stamp = clock();
      paused = false;
    }
  }

  void reset() {
    accumulated = 0;
    stamp = clock();
  }

};

std::ostream &operator<<(std::ostream &os, const TimeStamp &ts);

#endif
