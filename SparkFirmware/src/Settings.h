#ifndef Settings_h
#define Settings_h

#include "Particle.h"

class Settings
{
  public:
    void writeInt(int p_address, int p_value);
    unsigned int readInt(int p_address);
  private:
};
#endif
