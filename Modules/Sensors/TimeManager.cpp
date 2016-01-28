#include "TimeManager.h"

namespace TimeManager{


long int GetLocalTimestamp(){
    std::chrono::milliseconds local_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    return local_timestamp.count();
}

}
